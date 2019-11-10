#include "display.h"
#include "pervasive.h"
#include "dsi.h"
#include "oled.h"
#include "lcd.h"
#include "hdmi.h"
#include "libc.h"

#define FB_ADDR	0x20000000

static const struct iftu_csc_params csc_identity_matrix_C01724 = {
	0,     0, 0x3FF,
	0, 0x3FF,     0,
	{
		{0x200,     0,     0},
		{    0, 0x200,     0},
		{    0,     0, 0x200}
	}
};

static const struct iftu_csc_params csc_identity_matrix_C019AC = {
	0,     0, 0x3FF,
	0, 0x3FF,     0,
	{
		{0x200,     0,     0},
		{    0, 0x200,     0},
		{    0,     0, 0x200}
	}
};

static const struct iftu_csc_params YCbCr_to_RGB_HDTV_C0177C = {
	0x40, 0x202, 0x3FF,
	   0,     0,     0,
	{
		{0x254,     0, 0x395},
		{0x254, 0xF93, 0xEF0},
		{0x254, 0x439,     0}
	}
};

static const struct iftu_csc_params YCbCr_to_RGB_HDTV_C0180C = {
	0x40, 0x202, 0x3FF,
	   0,     0,     0,
	{
		{0x254,     0, 0x395},
		{0x254, 0xF93, 0xEF0},
		{0x254, 0x439,     0}
	}
};

static const struct iftu_csc_params YPbPr_to_RGB_HDTV_C017D0 = {
	0, 0x202, 0x3FF,
	0,     0,     0,
	{
		{0x200,     0, 0x326},
		{0x200, 0xFA1, 0xF11},
		{0x200, 0x3B6,     0}
	}
};

static const struct iftu_csc_params stru_C01970 = {
	0x40,  0x40, 0x3AC,
	0x40, 0x3AC,  0x40,
	{
		{0x1B7,     0,     0},
		{    0, 0x1B7,     0},
		{    0,     0, 0x1B7}
	}
};

static struct display_config current_config;

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
	enum iftu_plane_config config, const struct display_config *dispcfg)
{
	struct iftu_plane_fb_config fb_cfg;
	unsigned int dst_x, dst_y;

	iftu_plane_set_alpha(bus, plane, 256);
	iftu_plane_set_csc0(bus, plane, &YCbCr_to_RGB_HDTV_C0180C);
	iftu_plane_set_csc1(bus, plane, &stru_C01970);
	iftu_plane_set_csc_enabled(bus, plane, true);

	fb_cfg.paddr = dispcfg->addr;
	fb_cfg.pixelformat = IFTU_FB_PIXELFORMAT_A8B8G8R8;
	fb_cfg.width = dispcfg->width;
	fb_cfg.height = dispcfg->height;

	if (bus == IFTU_BUS_OLED_LCD) {
		dst_x = 0;
		dst_y = 0;
	} else {
		dst_x = 26;
		dst_y = 15;
	}

	iftu_plane_config_set_config(bus, plane, config, &fb_cfg, dst_x, dst_y,
				     dispcfg->width, dispcfg->height);
}

static void display_iftu_setup(enum iftu_bus bus)
{
	display_init_iftu_plane(bus, IFTU_PLANE_A, IFTU_PLANE_CONFIG_0, &current_config);
	display_init_iftu_plane(bus, IFTU_PLANE_A, IFTU_PLANE_CONFIG_1, &current_config);
	display_init_iftu_plane(bus, IFTU_PLANE_B, IFTU_PLANE_CONFIG_0, &current_config);
	display_init_iftu_plane(bus, IFTU_PLANE_B, IFTU_PLANE_CONFIG_1, &current_config);

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
	dsi_start_master(DSI_BUS_OLED_LCD, vic);
	dsi_start_display(DSI_BUS_OLED_LCD, vic, 1);

	oled_init();
}

static void display_init_lcd(void)
{
	static const unsigned int vic = 0;
	unsigned int pixelclock;

	dsi_get_pixelclock_for_vic(vic, 24, &pixelclock);

	pervasive_dsi_set_pixelclock(0, pixelclock);
	pervasive_clock_enable_dsi(0, 0xF);
	pervasive_reset_exit_dsi(0, 7);

	dsi_init();
	dsi_start_master(DSI_BUS_OLED_LCD, vic);
	dsi_start_display(DSI_BUS_OLED_LCD, vic, 1);

	lcd_init();
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
	dsi_start_master(DSI_BUS_HDMI, vic);
	dsi_start_display(DSI_BUS_HDMI, vic, 0);

	hdmi_init();

	if (hdmi_get_hpd_state())
		hdmi_connect();
}

void display_init(enum display_type type)
{
	if (type == DISPLAY_TYPE_OLED)
		display_init_oled();
	else if (type == DISPLAY_TYPE_LCD)
		display_init_lcd();
	else if (type == DISPLAY_TYPE_HDMI)
		display_init_hdmi();

	current_config.addr = FB_ADDR;

	if (type == DISPLAY_TYPE_OLED || type == DISPLAY_TYPE_LCD) {
		current_config.pitch = 960;
		current_config.width = 960;
		current_config.height = 544;
	} else {
		current_config.pitch = 1280;
		current_config.width = 1280;
		current_config.height = 720;
	}

	display_iftu_setup(iftu_bus_for_display_type(type));
}

const struct display_config *display_get_current_config(void)
{
	return &current_config;
}
