#include <stdarg.h>
#include <stdio.h>
#include "font.h"
#include "display.h"

#define GLYPH_SIZE	16
#define BG_COLOR	BLACK

extern uint8_t msx_font[];

void font_draw_char(int x, int y, uint32_t color, char c)
{
	int i, j;
	uint8_t *glyph = &msx_font[c * 8];

	if (c < 0x20 || c > 0x7E)
		return;

	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			uint32_t c = BG_COLOR;

			if (*glyph & (128 >> j))
				c = color;

			draw_rectangle(x + 2 * j, y + 2 * i, 2, 2, c);
		}
		glyph++;
	}
}

void font_draw_string(int x, int y, uint32_t color, const char *s)
{
	const struct display_config *config = display_get_current_config();
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

		if (x + GLYPH_SIZE >= config->width)
			x = start_x;

		if (y + GLYPH_SIZE >= config->height)
			y = 0;

		s++;
	}
}

void font_draw_stringf(int x, int y, uint32_t color, const char *s, ...)
{
	char buf[256];
	va_list argptr;

	va_start(argptr, s);
	vsnprintf(buf, sizeof(buf), s, argptr);
	va_end(argptr);

	font_draw_string(x, y, color, buf);
}
