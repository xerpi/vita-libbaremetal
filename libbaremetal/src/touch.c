#include <stddef.h>
#include "touch.h"
#include "syscon.h"
#include "libc.h"
#include "utils.h"

#define TOUCH_REPORT_DATA_SIZE      (0x7a - 2)
#define TOUCH_REPORT_PORT_ENTRIES   10

static uint16_t g_front_vendor_id;
static uint16_t g_front_fw_version;
static uint16_t g_front_unk1;
static uint8_t g_front_unk2;
static uint8_t g_front_unk3;
static uint16_t g_back_vendor_id;
static uint16_t g_back_fw_version;
static uint16_t g_back_unk1;
static uint8_t g_back_unk2;
static uint8_t g_back_unk3;
static uint8_t g_front_vendor_id_non_dependant;
static uint8_t g_back_vendor_id_dependant;
static int g_syscon_hw_version_dependant;

static void touch_read_panel_info_1(void)
{
	struct syscon_touchpanel_device_info info;

	syscon_get_touchpanel_device_info(&info);

	g_front_vendor_id = info.front_vendor_id;
	g_front_fw_version = info.front_fw_version;
	g_back_vendor_id = info.back_vendor_id;
	g_back_fw_version = info.back_fw_version;

	if (syscon_get_baryon_version() > 0x90002) {
		uint16_t data;

		syscon_get_touchpanel_unk_info_front(&data);
		g_front_unk1 = (data >> 8) | (data << 8);

		syscon_get_touchpanel_unk_info_back(&data);
		g_back_unk1 = (data >> 8) | (data << 8);
	}
}

static void touch_read_panel_info_2(void)
{
	struct syscon_touchpanel_device_info_ext info;

	syscon_get_touchpanel_device_info_ext(&info);

	g_front_vendor_id = info.front_vendor_id;
	g_front_fw_version = info.front_fw_version;
	g_front_unk1 = info.front_unk1;
	g_front_unk2 = info.front_unk2;
	g_front_unk3 = info.front_unk3;
	g_back_vendor_id = info.back_vendor_id;
	g_back_fw_version = info.back_fw_version;
	g_back_unk1 = info.back_unk1;
	g_back_unk2 = info.back_unk2;
	g_back_unk3 = info.back_unk3;
}

void touch_init(void)
{
	uint32_t baryon_version = syscon_get_baryon_version();

	//syscon_ctrl_device_reset(0xC, 0);
	//delay(1s)
	syscon_ctrl_device_reset(0xC, 1);

	if (baryon_version < 0x1000600) {
		touch_read_panel_info_1();
	} else {
		touch_read_panel_info_2();
	}

	g_front_vendor_id_non_dependant = 0xe0;
	if (g_back_vendor_id == 0x800a)
		g_back_vendor_id_dependant = 0xd0;
	else
		g_back_vendor_id_dependant = 0xb0;

	if (baryon_version > 0x90002)
		syscon_touch_set_sampling_cycle(0, 0);

	if (syscon_get_hardware_info() & 0x10000)
		g_syscon_hw_version_dependant = 0;
	else
		g_syscon_hw_version_dependant = 1;
}

