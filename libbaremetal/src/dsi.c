#include <stddef.h>
#include "dsi.h"
#include "pervasive.h"
#include "libc.h"
#include "utils.h"

#define DSI_BASE_ADDR	0xE5050000
#define DSI_REGS(i)	((void *)(DSI_BASE_ADDR + (i) * 0x10000))

struct dsi_timing_subsubinfo {
	uint32_t unk00;
	uint32_t unk04;
	uint32_t unk08;
	uint32_t unk0C;
	uint32_t unk10;
};

struct dsi_timing_subinfo {
	uint32_t unk00;
	uint32_t unk04;
	uint32_t unk08;
	uint32_t unk0C;
	struct dsi_timing_subsubinfo subsubinfo;
};

enum dsi_timing_info_flags {
	DSI_TIMING_INFO_POL_POS	= 0 << 0,
	DSI_TIMING_INFO_POL_NEG	= 1 << 0,
};

struct dsi_timing_info {
	uint32_t flags; /* enum dsi_timing_info_flags */
	uint32_t pixelclock_24bpp;
	const struct dsi_timing_subinfo *subinfo_24bpp;
	uint32_t pixelclock_30bpp;
	const struct dsi_timing_subinfo *subinfo_30bpp;
	uint32_t htotal;
	uint32_t vtotal;
	uint32_t mode; // 1 = interlaced, 0 = progressive
	uint32_t HFP;
	uint32_t HSW;
	uint32_t HBP;
	uint32_t VFP;
	uint32_t VSW;
	uint32_t VBP;
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

/*                                               flags pixelclk24 subinfo_24  pixelclk30 subinfo_30  hline vline mode HFP  HSW HBP  VFP VSW VBP */
static const struct dsi_timing_info stru_BD0D14 = {5,   2243100, &stru_A19358, 0,       0,            1050, 594,  0,  20,  66,  4,   4,  4, 42}; /* 960x544p */
static const struct dsi_timing_info stru_BD0BC8 = {5,   2243100, &stru_A19358, 0,       0,            1250, 594,  0,  20,  266, 4,   4,  4, 46}; /* 960x540p */
static const struct dsi_timing_info stru_BD0ED0 = {5,   2243100, &stru_A19358, 0,       0,            1375, 567,  0,  20,  391, 4,   4,  4, 15}; /* 960x544p */
static const struct dsi_timing_info stru_BD0E60 = {0xF, 2653427, &stru_BD0C5C, 0,       0,            1428, 775,  0,  114, 8,   24,  4,  4, 47}; /* 1280x720p */
static const struct dsi_timing_info stru_BD0E98 = {1,   1080000, &stru_BD0E04, 1350000, &stru_BD0AC4, 858,  525,  0,  16,  62,  60,  9,  6, 30}; /* 720x480p @ 60Hz (VIC 2/3) */
static const struct dsi_timing_info stru_BD0E28 = {1,   1080000, &stru_BD0E04, 1350000, &stru_BD0AC4, 858,  526,  0,  16,  62,  60,  10, 6, 30}; /* 720x480p */
static const struct dsi_timing_info stru_BD0DCC = {1,   1080000, &stru_BD0E04, 1350000, &stru_BD0AC4, 864,  625,  0,  12,  64,  68,  5,  5, 39}; /* 720x576p @ 50Hz (VIC 17/18)*/
static const struct dsi_timing_info stru_BD0B90 = {1,   1080000, &stru_BD0E04, 1350000, &stru_BD0AC4, 864,  626,  0,  12,  64,  68,  6,  5, 40}; /* 720x575p */
static const struct dsi_timing_info stru_BD0CB8 = {0,   2967033, &stru_BD0C00, 3708791, &stru_BD0CF0, 2200, 1125, 1,  88,  44,  148, 2,  5, 16}; /* 1920x1080i @ 60Hz (VIC 5) */
static const struct dsi_timing_info stru_BD0B58 = {0,   2970000, &stru_BD0B34, 3712500, &stru_BD0D4C, 2640, 1125, 1,  528, 44,  148, 2,  5, 16}; /* 1920x1080i @ 50Hz (VIC 20) */
static const struct dsi_timing_info stru_BD0F68 = {0,   2967033, &stru_BD0C00, 3708791, &stru_BD0CF0, 1650, 750,  0,  110, 40,  220, 5,  5, 20}; /* 1280x720p @ 60Hz (VIC 4) */
static const struct dsi_timing_info stru_BD0D70 = {0,   2970000, &stru_BD0B34, 3712500, &stru_BD0D4C, 1980, 750,  0,  440, 40,  220, 5,  5, 20}; /* 1280x720p @ 50Hz (VIC 19) */
static const struct dsi_timing_info stru_BD0C80 = {0,   2967033, &stru_BD0C00, 3708791, &stru_BD0CF0, 2200, 1125, 0,  88,  44,  148, 4,  5, 36}; /* 1920x1080p @ 60Hz (VIC 16) */
static const struct dsi_timing_info stru_BD0A8C = {0,   2970000, &stru_BD0B34, 3712500, &stru_BD0D4C, 2640, 1125, 0,  528, 44,  148, 4,  5, 36}; /* 1920x1080p @ 50Hz (VIC 31) */
static const struct dsi_timing_info stru_BD0AE8 = {0,   2967033, &stru_BD0C00, 3708791, &stru_BD0CF0, 2750, 1125, 0,  638, 44,  148, 4,  5, 36}; /* 1920x1080p @ 24Hz (VIC 32) */

static struct {
	uint32_t vic;
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

static int dsi_lanes_for_bus[] = {
	[DSI_BUS_0] = 2,
	[DSI_BUS_1] = 3
};

static int dsi_unk07_for_bus[] = {
	[DSI_BUS_0] = 0,
	[DSI_BUS_1] = 2
};

static const struct dsi_timing_info *
dsi_get_timing_info_for_vic(uint32_t vic)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(dsi_timing_info_lookup); i++) {
		if (dsi_timing_info_lookup[i].vic == vic)
			return dsi_timing_info_lookup[i].timing_info;
	}

