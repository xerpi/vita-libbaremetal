#include "syscon.h"
#include "sysroot.h"
#include "pervasive.h"
#include "spi.h"
#include "gpio.h"
#include "utils.h"
#include "libc.h"

static uint32_t g_baryon_version;

int syscon_init(void)
{
	uint8_t version[SYSCON_RX_HEADER_SIZE + sizeof(g_baryon_version)];

	spi_init(0);

	gpio_set_port_mode(0, GPIO_PORT_SYSCON_OUT, GPIO_PORT_MODE_OUTPUT);
	gpio_set_port_mode(0, GPIO_PORT_SYSCON_IN, GPIO_PORT_MODE_INPUT);
	gpio_set_intr_mode(0, GPIO_PORT_SYSCON_IN, 3);

	syscon_command_read(1, version, sizeof(version));
	memcpy(&g_baryon_version, &version[SYSCON_RX_DATA], sizeof(g_baryon_version));

	if (g_baryon_version > 0x1000003)
		syscon_short_command_write(0x80, 0x12, 2);
	else if (g_baryon_version > 0x70501)
		syscon_short_command_write(0x80, 2, 2);

	return 0;
}

static void syscon_raw_write(const uint8_t *buffer, int length)
{
	uint32_t hash = 0;
	int i = 0;

	gpio_port_clear(0, GPIO_PORT_SYSCON_OUT);
	spi_write_start(0);

	while (length >= 2) {
		uint8_t lo = buffer[i];
		uint8_t hi = buffer[i + 1];
		spi_write(0, lo | ((uint32_t)hi << 8));
		hash += lo + hi;
		i += 2;
		length -= 2;
	}

	if (length) {
		hash = ~(hash + buffer[i]) & 0xFF;
		spi_write(0, buffer[i] | ((uint32_t)hash << 8));
	} else {
		spi_write(0, ~hash & 0xFF);
	}

	spi_write_end(0);
	gpio_port_set(0, GPIO_PORT_SYSCON_OUT);
}

static void syscon_raw_read(uint8_t *buffer, int max_length)
{
	int i = 0;

	while (!gpio_query_intr(0, GPIO_PORT_SYSCON_IN))
		;

	gpio_acquire_intr(0, GPIO_PORT_SYSCON_IN);

	while (spi_read_available(0)) {
		uint32_t data = spi_read(0);
		if (i < max_length)
			buffer[i] = data & 0xFF;
		if (i + 1 < max_length)
			buffer[i + 1] = (data >> 8) & 0xFF;
		i += 2;
	}

	spi_read_end(0);
	gpio_port_clear(0, GPIO_PORT_SYSCON_OUT);
}

void syscon_transfer(const uint8_t *tx, int tx_size, uint8_t *rx, int max_rx_size)
{
	uint8_t ret;

	do {
		syscon_raw_write(tx, tx_size);
		syscon_raw_read(rx, max_rx_size);
		ret = rx[SYSCON_RX_RESULT];
	} while (ret == 0x80 || ret == 0x81);
}

void syscon_command_read(uint16_t cmd, void *buffer, int max_length)
{
	uint8_t tx[SYSCON_TX_HEADER_SIZE];

	tx[SYSCON_TX_CMD_LO] = cmd & 0xFF;
	tx[SYSCON_TX_CMD_HI] = (cmd >> 8) & 0xFF;
	tx[SYSCON_TX_LENGTH] = 1;

	syscon_transfer(tx, sizeof(tx), buffer, max_length);
}

void syscon_short_command_write(uint16_t cmd, uint32_t data, int length)
{
	uint8_t tx[SYSCON_TX_HEADER_SIZE + 4];
	uint8_t rx[16];

	tx[SYSCON_TX_CMD_LO] = cmd & 0xFF;
	tx[SYSCON_TX_CMD_HI] = (cmd >> 8) & 0xFF;
	tx[SYSCON_TX_LENGTH] = length + 1;

	tx[SYSCON_TX_DATA(0)] = data & 0xFF;
	tx[SYSCON_TX_DATA(1)] = (data >> 8) & 0xFF;
	tx[SYSCON_TX_DATA(2)] = (data >> 16) & 0xFF;
	tx[SYSCON_TX_DATA(3)] = (data >> 24) & 0xFF;

	syscon_transfer(tx, SYSCON_TX_HEADER_SIZE + length, rx, sizeof(rx));
}

