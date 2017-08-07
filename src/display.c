#include "display.h"
#include "pervasive.h"
#include "dsi.h"
#include "oled.h"
#include "hdmi.h"
#include "libc.h"

static const struct iftu_csc_params csc_identity_matrix_C01724 = {0,    0,     0x3FF, 0,    0x3FF, 0,    0x200, 0, 0,     0,     0x200, 0,     0,     0,     0x200};
static const struct iftu_csc_params csc_identity_matrix_C019AC = {0,    0,     0x3FF, 0,    0x3FF, 0,    0x200, 0, 0,     0,     0x200, 0,     0,     0,     0x200};
static const struct iftu_csc_params YCbCr_to_RGB_HDTV_C0177C   = {0x40, 0x202, 0x3FF, 0,    0,     0,    0x254, 0, 0x395, 0x254, 0xF93, 0xEF0, 0x254, 0x439, 0};
static const struct iftu_csc_params YCbCr_to_RGB_HDTV_C0180C   = {0x40, 0x202, 0x3FF, 0,    0,     0,    0x254, 0, 0x395, 0x254, 0xF93, 0xEF0, 0x254, 0x439, 0};
static const struct iftu_csc_params YPbPr_to_RGB_HDTV_C017D0   = {0,    0x202, 0x3FF, 0,    0,     0,    0x200, 0, 0x326, 0x200, 0xFA1, 0xF11, 0x200, 0x3B6, 0};
static const struct iftu_csc_params stru_C01970                = {0x40, 0x40,  0x3AC, 0x40, 0x3AC, 0x40, 0x1B7, 0, 0,     0,     0x1B7, 0,     0,     0,     0x1B7};

static inline enum iftu_bus iftu_bus_for_display_type(enum display_type type)
{
	switch (type) {
	case DISPLAY_TYPE_OLED:
	case DISPLAY_TYPE_LCD:
	default:
		return IFTU_BUS_0;
	case DISPLAY_TYPE_HDMI:
		return IFTU_BUS_1;
	}
}

static void display_init_iftu_plane(enum iftu_bus bus, enum iftu_plane plane,
	enum iftu_plane_config config, unsigned int fb_paddr)
{
	struct iftu_plane_fb_config fb_cfg;

	iftu_plane_set_alpha(bus, plane, 256);
	iftu_plane_set_csc0(bus, plane, &YCbCr_to_RGB_HDTV_C0180C);
	iftu_plane_set_csc1(bus, plane, &stru_C01970);
	iftu_plane_set_csc_enabled(bus, plane, true);

	fb_cfg.paddr = fb_paddr;
	fb_cfg.pixelformat = IFTU_FB_PIXELFORMAT_A8B8G8R8;
	fb_cfg.width = 1280;
	fb_cfg.height = 720;

	iftu_plane_config_set_fb_config(bus, plane, config, &fb_cfg);
}

static void display_iftu_setup(enum iftu_bus bus)
{
	display_init_iftu_plane(bus, IFTU_PLANE_A, IFTU_PLANE_CONFIG_0, FB_ADDR);
	display_init_iftu_plane(bus, IFTU_PLANE_A, IFTU_PLANE_CONFIG_1, FB_ADDR);
	display_init_iftu_plane(bus, IFTU_PLANE_B, IFTU_PLANE_CONFIG_0, FB_ADDR);
	display_init_iftu_plane(bus, IFTU_PLANE_B, IFTU_PLANE_CONFIG_1, FB_ADDR);

	/*
	 * Enable the first plane and select the first plane config.
	 */
	iftu_bus_plane_config_select(bus, IFTU_PLANE_A, IFTU_PLANE_CONFIG_0);

	/*
	 * Disable the second plane.
	 */
	iftu_plane_config_set_enabled(bus, IFTU_PLANE_B, IFTU_PLANE_CONFIG_0, false);
	iftu_plane_config_set_enabled(bus, IFTU_PLANE_B, IFTU_PLANE_CONFIG_1, false);

	iftu_bus_alpha_blending_control(bus, 0);

	iftu_bus_enable(bus);
}

static void display_init_oled(void)
{
	static const unsigned int vic = 0;
	unsigned int pixelclock;

	dsi_get_pixelclock_for_vic(vic, 24, &pixelclock);

	pervasive_dsi_set_pixelclock(0, pixelclock);
	pervasive_clock_enable_dsi(0, 0xF);
	pervasive_reset_exit_dsi(0, 7);

	dsi_init();
	dsi_enable_bus(DSI_BUS_OLED_LCD, vic);

	dsi_unk(DSI_BUS_OLED_LCD, vic, 1);

	oled_init();
}

static void display_init_lcd(void)
{
	/* TODO */
}

static void display_init_hdmi(void)
{
	static const unsigned int vic = 0x8600;
	unsigned int pixelclock;

	dsi_get_pixelclock_for_vic(vic, 24, &pixelclock);

	pervasive_dsi_set_pixelclock(1, pixelclock);
	pervasive_clock_enable_dsi(1, 0xF);
	pervasive_reset_exit_dsi(1, 7);

	dsi_init();
	dsi_enable_bus(DSI_BUS_HDMI, vic);

	dsi_unk(DSI_BUS_HDMI, vic, 0);

	hdmi_init();
}

void display_init(enum display_type type)
{
	if (type == DISPLAY_TYPE_OLED)
		display_init_oled();
	else if (type == DISPLAY_TYPE_LCD)
		display_init_lcd();
	else if (type == DISPLAY_TYPE_HDMI)
		display_init_hdmi();

	display_iftu_setup(iftu_bus_for_display_type(type));
}
