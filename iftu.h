#ifndef IFTU_H
#define IFTU_H

#define IFTU_FB_PIXELFORMAT_A8B8G8R8	0x10

struct iftu_plane_fb_config {
	unsigned int paddr;
	unsigned int pixelformat;
	unsigned int width;
	unsigned int height;
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

void iftu_bus_enable(int bus);
void iftu_bus_plane_config_select(int bus, int plane, int config);
void iftu_bus_alpha_blending_control(int bus, int ctrl);

void iftu_plane_set_alpha(int bus, int plane, unsigned int alpha);
void iftu_plane_set_csc_enabled(int bus, int plane, int enabled);
void iftu_plane_set_csc0(int bus, int plane, const struct iftu_csc_params *csc);
void iftu_plane_set_csc1(int bus, int plane, const struct iftu_csc_params *csc);

void iftu_plane_set_fb_config(int bus, int plane, int config, struct iftu_plane_fb_config *cfg);

#endif