	return NULL;
}

void dsi_init(void)
{
	if ((pervasive_get_soc_revision() & 0x1FF00) > 0xFF)
		memcpy(&stru_A19358, &stru_BD0FA0, sizeof(stru_A19358));
	else
		memcpy(&stru_A19358, &stru_BD0F08, sizeof(stru_A19358));
}

int dsi_get_dimensions_for_vic(uint32_t vic, uint32_t *width, uint32_t *height)
{
	const struct dsi_timing_info *info = dsi_get_timing_info_for_vic(vic);
	if (!info)
		return -1;

	if (width) {
		*width = info->htotal - (info->HBP + info->HFP) - info->HSW;
		if (info->flags & (1 << 3))
			*width -= 2;
	}

	if (height) {
		if (info->mode == 1)
			*height = info->vtotal + 1 - 2 * (info->VFP + info->VSW + info->VBP);
		else
			*height = info->vtotal - (info->VBP + info->VSW) - info->VFP;
	}

	return 0;
}

int dsi_get_pixelclock_for_vic(uint32_t vic, uint32_t bpp, uint32_t *pixelclock)
{
	const struct dsi_timing_info *info = dsi_get_timing_info_for_vic(vic);
	if (!info)
		return -1;

	if (pixelclock) {
		if (bpp == 24)
			*pixelclock = info->pixelclock_24bpp;
		else
			*pixelclock = info->pixelclock_30bpp;
	}

	return 0;
}

