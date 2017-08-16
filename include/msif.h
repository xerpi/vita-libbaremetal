#ifndef MSIF_H
#define MSIF_H

struct msif_init2_arg {
	unsigned int unk00;
	unsigned int unk04;
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
void msif_init1(void);
void msif_init2(struct msif_init2_arg *arg);

#endif
