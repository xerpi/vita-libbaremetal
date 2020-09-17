#ifndef TOUCH_H
#define TOUCH_H

#include <stdint.h>

struct touch_report {
	uint8_t id; // bit 8: invalid
	uint8_t force;
	uint16_t x; // 11 bits
	uint16_t y; // 11 bits
};

void touch_init(void);
void touch_enable1(void);
void touch_enable2(void);
void touch_enable3(void);
void touch_enable4(void);
void touch_read(unsigned char *buffer);
void touch_enable_back(void);

#endif
