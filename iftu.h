#ifndef IFTU_H
#define IFTU_H

struct iftu_plane_info {
	unsigned int pixelformat;
	unsigned int width;
	unsigned int height;
	unsigned int leftover_stride;
	unsigned int unk10; // always 0
	unsigned int paddr;
	unsigned int unk18; // always 0
	unsigned int unk1C; // always 0
	unsigned int unk20; // always 0
	unsigned int src_x; // in (0x100000 / 960) multiples
	unsigned int src_y; // in (0x100000 / 544) multiples
	unsigned int src_w; // in (0x100000 / 960) multiples
	unsigned int src_h; // in (0x100000 / 544) multiples
	unsigned int dst_x;
	unsigned int dst_y;
	unsigned int dst_w;
	unsigned int dst_h;
	unsigned int vfront_porch;
	unsigned int vback_porch;
	unsigned int hfront_porch;
	unsigned int hback_porch;
};

void iftu_crtc_enable(int crtc);
void iftu_set_plane(int plane, struct iftu_plane_info *info);

#endif
