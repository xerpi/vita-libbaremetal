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

void iftu_crtc_enable(int crtc)
{
	volatile unsigned int *iftu_cregs = IFTU_CREGS(crtc);

	iftu_cregs[0] = 1;
}

void iftu_set_plane(int plane, struct iftu_plane_info *info)
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

}
