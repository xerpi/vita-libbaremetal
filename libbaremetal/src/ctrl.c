#include <stddef.h>
#include "ctrl.h"
#include "syscon.h"
#include "libc.h"
#include "utils.h"

void ctrl_read(struct ctrl_data *data)
{
	// Packet 0x101 is 10 bytes long
	// Packet 0x104 is 14 bytes long, includes arrows pressure
	uint8_t buffer[10];

	//syscon_command_read(0x101, buffer, sizeof(buffer));
	struct syscon_packet packet;
	memset(&packet, 0, sizeof(packet));
	memset(&packet, 0, sizeof(packet));
	packet.tx[0] = 1;
	packet.tx[1] = 1;
	packet.tx[2] = 1;
	syscon_packet_exec(&packet);
	//memcpy(buffer, &packet.rx[SYSCON_RX_DATA], packet.rx[SYSCON_RX_LENGTH]);

	data->buttons = (packet.rx[4] | (packet.rx[5] << 8) | (packet.rx[6] << 16) |
	                (packet.rx[6] << 24)) ^ 0x4037fcf9;
	data->lx = packet.rx[8];
	data->ly = packet.rx[9];
	data->rx = packet.rx[10];
	data->ry = packet.rx[11];
}

void ctrl_set_analog_sampling(int enabled)
{
	struct syscon_packet packet;
	memset(&packet, 0, sizeof(packet));
	packet.tx[0] = 0x80;
	packet.tx[1] = 1;
	packet.tx[2] = 2;
	if (enabled) {
		if (syscon_get_baryon_version() < 0x90202)
			packet.tx[3] = 1;
		else
			packet.tx[3] = 3;
	} else {
		packet.tx[3] = 0;
	}
	syscon_packet_exec(&packet);
}
