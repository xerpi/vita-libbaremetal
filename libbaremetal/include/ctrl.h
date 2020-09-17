#ifndef CTRL_H
#define CTRL_H

#include <stdint.h>

#define CTRL_BUTTON_HELD(ctrl, button)		((ctrl) & (button))
#define CTRL_BUTTON_PRESSED(ctrl, old, button)	(((ctrl) & ~(old)) & (button))

#define CTRL_UP		(1 << 0)
#define CTRL_RIGHT	(1 << 1)
#define CTRL_DOWN	(1 << 2)
#define CTRL_LEFT	(1 << 3)
#define CTRL_TRIANGLE	(1 << 4)
#define CTRL_CIRCLE	(1 << 5)
#define CTRL_CROSS	(1 << 6)
#define CTRL_SQUARE	(1 << 7)
#define CTRL_SELECT	(1 << 8)
#define CTRL_L		(1 << 9)
#define CTRL_R		(1 << 10)
#define CTRL_START	(1 << 11)
#define CTRL_PSBUTTON	(1 << 12)
#define CTRL_POWER	(1 << 14)
#define CTRL_VOLUP	(1 << 16)
#define CTRL_VOLDOWN	(1 << 17)
#define CTRL_HEADPHONE	(1 << 27)

struct ctrl_data {
	uint32_t buttons;
	uint8_t lx;
	uint8_t ly;
	uint8_t rx;
	uint8_t ry;
};

void ctrl_read(struct ctrl_data *data);
void ctrl_set_analog_sampling(int enable);

#endif
