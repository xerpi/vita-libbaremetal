#include "iftu.h"
#include "utils.h"

#define IFTU0REGA_BASE_ADDR		0xE5020000
#define IFTU0REGB_BASE_ADDR		0xE5021000
#define IFTU0CREG_BASE_ADDR		0xE5022000
#define IFTU1REGA_BASE_ADDR		0xE5030000
#define IFTU1REGB_BASE_ADDR		0xE5031000
#define IFTU1CREG_BASE_ADDR		0xE5032000

#define IFTU_REGS(i)			((void *)( \
						(i) == 0 ? IFTU0REGA_BASE_ADDR : \
						(i) == 1 ? IFTU0REGB_BASE_ADDR : \
						(i) == 2 ? IFTU1REGA_BASE_ADDR : \
						           IFTU1REGB_BASE_ADDR))

#define IFTU_CREGS(i) ((void *)(((i) & 1) == 0 ? IFTU0CREG_BASE_ADDR : IFTU1CREG_BASE_ADDR))

static const struct iftu_csc_params csc_identity_matrix_C01724 = {0,    0,     0x3FF, 0,    0x3FF, 0,    0x200, 0, 0,     0,     0x200, 0,     0,     0,     0x200};
static const struct iftu_csc_params csc_identity_matrix_C019AC = {0,    0,     0x3FF, 0,    0x3FF, 0,    0x200, 0, 0,     0,     0x200, 0,     0,     0,     0x200};
static const struct iftu_csc_params YCbCr_to_RGB_HDTV_C0177C   = {0x40, 0x202, 0x3FF, 0,    0,     0,    0x254, 0, 0x395, 0x254, 0xF93, 0xEF0, 0x254, 0x439, 0};
static const struct iftu_csc_params YCbCr_to_RGB_HDTV_C0180C   = {0x40, 0x202, 0x3FF, 0,    0,     0,    0x254, 0, 0x395, 0x254, 0xF93, 0xEF0, 0x254, 0x439, 0};
static const struct iftu_csc_params YPbPr_to_RGB_HDTV_C017D0   = {0,    0x202, 0x3FF, 0,    0,     0,    0x200, 0, 0x326, 0x200, 0xFA1, 0xF11, 0x200, 0x3B6, 0};
static const struct iftu_csc_params stru_C01970                = {0x40, 0x40,  0x3AC, 0x40, 0x3AC, 0x40, 0x1B7, 0, 0,     0,     0x1B7, 0,     0,     0,     0x1B7};

void iftu_crtc_enable(int crtc)
{
	volatile unsigned int *iftu_cregs = IFTU_CREGS(crtc);

	iftu_cregs[0] = 1;
}

void iftu_init_plane(int plane)
{
	volatile unsigned int *iftu_regs = IFTU_REGS(plane);
	volatile unsigned int *iftu_cregs = IFTU_CREGS((plane >> 1) & 1);

	iftu_regs[0x20] = 0;
	iftu_regs[0x21] = 0;
	iftu_regs[0x22] = 0;

	iftu_regs[(0x200 + 0 * 0x100 + 0xA0) / 4] = 0x2000;
	iftu_regs[(0x200 + 0 * 0x100 + 0xA4) / 4] = 1080;
	iftu_regs[(0x200 + 0 * 0x100 + 0xA8) / 4] = 720;

	iftu_regs[(0x200 + 1 * 0x100 + 0xA0) / 4] = 0x2000;
	iftu_regs[(0x200 + 1 * 0x100 + 0xA4) / 4] = 1080;
	iftu_regs[(0x200 + 1 * 0x100 + 0xA8) / 4] = 720;

	iftu_cregs[(0x10 + 0 * 0x8 + 0x00) / 4] = 0;
	iftu_cregs[(0x10 + 0 * 0x8 + 0x04) / 4] = 0;
	iftu_cregs[(0x10 + 1 * 0x8 + 0x00) / 4] = 0;
	iftu_cregs[(0x10 + 1 * 0x8 + 0x04) / 4] = 0;

	iftu_cregs[0x20 / 4] = 2;

	iftu_cregs[0] = 1;
	iftu_cregs[1] = 1;

	dmb();
}

