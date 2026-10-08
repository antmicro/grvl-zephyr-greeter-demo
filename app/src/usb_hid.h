#ifndef HID_DEMO_USB_HID_H
#define HID_DEMO_USB_HID_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int usb_hid_init(void);
void usb_hid_mouse_state(int32_t *x, int32_t *y, bool *pressed);
bool usb_hid_keyboard_pop_event(uint16_t *code, bool *pressed);

#ifdef __cplusplus
}
#endif

#endif /* HID_DEMO_USB_HID_H */
