#include "syscon.h"
#include "pervasive.h"
#include "spi.h"
#include "gpio.h"
#include "utils.h"
#include "libc.h"

extern void console_printf(const char *s, ...);
extern void PRINT_BUF(const char *name, const void *buff, int size);
#define PRINT(...) console_printf(__VA_ARGS__)

static void syscon_packet_send(struct syscon_packet *packet)
{
	int i = 0;
	unsigned char cmd_size = packet->tx[SYSCON_TX_LENGTH];
	unsigned char tx_total_size = cmd_size + 3;
	unsigned int offset;
	(void)offset;

	gpio_port_clear(0, GPIO_PORT_SYSCON_OUT);
	spi_write_start(0);

	if (cmd_size <= 29) {
		offset = 2;
	} else {
		/* TODO */
	}

	do {
		spi_write(0, (packet->tx[i + 1] << 8) | packet->tx[i]);
		i += 2;
	} while (i < tx_total_size);

	spi_write_end(0);
	gpio_port_set(0, GPIO_PORT_SYSCON_OUT);
}

static void syscon_packet_sync_recv(struct syscon_packet *packet)
{
	int i = 0;

	while (!gpio_query_intr(0, GPIO_PORT_SYSCON_IN))
		;

	gpio_acquire_intr(0, GPIO_PORT_SYSCON_IN);

	unsigned short cmd = (packet->tx[0] | (unsigned short)packet->tx[1] << 8);

	while (spi_read_available(0)) {
		unsigned int data = spi_read(0);
		packet->rx[i] = data & 0xFF;
		packet->rx[i + 1] = (data >> 8) & 0xFF;
		if (cmd != 0x101) {
			//PRINT("%02X%02X", packet->rx[i], packet->rx[i + 1]);
		}
		i += 2;
	}
	if (cmd != 0x101) {
		//PRINT("\n");
	}

	spi_read_end(0);
	gpio_port_clear(0, GPIO_PORT_SYSCON_OUT);

	memset(&packet->rx[i], 0, sizeof(packet->rx) - i);
}

static inline void syscon_packet_compute_checksum(struct syscon_packet *packet)
{
	unsigned char length = packet->tx[SYSCON_TX_LENGTH];
	unsigned char hash = 0;

	for (int i = 0; i < length + 2; i++)
		hash += packet->tx[i];

	packet->tx[2 + length] = ~hash;
}

// static
unsigned char syscon_packet_exec(struct syscon_packet *packet)
{
	unsigned char ret;

	syscon_packet_compute_checksum(packet);

	unsigned short cmd = (packet->tx[0] | (unsigned short)packet->tx[1] << 8);
	//if (cmd != 0x101) {
		//PRINT("cmd 0x%X, ", packet->tx[0] | (packet->tx[1] << 8));
	//}

	do  {
		syscon_packet_send(packet);
		syscon_packet_sync_recv(packet);
		ret = packet->rx[SYSCON_RX_RESULT];
	} while (ret == 0x80 || ret == 0x81);

	//if (cmd != 0x101) {
		//PRINT_BUF("rx:", packet->rx, 14);
		/*unsigned short sum = 0;
		for (int i = 0; i < sizeof(packet->rx); i++) {
			sum += packet->rx[i];
		}
		PRINT("cmd 0x%04X: %04X\n", cmd, sum);*/
	//}
	return ret;
}

static unsigned char syscon_command_read(unsigned short cmd, void *buffer, int size)
{
	struct syscon_packet packet;
	unsigned char rx_size;
	unsigned char ret;

	packet.tx[SYSCON_TX_CMD_LO] = cmd & 0xFF;
	packet.tx[SYSCON_TX_CMD_HI] = (cmd >> 8) & 0xFF;
	packet.tx[SYSCON_TX_LENGTH] = 1;

	ret = syscon_packet_exec(&packet);

	rx_size = packet.rx[SYSCON_RX_LENGTH];
	if (rx_size >= 2)
		rx_size -= 2;
	else
		rx_size = 0;

	if (rx_size > size)
		rx_size = size;

	memcpy(buffer, &packet.rx[SYSCON_RX_DATA], rx_size);
	memset((char *)buffer + rx_size, 0, size - rx_size);
	return ret;
}

