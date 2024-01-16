#include "draw.h"
#include "display.h"

void draw_rectangle(int x, int y, int w, int h, uint32_t color)
{
	int i, j;

	for (i = y; i < y + h; i++)
		for (j = x; j < x + w; j++)
			draw_pixel(j, i, color);
}
