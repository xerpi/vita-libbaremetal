#include "dsi.h"
#include "pervasive.h"
#include "libc.h"
#include "utils.h"

#define DSI0_BASE_ADDR			0xE5050000
#define DSI1_BASE_ADDR			0xE5060000

#define DSI_REGS(i)			((void *)((i) == 0 ? DSI0_BASE_ADDR : DSI1_BASE_ADDR))

struct dsi_timing_subsubinfo {
	unsigned int unk00;
	unsigned int unk04;
	unsigned int unk08;
	unsigned int unk0C;
	unsigned int unk10;
};

struct dsi_timing_subinfo {
	unsigned int unk00;
	unsigned int unk04;
	unsigned int unk08;
	unsigned int unk0C;
	struct dsi_timing_subsubinfo subsubinfo;
};

struct dsi_timing_info {
	unsigned int flags; // bit 3 = enable CRC?
	unsigned int pixelclock_24bpp;
	const struct dsi_timing_subinfo *subinfo_24bpp;
	unsigned int pixelclock_30bpp;
	const struct dsi_timing_subinfo *subinfo_30bpp;
	unsigned int htotal; // horizontal line time
	unsigned int vtotal; // vertical line time
	unsigned int mode; // 1 = interlaced, 0 = progressive
	unsigned int HFP;
	unsigned int HSW;
	unsigned int HBP;
	unsigned int VFP;
	unsigned int VSW;
	unsigned int VBP;
};

static struct dsi_timing_subinfo stru_A19358;

/* subinfo_24bpp */
static const struct dsi_timing_subinfo stru_BD0F08 = {0x2BCF, 6, 0xF05,  1, {0x806, 0x4906, 0x16, 3, 0x20}};
static const struct dsi_timing_subinfo stru_BD0FA0 = {0x2BCF, 6, 0x1007, 1, {0x808, 0x4906, 0x16, 3, 0x20}};
static const struct dsi_timing_subinfo stru_BD0C5C = {0x33E0, 7, 0x150A, 2, {0x40B, 0x40CA, 8,    4, 0x20}};
static const struct dsi_timing_subinfo stru_BD0E04 = {0x1519, 3, 0x703,  3, {3,     0x34BD, 7,    0, 0x20}};
static const struct dsi_timing_subinfo stru_BD0C00 = {0x39F5, 7, 0x1507, 5, {0x507, 0x4871, 0xB,  0, 0x20}};
static const struct dsi_timing_subinfo stru_BD0B34 = {0x3A03, 7, 0x1507, 5, {0x507, 0x4884, 0xB,  0, 0x20}};

/* subinfo_30bpp */
static const struct dsi_timing_subinfo stru_BD0AC4 = {0x1A60, 3, 0xB03,  3, {3,     0x41EC, 7,    0, 0x20}};
static const struct dsi_timing_subinfo stru_BD0CF0 = {0x4876, 9, 0x1F09, 5, {0x709, 0x4871, 0xB,  1, 0x20}};
static const struct dsi_timing_subinfo stru_BD0D4C = {0x488A, 9, 0x1F09, 5, {0x709, 0x4890, 0xB,  1, 0x20}};

