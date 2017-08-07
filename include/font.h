#ifndef FONT_H
#define FONT_H

#include <stdarg.h>
#include "draw.h"

void font_draw_char(int x, int y, unsigned int color, char c);
void font_draw_string(int x, int y, unsigned int color, const char *s);
void font_draw_stringf(int x, int y, unsigned int color, const char *s, ...);

#endif
