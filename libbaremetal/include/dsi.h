#ifndef DSI_H
#define DSI_H

#include <stdint.h>

enum dsi_bus {
	DSI_BUS_0		= 0,
	DSI_BUS_1		= 1,
	DSI_BUS_OLED_LCD	= DSI_BUS_0,
	DSI_BUS_HDMI		= DSI_BUS_1,
};

void dsi_init(void);
int dsi_get_dimensions_for_vic(uint32_t vic, uint32_t *width, uint32_t *height);
int dsi_get_pixelclock_for_vic(uint32_t vic, uint32_t bpp, uint32_t *pixelclock);
void dsi_start_master(enum dsi_bus bus, uint32_t vic);
void dsi_stop_master(enum dsi_bus bus);
void dsi_start_display(enum dsi_bus bus, uint32_t vic, uint32_t unk);

#endif
