#include "syscon.h"
#include "spi.h"
#include "gpio.h"
#include "utils.h"
#include "libc.h"
#include "log.h"

struct syscon_packet {
	unsigned char tx[32];	/* tx[0..1] = cmd, tx[2] = size */
	unsigned char rx[32];	/* rx[0] = status, rx[1] = size, rx[2] = response */
};

static void syscon_packet_start(struct syscon_packet *packet)
{
	int i = 0;
	unsigned char cmd_size = packet->tx[2];
	unsigned char tx_total_size = cmd_size + 3;
	unsigned int offset;

	gpio_port_clear(0, 3);
	spi_write_start(0);

	if (cmd_size <= 29) {
		offset = 2;
	} else {

	}

	do {
		unsigned char data0 = packet->tx[i];
		unsigned char data1 = packet->tx[i + 1];
		spi_write(0, data0 | (data1 << 8));
		i += 2;
	} while (i < tx_total_size);

	spi_write_end(0);
	gpio_port_set(0, 3);
}

static void syscon_cmd_sync(struct syscon_packet *packet)
{
	int i = 0;

	while (!gpio_query_intr(0, 4))
		;

	gpio_acquire_intr(0, 4);

	while (spi_read_avaiable(0)) {
		unsigned int data = spi_read(0);
		packet->rx[i] = data & 0xFF;
		packet->rx[i + 1] = (data >> 8) & 0xFF;
		i += 2;
	}

	spi_read_end(0);
	gpio_port_clear(0, 3);
}

static void syscon_common_read(unsigned int *buffer, unsigned short cmd)
{
	struct syscon_packet packet;

	packet.tx[0] = cmd & 0xFF;
	packet.tx[1] = (cmd >> 8) & 0xFF;
	packet.tx[2] = 1;

	memset(packet.rx, -1, sizeof(packet.rx));

	syscon_packet_start(&packet);
	syscon_cmd_sync(&packet);

	memcpy(buffer, &packet.rx[4], packet.rx[2] - 2);
}

static void syscon_common_write(unsigned int data, unsigned short cmd, unsigned int length)
{
	int i;
	unsigned char hash = 0;
	struct syscon_packet packet;

	packet.tx[0] = cmd & 0xFF;
	packet.tx[1] = (cmd >> 8) & 0xFF;
	packet.tx[2] = length;

	packet.tx[3] = data & 0xFF;
	packet.tx[4] = (data >> 8) & 0xFF;
	packet.tx[5] = (data >> 16) & 0xFF;
	packet.tx[6] = (data >> 24) & 0xFF;

	/*
	 * Calculate packet hash
	 */
	for (i = 0; i < length + 2; i++)
		hash += packet.tx[i];

	packet.tx[2 + length] = ~hash;
	memset(&packet.tx[3 + length], -1, sizeof(packet.rx) - (3 + length));
	memset(packet.rx, -1, sizeof(packet.rx));

	syscon_packet_start(&packet);
	syscon_cmd_sync(&packet);
}

int syscon_init(void)
{
	gpio_set_port_mode(0, 3, GPIO_PORT_MODE_OUTPUT);
	gpio_set_port_mode(0, 4, GPIO_PORT_MODE_INPUT);
	gpio_set_intr_mode(0, 4, 3);

	return 0;
}

void syscon_set_hdmi_cdc_hpd(int enable)
{
	syscon_common_write(enable, 0x886, 2);
}
