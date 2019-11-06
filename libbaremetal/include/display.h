#ifndef DISPLAY_H
#define DISPLAY_H

#include "iftu.h"

enum display_type {
	DISPLAY_TYPE_OLED,
	DISPLAY_TYPE_LCD,
	DISPLAY_TYPE_HDMI,
};

struct display_config {
	unsigned int addr;
	unsigned int pitch;
	unsigned int width;
	unsigned int height;
};

void display_init(enum display_type type);
const struct display_config *display_get_current_config(void);

#endif
