#include "display.h"
#include "iftu.h"
#include "libc.h"

static const struct iftu_csc_params csc_identity_matrix_C01724 = {0,    0,     0x3FF, 0,    0x3FF, 0,    0x200, 0, 0,     0,     0x200, 0,     0,     0,     0x200};
static const struct iftu_csc_params csc_identity_matrix_C019AC = {0,    0,     0x3FF, 0,    0x3FF, 0,    0x200, 0, 0,     0,     0x200, 0,     0,     0,     0x200};
static const struct iftu_csc_params YCbCr_to_RGB_HDTV_C0177C   = {0x40, 0x202, 0x3FF, 0,    0,     0,    0x254, 0, 0x395, 0x254, 0xF93, 0xEF0, 0x254, 0x439, 0};
static const struct iftu_csc_params YCbCr_to_RGB_HDTV_C0180C   = {0x40, 0x202, 0x3FF, 0,    0,     0,    0x254, 0, 0x395, 0x254, 0xF93, 0xEF0, 0x254, 0x439, 0};
static const struct iftu_csc_params YPbPr_to_RGB_HDTV_C017D0   = {0,    0x202, 0x3FF, 0,    0,     0,    0x200, 0, 0x326, 0x200, 0xFA1, 0xF11, 0x200, 0x3B6, 0};
static const struct iftu_csc_params stru_C01970                = {0x40, 0x40,  0x3AC, 0x40, 0x3AC, 0x40, 0x1B7, 0, 0,     0,     0x1B7, 0,     0,     0,     0x1B7};

static void display_fill_fb(void *addr, unsigned int color)
{
	int i, j;

	for (i = 0; i < SCREEN_HEIGHT; i++)
		for (j = 0; j < SCREEN_PITCH; j++)
			*(unsigned int *)(addr + 4 * (j + i * SCREEN_PITCH)) = color;
}

static void display_init_iftu_plane(int bus, int plane)
{
	iftu_plane_set_alpha(bus, plane, 256);
	iftu_plane_set_csc0(bus, plane, &YCbCr_to_RGB_HDTV_C0180C);
	iftu_plane_set_csc1(bus, plane, &stru_C01970);
	iftu_plane_set_csc_enabled(bus, plane, 1);

	for (int i = 0; i < 2; i++) {
		static const unsigned int colors[] = {
			RED, GREEN, BLUE, PURP
		};

		struct iftu_plane_fb_config fb_cfg;
		int index = 2 * plane + i;

		fb_cfg.paddr = FB_ADDR(index);
		fb_cfg.pixelformat = IFTU_FB_PIXELFORMAT_A8B8G8R8;
		fb_cfg.width = 1280;
		fb_cfg.height = 720;

		display_fill_fb((void *)fb_cfg.paddr, colors[index]);

		iftu_plane_set_fb_config(bus, plane, i, &fb_cfg);
	}
}

void display_init(int bus)
{
	display_init_iftu_plane(1, 0);
	display_init_iftu_plane(1, 1);

	iftu_bus_alpha_blending_control(1, 0);

	iftu_bus_plane_config_select(1, 0, 0);
	iftu_bus_plane_config_select(1, 1, 0);

	iftu_bus_enable(1);
}
