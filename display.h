#ifndef DISPLAY_H
#define DISPLAY_H

#define RGBA8(r, g, b, a)	((((a) & 0xFF) << 24) | (((b) & 0xFF) << 16) | \
				 (((g) & 0xFF) << 8) | (((r) & 0xFF) << 0))

#define RED			RGBA8(255, 0,   0,   255)
#define GREEN			RGBA8(0,   255, 0,   255)
#define BLUE			RGBA8(0,   0,   255, 255)
#define CYAN			RGBA8(0,   255, 255, 255)
#define LIME			RGBA8(50,  205, 50,  255)
#define PURP			RGBA8(147, 112, 219, 255)
#define WHITE			RGBA8(255, 255, 255, 255)
#define BLACK			RGBA8(0,   0,   0,   255)

#define SCREEN_PITCH		1280
#define SCREEN_HEIGHT		720

#define FB_BASE_ADDR		0x20000000
#define FB_ADDR(i)		(FB_BASE_ADDR + (i) * 4 * SCREEN_PITCH * SCREEN_HEIGHT)

void display_init(int bus);

#endif
