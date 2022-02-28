#ifndef SYSCON_H
#define SYSCON_H

#include <stdint.h>

#define SYSCON_TX_HEADER_SIZE	3
#define SYSCON_TX_CMD_LO	0
#define SYSCON_TX_CMD_HI	1
#define SYSCON_TX_LENGTH	2
#define SYSCON_TX_DATA(i)	(3 + (i))

#define SYSCON_RX_HEADER_SIZE	4
#define SYSCON_RX_STATUS_LO	0
#define SYSCON_RX_STATUS_HI	1
#define SYSCON_RX_LENGTH	2
#define SYSCON_RX_RESULT	3
#define SYSCON_RX_DATA		4

#define SYSCON_RESET_TYPE_POWEROFF	0
#define SYSCON_RESET_TYPE_SUSPEND	1
#define SYSCON_RESET_TYPE_COLD_RESET	2
#define SYSCON_RESET_TYPE_SOFT_RESET	17

struct syscon_touchpanel_device_info {
	uint16_t front_vendor_id;
	uint16_t front_fw_version;
	uint16_t back_vendor_id;
	uint16_t back_fw_version;
};

struct syscon_touchpanel_device_info_ext {
	uint16_t front_vendor_id;
	uint16_t front_fw_version;
	uint16_t front_unk1;
	uint8_t front_unk2;
	uint8_t front_unk3;
	uint16_t unused1;
	uint16_t back_vendor_id;
	uint16_t back_fw_version;
	uint16_t back_unk1;
	uint8_t back_unk2;
	uint8_t back_unk3;
	uint16_t unused2;
};

int syscon_init(void);
void syscon_transfer(const uint8_t *tx, int tx_size, uint8_t *rx, int max_rx_size);
void syscon_command_read(uint16_t cmd, void *buffer, int max_length);
void syscon_short_command_write(uint16_t cmd, uint32_t data, int length);
int syscon_scratchpad_read(uint16_t offset, void *buffer, int size);
int syscon_scratchpad_write(uint16_t offset, const void *buffer, int size);
int syscon_get_baryon_version(void);
int syscon_get_hardware_info(void);
void syscon_reset_device(int type, int mode);
void syscon_set_hdmi_cdc_hpd(int enable);
void syscon_msif_set_power(int enable);
void syscon_ctrl_device_reset(unsigned int param_1, unsigned int param_2);
void syscon_get_touchpanel_device_info(struct syscon_touchpanel_device_info *info);
void syscon_get_touchpanel_device_info_ext(struct syscon_touchpanel_device_info_ext *info);
void syscon_get_touchpanel_unk_info_front(uint16_t *data);
void syscon_get_touchpanel_unk_info_back(uint16_t *data);
void syscon_touch_set_sampling_cycle(int cycles_front, int cycles_back);

#endif
