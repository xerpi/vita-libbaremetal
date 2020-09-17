#include <stddef.h>
#include "touch.h"
#include "syscon.h"
#include "libc.h"
#include "utils.h"

extern void console_printf(const char *s, ...);
extern void PRINT_BUF(const char *name, const void *buff, int size);
#define PRINT(...) console_printf(__VA_ARGS__)

static unsigned int g_syscon_baryon;
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
static uint8_t g_max_report_front;
static uint8_t g_max_report_back;
static uint8_t g_enabled_port_mask;
static uint8_t g_front_packet_size;
static uint8_t g_enable_front_cmd;
static uint8_t g_back_packet_size;
static uint8_t g_enable_back_cmd;
static uint8_t g_front_sampling;
static uint8_t g_back_sampling;
static uint8_t g_cur_front_cmd;
static uint8_t g_cur_back_cmd;


unsigned char syscon_packet_exec(struct syscon_packet *packet);

static void touch_read_panel_info_1()
{
	struct syscon_touchpanel_device_info info;

	syscon_get_touchpanel_device_info(&info);

	g_front_vendor_id = info.front_vendor_id;
	g_front_fw_version = info.front_fw_version;
	g_back_vendor_id = info.back_vendor_id;
	g_back_fw_version = info.back_fw_version;

	if (g_syscon_baryon > 0x90002) {
		unsigned short data;

		syscon_9B6B8BB9(&data);
		g_front_unk1 = (data >> 8) | (data << 8);

		syscon_C4A61241(&data);
		g_back_unk1 = (data >> 8) | (data << 8);
	}
}

static void touch_read_panel_info_2()
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

void touch_init()
{
	g_syscon_baryon = syscon_get_baryon_version();

	//syscon_ctrl_device_reset(0xC, 0);
	//for (volatile int i = 0; i < 1000000; i++);
	//delay(1s)
	syscon_ctrl_device_reset(0xC, 1);

	if (g_syscon_baryon < 0x1000600) {
		touch_read_panel_info_1();
	} else {
		touch_read_panel_info_2();
	}

	g_front_vendor_id_non_dependant = 0xe0;
	if (g_back_vendor_id == 0x800a)
		g_back_vendor_id_dependant = 0xd0;
	else
		g_back_vendor_id_dependant = 0xb0;

	if (g_syscon_baryon > 0x90002) {
		g_front_sampling = 0;
		g_back_sampling = 0;
		g_cur_front_cmd = 0;
		g_cur_back_cmd = 0;
		syscon_1546A141(0, 0);
	}

	if (syscon_get_hardware_info() & 0x10000)
		g_syscon_hw_version_dependant = 0;
	else
		g_syscon_hw_version_dependant = 1;

	g_max_report_front = 6; // 6;
	g_max_report_back = 4; // 4;

	g_enable_front_cmd = 0xFFu;
	g_enable_back_cmd = 0xFFu;
	g_front_packet_size = 0x10;
	g_back_packet_size = 0x10;

	g_enabled_port_mask = 0;
}

void touch_read1(unsigned char *buffer)
{
#if 0
	struct syscon_packet packet;
	memset(&packet, 0, sizeof(packet));

	packet.tx[0] = 0x81;
	packet.tx[1] = 3;
	packet.tx[2] = 5;
	packet.tx[3] = bVar7 & 1;
	packet.tx[4] = bVar7 | DAT_810073dc;
	packet.tx[5] = (unsigned char )((uint)((int)pvVar6 << 30) >> 31);
	packet.tx[6] = bVar7 | DAT_810074d0;
	if (pvVar6 != 0) {
		uVar5 = (uint)pvVar6 & 3;
		if ((uVar5 == 2) || (uVar5 == 3)) {
			packet.tx[4] = 0;
			packet.tx[6] = g_back_vendor_id_dependant;
		} else {
			if (uVar5 == 1) {
				packet.tx[4] = DAT_810075d0;
				packet.tx[6] = 0;
			}
			else {
				packet.tx[4] = 0;
				packet.tx[6] = 0;
			}
		}
	}
	_DAT_81007
	238 = (uint)pvVar6 | 0x80000000;
	pvVar8 = (void *)0x2;
	some_state_2 = 1;
	DAT_81007346 = 0;

	packet.tx[5] = (byte)((uint)((int)pvVar6 << 0x1e) >> 0x1f);
#endif
}

// g_front_packet_pending == 0
// g_touch_status != 2
// (g_touch_status & 0xfffffffd) == 1) || (g_touch_status == 5)
void touch_enable1(void)
{
	struct syscon_packet packet;
	memset(&packet, 0, sizeof(packet));
	packet.tx[0] = 0x81;
	packet.tx[1] = 3;
	packet.tx[2] = 5;
	packet.tx[3] = 0;
	packet.tx[4] = g_max_report_front;
	packet.tx[5] = 0;
	packet.tx[6] = g_max_report_back;

	syscon_packet_exec(&packet);
}

