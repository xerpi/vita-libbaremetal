#include "font.h"

#define GLYPH_SIZE 16

extern unsigned char msx_font[];

void font_draw_char(int x, int y, unsigned int color, char c)
{
	int i, j;
	unsigned char *glyph = &msx_font[c * 8];

	if (c < 0x20 || c > 0x7E)
		return;

	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			if (*glyph & (128 >> j))
				draw_rectangle(x + 2 * j, y + 2 * i, 2, 2, color);
		}
		glyph++;
	}
}

void font_draw_string(int x, int y, unsigned int color, const char *s)
{
	int start_x = x;

	while (*s) {
		if (*s == '\n') {
			x = start_x;
			y += GLYPH_SIZE;
		} else if (*s == '\t') {
			x += 4 * GLYPH_SIZE;
		} else {
			font_draw_char(x, y, color, *s);
			x += GLYPH_SIZE;
		}

		if (x + GLYPH_SIZE >= SCREEN_WIDTH)
			x = start_x;

		if (y + GLYPH_SIZE >= SCREEN_HEIGHT)
			y = 0;

		s++;
	}
}