int syscon_get_baryon_version(void)
{
	return g_baryon_version;
}

int syscon_get_hardware_info(void)
{
	return sysroot_get_hw_info();
}

void syscon_reset_device(int type, int mode)
{
	smc(0x11A, type, mode, 0, 0);
}

void syscon_set_hdmi_cdc_hpd(int enable)
{
	syscon_short_command_write(0x886, enable, 1);
}

void syscon_msif_set_power(int enable)
{
	syscon_short_command_write(0x89B, enable, 1);
}

void syscon_ctrl_device_reset(unsigned int param_1, unsigned int param_2)
{
	syscon_short_command_write(0x88F, param_2 | (param_1 << 8), 2);
}

void syscon_get_touchpanel_device_info(struct syscon_touchpanel_device_info *info)
{
	unsigned char buffer[SYSCON_RX_HEADER_SIZE + sizeof(*info)];
	uint8_t *data = &buffer[SYSCON_RX_DATA];

	syscon_command_read(0x380, buffer, sizeof(buffer));

	info->front_vendor_id  = (data[0] << 8) | data[1];
	info->front_fw_version = (data[2] << 8) | data[3];
	info->back_vendor_id   = (data[4] << 8) | data[5];
	info->back_fw_version  = (data[6] << 8) | data[7];
}

void syscon_get_touchpanel_device_info_ext(struct syscon_touchpanel_device_info_ext *info)
{
	uint8_t buffer[SYSCON_RX_HEADER_SIZE + sizeof(*info)];
	uint8_t *data = &buffer[SYSCON_RX_DATA];

	syscon_command_read(0x390, buffer, sizeof(buffer));

	info->front_vendor_id  = (data[0] << 8) | data[1];
	info->front_fw_version = (data[2] << 8) | data[3];
	info->front_unk1       = (data[4] << 8) | data[5];
	info->front_unk2       = data[6];
	info->front_unk3       = data[7];
	info->unused1          = (data[9] << 8) | data[8];
	info->back_vendor_id   = (data[10] << 8) | data[11];
	info->back_fw_version  = (data[12] << 8) | data[13];
	info->back_unk1        = (data[14] << 8) | data[15];
	info->back_unk2        = data[16];
	info->back_unk3        = data[17];
	info->unused2          = (data[19] << 8) | data[18];
}

void syscon_get_touchpanel_unk_info_front(uint16_t *data)
{
	uint8_t buff[SYSCON_RX_HEADER_SIZE + 2];
	syscon_command_read(0x3a7, buff, sizeof(buff));
	*data = buff[SYSCON_RX_DATA] | ((uint16_t)buff[SYSCON_RX_DATA + 1] << 8);
}

void syscon_get_touchpanel_unk_info_back(uint16_t *data)
{
	uint8_t buff[SYSCON_RX_HEADER_SIZE + 2];
	syscon_command_read(0x3b7, buff, sizeof(buff));
	*data = buff[SYSCON_RX_DATA] | ((uint16_t)buff[SYSCON_RX_DATA + 1] << 8);
}

void syscon_touch_set_sampling_cycle(int cycles_front, int cycles_back)
{
	uint8_t buffer[SYSCON_TX_HEADER_SIZE + 8];
	uint8_t rx[16];

	buffer[0] = 0x87;
	buffer[1] = 3;
	buffer[2] = 9;

	if (cycles_front >= 0) {
		buffer[3] = 1;
		buffer[4] = cycles_front & 0xFF;
	} else {
		buffer[3] = 0;
		buffer[4] = 0;
	}

	buffer[5] = 0;
	buffer[6] = 0;

	if (cycles_back >= 0) {
		buffer[7] = 1;
		buffer[8] = cycles_back & 0xFF;
	} else {
		buffer[7] = 0;
		buffer[8] = 0;
	}

	buffer[9] = 0;
	buffer[10] = 0;

	syscon_transfer(buffer, sizeof(buffer), rx, sizeof(rx));
}
