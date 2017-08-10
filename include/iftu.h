#ifndef IFTU_H
#define IFTU_H

#include <stdbool.h>

enum iftu_bus {
	IFTU_BUS_0			= 0,
	IFTU_BUS_1			= 1,
	IFTU_BUS_OLED_LCD		= IFTU_BUS_0,
	IFTU_BUS_HDMI			= IFTU_BUS_1,
};

enum iftu_plane {
	IFTU_PLANE_A			= 0,
	IFTU_PLANE_B			= 1
};

enum iftu_plane_config {
	IFTU_PLANE_CONFIG_0		= 0,
	IFTU_PLANE_CONFIG_1		= 1
};

enum iftu_fb_pixelformat {
	IFTU_FB_PIXELFORMAT_A8B8G8R8	= 0x10
};

struct iftu_plane_fb_config {
	unsigned int paddr;
	enum iftu_fb_pixelformat pixelformat;
	unsigned int width;
	unsigned int height;
};

struct iftu_csc_params {
	unsigned int unk00;
	unsigned int unk04;
	unsigned int unk08;
	unsigned int unk0C;
	unsigned int unk10;
	unsigned int unk14;
	unsigned int csc_rr;
	unsigned int csc_rg;
	unsigned int csc_rb;
	unsigned int csc_gr;
	unsigned int csc_gg;
	unsigned int csc_gb;
	unsigned int csc_br;
	unsigned int csc_bg;
	unsigned int csc_bb;
};

void iftu_bus_enable(enum iftu_bus bus);
void iftu_bus_plane_config_select(enum iftu_bus bus, enum iftu_plane plane,
				  enum iftu_plane_config config);
void iftu_bus_alpha_blending_control(enum iftu_bus bus, int ctrl);

void iftu_plane_set_alpha(enum iftu_bus bus, enum iftu_plane plane,
			  unsigned int alpha);
void iftu_plane_set_csc_enabled(enum iftu_bus bus, enum iftu_plane plane, bool enabled);
void iftu_plane_set_csc0(enum iftu_bus bus, enum iftu_plane plane,
			 const struct iftu_csc_params *csc);
void iftu_plane_set_csc1(enum iftu_bus bus, enum iftu_plane plane,
			 const struct iftu_csc_params *csc);

void iftu_plane_config_set_config(enum iftu_bus bus, enum iftu_plane plane,
				  enum iftu_plane_config config,
				  const struct iftu_plane_fb_config *fb,
				  unsigned int dst_x, unsigned int dst_y,
				  unsigned int dst_w, unsigned int dst_h);
void iftu_plane_config_set_enabled(enum iftu_bus bus, enum iftu_plane plane,
				   enum iftu_plane_config config, bool enabled);

#endif
