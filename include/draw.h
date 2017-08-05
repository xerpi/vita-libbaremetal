#ifndef DRAW_H
#define DRAW_H

#include "display.h"

#define RGBA8(r, g, b, a)	((((a) & 0xFF) << 24) | (((b) & 0xFF) << 16) | \
				 (((g) & 0xFF) <<  8) | (((r) & 0xFF) <<  0))

#define RED			RGBA8(255, 0,   0,   255)
#define GREEN			RGBA8(0,   255, 0,   255)
#define BLUE			RGBA8(0,   0,   255, 255)
#define YELLOW			RGBA8(255, 255, 0,   255)
#define CYAN			RGBA8(0,   255, 255, 255)
#define PINK			RGBA8(255, 0,   255, 255)
#define LIME			RGBA8(50,  205, 50,  255)
#define PURP			RGBA8(147, 112, 219, 255)
#define WHITE			RGBA8(255, 255, 255, 255)
#define BLACK			RGBA8(0,   0,   0,   255)

static inline void draw_pixel(int x, int y, unsigned int color)
{
	*(unsigned int *)(FB_ADDR + 4 * (x + y * SCREEN_PITCH)) = color;
}

void draw_rectangle(int x, int y, int w, int h, unsigned int color);

#define draw_fill_screen(color) draw_rectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, color)

#endif