/*                                              flags pixelclk24 subinfo_24bpp pixelclk30 subinfo_30bpp hline  vline mode HFP   HSW   HBP   VFP  VSW VBP */
static const struct dsi_timing_info stru_BD0D14 = {5,   0x223A1C, &stru_A19358, 0,        0,            0x41A, 0x252, 0, 0x14,  0x42,  4,    4,   4, 0x2A};
static const struct dsi_timing_info stru_BD0BC8 = {5,   0x223A1C, &stru_A19358, 0,        0,            0x4E2, 0x252, 0, 0x14,  0x10A, 4,    4,   4, 0x2E};
static const struct dsi_timing_info stru_BD0ED0 = {5,   0x223A1C, &stru_A19358, 0,        0,            0x55F, 0x237, 0, 0x14,  0x187, 4,    4,   4, 0xF};
static const struct dsi_timing_info stru_BD0E60 = {0xF, 0x287CF3, &stru_BD0C5C, 0,        0,            0x594, 0x307, 0, 0x72,  8,     0x18, 4,   4, 0x2F};
static const struct dsi_timing_info stru_BD0E98 = {1,   0x107AC0, &stru_BD0E04, 0x149970, &stru_BD0AC4, 0x35A, 0x20D, 0, 0x10,  0x3E,  0x3C, 9,   6, 0x1E}; // VIC 2/3
static const struct dsi_timing_info stru_BD0E28 = {1,   0x107AC0, &stru_BD0E04, 0x149970, &stru_BD0AC4, 0x35A, 0x20E, 0, 0x10,  0x3E,  0x3C, 0xA, 6, 0x1E};
static const struct dsi_timing_info stru_BD0DCC = {1,   0x107AC0, &stru_BD0E04, 0x149970, &stru_BD0AC4, 0x360, 0x271, 0, 0xC,   0x40,  0x44, 5,   5, 0x27};
static const struct dsi_timing_info stru_BD0B90 = {1,   0x107AC0, &stru_BD0E04, 0x149970, &stru_BD0AC4, 0x360, 0x272, 0, 0xC,   0x40,  0x44, 6,   5, 0x28};
static const struct dsi_timing_info stru_BD0CB8 = {0,   0x2D45F9, &stru_BD0C00, 0x389777, &stru_BD0CF0, 0x898, 0x465, 1, 0x58,  0x2C,  0x94, 2,   5, 0x10};
static const struct dsi_timing_info stru_BD0B58 = {0,   0x2D5190, &stru_BD0B34, 0x38A5F4, &stru_BD0D4C, 0xA50, 0x465, 1, 0x210, 0x2C,  0x94, 2,   5, 0x10};
static const struct dsi_timing_info stru_BD0F68 = {0,   0x2D45F9, &stru_BD0C00, 0x389777, &stru_BD0CF0, 0x672, 0x2EE, 0, 0x6E,  0x28,  0xDC, 5,   5, 0x14};
static const struct dsi_timing_info stru_BD0D70 = {0,   0x2D5190, &stru_BD0B34, 0x38A5F4, &stru_BD0D4C, 0x7BC, 0x2EE, 0, 0x1B8, 0x28,  0xDC, 5,   5, 0x14};
static const struct dsi_timing_info stru_BD0C80 = {0,   0x2D45F9, &stru_BD0C00, 0x389777, &stru_BD0CF0, 0x898, 0x465, 0, 0x58,  0x2C,  0x94, 4,   5, 0x24};
static const struct dsi_timing_info stru_BD0A8C = {0,   0x2D5190, &stru_BD0B34, 0x38A5F4, &stru_BD0D4C, 0xA50, 0x465, 0, 0x210, 0x2C,  0x94, 4,   5, 0x24};
static const struct dsi_timing_info stru_BD0AE8 = {0,   0x2D45F9, &stru_BD0C00, 0x389777, &stru_BD0CF0, 0xABE, 0x465, 0, 0x27E, 0x2C,  0x94, 4,   5, 0x24};

static struct {
	unsigned int value;
	const struct dsi_timing_info *timing_info;
} dsi_timing_info_lookup[] = {
	{0, &stru_BD0D14},
	{0x80, &stru_BD0BC8},
	{0x20, &stru_BD0ED0},
	{0x8900, &stru_BD0E60},
	{0x8300, &stru_BD0E98},
	{0x8370, &stru_BD0E28},
	{0x8480, &stru_BD0DCC},
	{0x84F0, &stru_BD0B90},
	{0x8500, &stru_BD0CB8},
	{0x8580, &stru_BD0B58},
	{0x8600, &stru_BD0F68},
	{0x8680, &stru_BD0D70},
	{0x8710, &stru_BD0C80},
	{0x8790, &stru_BD0A8C},
	{0x8730, &stru_BD0AE8},
};

void dsi_init(void)
{
	if ((pervasive_read_misc(0x0000) & 0x1FF00) > 0xFF)
		memcpy(&stru_A19358, &stru_BD0FA0, sizeof(stru_A19358));
	else
		memcpy(&stru_A19358, &stru_BD0F08, sizeof(stru_A19358));
}