void touch_configure(int port_mask, uint8_t max_report_front, uint8_t max_report_back)
{
	uint8_t buffer[SYSCON_TX_HEADER_SIZE + 4];
	uint8_t rx[16];
	uint8_t front_cfg, back_cfg;

	port_mask &= 3;

	if (port_mask == 0) {
		front_cfg = 0;
		back_cfg = 0;
	} else {
		if ((port_mask == 2) || (port_mask == 3)) {
			front_cfg = 0;
			back_cfg = g_back_vendor_id_dependant;
		} else if (port_mask == 1) {
			front_cfg = g_front_vendor_id_non_dependant;
			back_cfg = 0;
		} else {
			front_cfg = 0;
			back_cfg = 0;
		}
	}

	if (max_report_front > TOUCH_MAX_REPORT_FRONT)
		max_report_front = TOUCH_MAX_REPORT_FRONT;

	if (max_report_back > TOUCH_MAX_REPORT_BACK)
		max_report_back = TOUCH_MAX_REPORT_BACK;

	buffer[SYSCON_TX_CMD_LO] = 0x81;
	buffer[SYSCON_TX_CMD_HI] = 3;
	buffer[SYSCON_TX_LENGTH] = 5;
	buffer[SYSCON_TX_DATA(0)] = port_mask & 1;
	buffer[SYSCON_TX_DATA(1)] = front_cfg | max_report_front;
	buffer[SYSCON_TX_DATA(2)] = (port_mask >> 1) & 1;
	buffer[SYSCON_TX_DATA(3)] = back_cfg | max_report_back;

	syscon_transfer(buffer, sizeof(buffer), rx, sizeof(rx));
}

void touch_set_sampling_cycle(int port_mask, uint8_t cycles_front, uint8_t cycles_back)
{
	uint8_t buffer[SYSCON_TX_HEADER_SIZE + 8];
	uint8_t rx[16];

	buffer[SYSCON_TX_CMD_LO] = 0x87;
	buffer[SYSCON_TX_CMD_HI] = 3;
	buffer[SYSCON_TX_LENGTH] = 9;
	buffer[SYSCON_TX_DATA(0)] = port_mask & 1;
	buffer[SYSCON_TX_DATA(1)] = cycles_front;
	buffer[SYSCON_TX_DATA(2)] = 0;
	buffer[SYSCON_TX_DATA(3)] = 0;
	buffer[SYSCON_TX_DATA(4)] = (port_mask >> 1) & 1;
	buffer[SYSCON_TX_DATA(5)] = cycles_back;
	buffer[SYSCON_TX_DATA(6)] = 0;
	buffer[SYSCON_TX_DATA(7)] = 0;

	syscon_transfer(buffer, sizeof(buffer), rx, sizeof(rx));
}

void touch_read(int port_mask, struct touch_data *data)
{
	static uint8_t buffer[SYSCON_RX_HEADER_SIZE + TOUCH_REPORT_DATA_SIZE];
	uint8_t *raw_data = &buffer[SYSCON_RX_DATA];
	int i, offset;
	uint8_t cmd_lo, id;

	if ((port_mask & 3) == 3)
		cmd_lo = 0;
	else
		cmd_lo = port_mask & 3;

	syscon_command_read(0x300 + cmd_lo, buffer, sizeof(buffer));

	data->num_front = 0;
	if (port_mask & TOUCH_PORT_FRONT) {
		offset = 0;
		for (i = 0; i < TOUCH_MAX_REPORT_FRONT; i++) {
			id = raw_data[offset];
			if (!(id & 0x80)) {
				struct touch_report *report = &data->front[data->num_front++];
				report->id = id;
				report->force = raw_data[offset + 5];
				report->x = raw_data[offset + 1] | (((uint16_t)raw_data[offset + 2] & 7) << 8);
				report->y = raw_data[offset + 3] | (((uint16_t)raw_data[offset + 4] & 7) << 8);
			}
			offset += 6;
		}
	}

	data->num_back = 0;
	if (port_mask & TOUCH_PORT_BACK) {
		offset = TOUCH_REPORT_PORT_ENTRIES * 6;
		for (i = 0; i < TOUCH_MAX_REPORT_BACK; i++) {
			id = raw_data[offset];
			if (!(id & 0x80)) {
				struct touch_report *report = &data->back[data->num_back++];
				report->id = id;
				report->force = raw_data[offset + 5];
				report->x = raw_data[offset + 1] | (((uint16_t)raw_data[offset + 2] & 7) << 8);
				report->y = raw_data[offset + 3] | (((uint16_t)raw_data[offset + 4] & 7) << 8);
			}
			offset += 6;
		}
	}
}