void dsi_start_master(enum dsi_bus bus, uint32_t vic)
{
	static const int pixel_size = 24;

	uint32_t packet[64];
	uint32_t packet_length;
	const struct dsi_timing_subinfo *subinfo;
	const struct dsi_timing_info *timing_info = dsi_get_timing_info_for_vic(vic);
	volatile uint32_t *dsi_regs = DSI_REGS(bus);
	int lanes = dsi_lanes_for_bus[bus];
	int unk07 = dsi_unk07_for_bus[bus];

	pervasive_dsi_misc_unk_enable(bus);

	dsi_regs[0x15] = 0;
	dsi_regs[0x146] = 1;

	if ((pervasive_get_soc_revision() & 0x1FF00) > 0xFF) {
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
		dsi_regs[0x20F] = subinfo->unk04 | (subinfo->unk04 << 16);
	}

	uint32_t flags = timing_info->flags;
	uint32_t mode = timing_info->mode;
	uint32_t htotal = timing_info->htotal;
	uint32_t vtotal = timing_info->vtotal;
	uint32_t HFP = timing_info->HFP;
	uint32_t HBP = timing_info->HBP;
	uint32_t HSW = timing_info->HSW;
	uint32_t hactive = htotal - (HFP + HSW + HBP);
	uint32_t VFP = timing_info->VFP;
	uint32_t VSW = timing_info->VSW;
	uint32_t VBP = timing_info->VBP;
	uint32_t vactive = vtotal - (VFP + VSW + VBP);

	if (bus == 1) {
		dsi_regs[0x20D] = 0xF;
		dsi_regs[0x20E] = 0;
		dsi_regs[0x201] = 1;
		dsi_regs[0x145] = 0xFFFFFFFF;

		if (unk07 == 1) {
			if (((flags << 0x1C) & 0x80000000) != 0)
				hactive -= 2;

			packet[0] = 0x40effff;
			if (lanes == 3)
				packet[1] = 0xa30000a4;
			else
				packet[1] = 0xa30000a2;
			packet[2] = 0x28000004;
			packet[3] = 0x10000001;
			packet[4] = 0x28000001;
			packet[5] = 0x10000021;
			packet[6] = (((VBP + VSW) - 3) & 0xffff) | 0x4010000;
			packet[7] = 0x28000001;
			packet[8] = 0x10000021;
			packet[9] = 0x28000001;
			packet[10] = 0x10000021;
			packet[11] = 0x40005619;
			packet[12] = ((hactive * 768) & 0xffffffU) | 0x4000003e;
			packet[13] = 0x40015019;
			packet[14] = (((((vtotal - 2) - VSW) - VBP) - VFP) & 0xffff) | 0x4030000;
			packet[15] = 0x10000021;
			packet[16] = 0x40005619;
			packet[17] = ((hactive * 768) & 0xffffffU) | 0x4000003e;
			packet[18] = 0x40015019;
			packet[19] = 0x10000021;
			packet[20] = ((VFP - 2) & 0xffff) | 0x4010000;
			packet[21] = 0x28000001;
			packet[22] = 0x10000021;
			packet[23] = 0xc000000;
			packet_length = 24;
			goto packet_write;
		}

		uint32_t uVar6;
		uint32_t *puVar4;

		if (mode == 0) {
			uint32_t foo1;
			uint32_t foo2;
			uint32_t foo3;
			uVar6 = VBP - 2;

			if (flags & 1)
				foo1 = ((HFP * pixel_size >> 3) - 0xc) | 0x80000000;
			else
				foo1 = 0;

			if (flags & 2)
				foo2 = ((HSW * pixel_size >> 3) - 10) | 0x80000000;
			else
				foo2 = 0;

			if (flags & 4)
				foo3 = ((HBP * pixel_size >> 3) - 10) | 0x80000000;
			else
				foo3 = 0;

			if (foo1)
				packet[0] = 0x417ffff;
			else
				packet[0] = 0x40dffff;
			if (lanes == 3)
				packet[1] = 0xa30000a4;
			else
				packet[1] = 0xa30000a2;
			packet[2] = 0x28000004;
			packet[3] = 0x10000001;
			packet[5] = 0x10000031;
			packet[8] = 0x10000021;
			packet[10] = 0x10000031;
			packet[12] = 0x10000011;
			packet[14] = 0x10000031;
			packet[17] = 0x10000021;
			if (foo2 == 0)
				packet[18] = 0x28000002;
			else
				packet[18] = ((foo2 & 0xffff) << 8) | 0x40000019;
			packet[19] = 0x10000031;
			if (foo1 == 0) {
				packet[20] = ((vactive - 1) & 0xffff) | 0x4050000;
				packet[21] = 0x28000001;
				packet[22] = 0x10000021;
				if (foo2 != 0)
					packet[23] = ((foo2 & 0xffff) << 8) | 0x40000019;
				else
					packet[23] = 0x28000002;
				packet[24] = 0x10000031;
				if (foo3 != 0)
					packet[25] = ((foo3 & 0xffff) << 8) | 0x40000019;
				else
					packet[25] = 0x28000010;
				if (pixel_size == 0x18)
					packet[26] = (hactive * 768 & 0xffffffU) | 0x4000003e;
				else
					packet[26] = (hactive * 960 & 0xffff00U) | 0x40000029;
				packet[27] = ((VFP - 1) & 0xffff) | 0x4030000;
				packet[28] = 0x28000001;
				packet[29] = 0x10000021;
				if (foo2 != 0)
					packet[30] = ((foo2 & 0xffff) << 8) | 0x40000019;
				else
					packet[30] = 0x28000002;
				packet[31] = 0x10000031;
				puVar4 = &packet[32];
			} else {
				packet[20] = 0x28000001;
				packet[21] = 0x10000021;
				if (foo2 != 0)
					packet[22] = ((foo2 & 0xffff) << 8) | 0x40000019;
				else
					packet[22] = 0x28000002;
				packet[23] = 0x10000031;
				if (foo3 != 0)
					packet[24] = ((foo3 & 0xffff) << 8) | 0x40000019;
				else
					packet[24] = 0x28000010;
				if (pixel_size == 0x18)
					packet[25] = (hactive * 768 & 0xffffffU) | 0x4000003e;
				else
					packet[25] = (hactive * 960 & 0xffff00U) | 0x40000029;
				packet[26] = (foo1 & 0xffff) << 8 | 0x40000019;
				packet[28] = 0x10000021;
				packet[27] = ((vactive - 2) & 0xffff) | 0x4050000;
				if (foo2 != 0)
					packet[29] = (foo2 & 0xffff) << 8 | 0x40000019;
				else
					packet[29] = 0x28000002;
				packet[30] = 0x10000031;
				if (foo3 == 0)
					packet[31] = 0x28000010;
				else
					packet[31] = (foo3 & 0xffff) << 8 | 0x40000019;
				if (pixel_size == 0x18)
					packet[32] = ((hactive * 768) & 0xffffffU) | 0x4000003e;
				else
					packet[32] = ((hactive * 960) & 0xffff00U) | 0x40000029;
				packet[33] = (foo1 & 0xffff) << 8 | 0x40000019;
				packet[34] = 0x10000021;
				if (foo2 == 0)
					packet[35] = 0x28000002;
				else
					packet[35] = (foo2 & 0xffff) << 8 | 0x40000019;
				packet[36] = 0x10000031;
				packet[37] = ((VFP - 2) & 0xffff) | 0x4030000;
				packet[38] = 0x28000001;
				packet[39] = 0x10000021;
				if (foo2 == 0)
					packet[40] = 0x28000002;
				else
					packet[40] = (foo2 & 0xffff) << 8 | 0x40000019;
				packet[41] = 0x10000031;
				puVar4 = &packet[42];
			}
			*puVar4 = 0xc000000;
			packet_length = (int)puVar4 + (4 - (int)packet);
		} else {
			uint32_t tmp1;
			uint32_t tmp2;
			uVar6 = VBP - 3;

			if (flags & 2)
				tmp1 = (((HSW * pixel_size) >> 3) - 10) | 0x80000000;
			else
				tmp1 = 0;

			if (flags & 4)
				tmp2 = (((HBP * pixel_size) >> 3) - 10) | 0x80000000;
			else
				tmp2 = 0;

			packet[0] = 0x415ffff;
			if (lanes == 3)
				packet[1] = 0xa30000a4;
			else
				packet[1] = 0xa30000a2;
			packet[2] = 0x28800005;
			packet[3] = 0x10000081;
			packet[5] = 0x100000b1;
			packet[8] = 0x100000a1;
			packet[10] = 0x100000b1;
			packet[12] = 0x10000091;
			packet[14] = 0x100000b1;
			packet[17] = 0x100000a1;
			packet[19] = 0x100000b1;
			packet[20] = ((((vtotal + 1 - 2 * (VSW + VFP + VBP)) >> 1) - 1) & 0xffff) | 0x4050000;
			packet[21] = 0x28000001;
			packet[22] = 0x100000a1;
			if (tmp1 == 0)
				packet[23] = 0x28000002;
			else
				packet[23] = (tmp1 & 0xffff) << 8 | 0x40000099;
			packet[24] = 0x100000b1;
			if (tmp2 != 0)
				packet[25] = (tmp2 & 0xffff) << 8 | 0x40000099;
			else
				packet[25] = 0x28000010;
			if (pixel_size == 0x18)
				packet[26] = ((hactive * 768) & 0xffffffU) | 0x400000be;
			else
				packet[26] = ((hactive * 960) & 0xffff00U) | 0x400000a9;
			packet[27] = (VFP & 0xffff) | 0x4030000;
			packet[28] = 0x28000001;
			packet[29] = 0x100000a1;
			packet[31] = 0x100000b1;
			packet[32] = 0x28000004;
			packet[33] = 0x100000c1;
			packet[34] = ((VSW - 1) & 0xffff) | 0x4030000;
			packet[35] = 0x28000001;
			packet[36] = 0x100000a1;
			packet[38] = 0x100000b1;
			packet[39] = 0x28000008;
			packet[40] = 0x100000d1;
			packet[41] = ((VBP - 2) & 0xffff) | 0x4030000;
			packet[42] = 0x28000001;
			packet[43] = 0x100000a1;
			packet[45] = 0x100000b1;
			packet[46] = packet[20];
			packet[47] = 0x28000001;
			packet[48] = 0x100000a1;
			if (tmp1 == 0)
				packet[49] = 0x28000002;
			else
				packet[49] = (tmp1 & 0xffff) << 8 | 0x40000099;
			packet[50] = 0x100000b1;
			if (tmp2 != 0)
				packet[51] = (tmp2 & 0xffff) << 8 | 0x40000099;
			else
				packet[51] = 0x28000010;
			if (pixel_size == 0x18)
				packet[52] = ((hactive * 768) & 0xffffffU) | 0x400000be;
			else
				packet[52] = ((hactive * 960) & 0xffff00U) | 0x400000a9;
			packet[53] = ((VFP - 1) & 0xffff) | 0x4030000;
			packet[54] = 0x28000001;
			packet[55] = 0x100000a1;
			if (tmp1 != 0)
				packet[56] = (tmp1 & 0xffff) << 8 | 0x40000099;
			else
				packet[56] = 0x28000002;
			packet[57] = 0x100000b1;
			packet[58] = 0xc000000;
			packet[18] = packet[23];
			packet[30] = packet[49];
			packet[37] = packet[49];
			packet[44] = packet[49];
			packet_length = 0xec;
		}

		packet[4] = packet[18];
		packet[6] = ((VSW - 2) & 0xffff) | 0x4030000;
		packet[7] = 0x28000001;
		packet[9] = packet[18];
		packet[11] = 0x28000008;
		packet[13] = packet[18];
		packet[15] = (uVar6 & 0xffff) | 0x4030000;
		packet[16] = 0x28000001;
		packet_length = packet_length >> 2;
		if (packet_length > 0)
			goto packet_write;
	} else {
		dsi_regs[0x20D] = 7;
		dsi_regs[0x20E] = 0;
		dsi_regs[0x201] = 1;
		dsi_regs[0x145] = 0xFFFFFFFF;

		packet[0] = 0x407ffff;
		packet[1] = 0xa3000082;
		packet[2] = 0x28000004;
		packet[3] = 0x10000001;
		if (unk07 == 0)
			packet[4] = 0x400b4019;
		else
			packet[4] = 0x400b7819;
		packet[5] = (((VSW + VBP) - 2) & 0xffff) | 0x4020000;
		packet[6] = 0x28000001;
		packet[7] = 0x10000021;
		if (unk07 == 0)
			packet[8] = 0x400b4019;
		else
			packet[8] = 0x400b7819;
		packet[9] = (((((vtotal - 1) - VSW) - VBP) - VFP) & 0xffff) | 0x4030000;
		packet[10] = 0x28000001;
		packet[11] = 0x10000021;
		packet[12] = (((HBP * 768) - 0xa00) & 0xffffff) | 0x40000019;
		packet[13] = ((((htotal - (HFP + HSW)) - HBP) * 768) & 0xffffff) | 0x4000003e;
		packet[14] = ((VFP - 1) & 0xffff) | 0x4020000;
		packet[15] = 0x28000001;
		packet[16] = 0x10000021;
		if (unk07 == 0)
			packet[17] = 0x400b4019;
		else
			packet[17] = 0x400b7819;
		packet[18] = 0xc000000;
		packet_length = 19;
packet_write:
		for (uint32_t i = 0; i < packet_length; i++)
			dsi_regs[0x140] = packet[i];
	}

	dsi_regs[0x147] = 1;
}

