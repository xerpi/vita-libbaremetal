#ifndef MSIF_H
#define MSIF_H

#include <stdint.h>

#define MS_SECTOR_SIZE	512

struct msif_info {
	uint32_t unk00;
	uint32_t write_protected;
	uint32_t unk08;
	uint32_t unk0C;
	uint32_t unk10;
	uint32_t unk14;
	uint32_t unk18;
	uint32_t unk1C;
	uint32_t unk20;
	uint32_t unk24;
	uint32_t unk28;
	uint32_t unk2C;
	uint32_t unk30;
	uint32_t unk34;
};

void msif_init(void);
void msif_setup(const uint8_t key[32]);
void msif_get_info(struct msif_info *info);
void msif_read_sector(uint32_t sector, void *buff);
void msif_read_atrb(uint32_t address, void *buff);
void msif_read_short_data(uint8_t cmd, void *buff, uint32_t size);
void msif_write_short_data(uint8_t cmd, const void *buff, uint32_t size);

#endif
