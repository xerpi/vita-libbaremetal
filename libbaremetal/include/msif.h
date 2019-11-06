#ifndef MSIF_H
#define MSIF_H

#define MS_SECTOR_SIZE	512

struct msif_info {
	unsigned int unk00;
	unsigned int write_protected;
	unsigned int unk08;
	unsigned int unk0C;
	unsigned int unk10;
	unsigned int unk14;
	unsigned int unk18;
	unsigned int unk1C;
	unsigned int unk20;
	unsigned int unk24;
	unsigned int unk28;
	unsigned int unk2C;
	unsigned int unk30;
	unsigned int unk34;
};

void msif_init(void);
void msif_setup(const unsigned char key[32]);
void msif_get_info(struct msif_info *info);
void msif_read_sector(unsigned int sector, void *buff);
void msif_read_atrb(unsigned int address, void *buff);
void msif_read_short_data(unsigned char cmd, void *buff, unsigned int size);
void msif_write_short_data(unsigned char cmd, const void *buff, unsigned int size);

#endif