void iftu_set_source_fb(int plane, struct iftu_plane_source_fb_info *info)
{
	static const unsigned int crtc_mask = 0b10;
	const unsigned int plane_regs_offset = 0x200 + ((crtc_mask >> 1) & 1) * 0x100;

	volatile unsigned int *iftu_plane_regs = IFTU_REGS(plane) + plane_regs_offset;

	iftu_plane_regs[0x00] = info->paddr;
	iftu_plane_regs[0x01] = info->unk18;
	iftu_plane_regs[0x02] = info->unk1C;

	iftu_plane_regs[0x08] = info->unk20;
	iftu_plane_regs[0x09] = info->src_x;
	iftu_plane_regs[0x0A] = info->src_y;

	iftu_plane_regs[0x10] = info->pixelformat;
	iftu_plane_regs[0x11] = info->width;
	iftu_plane_regs[0x12] = info->height;
	iftu_plane_regs[0x13] = 1; // Control reg

	iftu_plane_regs[0x15] = info->leftover_stride;
	iftu_plane_regs[0x16] = info->unk10;

	iftu_plane_regs[0x18] = info->vfront_porch;
	iftu_plane_regs[0x19] = info->vback_porch;
	iftu_plane_regs[0x1A] = info->hfront_porch;
	iftu_plane_regs[0x1B] = info->hback_porch;

	iftu_plane_regs[0x30] = info->src_w;
	iftu_plane_regs[0x31] = info->src_h;
	iftu_plane_regs[0x32] = info->dst_x;
	iftu_plane_regs[0x33] = info->dst_y;
	iftu_plane_regs[0x34] = info->dst_w;
	iftu_plane_regs[0x35] = info->dst_h;

	dmb();
}

void iftu_set_dst_conversion(int plane, unsigned int dst_width, unsigned int dst_height,
	unsigned int dst_pixelformat, unsigned int unk44)
{

}

void iftu_set_csc1(int plane, const struct iftu_csc_params *csc)
{
	volatile unsigned int *iftu_regs = IFTU_REGS(plane);

	iftu_regs[0x4C] = csc->unk00;
	iftu_regs[0x4D] = csc->unk04;
	iftu_regs[0x4E] = csc->unk08;
	iftu_regs[0x4F] = csc->unk0C;
	iftu_regs[0x50] = csc->unk10;
	iftu_regs[0x51] = csc->unk14;
	iftu_regs[0x52] = csc->csc_rr;
	iftu_regs[0x53] = csc->csc_rg;
	iftu_regs[0x54] = csc->csc_rb;
	iftu_regs[0x55] = csc->csc_gr;
	iftu_regs[0x56] = csc->csc_gg;
	iftu_regs[0x57] = csc->csc_gb;
	iftu_regs[0x58] = csc->csc_br;
	iftu_regs[0x59] = csc->csc_bg;
	iftu_regs[0x5A] = csc->csc_bb;

	dmb();
}

void iftu_set_csc2(int plane, const struct iftu_csc_params *csc)
{
	volatile unsigned int *iftu_regs = IFTU_REGS(plane);

	iftu_regs[0x41] = csc->unk00;
	iftu_regs[0x42] = csc->unk04;
	iftu_regs[0x43] = csc->csc_rr;
	iftu_regs[0x44] = csc->csc_rg;
	iftu_regs[0x45] = csc->csc_rb;
	iftu_regs[0x46] = csc->csc_gr;
	iftu_regs[0x47] = csc->csc_gg;
	iftu_regs[0x48] = csc->csc_gb;
	iftu_regs[0x49] = csc->csc_br;
	iftu_regs[0x4A] = csc->csc_bg;
	iftu_regs[0x4B] = csc->csc_bb;

	dmb();
}

void iftu_set_control_value(int plane, int value)
{
	unsigned int tmp;
	unsigned int crtc = (plane >> 1) & 1;
	volatile unsigned int *iftu_cregs = IFTU_CREGS(crtc);

	tmp = 0; //iftu_cregs[1];

	if (value == 0x80) {
		iftu_cregs[8] = 0;
		tmp &= ~1;
	} else {
		iftu_cregs[8] = value;
		tmp |= 1;
	}

	iftu_cregs[1] = tmp;
}

void iftu_set_alpha(int plane, unsigned int alpha)
{
	volatile unsigned int *iftu_regs = IFTU_REGS(plane);

	if (!((plane >> 1) & 1))
		return;

	if (alpha == 0x100)
		iftu_regs[0x23] = 0;
	else
		iftu_regs[0x23] = alpha;

	iftu_regs[0x28] = (alpha == 0x100);
}
