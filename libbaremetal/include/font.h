#ifndef FONT_H
#define FONT_H

#include "draw.h"

void font_draw_char(int x, int y, uint32_t color, char c);
void font_draw_string(int x, int y, uint32_t color, const char *s);
void font_draw_stringf(int x, int y, uint32_t color, const char *s, ...) __attribute__((format(printf, 4, 5)));

#endif