static unsigned char syscon_command_write(unsigned short cmd, unsigned int data, unsigned int length)
{
	struct syscon_packet packet;

	packet.tx[SYSCON_TX_CMD_LO] = cmd & 0xFF;
	packet.tx[SYSCON_TX_CMD_HI] = (cmd >> 8) & 0xFF;
	packet.tx[SYSCON_TX_LENGTH] = length;

	packet.tx[SYSCON_TX_DATA(0)] = data & 0xFF;
	packet.tx[SYSCON_TX_DATA(1)] = (data >> 8) & 0xFF;
	packet.tx[SYSCON_TX_DATA(2)] = (data >> 16) & 0xFF;
	packet.tx[SYSCON_TX_DATA(3)] = (data >> 24) & 0xFF;

	return syscon_packet_exec(&packet);
}

int syscon_init(void)
{
	unsigned int syscon_version;

	spi_init(0);

	gpio_set_port_mode(0, GPIO_PORT_SYSCON_OUT, GPIO_PORT_MODE_OUTPUT);
	gpio_set_port_mode(0, GPIO_PORT_SYSCON_IN, GPIO_PORT_MODE_INPUT);
	gpio_set_intr_mode(0, GPIO_PORT_SYSCON_IN, 3);

	syscon_command_read(1, &syscon_version, sizeof(syscon_version));

	if (syscon_version > 0x1000003)
		syscon_command_write(0x80, 0x12, 3);
	else if (syscon_version > 0x70501)
		syscon_command_write(0x80, 2, 3);

	return 0;
}

int syscon_get_baryon_version(void)
{
	/* TODO */
	return 0x100060D;
}

int syscon_get_hardware_info(void)
{
	return 0x102400;
}

void syscon_reset_device(int type, int mode)
{
	smc(0x11A, type, mode, 0, 0);
}

void syscon_set_hdmi_cdc_hpd(int enable)
{
	syscon_command_write(0x886, enable, 2);
}

void syscon_msif_set_power(int enable)
{
	syscon_command_write(0x89B, enable, 2);
}

void syscon_ctrl_device_reset(unsigned int param_1, unsigned int param_2)
{
	syscon_command_write(0x88F, param_2 | (param_1 << 8), 3);
}

void syscon_get_touchpanel_device_info(struct syscon_touchpanel_device_info *info)
{
	unsigned char buff[8];
	syscon_command_read(0x380, buff, sizeof(buff));
	info->front_vendor_id  = (buff[0] << 8) | buff[1];
	info->front_fw_version = (buff[2] << 8) | buff[3];
	info->back_vendor_id   = (buff[4] << 8) | buff[5];
	info->back_fw_version  = (buff[6] << 8) | buff[7];
}

void syscon_get_touchpanel_device_info_ext(struct syscon_touchpanel_device_info_ext *info)
{
	unsigned char buff[20];
	syscon_command_read(0x390, buff, sizeof(buff));
	info->front_vendor_id  = (buff[0] << 8) | buff[1];
	info->front_fw_version = (buff[2] << 8) | buff[3];
	info->front_unk1       = (buff[4] << 8) | buff[5];
	info->front_unk2       = buff[6];
	info->front_unk3       = buff[7];
	info->unused1          = (buff[9] << 8) | buff[8];
	info->back_vendor_id   = (buff[10] << 8) | buff[11];
	info->back_fw_version  = (buff[12] << 8) | buff[13];
	info->back_unk1        = (buff[14] << 8) | buff[15];
	info->back_unk2        = buff[16];
	info->back_unk3        = buff[17];
	info->unused2          = (buff[19] << 8) | buff[18];
}

void syscon_9B6B8BB9(unsigned short *data)
{
	syscon_command_read(0x3a7, data, sizeof(*data));
}

void syscon_C4A61241(unsigned short *data)
{
	syscon_command_read(0x3b7, data, sizeof(*data));
}

void syscon_1546A141(int cycles_front, int cycles_back)
{
	struct syscon_packet packet;

	packet.tx[0] = 0x87;
	packet.tx[1] = 3;
	packet.tx[2] = 9;

	if (cycles_front >= 0) {
		packet.tx[3] = 1;
		packet.tx[4] = cycles_front & 0xFF;
	} else {
		packet.tx[3] = 0;
		packet.tx[4] = 0;
	}

	packet.tx[5] = 0;
	packet.tx[6] = 0;

	if (cycles_back >= 0) {
		packet.tx[7] = 1;
		packet.tx[8] = cycles_back & 0xFF;
	} else {
		packet.tx[7] = 0;
		packet.tx[8] = 0;
	}

	packet.tx[9] = 0;
	packet.tx[10] = 0;

	syscon_packet_exec(&packet);

	//PRINT_BUF("syscon_1546A141:", packet.rx, 8);
}