void dsi_enable_bus(int bus)
{
	static const int pixel_size = 24;
	static const int lanes = 3;
	static const int unk07 = 2;

	unsigned int packet[40];
	unsigned int packet_size;
	const struct dsi_timing_subinfo *subinfo;
	const struct dsi_timing_info *timing_info = dsi_timing_info_lookup[10].timing_info; // 0x8600
	volatile unsigned int *dsi_regs = DSI_REGS(bus);

	pervasive_dsi_misc_unk(bus);

	dsi_regs[0x15] = 0;
	dsi_regs[0x146] = 1;

	if ((pervasive_read_misc(0x0000) & 0x1FF00) > 0xFF) {
		dsi_regs[0x240] = (dsi_regs[0x240] & 0xFFFFFFFC) | 2;
		dsi_regs[0x241] = (dsi_regs[0x241] & 0xFFFFFFFC) | 2;
		dsi_regs[0x242] = (dsi_regs[0x242] & 0xFFFFFFFC) | 2;
		if (lanes == 3)
			dsi_regs[0x243] = (dsi_regs[0x243] & 0xFFFFFFFC) | 2;
	}

	dsi_regs[0x250] = 0x200;
	dsi_regs[0x251] = 0x200;
	dsi_regs[0x252] = 0x200;

	if (lanes == 3)
		dsi_regs[0x253] = 0x200;

	if (pixel_size == 24)
		subinfo = timing_info->subinfo_24bpp;
	else
		subinfo = timing_info->subinfo_30bpp;

	if (subinfo) {
		dsi_regs[0x204] = subinfo->unk00;
		dsi_regs[0x205] = subinfo->unk04;
		dsi_regs[0x206] = subinfo->unk08;
		dsi_regs[0x207] = subinfo->unk0C;
		dsi_regs[0x208] = subinfo->subsubinfo.unk00;
		dsi_regs[0x209] = subinfo->subsubinfo.unk04;
		dsi_regs[0x20A] = subinfo->subsubinfo.unk08;
		dsi_regs[0x20B] = subinfo->subsubinfo.unk0C;
		dsi_regs[0x20C] = subinfo->subsubinfo.unk10;
		dsi_regs[0x20F] = subinfo->unk04 | (subinfo->unk04 << 0x10);
	}

	unsigned int flags = timing_info->flags;
	unsigned int HFP = timing_info->HFP;
	unsigned int HBP = timing_info->HBP;
	unsigned int HSW = timing_info->HSW;
	unsigned int VFP = timing_info->VFP;
	unsigned int VSW = timing_info->VSW;
	unsigned int VBP = timing_info->VBP;

	if (bus == 1) {
		dsi_regs[0x20D] = 0xF;
		dsi_regs[0x20E] = 0;
		dsi_regs[0x201] = 1;
		dsi_regs[0x145] = 0xFFFFFFFF;

		if (unk07 == 1) {
			unsigned int v81 = 0xA30000A2;
			unsigned int v82 = timing_info->htotal - (timing_info->HBP + timing_info->HSW) - timing_info->HFP;

			if (((timing_info->flags << 0x1C) & 0x80000000) != 0)
				v82 -= 2;

			unsigned int v86 = timing_info->vtotal - 2 - VSW - VBP - VFP;
			unsigned int v89 = (0x300 * v82) & 0xFFFFFF;

			if (lanes == 3)
				v81 = 0xA30000A4;

			packet[1] = 0x40EFFFF;
			packet[2] = v81;
			packet[3] = 0x28000004;
			packet[4] = 0x10000001;
			packet[5] = 0x28000001;
			packet[6] = 0x10000021;
			packet[7] = (VBP + VSW - 3) | 0x4010000;
			packet[8] = 0x28000001;
			packet[9] = 0x10000021;
			packet[0xA] = 0x28000001;
			packet[0xB] = 0x10000021;
			packet[0xC] = 0x40005619;
			packet[0xD] = v89 | 0x4000003E;
			packet[0xE] = 0x40015019;
			packet[0xF] = (v86 | 0x4000000) | 0x30000;
			packet[0x10] = 0x10000021;
			packet[0x11] = 0x40005619;
			packet[0x12] = v89 | 0x4000003E;
			packet[0x13] = 0x40015019;
			packet[0x14] = 0x10000021;
			packet[0x15] = (VFP - 2) | 0x4010000;
			packet[0x16] = 0x28000001;
			packet[0x17] = 0x10000021;
			packet[0x18] = 0xC000000;

			packet_size = 0x18;
		} else {
			if (timing_info->mode) {
				unsigned int v61 = (unsigned int)(timing_info->vtotal + 1 - 2 * (VSW + timing_info->VFP + VBP)) >> 1;
				unsigned int v62 = timing_info->htotal - (HSW + HBP) - timing_info->HFP;
				unsigned int v63 = timing_info->flags & 2;
				unsigned int v64 = timing_info->flags & 4;
				unsigned int v65;
				unsigned int v70;
				unsigned int v98;

				if (v63)
					v63 = pixel_size * HSW;
				if (timing_info->flags & 2)
					v63 = ((v63 >> 3) - 0xA) | 0x80000000;

				if (v64)
					v64 = pixel_size * HBP;

				if (timing_info->flags & 4)
					v64 = ((v64 >> 3) - 0xA) | 0x80000000;

				if (lanes == 3)
					v65 = 0xA30000A4;
				else
					v65 = 0xA30000A2;
				packet[2] = v65;
				packet[1] = 0x415FFFF;
				packet[3] = 0x28800005;
				packet[4] = 0x10000081;
				if (v63) {
					v98 = (v61 - 1) | 0x4050000;
					packet[0x15] = (v61 - 1) | 0x4050000;
					packet[5] = ((v63 << 8) & 0xFFFFFF) | 0x40000099;
					packet[0xA] = ((v63 << 8) & 0xFFFFFF) | 0x40000099;
					packet[0xE] = ((v63 << 8) & 0xFFFFFF) | 0x40000099;
					packet[0x13] = ((v63 << 8) & 0xFFFFFF) | 0x40000099;
					packet[0x18] = ((v63 << 8) & 0xFFFFFF) | 0x40000099;
					packet[7] = (VSW - 2) | 0x4030000;
					packet[6] = 0x100000B1;
					packet[0xB] = 0x100000B1;
					packet[0xF] = 0x100000B1;
					packet[0x14] = 0x100000B1;
					packet[0x10] = (VBP - 3) | 0x4030000;
					packet[8] = 0x28000001;
					packet[0x11] = 0x28000001;
					packet[0x16] = 0x28000001;
					packet[9] = 0x100000A1;
					packet[0x12] = 0x100000A1;
					packet[0x17] = 0x100000A1;
					packet[0xC] = 0x28000008;
					packet[0xD] = 0x10000091;
				} else {
					v98 = (v61 - 1) | 0x4050000;
					packet[0x15] = (v61 - 1) | 0x4050000;
					packet[0x10] = (VBP - 3) | 0x4030000;
					packet[5] = 0x28000002;
					packet[7] = (VSW - 2) | 0x4030000;
					packet[0xA] = 0x28000002;
					packet[0xE] = 0x28000002;
					packet[0x13] = 0x28000002;
					packet[0x18] = 0x28000002;
					packet[6] = 0x100000B1;
					packet[0xB] = 0x100000B1;
					packet[0xF] = 0x100000B1;
					packet[0x14] = 0x100000B1;
					packet[8] = 0x28000001;
					packet[0x11] = 0x28000001;
					packet[0x16] = 0x28000001;
					packet[9] = 0x100000A1;
					packet[0x12] = 0x100000A1;
					packet[0x17] = 0x100000A1;
					packet[0xC] = 0x28000008;
					packet[0xD] = 0x10000091;
				}
				packet[0x19] = 0x100000B1;

				unsigned int v66 = 0x28000010;
				if (v64)
					v66 = ((v64 << 8) & 0xFFFFFF) | 0x40000099;
				packet[0x1A] = v66;

				unsigned int v67;
				if (pixel_size == 0x18)
					v67 = ((0x300 * v62) & 0xFFFFFF) | 0x400000BE;
				else
					v67 = ((0x3C0 * v62) & 0xFFFF00) | 0x400000A9;

				packet[0x1B] = v67;
				packet[0x1C] = VFP | 0x4030000;
				packet[0x1D] = 0x28000001;
				packet[0x1E] = 0x100000A1;

				if (v63) {
					packet[0x24] = 0x28000001;
					packet[0x25] = 0x100000A1;
					packet[0x2B] = 0x28000001;
					packet[0x2C] = 0x100000A1;
					packet[0x23] = (VSW - 1) | 0x4030000;
					packet[0x2A] = (VBP - 2) | 0x4030000;
					packet[0x2F] = v98;
					packet[0x30] = 0x28000001;
					packet[0x31] = 0x100000A1;
					packet[0x1F] = ((v63 << 8) & 0xFFFFFF) | 0x40000099;
					packet[0x26] = ((v63 << 8) & 0xFFFFFF) | 0x40000099;
					packet[0x2D] = ((v63 << 8) & 0xFFFFFF) | 0x40000099;
					packet[0x32] = ((v63 << 8) & 0xFFFFFF) | 0x40000099;
					packet[0x20] = 0x100000B1;
					packet[0x27] = 0x100000B1;
					packet[0x2E] = 0x100000B1;
					packet[0x21] = 0x28000004;
					packet[0x22] = 0x100000C1;
					packet[0x28] = 0x28000008;
					packet[0x29] = 0x100000D1;
				} else {
					packet[0x24] = 0x28000001;
					packet[0x25] = 0x100000A1;
					packet[0x2B] = 0x28000001;
					packet[0x2C] = 0x100000A1;
					packet[0x23] = (VSW - 1) | 0x4030000;
					packet[0x2A] = (VBP - 2) | 0x4030000;
					packet[0x2F] = v98;
					packet[0x30] = 0x28000001;
					packet[0x31] = 0x100000A1;
					packet[0x1F] = 0x28000002;
					packet[0x26] = 0x28000002;
					packet[0x2D] = 0x28000002;
					packet[0x32] = 0x28000002;
					packet[0x20] = 0x100000B1;
					packet[0x27] = 0x100000B1;
					packet[0x2E] = 0x100000B1;
					packet[0x21] = 0x28000004;
					packet[0x22] = 0x100000C1;
					packet[0x28] = 0x28000008;
					packet[0x29] = 0x100000D1;
				}

				packet[0x33] = 0x100000B1;

				unsigned int v69 = 0x28000010;
				if (v64)
					v69 = ((v64 << 8) & 0xFFFFFF) | 0x40000099;

				packet[0x34] = v69;

				if (pixel_size == 0x18)
					v70 = ((0x300 * v62) & 0xFFFFFF) | 0x400000BE;
				else
					v70 = ((0x3C0 * v62) & 0xFFFF00) | 0x400000A9;

				packet[0x35] = v70;
				packet[0x36] = (VFP - 1) | 0x4030000;
				packet[0x37] = 0x28000001;
				packet[0x38] = 0x100000A1;

				unsigned int v72 = 0x28000002;
				if (v63)
					v72 = ((v63 << 8) & 0xFFFFFF) | 0x40000099;

				packet[0x39] = v72;
				packet[0x3A] = 0x100000B1;
				packet[0x3B] = 0xC000000;

				//packet_size = &packet[0x3C] - &packet[1];
				packet_size = 0xEC;
			} else {
				char *v57;
				unsigned int v38 = timing_info->HFP;
				unsigned int v40 = timing_info->vtotal - (VFP + timing_info->VBP);
				unsigned int v41 = VSW;
				unsigned int v42 = timing_info->HSW;
				unsigned int v44 = timing_info->htotal - (HBP + HFP) - HSW;
				unsigned int v45 = flags & 1;
				unsigned int v46 = 0x40DFFFF;

				packet[0] = v40 - VSW;

				if (flags & 1)
					v41 = pixel_size;
				if (flags & 1)
					v38 *= v41;
				if (flags & 1)
					v45 = ((v38 >> 3) - 0xC) | 0x80000000;

				unsigned int v47 = flags & 2;

				if (flags & 2)
					v38 = pixel_size;
				else
					v42 = 0;

				if (flags & 2)
					v42 *= v38;
				if (flags & 2)
					v42 = ((v42 >> 3) - 0xA) | 0x80000000;

				unsigned int v49 = flags & 4;
				unsigned int v48 = (v49 == 0);

				unsigned int v97;

				if (v49)
					v47 = pixel_size;
				else
					v97 = 0;

				unsigned int v50 = 0xA30000A2;
				if (lanes == 3)
					v50 = 0xA30000A4;

				if (!v48)
					v47 *= HBP;
				if (!v48)
					v97 = ((v47 >> 3) - 0xA) | 0x80000000;

				if (v45)
					v46 = 0x417FFFF;

				packet[1] = v46;
				packet[2] = v50;
				packet[3] = 0x28000004;
				packet[4] = 0x10000001;
				if (v42) {
					packet[5] = ((v42 << 8) & 0xFFFFFF) | 0x40000019;
					packet[7] = (VSW - 2) | 0x4030000;
					packet[0x10] = (VBP - 2) | 0x4030000;
					packet[0xA] = ((v42 << 8) & 0xFFFFFF) | 0x40000019;
					packet[0xE] = ((v42 << 8) & 0xFFFFFF) | 0x40000019;
					packet[0x13] = ((v42 << 8) & 0xFFFFFF) | 0x40000019;
					packet[6] = 0x10000031;
					packet[0xB] = 0x10000031;
					packet[0xF] = 0x10000031;
					packet[8] = 0x28000001;
					packet[0x11] = 0x28000001;
					packet[9] = 0x10000021;
					packet[0x12] = 0x10000021;
					packet[0xC] = 0x28000008;
					packet[0xD] = 0x10000011;
				} else {
					packet[5] = 0x28000002;
					packet[0xA] = 0x28000002;
					packet[0xE] = 0x28000002;
					packet[7] = (VSW - 2) | 0x4030000;
					packet[0x10] = (VBP - 2) | 0x4030000;
					packet[0x13] = 0x28000002;
					packet[6] = 0x10000031;
					packet[0xB] = 0x10000031;
					packet[0xF] = 0x10000031;
					packet[8] = 0x28000001;
					packet[0x11] = 0x28000001;
					packet[9] = 0x10000021;
					packet[0x12] = 0x10000021;
					packet[0xC] = 0x28000008;
					packet[0xD] = 0x10000011;
				}
				packet[0x14] = 0x10000031;
				if (v45) {
					packet[0x15] = 0x28000001;
					packet[0x16] = 0x10000021;

					unsigned int v75 = 0x28000002;
					if (v42)
						v75 = ((v42 << 8) & 0xFFFFFF) | 0x40000019;
					packet[0x17] = v75;

					packet[0x18] = 0x10000031;

					unsigned int v76 = 0x28000010;
					if (v97)
						v76 = ((v97 << 8) & 0xFFFFFF) | 0x40000019;
					packet[0x19] = v76;

					unsigned int v77;
					if (pixel_size == 0x18)
						v77 = ((0x300 * v44) & 0xFFFFFF) | 0x4000003E;
					else
						v77 = ((0x3C0 * v44) & 0xFFFF00) | 0x40000029;
					packet[0x1A] = v77;

					packet[0x1B] = ((v45 << 8) & 0xFFFFFF) | 0x40000019;
					packet[0x1C] = ((packet[0] & 0xFFFF) - 2) | 0x4050000;
					packet[0x1D] = 0x10000021;

					unsigned int  v78 = 0x28000002;
					if (v42)
						v78 = ((v42 << 8) & 0xFFFFFF) | 0x40000019;
					packet[0x1E] = v78;

					packet[0x1F] = 0x10000031;

					unsigned int v79;
					if (v97)
						v79 = ((v97 << 8) & 0xFFFFFF) | 0x40000019;
					else
						v79 = 0x28000010;
					packet[0x20] = v79;

					unsigned int v80;
					if (pixel_size == 0x18)
						v80 = ((0x300 * v44) & 0xFFFFFF) | 0x4000003E;
					else
						v80 = ((0x3C0 * v44) & 0xFFFF00) | 0x40000029;
					packet[0x21] = v80;

					packet[0x22] = ((v45 << 8) & 0xFFFFFF) | 0x40000019;
					packet[0x23] = 0x10000021;

					if (v42) {
						packet[0x28] = 0x10000021;
						packet[0x24] = ((v42 << 8) & 0xFFFFFF) | 0x40000019;
						packet[0x29] = ((v42 << 8) & 0xFFFFFF) | 0x40000019;
						packet[0x25] = 0x10000031;
						packet[0x26] = (VFP - 2) | 0x4030000;
						packet[0x27] = 0x28000001;
					} else {
						packet[0x28] = 0x10000021;
						packet[0x24] = 0x28000002;
						packet[0x29] = 0x28000002;
						packet[0x25] = 0x10000031;
						packet[0x26] = (VFP - 2) | 0x4030000;
						packet[0x27] = 0x28000001;
					}
					packet[0x2A] = 0x10000031;

					v57 = (char *)&packet[0x2B];
				} else {
					packet[0x15] = ((packet[0] & 0xFFFF) - 1) | 0x4050000;
					packet[0x16] = 0x28000001;
					packet[0x17] = 0x10000021;

					unsigned int v53 = 0x28000002;
					if (v42)
						v53 = ((v42 << 8) & 0xFFFFFF) | 0x40000019;
					packet[0x18] = v53;

					packet[0x19] = 0x10000031;

					unsigned int v54 = 0x28000010;
					if (v97)
						v54 = ((v97 << 8) & 0xFFFFFF) | 0x40000019;
					packet[0x1A] = v54;

					unsigned int v55;
					if (pixel_size == 0x18)
						v55 = ((0x300 * v44) & 0xFFFFFF) | 0x4000003E;
					else
						v55 = ((0x3C0 * v44) & 0xFFFF00) | 0x40000029;
					packet[0x1B] = v55;

					packet[0x1C] = (VFP - 1) | 0x4030000;
					packet[0x1D] = 0x28000001;
					packet[0x1E] = 0x10000021;

					unsigned int v56 = 0x28000002;
					if (v42)
						v56 = ((v42 << 8) & 0xFFFFFF) | 0x40000019;
					packet[0x1F] = v56;

					packet[0x20] = 0x10000031;

					v57 = (char *)&packet[0x21];
				}
				*(unsigned int *)v57 = 0xC000000;
				packet_size = (v57 + 4 - (char *)&packet[1]) >> 2;
			}
			if (packet_size <= 0)
				goto LABEL_18;
		}
	} else {
		unsigned int v32;

		unsigned int htotal = timing_info->htotal;
		unsigned int horizontal_porches = timing_info->HFP + timing_info->HSW;
		unsigned int vertical_porches = VSW + VBP;
		unsigned int vtotal = timing_info->vtotal;
		unsigned int horizontal_pixels = htotal - horizontal_porches;

		if (unk07)
			v32 = 0x400B7819;
		else
			v32 = 0x400B4019;

		dsi_regs[0x20D] = 7;
		dsi_regs[0x20E] = 0;
		dsi_regs[0x201] = 1;
		dsi_regs[0x145] = 0xFFFFFFFF;

		packet[1] = 0x407FFFF;
		packet[2] = 0xA3000082;
		packet[3] = 0x28000004;
		packet[4] = 0x10000001;
		packet[5] = v32;
		packet[6] = (vertical_porches - 2) | 0x4020000;
		packet[7] = 0x28000001;
		packet[8] = 0x10000021;
		packet[9] = v32;
		packet[0xA] = ((vtotal - 1) - VSW - VBP - VFP) | 0x4030000;
		packet[0xB] = 0x28000001;
		packet[0xC] = 0x10000021;
		packet[0xD] = ((0x300 * HBP - 0xA00) & 0xFFFFFF) | 0x40000019;
		packet[0xE] = ((0x300 * (horizontal_pixels - HBP)) & 0xFFFFFF) | 0x4000003E;
		packet[0xF] = (VFP - 1) | 0x4020000;
		packet[0x10] = 0x28000001;
		packet[0x11] = 0x10000021;
		packet[0x12] = v32;
		packet[0x13] = 0xC000000;

		packet_size = 0x13;
	}

	unsigned int *packer_ptr = packet;
	unsigned int i = 0;
	do {
		unsigned int data = packer_ptr[1];
		packer_ptr++;
		i++;
		dsi_regs[0x140] = data;
	} while (i < packet_size);

LABEL_18:
	dsi_regs[0x147] = 1;
}

