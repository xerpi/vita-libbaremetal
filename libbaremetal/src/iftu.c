#include "iftu.h"
#include "utils.h"

#define IFTUREG_BASE_ADDR			0xE5020000
#define IFTUCREG_BASE_ADDR			0xE5022000

#define IFTU_CREGS(bus)				(IFTUCREG_BASE_ADDR + (bus) * 0x10000)

#define IFTU_CREG_CONTROL			0x00
#define IFTU_CREG_CONTROL_ENABLE		BIT(0)

#define IFTU_CREG_CONTROL2			0x04
#define IFTU_CREG_CONTROL2_ALPHA_EN		BIT(0)

#define IFTU_CREG_PLANE_CONFIG_BASE(i)		(0x10 + (i) * 0x08)
#define IFTU_CREG_PLANE_CONFIG_SELECT		0x00

#define IFTU_CREG_ALPHA_BLENDING_CONTROL	0x20

#define IFTU_PLANE_REGS(bus, plane)		(IFTUREG_BASE_ADDR + (bus) * 0x10000 + (plane) * 0x1000)

#define IFTU_PLANE_ALPHA_VALUE			0x08C
#define IFTU_PLANE_ALPHA_CONTROL		0x0A0
#define IFTU_PLANE_CSC_CONTROL			0x100
#define IFTU_PLANE_CSC_UNK_104			0x104
#define IFTU_PLANE_CSC_UNK_108			0x108
#define IFTU_PLANE_CSC_RR_0			0x10C
#define IFTU_PLANE_CSC_RG_0			0x110
#define IFTU_PLANE_CSC_RB_0			0x114
#define IFTU_PLANE_CSC_GR_0			0x118
#define IFTU_PLANE_CSC_GG_0			0x11C
#define IFTU_PLANE_CSC_GB_0			0x120
#define IFTU_PLANE_CSC_BR_0			0x124
#define IFTU_PLANE_CSC_BG_0			0x128
#define IFTU_PLANE_CSC_BB_0			0x12C
#define IFTU_PLANE_CSC_UNK_130			0x130
#define IFTU_PLANE_CSC_UNK_134			0x134
#define IFTU_PLANE_CSC_RR_1			0x138
#define IFTU_PLANE_CSC_RG_1			0x13C
#define IFTU_PLANE_CSC_RB_1			0x140
#define IFTU_PLANE_CSC_GR_1			0x144
#define IFTU_PLANE_CSC_GG_1			0x148
#define IFTU_PLANE_CSC_GB_1			0x14C
#define IFTU_PLANE_CSC_BR_1			0x150
#define IFTU_PLANE_CSC_BG_1			0x154
#define IFTU_PLANE_CSC_BB_1			0x158
#define IFTU_PLANE_CSC_UNK_15C			0x15C
#define IFTU_PLANE_CSC_UNK_160			0x160
#define IFTU_PLANE_CSC_UNK_164			0x164
#define IFTU_PLANE_CSC_UNK_168			0x168

#define IFTU_PLANE_CONFIG_REGS(bus, plane, cfg)	(IFTU_PLANE_REGS(bus, plane) + 0x200 + (cfg) * 0x100)

#define IFTU_PLANE_CONFIG_FB_PADDR		0x00
#define IFTU_PLANE_CONFIG_SRC_X			0x24
#define IFTU_PLANE_CONFIG_SRC_Y			0x28
#define IFTU_PLANE_CONFIG_SRC_PIXELFMT		0x40
#define IFTU_PLANE_CONFIG_SRC_FB_WIDTH		0x44
#define IFTU_PLANE_CONFIG_SRC_FB_HEIGHT		0x48
#define IFTU_PLANE_CONFIG_CONTROL		0x4C
#define IFTU_PLANE_CONFIG_DST_PIXELFMT		0xA0
#define IFTU_PLANE_CONFIG_DST_WIDTH		0xA4
#define IFTU_PLANE_CONFIG_DST_HEIGHT		0xA8
#define IFTU_PLANE_CONFIG_SRC_W			0xC0
#define IFTU_PLANE_CONFIG_SRC_H			0xC4
#define IFTU_PLANE_CONFIG_DST_X			0xC8
#define IFTU_PLANE_CONFIG_DST_Y			0xCC

void iftu_bus_enable(enum iftu_bus bus)
{
	uintptr_t cregs = IFTU_CREGS(bus);

	write32(IFTU_CREG_CONTROL_ENABLE, cregs + IFTU_CREG_CONTROL);
	write32(IFTU_CREG_CONTROL2_ALPHA_EN, cregs + IFTU_CREG_CONTROL2);
	dmb();
}

void iftu_bus_plane_config_select(enum iftu_bus bus, enum iftu_plane plane, enum iftu_plane_config config)
{
	uintptr_t cregs = IFTU_CREGS(bus);

	write32(config, cregs + IFTU_CREG_PLANE_CONFIG_BASE(plane) +
			       IFTU_CREG_PLANE_CONFIG_SELECT);
	dmb();
}

void iftu_bus_alpha_blending_control(enum iftu_bus bus, int ctrl)
{
	uintptr_t cregs = IFTU_CREGS(bus);

	write32(ctrl, cregs + IFTU_CREG_ALPHA_BLENDING_CONTROL);
	dmb();
}

void iftu_plane_set_alpha(enum iftu_bus bus, enum iftu_plane plane, uint32_t alpha)
{
	uintptr_t regs = IFTU_PLANE_REGS(bus, plane);

	if (alpha == 256)
		write32(0, regs + IFTU_PLANE_ALPHA_VALUE);
	else
		write32(alpha, regs + IFTU_PLANE_ALPHA_VALUE);

	write32(alpha == 256, regs + IFTU_PLANE_ALPHA_CONTROL);

	dmb();
}

