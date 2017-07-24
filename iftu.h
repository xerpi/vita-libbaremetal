#ifndef IFTU_H
#define IFTU_H

struct iftu_plane_source_fb_info {
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

struct iftu_csc_params {
	unsigned int unk00;
	unsigned int unk04;
	unsigned int unk08;
	unsigned int unk0C;
	unsigned int unk10;
	unsigned int unk14;
	unsigned int csc_rr;
	unsigned int csc_rg;
	unsigned int csc_rb;
	unsigned int csc_gr;
	unsigned int csc_gg;
	unsigned int csc_gb;
	unsigned int csc_br;
	unsigned int csc_bg;
	unsigned int csc_bb;
};

void iftu_crtc_enable(int crtc);
void iftu_init_plane(int plane);
void iftu_set_source_fb(int plane, struct iftu_plane_source_fb_info *info);
void iftu_set_dst_conversion(int plane, unsigned int dst_width, unsigned int dst_height,
	unsigned int dst_pixelformat, unsigned int unk44);
void iftu_set_csc1(int plane, const struct iftu_csc_params *csc);
void iftu_set_csc2(int plane, const struct iftu_csc_params *csc);
void iftu_set_control_value(int plane, int value);
void iftu_set_alpha(int plane, unsigned int alpha);

#endif
