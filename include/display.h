#ifndef DISPLAY_H
#define DISPLAY_H

#include "iftu.h"

#define SCREEN_WIDTH		960
#define SCREEN_HEIGHT		544
#define SCREEN_PITCH		960

#define FB_ADDR			0x20000000
#define FB_SIZE			(4 * SCREEN_PITCH * SCREEN_HEIGHT)

enum display_type {
	DISPLAY_TYPE_OLED,
	DISPLAY_TYPE_LCD,
	DISPLAY_TYPE_HDMI,
};

void display_init(enum display_type type);

#endif