void iftu_plane_set_csc_enabled(enum iftu_bus bus, enum iftu_plane plane, bool enabled)
{
	uintptr_t regs = IFTU_PLANE_REGS(bus, plane);

	write32(enabled, regs + IFTU_PLANE_CSC_CONTROL);

	dmb();
}

void iftu_plane_set_csc0(enum iftu_bus bus, enum iftu_plane plane, const struct iftu_csc_params *csc)
{
	uintptr_t regs = IFTU_PLANE_REGS(bus, plane);

	write32(csc->unk00, regs + IFTU_PLANE_CSC_UNK_104);
	write32(csc->unk04, regs + IFTU_PLANE_CSC_UNK_108);
	write32(csc->ctm[0][0], regs + IFTU_PLANE_CSC_RR_0);
	write32(csc->ctm[0][1], regs + IFTU_PLANE_CSC_RG_0);
	write32(csc->ctm[0][2], regs + IFTU_PLANE_CSC_RB_0);
	write32(csc->ctm[1][0], regs + IFTU_PLANE_CSC_GR_0);
	write32(csc->ctm[1][1], regs + IFTU_PLANE_CSC_GG_0);
	write32(csc->ctm[1][2], regs + IFTU_PLANE_CSC_GB_0);
	write32(csc->ctm[2][0], regs + IFTU_PLANE_CSC_BR_0);
	write32(csc->ctm[2][1], regs + IFTU_PLANE_CSC_BG_0);
	write32(csc->ctm[2][2], regs + IFTU_PLANE_CSC_BB_0);

	dmb();
}

void iftu_plane_set_csc1(enum iftu_bus bus, enum iftu_plane plane, const struct iftu_csc_params *csc)
{
	uintptr_t regs = IFTU_PLANE_REGS(bus, plane);

	write32(csc->unk00, regs + IFTU_PLANE_CSC_UNK_130);
	write32(csc->unk04, regs + IFTU_PLANE_CSC_UNK_134);
	write32(csc->ctm[0][0], regs + IFTU_PLANE_CSC_RR_1);
	write32(csc->ctm[0][1], regs + IFTU_PLANE_CSC_RG_1);
	write32(csc->ctm[0][2], regs + IFTU_PLANE_CSC_RB_1);
	write32(csc->ctm[1][0], regs + IFTU_PLANE_CSC_GR_1);
	write32(csc->ctm[1][1], regs + IFTU_PLANE_CSC_GG_1);
	write32(csc->ctm[1][2], regs + IFTU_PLANE_CSC_GB_1);
	write32(csc->ctm[2][0], regs + IFTU_PLANE_CSC_BR_1);
	write32(csc->ctm[2][1], regs + IFTU_PLANE_CSC_BG_1);
	write32(csc->ctm[2][2], regs + IFTU_PLANE_CSC_BB_1);
	write32(csc->unk08, regs + IFTU_PLANE_CSC_UNK_15C);
	write32(csc->unk0C, regs + IFTU_PLANE_CSC_UNK_160);
	write32(csc->unk10, regs + IFTU_PLANE_CSC_UNK_164);
	write32(csc->unk14, regs + IFTU_PLANE_CSC_UNK_168);

	dmb();
}

void iftu_plane_config_set_config(enum iftu_bus bus, enum iftu_plane plane,
				  enum iftu_plane_config config,
				  const struct iftu_plane_fb_config *fb,
				  uint32_t dst_x, uint32_t dst_y,
				  uint32_t dst_w, uint32_t dst_h)
{
	uintptr_t regs = IFTU_PLANE_CONFIG_REGS(bus, plane, config);

	/* TODO: Properly use pitch instead of width */

	write32(fb->paddr, regs + IFTU_PLANE_CONFIG_FB_PADDR);
	write32(0, regs + IFTU_PLANE_CONFIG_SRC_X);
	write32(0, regs + IFTU_PLANE_CONFIG_SRC_Y);
	write32(fb->pixelformat, regs + IFTU_PLANE_CONFIG_SRC_PIXELFMT);
	write32(fb->width, regs + IFTU_PLANE_CONFIG_SRC_FB_WIDTH);
	write32(fb->height, regs + IFTU_PLANE_CONFIG_SRC_FB_HEIGHT);
	write32(0, regs + IFTU_PLANE_CONFIG_CONTROL);
	write32(0x2000, regs + IFTU_PLANE_CONFIG_DST_PIXELFMT);
	write32(dst_w, regs + IFTU_PLANE_CONFIG_DST_WIDTH);
	write32(dst_h, regs + IFTU_PLANE_CONFIG_DST_HEIGHT);
	write32(0x10000, regs + IFTU_PLANE_CONFIG_SRC_W);
	write32(0x10000, regs + IFTU_PLANE_CONFIG_SRC_H);
	write32(dst_x, regs + IFTU_PLANE_CONFIG_DST_X);
	write32(dst_y, regs + IFTU_PLANE_CONFIG_DST_Y);

	dmb();
}

void iftu_plane_config_set_enabled(enum iftu_bus bus, enum iftu_plane plane,
				   enum iftu_plane_config config, bool enabled)
{
	uintptr_t regs = IFTU_PLANE_CONFIG_REGS(bus, plane, config);

	write32(!enabled, regs + IFTU_PLANE_CONFIG_CONTROL);

	dmb();
}