// g_front_packet_pending == 0
// g_touch_status == 2
// g_enabled_port_mask != required_port_mask
void touch_enable2(void)
{
	struct syscon_packet packet;
	memset(&packet, 0, sizeof(packet));

	uint8_t required_port_mask = 3;
	uint8_t tx4, tx6;

	if (required_port_mask == 0) {
		tx4 = 0;
		tx6 = 0;
	} else {
		if ((required_port_mask == 2) || (required_port_mask == 3)) {
			tx4 = 0;
			tx6 = g_back_vendor_id_dependant;
		} else if (required_port_mask == 1) {
			tx4 = g_front_vendor_id_non_dependant;
			tx6 = 0;
		} else {
			tx4 = 0;
			tx6 = 0;
		}
	}

	packet.tx[0] = 0x81;
	packet.tx[1] = 3;
	packet.tx[2] = 5;
	packet.tx[3] = required_port_mask & 1;
	packet.tx[4] = tx4 | g_max_report_front;
	packet.tx[5] = (required_port_mask >> 1) & 1;
	packet.tx[6] = tx6 | g_max_report_back;

	syscon_packet_exec(&packet);

	//PRINT_BUF("enable2:", packet.rx, 16);
}

// g_front_packet_pending == 0
// g_touch_status == 2
// g_enabled_port_mask == required_port_mask
void touch_enable3(void)
{
	struct syscon_packet packet;
	memset(&packet, 0, sizeof(packet));

	uint8_t required_port_mask = 3;

	uint8_t byte_1D3C3EA = 0;
	uint8_t byte_1D3C4DE = 0;

	uint8_t v32, v30;
	uint8_t tmp1, tmp2;
	uint8_t front_set_status, back_set_status;
	int change_front, change_back;

	uint8_t cur_enabled_port_mask = g_enabled_port_mask & 3;

	if (g_enabled_port_mask == required_port_mask) {
		if (g_syscon_baryon <= 0x90002) {
			cur_enabled_port_mask = required_port_mask;
		} else {
			if (g_enabled_port_mask & 1 && (v32 = byte_1D3C3EA, byte_1D3C3EA = 0, v32)) {
				if (v32 == 2)
					tmp1 = g_front_packet_size;
				else if (v32 == 3)
					tmp1 = 0;
				else
					tmp1 = g_enable_front_cmd;
				front_set_status = tmp1;
				g_cur_front_cmd = tmp1;
			} else {
				front_set_status = g_cur_front_cmd;
			}
			if (g_enabled_port_mask & 2 && (v30 = byte_1D3C4DE, byte_1D3C4DE = 0, v30)) {
				if (v30 == 2)
					tmp2 = g_back_packet_size;
				else if (v30 == 3)
					tmp2 = 0;
				else
					tmp2 = g_enable_back_cmd;
				back_set_status = tmp2;
				g_cur_back_cmd = tmp2;
			} else {
				back_set_status = g_cur_back_cmd;
			}

			change_front = g_front_sampling != front_set_status;
			change_back = g_back_sampling != back_set_status;

			if (change_back || change_front) {
				packet.tx[0] = 0x87;
				packet.tx[1] = 3;
				packet.tx[2] = 9;
				packet.tx[3] = change_front;
				packet.tx[4] = front_set_status;
				packet.tx[5] = 0;
				packet.tx[6] = 0;
				packet.tx[7] = change_back;
				packet.tx[8] = back_set_status;
				packet.tx[9] = 0;
				packet.tx[10] = 0;
			}
		}
	}
}

void touch_enable4(void)
{
	struct syscon_packet packet;
	memset(&packet, 0, sizeof(packet));

	packet.tx[0] = 0x87;
	packet.tx[1] = 3;
	packet.tx[2] = 9;
	packet.tx[3] = 1; // enable front
	packet.tx[4] = 0x10;
	packet.tx[5] = 0;
	packet.tx[6] = 0;
	packet.tx[7] = 1; // enable back
	packet.tx[8] = 0x10;
	packet.tx[9] = 0;
	packet.tx[10] = 0;

	syscon_packet_exec(&packet);
}

void touch_read(unsigned char *buffer)
{
	struct syscon_packet packet;
	memset(&packet, 0, sizeof(packet));

	/*uint8_t required_port_mask = 3;

	if (required_port_mask == 3) {
		packet.tx[0] = 0;
	} else {
		if (required_port_mask == 1)
			packet.tx[0] = 1;
		else
			packet.tx[0] = 2;
	}*/
	memset(&packet, 0, sizeof(packet));
	packet.tx[0] = 0;
	packet.tx[1] = 3;
	packet.tx[2] = 1;
	syscon_packet_exec(&packet);
	memcpy(buffer, &packet.rx[SYSCON_RX_DATA], packet.rx[SYSCON_RX_LENGTH]);
}

#if 0
	struct syscon_packet packet2;
	unsigned char uVar6 = 0, bVar3 = 0, DAT_810073e9 = 0;
	unsigned int iVar4 = 0;
	packet2.tx[0] = 0x87;
	packet2.tx[1] = 3;
	packet2.tx[2] = 9;
	packet2.tx[3] = uVar6;
	packet2.tx[4] = DAT_810073e9;
	packet2.tx[5] = 0;
	packet2.tx[6] = 0;
	packet2.tx[7] = (iVar4 >> 0x1f) & 0xFF;
	packet2.tx[8] = bVar3;
	packet2.tx[9] = 0;
	packet2.tx[10] = 0;

	syscon_packet_exec(&packet2);
	//memcpy(buffer, &packet2.rx[4], packet2.rx[SYSCON_RX_LENGTH] - 2);
}
#endif

