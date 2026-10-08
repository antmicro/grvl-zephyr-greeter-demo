#include "usb_hid.h"

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/input/input.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <zephyr/logging/log.h>

#if defined(CONFIG_USB_HOST_STACK) && DT_NODE_EXISTS(DT_NODELABEL(zephyr_uhc0))

#include <zephyr/usb/usbh.h>

#define DISPLAY_WIDTH  DT_PROP(DT_CHOSEN(zephyr_display), width)
#define DISPLAY_HEIGHT DT_PROP(DT_CHOSEN(zephyr_display), height)
#define KEY_EVENT_QUEUE_LEN 32

USBH_CONTROLLER_DEFINE(uhc0_ctx, DEVICE_DT_GET(DT_NODELABEL(zephyr_uhc0)));
USBH_CONTROLLER_DEFINE(uhc1_ctx, DEVICE_DT_GET(DT_NODELABEL(zephyr_uhc1)));

K_MUTEX_DEFINE(mouse_cursor_lock);

static struct {
	int32_t x;
	int32_t y;
	bool pressed;
} mouse_cursor;

struct key_event {
	uint16_t code;
	bool pressed;
};

RING_BUF_DECLARE(key_events_queue, KEY_EVENT_QUEUE_LEN * sizeof(struct key_event));

static void handle_rel_event(struct input_event *event, void *user_data)
{
	if (event->code == INPUT_REL_X) {
		mouse_cursor.x = CLAMP(mouse_cursor.x + event->value, 0, DISPLAY_WIDTH - 1);
	} else if (event->code == INPUT_REL_Y) {
		mouse_cursor.y = CLAMP(mouse_cursor.y + event->value, 0, DISPLAY_HEIGHT - 1);
	}
}

static void handle_key_event(struct input_event *event, void *user_data)
{
	if (event->code == INPUT_BTN_LEFT) {
		mouse_cursor.pressed = event->value != 0;
		return;
	}

	const struct key_event ev = {
		.code = event->code,
		.pressed = event->value
	};

	ring_buf_put(&key_events_queue, (uint8_t *)&ev, sizeof(ev));
}

static void usb_hid_event_cb(struct input_event *event, void *user_data)
{
	ARG_UNUSED(user_data);

	if (strncmp(event->dev->name, "usbh_hid_boot_", strlen("usbh_hid_boot_")) != 0) {
		return;
	}

	k_mutex_lock(&mouse_cursor_lock, K_FOREVER);
	if (event->type == INPUT_EV_REL) {
		handle_rel_event(event, user_data);
	} else if (event->type == INPUT_EV_KEY) {
		handle_key_event(event, user_data);
	}
	k_mutex_unlock(&mouse_cursor_lock);
}

INPUT_CALLBACK_DEFINE(NULL, usb_hid_event_cb, NULL);

int usb_hid_init(void)
{
	int err = usbh_init(&uhc0_ctx);
	if (unlikely(err != 0) && err != -EALREADY) {
		return err;
	}

	err = usbh_init(&uhc1_ctx);
	return err == -EALREADY ? 0 : err;
}

void usb_hid_mouse_state(int32_t *x, int32_t *y, bool *pressed)
{
	k_mutex_lock(&mouse_cursor_lock, K_FOREVER);
	*x = mouse_cursor.x;
	*y = mouse_cursor.y;
	*pressed = mouse_cursor.pressed;
	k_mutex_unlock(&mouse_cursor_lock);
}

bool usb_hid_keyboard_pop_event(uint16_t *code, bool *pressed)
{
	if (ring_buf_is_empty(&key_events_queue)) {
		return false;
	}

	struct key_event ev;
	ring_buf_get(&key_events_queue, (uint8_t *)&ev, sizeof(ev));

	*code = ev.code;
	*pressed = ev.pressed;

	return true;
}

#else

int usb_hid_init(void)
{
	return -ENODEV;
}

void usb_hid_mouse_state(int32_t *x, int32_t *y, bool *pressed)
{
	*x = 0;
	*y = 0;
	*pressed = false;
}

bool usb_hid_keyboard_pop_event(uint16_t *code, bool *pressed)
{
	return false;
}

#endif
