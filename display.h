#ifndef DISPLAY_H
#define DISPLAY_H

#include "iftu.h"

#define SCREEN_WIDTH		1280
#define SCREEN_HEIGHT		720
#define SCREEN_PITCH		1280

#define FB_ADDR			0x20000000
#define FB_SIZE			(4 * SCREEN_PITCH * SCREEN_HEIGHT)

void display_init(enum iftu_bus bus);

#endif
