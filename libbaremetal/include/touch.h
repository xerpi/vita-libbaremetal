#ifndef TOUCH_H
#define TOUCH_H

#include <stdint.h>

#define TOUCH_PORT_FRONT (1 << 0)
#define TOUCH_PORT_BACK  (1 << 1)

#define TOUCH_MAX_REPORT_FRONT 6
#define TOUCH_MAX_REPORT_BACK  4

struct touch_report {
	uint8_t id;
	uint8_t force;
	uint16_t x;
	uint16_t y;
};

struct touch_data {
	uint32_t num_front;
	uint32_t num_back;
	struct touch_report front[TOUCH_MAX_REPORT_FRONT];
	struct touch_report back[TOUCH_MAX_REPORT_BACK];
};

void touch_init(void);
void touch_configure(int port_mask, uint8_t max_report_front, uint8_t max_report_back);
void touch_set_sampling_cycle(int port_mask, uint8_t cycles_front, uint8_t cycles_back);
void touch_read(int port_mask, struct touch_data *data);

#endif
