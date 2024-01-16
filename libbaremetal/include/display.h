#ifndef DISPLAY_H
#define DISPLAY_H

#include "iftu.h"

enum display_type {
	DISPLAY_TYPE_OLED,
	DISPLAY_TYPE_LCD,
	DISPLAY_TYPE_HDMI,
};

struct display_config {
	uint32_t addr;
	uint32_t pitch;
	uint32_t width;
	uint32_t height;
};

void display_init(enum display_type type);
const struct display_config *display_get_current_config(void);

#endif