void dsi_stop_master(enum dsi_bus bus)
{
	volatile uint32_t *dsi_regs = DSI_REGS(bus);
	int lanes = dsi_lanes_for_bus[bus];

	while ((dsi_regs[0x12] & 0xF) != 0)
		;

	dsi_regs[0x145] = 0xffffffff;
	dsi_regs[0x250] = 1;
	dsi_regs[0x251] = 1;
	dsi_regs[0x252] = 1;
	if (lanes == 3)
		dsi_regs[0x253] = 1;
	dsi_regs[0x20D] = 0;
	dsi_regs[0x201] = 1;
	dsb();

	pervasive_dsi_misc_unk_disable(bus);
}

void dsi_start_display(enum dsi_bus bus, uint32_t vic, uint32_t unk)
{
	static const uint32_t pixel_size = 24;
	static const uint32_t intr_mask = 2;

	static const uint32_t lookup[] = {2, 2, 3, 4};

	const struct dsi_timing_info *timing_info = dsi_get_timing_info_for_vic(vic);
	volatile uint32_t *dsi_regs = DSI_REGS(bus);
	int lanes = dsi_lanes_for_bus[bus];

	uint32_t flags = timing_info->flags;
	uint32_t htotal = timing_info->htotal;
	uint32_t vtotal = timing_info->vtotal;
	uint32_t HFP = timing_info->HFP;
	uint32_t HBP = timing_info->HBP;
	uint32_t HSW = timing_info->HSW;
	uint32_t VFP = timing_info->VFP;
	uint32_t VSW = timing_info->VSW;
	uint32_t VBP = timing_info->VBP;
	uint32_t mode = timing_info->mode;

	uint32_t hsync_end = HBP + HSW;
	uint32_t hact = htotal - (HBP + HSW) - HFP;
	if (flags & (1 << 3))
		hact -= 2;

	if (mode == 1)
		dsi_regs[1] = 3;
	else
		dsi_regs[1] = 0;

	uint32_t clocks_per_pixel;
	if (lanes == 2) {
		clocks_per_pixel = 6;
	} else {
		if (pixel_size == 24)
			clocks_per_pixel = 4;
		else
			clocks_per_pixel = 5;
	}

	uint32_t HFP_start = hact + hsync_end;
	uint32_t HSW_clocks = HSW * clocks_per_pixel;
	uint32_t htotal_clocks = htotal * clocks_per_pixel >> 1;
	dsi_regs[2] = htotal_clocks;

	uint32_t v20;
	uint32_t v21;

	if (mode == 1) { // Interlaced
		uint32_t v36 = HFP_start * clocks_per_pixel;
		uint32_t v37 = htotal * clocks_per_pixel >> 2;

		v20 = vtotal + 1;
		v21 = (vtotal + 1) >> 1;

		dsi_regs[3] = ((vtotal << 16) & 0x1FFF0000) | (v21 & 0x1FFF);
		dsi_regs[4] = 0x10001;
		dsi_regs[5] = (((hsync_end * clocks_per_pixel) >> 1) << 16) | (((v36 >> 1) + 1) & 0xFFFF);
		dsi_regs[7] = (((VBP + VSW - 1) << 16) & 0x1FFF0000) | ((v21 - VFP) & 0x1FFF);
		dsi_regs[8] = (((VBP + VSW + ((vtotal - 1) >> 1)) << 16) & 0x1FFF0000) | ((vtotal + 1 - VFP) & 0x1FFF);
		dsi_regs[9] = ((HSW_clocks >> 1) << 16) | 1;
		dsi_regs[0xB] = 0x10001;
		dsi_regs[0xC] = ((VSW << 16) & 0x1FFF0000) | (htotal_clocks & 0xFFFF);
		dsi_regs[0xD] = ((v21 << 16) & 0x1FFF0000) | (v37 + 1);
		dsi_regs[0xE] = (((VSW + v21) << 16) & 0x1FFF0000) | v37;
	} else { // Progressive
		v20 = vtotal + 1;
		v21 = (vtotal + 1) >> 1;

		dsi_regs[3] = vtotal;
		dsi_regs[4] = 0x10001;
		dsi_regs[5] = ((((HBP + HSW) * clocks_per_pixel) >> 1) << 16) | ((((HFP_start * clocks_per_pixel) >> 1) + 1) & 0xFFFF);
		dsi_regs[7] = (((VBP + VSW) << 16) & 0x1FFF0000) | ((vtotal + 1 - VFP) & 0x1FFF);
		dsi_regs[9] = ((HSW_clocks >> 1) << 16) | 1;
		dsi_regs[0xB] = 0x10001;
		dsi_regs[0xC] = ((VSW << 16) & 0x1FFF0000) | htotal_clocks;
	}

	if (lanes == 3 && pixel_size == 30) {
		uint32_t v34;
		uint32_t v35;

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
		} else {
			goto skip;
		}

		dsi_regs[0x1D] = v34;
		dsi_regs[0x1E] = v35;
		dsi_regs[1] |= 0x100;
	}

skip:
	dsi_regs[0xF] = 1;
	uint32_t vact_start = VBP + VSW;
	dsi_regs[0x10] = (((v21 + vact_start - 8) << 16) & 0x1FFF0000) | ((vact_start - 8) & 0x1FFF);
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

	dsi_regs[0x14] = dsi_regs[0x14];
	dsi_regs[0x15] = intr_mask;
	if (bus)
		dsi_regs[0x20E] = 1;
	else
		dsi_regs[0x20E] = 0;

	uint32_t v30;
	if (unk == 1)
		v30 = (bus ^ 1) & 1;
	else
		v30 = 0;

	dsi_regs[0x142] = 0xFFFFFFFF;

	uint32_t v31 = (unk == 2) ? (bus & 1) : 0;
	if (v30 || v31 || (unk - 1 > 3))
		dsi_regs[0] = 1;
	else
		dsi_regs[0] = lookup[unk - 1];

	dmb();
}
