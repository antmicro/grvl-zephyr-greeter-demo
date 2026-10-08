#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <string>
#include <cctype>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/errno_private.h>
#include <zephyr/sys/util.h>
#include <zephyr/toolchain.h>
#include <zephyr/version.h>
#include <zephyr/sys/reboot.h>

#include <grvl/platform/ZephyrApp.h>

#include <grvl/grvl.h>
#include <grvl/JSEngine.h>
#include <grvl/Manager.h>
#include <grvl/Misc.h>

#include <grvl/component/Label.h>
#include <grvl/container/Division.h>
#include <grvl/container/ListView.h>
#include <grvl/container/ScrollPanel.h>

#include "usb_hid.h"

LOG_MODULE_REGISTER(demo, CONFIG_APP_LOG_LEVEL);

namespace fs = std::filesystem;

static constexpr auto GRVL_THREAD_STACK_SIZE = KB(10);
static constexpr auto GRVL_THREAD_PRIORITY = 1;
static constexpr auto TARGET_FRAMERATE = 60;

K_TIMER_DEFINE(refresh_rate_timer, nullptr, nullptr);

#include <zephyr/fs/ext2.h>
#include <zephyr/fs/fs.h>

static constexpr auto ROMFS_PATH = "/romfs";

static int setup_romfs()
{
	int rc;
	static struct fs_mount_t mount;

	// this struct needs to stay static - it must not go out of scope
	// while we are still using the filesystem.
	mount.type = FS_EXT2;
	mount.mnt_point = ROMFS_PATH;
	mount.storage_dev = (void*) "SD";
	mount.flags = FS_MOUNT_FLAG_NO_FORMAT | FS_MOUNT_FLAG_READ_ONLY;

	if ((rc = fs_mount(&mount))) {
		LOG_ERR("Failed to mount sdcard (err: %d)", rc);
	}
	return rc;
}

static fs::path get_romfs_path()
{
	return fs::path(ROMFS_PATH);
}

static void load_fonts(fs::path &rpath, grvl::Manager &manager)
{
	LOG_DBG("Loading fonts");

	auto inter_regular_data = std::make_shared<grvl::TrueTypeData>(
		(rpath / "fonts/inter-regular.ttf").string().c_str());
	auto inter_medium_data = std::make_shared<grvl::TrueTypeData>(
		(rpath / "fonts/inter-medium.ttf").string().c_str());
	auto geistmono_medium_data = std::make_shared<grvl::TrueTypeData>(
		(rpath / "fonts/geistmono-medium.ttf").string().c_str());
	auto geistmono_regular_data = std::make_shared<grvl::TrueTypeData>(
		(rpath / "fonts/geistmono-regular.ttf").string().c_str());
	auto inter_regular_12 = new grvl::TrueTypeFont(inter_regular_data, 12);
	auto inter_regular_14 = new grvl::TrueTypeFont(inter_regular_data, 14);
	auto inter_regular_16 = new grvl::TrueTypeFont(inter_regular_data, 16);
	auto inter_regular_20 = new grvl::TrueTypeFont(inter_regular_data, 20);
	auto inter_regular_24 = new grvl::TrueTypeFont(inter_regular_data, 24);
	auto inter_medium_20 = new grvl::TrueTypeFont(inter_medium_data, 20);
	auto geistmono_medium_12 = new grvl::TrueTypeFont(geistmono_medium_data, 12);
	auto geistmono_regular_12 = new grvl::TrueTypeFont(geistmono_regular_data, 12);

	manager.AddFontToFontContainer("normal", inter_regular_16);
	manager.AddFontToFontContainer("inter-regular-12", inter_regular_12);
	manager.AddFontToFontContainer("inter-regular-14", inter_regular_14);
	manager.AddFontToFontContainer("inter-regular-16", inter_regular_16);
	manager.AddFontToFontContainer("inter-regular-20", inter_regular_20);
	manager.AddFontToFontContainer("inter-regular-24", inter_regular_24);
	manager.AddFontToFontContainer("inter-medium-20", inter_medium_20);
	manager.AddFontToFontContainer("geistmono-medium-12", geistmono_medium_12);
	manager.AddFontToFontContainer("geistmono-regular-12", geistmono_regular_12);
}

static void load_images(fs::path &rpath, grvl::Manager &manager)
{
	auto load_image = [&](const char *name, const char *path, grvl::Format format) {
		manager.AddImageContentToContainer(name,
			new grvl::ImageContent((rpath / path).c_str(), format));
	};

	LOG_DBG("Loading images");

	load_image("cursor", "images/cursor.png", grvl::Format::AL88);
	load_image("next", "images/next.png", grvl::Format::AL88);
	load_image("avatar", "images/avatar.png", grvl::Format::AL88);
	load_image("shutdown", "images/shutdown.png", grvl::Format::AL88);
	load_image("reboot", "images/reboot.png", grvl::Format::AL88);
	load_image("background", "images/background.png", grvl::Format::AL88);
	load_image("zephyr", "images/zephyr.png", grvl::Format::ARGB8888);
	load_image("grvl", "images/grvl.png", grvl::Format::AL88);
	load_image("success", "images/success.png", grvl::Format::ARGB8888);
}

static void grvl_thread(void *a1, void *a2, void *a3)
{
	const device* display = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));

	const int usb_err = usb_hid_init();
	if (usb_err) {
		LOG_ERR("Failed to initialize USB HID (err: %d)", usb_err);
	}

	setup_romfs();

	grvl::ZephyrApp app {display};
	grvl::Application::Init(&app);

	auto mgr = &grvl::Manager::GetInstance();

	fs::path romfs_path = get_romfs_path();

	grvl::JSEngine::SetSourceCodeWorkingDirectory(romfs_path.string());

	load_fonts(romfs_path, *mgr);
	load_images(romfs_path, *mgr);

	mgr->BuildFromXML((romfs_path / "gui.xml").string().c_str());
	mgr->InitializationFinished();
	mgr->SetActiveScreen("start", 0);

	LOG_INF("Ready to draw");

	while (true) {
		k_timer_start(&refresh_rate_timer, K_MSEC(DIV_ROUND_CLOSEST(1000, TARGET_FRAMERATE)), K_NO_WAIT);

		app.Render();
		app.Swap();

		k_timer_status_sync(&refresh_rate_timer);
	}
}

K_THREAD_DEFINE(grvl_tid, GRVL_THREAD_STACK_SIZE, grvl_thread, nullptr, nullptr, nullptr, GRVL_THREAD_PRIORITY, 0, 0);