void dsi_unk(int bus, int unk)
{
	static const unsigned int bus_index = 1;
	static const unsigned int lanes = 3;
	static const unsigned int pixel_size = 24;
	static const unsigned int intr_mask = 0;

	static const unsigned int lookup[] = {2, 2, 3, 4};

	const struct dsi_timing_info *timing_info = dsi_timing_info_lookup[10].timing_info; // 0x8600
	volatile unsigned int *dsi_regs = DSI_REGS(bus);

	unsigned int flags = timing_info->flags;
	unsigned int htotal = timing_info->htotal;
	unsigned int vtotal = timing_info->vtotal;
	unsigned int HFP = timing_info->HFP;
	unsigned int HBP = timing_info->HBP;
	unsigned int HSW = timing_info->HSW;
	unsigned int VFP = timing_info->VFP;
	unsigned int VSW = timing_info->VSW;
	unsigned int VBP = timing_info->VBP;
	unsigned int mode = timing_info->mode;

	unsigned int hsync_end = HBP + HSW;
	unsigned int hact = htotal - (HBP + HSW) - HFP;
	if (flags & 3)
		hact -= 2;

	if (mode == 1)
		dsi_regs[1] = 3;
	else
		dsi_regs[1] = 0;

	unsigned int v15;
	if (lanes == 2)
		v15 = 6;
	else if (pixel_size == 24)
		v15 = 4;
	else
		v15 = 5;

	unsigned int HFP_start = hact + hsync_end;
	unsigned int HSW_clocks = HSW * v15;
	unsigned int htotal_clocks = htotal * v15 >> 1;
	dsi_regs[2] = htotal_clocks;

	unsigned int v20;
	unsigned int v21;

	if (mode == 1) { // Interlaced
		unsigned int v36 = HFP_start * v15;
		unsigned int v37 = htotal * v15 >> 2;

		v20 = vtotal + 1;
		v21 = (vtotal + 1) >> 1;

		dsi_regs[3] = ((vtotal << 16) & 0x1FFF0000) | (v21 & 0x1FFF);
		dsi_regs[4] = 0x10001;
		dsi_regs[5] = (((hsync_end * v15) >> 1) << 16) | (((v36 >> 1) + 1) & 0xFFFF);
		dsi_regs[7] = (((VBP + VSW - 1) << 16) & 0x1FFF0000) | ((v21 - VFP) & 0x1FFF);
		dsi_regs[8] = (((VBP + VSW + ((vtotal - 1) >> 1)) << 16) & 0x1FFF0000) | ((vtotal + 1 - VFP) & 0x1FFF);
		dsi_regs[9] = ((HSW_clocks >> 1) << 0x10) | 1;
		dsi_regs[0xB] = 0x10001;
		dsi_regs[0xC] = ((VSW << 16) & 0x1FFF0000) | (htotal_clocks & 0xFFFF);
		dsi_regs[0xD] = ((v21 << 16) & 0x1FFF0000) | (v37 + 1);
		dsi_regs[0xE] = (((VSW + v21) << 16) & 0x1FFF0000) | v37;
	} else { // Progressive
		v20 = vtotal + 1;
		v21 = (vtotal + 1) >> 1;

		dsi_regs[3] = vtotal;
		dsi_regs[4] = 0x10001;
		dsi_regs[5] = (((HBP + HSW) * v15) >> 1 << 16) | ((((HFP_start * v15) >> 1) + 1) & 0xFFFF);
		dsi_regs[7] = (((VBP + VSW) << 16) & 0x1FFF0000) | ((vtotal + 1 - VFP) & 0x1FFF);
		dsi_regs[9] = (HSW_clocks >> 1 << 0x10) | 1;
		dsi_regs[0xB] = 0x10001;
		dsi_regs[0xC] = ((VSW << 16) & 0x1FFF0000) | htotal_clocks;
	}

	if (lanes == 3 && pixel_size == 30) {
		unsigned int v34;
		unsigned int v35;

		if (vtotal == 525) {
			dsi_regs[5] = 0x130083A;
			dsi_regs[6] = 0x131083A;
			dsi_regs[9] = 0x9A0001;
			dsi_regs[0xA] = 0x9B0861;
			v34 = 1088;
			v35 = 4;
		} else if (vtotal == 750) {
			dsi_regs[5] = 0x28A0F0B;
			dsi_regs[6] = 0x2890F0B;
			dsi_regs[9] = 0x640001;
			dsi_regs[0xA] = 0x63101D;
			v34 = 0;
			v35 = 1092;
		}

		dsi_regs[0x1D] = v34;
		dsi_regs[0x1E] = v35;
		dsi_regs[1] |= 0x100;
	}

	dsi_regs[0xF] = 1;
	unsigned int vsync_start = VBP + VSW;
	dsi_regs[0x10] = (((v21 + vsync_start - 8) << 16) & 0x1FFF0000) | ((vsync_start - 8) & 0x1FFF);
	if (mode == 1) {
		dsi_regs[0x17] = ((v21 << 16) & 0x1FFF0000) | 1;
		dsi_regs[0x18] = (((VBP + VSW + ((vtotal - 1) >> 1)) << 16) & 0x1FFF0000) | 1;
		dsi_regs[0x1B] = ((v20 << 0xE) & 0x1FFF0000) | 0x40000001;
		dsi_regs[0x19] = ((vtotal << 16) & 0x1FFF0000) | 1;
		dsi_regs[0x1A] = (((VBP + VSW - 1) << 16) & 0x1FFF0000) | 1;
		dsi_regs[0x1C] = (0xC000 * v20 & 0x1FFF0000) | 0x40000001;
	} else {
		dsi_regs[0x17] = 0x10001;
		dsi_regs[0x18] = (((VBP + VSW) << 16) & 0x1FFF0000) | 1;
		dsi_regs[0x1B] = (((VBP + VSW + (vtotal >> 1)) << 16) & 0x1FFF0000) | 0x40000001;
		dsi_regs[0x1C] = 0;
	}

	unsigned int intr = intr_mask;
	dsi_regs[0x14] = dsi_regs[0x14];
	dsi_regs[0x15] = intr;
	if (bus_index)
		dsi_regs[0x20E] = 1;
	else
		dsi_regs[0x20E] = 0;

	unsigned int v30;
	if (unk == 1)
		v30 = (bus ^ 1) & 1;
	else
		v30 = 0;

	dsi_regs[0x142] = 0xFFFFFFFF;

	unsigned int v31;
	if (v30 || (unk - 1 > 3) || ((unk == 2 ? (v31 = bus & 1) : (v31 = 0)), v31))
		dsi_regs[0] = 1;
	else
		dsi_regs[0] = lookup[unk - 1];

	dmb();
}
