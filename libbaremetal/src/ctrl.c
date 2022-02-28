#include <stddef.h>
#include "ctrl.h"
#include "syscon.h"
#include "libc.h"
#include "utils.h"

// Packet 0x101 is 10 bytes long
// Packet 0x104 is 14 bytes long, includes arrows pressure

void ctrl_read(struct ctrl_data *data)
{
	uint8_t buffer[SYSCON_RX_HEADER_SIZE + 10];

	syscon_command_read(0x101, buffer, sizeof(buffer));

	data->buttons = (buffer[4] | (buffer[5] << 8) | (buffer[6] << 16) |
			(buffer[6] << 24)) ^ 0x4037fcf9;
	data->lx = buffer[8];
	data->ly = buffer[9];
	data->rx = buffer[10];
	data->ry = buffer[11];
}

void ctrl_set_analog_sampling(int enable)
{
	uint8_t data;

	if (enable) {
		if (syscon_get_baryon_version() < 0x90202)
			data = 1;
		else
			data = 3;
	} else {
		data = 0;
	}

	syscon_short_command_write(0x180, data, 1);
}
