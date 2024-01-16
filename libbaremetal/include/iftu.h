#ifndef IFTU_H
#define IFTU_H

#include <stdbool.h>
#include <stdint.h>

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
	uint32_t paddr;
	enum iftu_fb_pixelformat pixelformat;
	uint32_t width;
	uint32_t height;
};

struct iftu_csc_params {
	uint32_t unk00;
	uint32_t unk04;
	uint32_t unk08;
	uint32_t unk0C;
	uint32_t unk10;
	uint32_t unk14;
	uint32_t ctm[3][3]; /* S3.9 fixed point format */
};

void iftu_bus_enable(enum iftu_bus bus);
void iftu_bus_plane_config_select(enum iftu_bus bus, enum iftu_plane plane,
				  enum iftu_plane_config config);
void iftu_bus_alpha_blending_control(enum iftu_bus bus, int ctrl);

void iftu_plane_set_alpha(enum iftu_bus bus, enum iftu_plane plane,
			  uint32_t alpha);
void iftu_plane_set_csc_enabled(enum iftu_bus bus, enum iftu_plane plane, bool enabled);
void iftu_plane_set_csc0(enum iftu_bus bus, enum iftu_plane plane,
			 const struct iftu_csc_params *csc);
void iftu_plane_set_csc1(enum iftu_bus bus, enum iftu_plane plane,
			 const struct iftu_csc_params *csc);

void iftu_plane_config_set_config(enum iftu_bus bus, enum iftu_plane plane,
				  enum iftu_plane_config config,
				  const struct iftu_plane_fb_config *fb,
				  uint32_t dst_x, uint32_t dst_y,
				  uint32_t dst_w, uint32_t dst_h);
void iftu_plane_config_set_enabled(enum iftu_bus bus, enum iftu_plane plane,
				   enum iftu_plane_config config, bool enabled);

#endif
