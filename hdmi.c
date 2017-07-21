#include "libc.h"
#include "hdmi.h"
#include "adv7511.h"
#include "lowio.h"
#include "log.h"

#define BIT(x) (1 << (x))
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define HDMI_I2C_BUS	1
#define HDMI_I2C_ADDR	0x7A

/* Configurable addresses */
#define HDMI_I2C_CEC_ADDR	0x7C

#if 0
static void hdmi_i2c_cmd_write(unsigned char addr, unsigned char reg, const unsigned char *data, int size)
{
	unsigned char buffer[8];
	int i = 0;

	while (size > 7) {
		buffer[0] = reg + i;
		memcpy(&buffer[1], &data[i], sizeof(buffer) - 1);
		i2c_transfer_write(HDMI_I2C_BUS, addr, buffer, sizeof(buffer));
		i += 7;
		size -= 7;
	}

	if (size > 0) {
		buffer[0] = reg + i;
		memcpy(&buffer[1], &data[i], size);
		i2c_transfer_write(HDMI_I2C_BUS, addr, buffer, size + 1);
	}
}

static void hdmi_i2c_cmd_write_read(unsigned char addr, unsigned char reg, unsigned char *buff, unsigned int size)
{
	unsigned char buffer;
	int i = 0;

	while (size > 8) {
		buffer = reg + i;
		i2c_transfer_write_read(HDMI_I2C_BUS, addr, &buffer, sizeof(buffer), addr, &buff[i], size);
		i += 8;
		size -= 8;
	}

	if (size > 0) {
		buffer = reg + i;
		i2c_transfer_write_read(HDMI_I2C_BUS, addr, &buffer, sizeof(buffer), addr, &buff[i], size);
	}
}

static inline void hdmi_i2c_cmd_write_1(unsigned char addr, unsigned char reg, unsigned char data)
{
	unsigned char buff = data;
	hdmi_i2c_cmd_write(addr, reg, &buff, sizeof(buff));
}

static inline void hdmi_i2c_cmd_write_read_1(unsigned char addr, unsigned char reg, unsigned char *data)
{
	hdmi_i2c_cmd_write_read(addr, reg, data, 1);
}

static void hdmi_set_hdcp(int enable)
{
	unsigned char buff, bits;

	if (enable)
		bits = 0x80;
	else
		bits = 0;

	hdmi_i2c_cmd_write_read_1(HDMI_I2C_ADDR, 0xAF, &buff);
	buff = (buff & 0x7F) | bits;
	hdmi_i2c_cmd_write_1(HDMI_I2C_ADDR, 0xAF, buff);
}

static void hdmi_set_frame_encryption(int enable)
{
	unsigned char buff, bits;

	if (enable)
		bits = 0x10;
	else
		bits = 0;

	hdmi_i2c_cmd_write_read_1(HDMI_I2C_ADDR, 0xAF, &buff);
	buff = (buff & 0xEF) | bits;
	hdmi_i2c_cmd_write_1(HDMI_I2C_ADDR, 0xAF, buff);
}

static void hdmi_set_unk(int enable)
{
	unsigned char buff, bits;

	if (enable)
		bits = 2;
	else
		bits = 0;

	hdmi_i2c_cmd_write_read_1(HDMI_I2C_ADDR, 0xAF, &buff);
	buff = (buff & 0xFD) | bits;
	hdmi_i2c_cmd_write_1(HDMI_I2C_ADDR, 0xAF, buff);
}

void hdmi_setup_cec(void)
{
	unsigned char buff[6];

	hdmi_i2c_cmd_write_read(HDMI_I2C_ADDR, 0x92, buff, sizeof(buff));

	buff[1] = 0;
	buff[3] &= ~0x40;
	buff[4] = 0;
	buff[5] = 0;

        buff[1] = 0;
        buff[4] = 0;
        buff[5] = 0;
        buff[3] |= 0x80;
        buff[2] |= 0xC6;

        hdmi_i2c_cmd_write(HDMI_I2C_ADDR, 0x92, buff, sizeof(buff));
}

void hdmi_setup_powerdown(int enable)
{
	unsigned char buff, bits;

	if (enable)
		bits = 0;
	else
		bits = 0x3C;

	hdmi_i2c_cmd_write_read_1(HDMI_I2C_ADDR, 0xA1, &buff);
	buff = (buff & 0xC3) | bits;
	hdmi_i2c_cmd_write_1(HDMI_I2C_ADDR, 0xA1, buff);
}

void hdmi_setup_edid_rereads_tries(void)
{
	unsigned char buff;

	hdmi_i2c_cmd_write_read_1(HDMI_I2C_ADDR, 0xC9, &buff);
	buff = (buff & 0xE0) | 0x11;
	hdmi_i2c_cmd_write_1(HDMI_I2C_ADDR, 0xC9, buff);
}

static int hdmi_init_old(void)
{
	unsigned char buff[4];

	hdmi_i2c_cmd_write_1(HDMI_I2C_ADDR, ADV7511_REG_CEC_I2C_ADDR, HDMI_I2C_CEC_ADDR);

	hdmi_i2c_cmd_write_1(HDMI_I2C_ADDR, 0x92, 0);
	hdmi_i2c_cmd_write_1(HDMI_I2C_ADDR, 0x93, 0);

	hdmi_i2c_cmd_write_read_1(HDMI_I2C_ADDR, 0x4B, &buff[0]);
	hdmi_i2c_cmd_write_1(HDMI_I2C_ADDR, 0x4B, (buff[0] & 0x3F) | (1 << 6));

	hdmi_i2c_cmd_write_1(HDMI_I2C_ADDR, 0x40, 0);
	hdmi_i2c_cmd_write_1(HDMI_I2C_ADDR, 0x44, 1);

	hdmi_set_hdcp(0);
	hdmi_set_frame_encryption(0);

	hdmi_setup_cec();

	hdmi_setup_powerdown(0);

	hdmi_set_unk(0);

	hdmi_i2c_cmd_write_read(HDMI_I2C_CEC_ADDR, 0, &buff[0], 3);
	hdmi_i2c_cmd_write_read(HDMI_I2C_ADDR, 0, &buff[3], 1);

	/*if ((*(unsigned int *)buff) != 0x14013375)
		return -1;*/

        hdmi_i2c_cmd_write_1(HDMI_I2C_CEC_ADDR, 0x16, 0x20);
        hdmi_i2c_cmd_write_1(HDMI_I2C_CEC_ADDR, 0x1C, 0x30);
        hdmi_i2c_cmd_write_1(HDMI_I2C_CEC_ADDR, 0x20, 0x30);

        hdmi_setup_edid_rereads_tries();

	return 0;
}

struct reg_sequence {
	unsigned char reg;
	unsigned char cmd;
};

static const struct reg_sequence adv7511_fixed_registers[] = {
	{ 0x98, 0x03 },
	{ 0x9a, 0xe0 },
	{ 0x9c, 0x30 },
	{ 0x9d, 0x61 },
	{ 0xa2, 0xa4 },
	{ 0xa3, 0xa4 },
	{ 0xe0, 0xd0 },
	{ 0xf9, 0x00 },
	{ 0x55, 0x02 },
};

static const struct reg_sequence adv7533_fixed_registers[] = {
	{ 0x16, 0x20 },
	{ 0x9a, 0xe0 },
	{ 0xba, 0x70 },
	{ 0xde, 0x82 },
	{ 0xe4, 0x40 },
	{ 0xe5, 0x80 },
};

static const struct reg_sequence adv7533_cec_fixed_registers[] = {
	{ 0x15, 0xd0 },
	{ 0x17, 0xd0 },
	{ 0x24, 0x20 },
	{ 0x57, 0x11 },
	{ 0x05, 0xc8 },
};

void hdmi_i2c_update_bits(unsigned char addr, unsigned char reg,
			  unsigned char mask, unsigned char val)
{
	unsigned char data;

	hdmi_i2c_cmd_write_read_1(addr, reg, &data);
	data &= ~mask;
	data |= val & mask;
	hdmi_i2c_cmd_write_1(addr, reg, data);
}

static int hdmi_packet_enable(unsigned char addr, unsigned int packet)
{
	if (packet & 0xff)
		hdmi_i2c_update_bits(addr, ADV7511_REG_PACKET_ENABLE0,
				   packet, 0xff);

	if (packet & 0xff00) {
		packet >>= 8;
		hdmi_i2c_update_bits(addr, ADV7511_REG_PACKET_ENABLE1,
				   packet, 0xff);
	}

	return 0;
}

static void hdmi_packet_disable(unsigned char addr,  unsigned int packet)
{
	if (packet & 0xff)
		hdmi_i2c_update_bits(addr, ADV7511_REG_PACKET_ENABLE0,
				   packet, 0x00);

	if (packet & 0xff00) {
		packet >>= 8;
		hdmi_i2c_update_bits(addr, ADV7511_REG_PACKET_ENABLE1,
				   packet, 0x00);
	}
}

int hdmi_init_old2(void)
{
	int i;
	unsigned char buff[4];

	hdmi_i2c_cmd_write_1(HDMI_I2C_ADDR, ADV7511_REG_CEC_I2C_ADDR, HDMI_I2C_CEC_ADDR);

	hdmi_i2c_cmd_write_1(HDMI_I2C_ADDR, 0x41, 0);

	for (i = 0; i < ARRAY_SIZE(adv7511_fixed_registers); i++)
		hdmi_i2c_cmd_write_1(HDMI_I2C_ADDR,
				     adv7511_fixed_registers[i].reg,
				     adv7511_fixed_registers[i].cmd);


	for (i = 0; i < ARRAY_SIZE(adv7533_fixed_registers); i++)
		hdmi_i2c_cmd_write_1(HDMI_I2C_ADDR,
				     adv7533_fixed_registers[i].reg,
				     adv7533_fixed_registers[i].cmd);

	/* set number of dsi lanes */
	hdmi_i2c_cmd_write_1(HDMI_I2C_CEC_ADDR, 0x1c, 1 << 4);

	if (1) {
		/* reset internal timing generator */
		hdmi_i2c_cmd_write_1(HDMI_I2C_CEC_ADDR, 0x27, 0xcb);
		hdmi_i2c_cmd_write_1(HDMI_I2C_CEC_ADDR, 0x27, 0x8b);
		hdmi_i2c_cmd_write_1(HDMI_I2C_CEC_ADDR, 0x27, 0xcb);
	} else {
		/* disable internal timing generator */
		hdmi_i2c_cmd_write_1(HDMI_I2C_CEC_ADDR, 0x27, 0x0b);
	}

	/* enable hdmi */
	hdmi_i2c_cmd_write_1(HDMI_I2C_CEC_ADDR, 0x03, 0x89);
	/* disable test mode */
	hdmi_i2c_cmd_write_1(HDMI_I2C_CEC_ADDR, 0x55, 0x00);


	for (i = 0; i < ARRAY_SIZE(adv7533_cec_fixed_registers); i++)
		hdmi_i2c_cmd_write_1(HDMI_I2C_CEC_ADDR,
				     adv7533_cec_fixed_registers[i].reg,
				     adv7533_cec_fixed_registers[i].cmd);


	return 0;
}

//                                         flags pixelclk24 subinfo_24bpp pixelclk30 subinfo_30bpp hline  vline mode HBP   HFP   HSS   VFP  VBP VSS
//atic const SceDsiTimingInfo stru_BD0A8C = {0,   0x2D5190, &stru_BD0B34, 0x38A5F4, &stru_BD0D4C, 0xA50, 0x465, 0, 0x210, 0x2C,  0x94, 4,   5, 0x24};

static void hdmi_dsi_config_timing_gen(void)
{
	unsigned int hsw, hfp, hbp, vsw, vfp, vbp;
	unsigned char clock_div_by_lanes[] = { 6, 4, 3 }; /* 2, 3, 4 lanes */

	unsigned int lanes = 3;
	unsigned int htotal = 0xA50;
	unsigned int vtotal = 0x465;
	hsw = 0x94;
	hfp = 0x2C;
	hbp = 0x210;
	vsw = 0x24;
	vfp = 4;
	vbp = 5;

	/* set pixel clock divider mode */
	hdmi_i2c_cmd_write_1(HDMI_I2C_CEC_ADDR, 0x16,
		     clock_div_by_lanes[lanes - 2] << 3);

	/* horizontal porch params */
	hdmi_i2c_cmd_write_1(HDMI_I2C_CEC_ADDR, 0x28, htotal >> 4);
	hdmi_i2c_cmd_write_1(HDMI_I2C_CEC_ADDR, 0x29, (htotal << 4) & 0xff);
	hdmi_i2c_cmd_write_1(HDMI_I2C_CEC_ADDR, 0x2a, hsw >> 4);
	hdmi_i2c_cmd_write_1(HDMI_I2C_CEC_ADDR, 0x2b, (hsw << 4) & 0xff);
	hdmi_i2c_cmd_write_1(HDMI_I2C_CEC_ADDR, 0x2c, hfp >> 4);
	hdmi_i2c_cmd_write_1(HDMI_I2C_CEC_ADDR, 0x2d, (hfp << 4) & 0xff);
	hdmi_i2c_cmd_write_1(HDMI_I2C_CEC_ADDR, 0x2e, hbp >> 4);
	hdmi_i2c_cmd_write_1(HDMI_I2C_CEC_ADDR, 0x2f, (hbp << 4) & 0xff);

	/* vertical porch params */
	hdmi_i2c_cmd_write_1(HDMI_I2C_CEC_ADDR, 0x30, vtotal >> 4);
	hdmi_i2c_cmd_write_1(HDMI_I2C_CEC_ADDR, 0x31, (vtotal << 4) & 0xff);
	hdmi_i2c_cmd_write_1(HDMI_I2C_CEC_ADDR, 0x32, vsw >> 4);
	hdmi_i2c_cmd_write_1(HDMI_I2C_CEC_ADDR, 0x33, (vsw << 4) & 0xff);
	hdmi_i2c_cmd_write_1(HDMI_I2C_CEC_ADDR, 0x34, vfp >> 4);
	hdmi_i2c_cmd_write_1(HDMI_I2C_CEC_ADDR, 0x35, (vfp << 4) & 0xff);
	hdmi_i2c_cmd_write_1(HDMI_I2C_CEC_ADDR, 0x36, vbp >> 4);
	hdmi_i2c_cmd_write_1(HDMI_I2C_CEC_ADDR, 0x37, (vbp << 4) & 0xff);
}

static void hdmi_dsi_power_on(void)
{
	static const int use_timing_gen = 1;
	int i;

	hdmi_dsi_config_timing_gen();

	/* set number of dsi lanes */
	hdmi_i2c_cmd_write_1(HDMI_I2C_CEC_ADDR, 0x1c, 3 << 4);

	if (use_timing_gen) {
		/* reset internal timing generator */
		hdmi_i2c_cmd_write_1(HDMI_I2C_CEC_ADDR, 0x27, 0xcb);
		hdmi_i2c_cmd_write_1(HDMI_I2C_CEC_ADDR, 0x27, 0x8b);
		hdmi_i2c_cmd_write_1(HDMI_I2C_CEC_ADDR, 0x27, 0xcb);
	} else {
		/* disable internal timing generator */
		hdmi_i2c_cmd_write_1(HDMI_I2C_CEC_ADDR, 0x27, 0x0b);
	}

	/* enable hdmi */
	hdmi_i2c_cmd_write_1(HDMI_I2C_CEC_ADDR, 0x03, 0x89);
	/* disable test mode */
	hdmi_i2c_cmd_write_1(HDMI_I2C_CEC_ADDR, 0x55, 0x00);

	for (i = 0; i < ARRAY_SIZE(adv7533_cec_fixed_registers); i++)
		hdmi_i2c_cmd_write_1(HDMI_I2C_CEC_ADDR,
				     adv7533_cec_fixed_registers[i].reg,
				     adv7533_cec_fixed_registers[i].cmd);
}

struct display_mode {
	int hdisplay;
	int hsync_start;
	int hsync_end;
	int vdisplay;
	int vsync_start;
	int vsync_end;
	int vrefresh;
	int hsync;
};

static void hdmi_mode_set(unsigned char addr, struct display_mode *mode)
{
	static const embedded_sync = 1;
	unsigned int low_refresh_rate;
	unsigned int hsync_polarity = 0;
	unsigned int vsync_polarity = 0;

	if (embedded_sync) {
		unsigned int hsync_offset, hsync_len;
		unsigned int vsync_offset, vsync_len;

		hsync_offset = mode->hsync_start -
			       mode->hdisplay;
		vsync_offset = mode->vsync_start -
			       mode->vdisplay;
		hsync_len = mode->hsync_end -
			    mode->hsync_start;
		vsync_len = mode->vsync_end -
			    mode->vsync_start;

		/* The hardware vsync generator has a off-by-one bug */
		vsync_offset += 1;

		hdmi_i2c_cmd_write_1(HDMI_I2C_ADDR, ADV7511_REG_HSYNC_PLACEMENT_MSB,
			     ((hsync_offset >> 10) & 0x7) << 5);
		hdmi_i2c_cmd_write_1(HDMI_I2C_ADDR, ADV7511_REG_SYNC_DECODER(0),
			     (hsync_offset >> 2) & 0xff);
		hdmi_i2c_cmd_write_1(HDMI_I2C_ADDR, ADV7511_REG_SYNC_DECODER(1),
			     ((hsync_offset & 0x3) << 6) |
			     ((hsync_len >> 4) & 0x3f));
		hdmi_i2c_cmd_write_1(HDMI_I2C_ADDR, ADV7511_REG_SYNC_DECODER(2),
			     ((hsync_len & 0xf) << 4) |
			     ((vsync_offset >> 6) & 0xf));
		hdmi_i2c_cmd_write_1(HDMI_I2C_ADDR, ADV7511_REG_SYNC_DECODER(3),
			     ((vsync_offset & 0x3f) << 2) |
			     ((vsync_len >> 8) & 0x3));
		hdmi_i2c_cmd_write_1(HDMI_I2C_ADDR, ADV7511_REG_SYNC_DECODER(4),
			     vsync_len & 0xff);

		hsync_polarity = 0; //!(mode->flags & DRM_MODE_FLAG_PHSYNC);
		vsync_polarity = 0; //!(mode->flags & DRM_MODE_FLAG_PVSYNC);
	} else {
		enum adv7511_sync_polarity mode_hsync_polarity;
		enum adv7511_sync_polarity mode_vsync_polarity;

		/**
		 * If the input signal is always low or always high we want to
		 * invert or let it passthrough depending on the polarity of the
		 * current mode.
		 **/
		/*if (mode->flags & DRM_MODE_FLAG_NHSYNC)
			mode_hsync_polarity = ADV7511_SYNC_POLARITY_LOW;
		else
			mode_hsync_polarity = ADV7511_SYNC_POLARITY_HIGH;

		if (mode->flags & DRM_MODE_FLAG_NVSYNC)
			mode_vsync_polarity = ADV7511_SYNC_POLARITY_LOW;
		else
			mode_vsync_polarity = ADV7511_SYNC_POLARITY_HIGH;

		if (adv7511->hsync_polarity != mode_hsync_polarity &&
		    adv7511->hsync_polarity !=
		    ADV7511_SYNC_POLARITY_PASSTHROUGH)
			hsync_polarity = 1;

		if (adv7511->vsync_polarity != mode_vsync_polarity &&
		    adv7511->vsync_polarity !=
		    ADV7511_SYNC_POLARITY_PASSTHROUGH)
			vsync_polarity = 1;*/
	}

	if (mode->vrefresh <= 24000)
		low_refresh_rate = ADV7511_LOW_REFRESH_RATE_24HZ;
	else if (mode->vrefresh <= 25000)
		low_refresh_rate = ADV7511_LOW_REFRESH_RATE_25HZ;
	else if (mode->vrefresh <= 30000)
		low_refresh_rate = ADV7511_LOW_REFRESH_RATE_30HZ;
	else
		low_refresh_rate = ADV7511_LOW_REFRESH_RATE_NONE;

	hdmi_i2c_update_bits(HDMI_I2C_ADDR, 0xfb,
		0x6, low_refresh_rate << 1);
	hdmi_i2c_update_bits(HDMI_I2C_ADDR, 0x17,
		0x60, (vsync_polarity << 6) | (hsync_polarity << 5));
}

#endif

int hdmi_init(void)
{
#if 0
	int i;
	unsigned char buff[4];

	hdmi_i2c_update_bits(HDMI_I2C_ADDR, ADV7511_REG_POWER,
			     ADV7511_POWER_POWER_DOWN, 0);

	/*for (i = 0; i < ARRAY_SIZE(adv7511_fixed_registers); i++)
		hdmi_i2c_cmd_write_1(HDMI_I2C_ADDR,
				     adv7511_fixed_registers[i].reg,
				     adv7511_fixed_registers[i].cmd);*/

	for (i = 0; i < ARRAY_SIZE(adv7533_fixed_registers); i++)
		hdmi_i2c_cmd_write_1(HDMI_I2C_ADDR,
				     adv7533_fixed_registers[i].reg,
				     adv7533_fixed_registers[i].cmd);

	hdmi_i2c_cmd_write_1(HDMI_I2C_ADDR, ADV7511_REG_CEC_I2C_ADDR,
			     HDMI_I2C_CEC_ADDR);

	hdmi_packet_enable(HDMI_I2C_ADDR, 0xffff);

	hdmi_i2c_update_bits(HDMI_I2C_ADDR, ADV7511_REG_POWER2,
			   ADV7511_REG_POWER2_HPD_SRC_MASK,
			   ADV7511_REG_POWER2_HPD_SRC_NONE);

	hdmi_dsi_power_on();


	while (1) {
		unsigned char buff = 0;
		hdmi_i2c_cmd_write_read(HDMI_I2C_ADDR, 0x3E, &buff, 1);
		LOG_HEX(buff);
	}

#endif
#if 0

	hdmi_i2c_update_bits(HDMI_I2C_ADDR, ADV7511_REG_POWER,
			     ADV7511_POWER_POWER_DOWN, 0);


	hdmi_i2c_update_bits(HDMI_I2C_ADDR, 0x98, 0xFF, 0x03);
	hdmi_i2c_update_bits(HDMI_I2C_ADDR, 0x9A, 0b111 << 5, 0b111 << 5);
	hdmi_i2c_update_bits(HDMI_I2C_ADDR, 0x9C, 0xFF, 0x30);
	hdmi_i2c_update_bits(HDMI_I2C_ADDR, 0x9D, 0b11, 0b01);
	hdmi_i2c_update_bits(HDMI_I2C_ADDR, 0xA2, 0xFF, 0xA4);
	hdmi_i2c_update_bits(HDMI_I2C_ADDR, 0xA3, 0xFF, 0xA4);
	hdmi_i2c_update_bits(HDMI_I2C_ADDR, 0xE9, 0xFF, 0xD0);
	hdmi_i2c_update_bits(HDMI_I2C_ADDR, 0xF9, 0xFF, 0x00);

	hdmi_i2c_update_bits(HDMI_I2C_ADDR, 0x15, 0b1111, 0);
	hdmi_i2c_update_bits(HDMI_I2C_ADDR, 0x16, 0b11 << 4, 0b11);
	hdmi_i2c_update_bits(HDMI_I2C_ADDR, 0x16, 0b11 << 2, 0b10);
	hdmi_i2c_update_bits(HDMI_I2C_ADDR, 0x17, 0b1, 1); // 16:9

	hdmi_i2c_update_bits(HDMI_I2C_ADDR, 0x16, 0b11 << 6, 0);
	hdmi_i2c_update_bits(HDMI_I2C_ADDR, 0x18, 0b1 << 7, 0b1 << 7);
	hdmi_i2c_update_bits(HDMI_I2C_ADDR, 0x18, 0b11 << 5 , 0);
	hdmi_i2c_update_bits(HDMI_I2C_ADDR, 0xAF, 0b1 << 1, 1);
	hdmi_i2c_update_bits(HDMI_I2C_ADDR, 0x40, 0b1 << 7, 1);
	hdmi_i2c_update_bits(HDMI_I2C_ADDR, 0x4C, 0b1111, 0b0100);

	//hdmi_i2c_update_bits(HDMI_I2C_ADDR, 0xD6, 0b11<<6, 0b11);

#endif
#if 0

	/* *(volatile unsigned int *)(0xE3103000 + 0x1D0) = 1;
	dsb();

	hdmi_i2c_cmd_write_1(HDMI_I2C_ADDR, 0x92, 0);
	hdmi_i2c_cmd_write_1(HDMI_I2C_ADDR, 0x93, 0);
	hdmi_i2c_cmd_write_1(HDMI_I2C_ADDR, 0x94, 0);
	hdmi_i2c_cmd_write_1(HDMI_I2C_ADDR, 0x95, 0);
	hdmi_i2c_cmd_write_1(HDMI_I2C_ADDR, 0x96, 0);
	hdmi_i2c_cmd_write_1(HDMI_I2C_ADDR, 0x97, 0);

	for (int i = 0; i < ARRAY_SIZE(adv7533_fixed_registers); i++)
		hdmi_i2c_cmd_write_1(HDMI_I2C_ADDR,
				     adv7533_fixed_registers[i].reg,
				     adv7533_fixed_registers[i].cmd);

	hdmi_i2c_update_bits(HDMI_I2C_ADDR, ADV7511_REG_POWER,
			     ADV7511_POWER_POWER_DOWN, 0);


	hdmi_i2c_cmd_write_1(HDMI_I2C_ADDR, ADV7511_REG_CEC_I2C_ADDR,
			     HDMI_I2C_CEC_ADDR);
	*/

	/*
	Main:
	0x42: 0x00000070
	0xD6: 0x00000048
	0xE1: 0x0000007C
	0xFB: 0x000000AA
	CEC:
	0x80: 0x00000004
	0x81: 0x00000007
	0x82: 0x00000013
	0x83: 0x00000057
	0x7F: 0x00000000
	*/

	*(volatile unsigned int *)(0xE3103000 + 0x1D0) = 1;
	dmb();

	delay(2000);

	hdmi_i2c_cmd_write_1(HDMI_I2C_ADDR, ADV7511_REG_POWER,
			      0);


	hdmi_i2c_cmd_write_1(HDMI_I2C_ADDR, 0Xe2, 1);
	delay(1000);
	hdmi_i2c_cmd_write_1(HDMI_I2C_ADDR, 0Xe2, 0);
	hdmi_i2c_cmd_write_1(HDMI_I2C_ADDR, ADV7511_REG_CEC_I2C_ADDR, HDMI_I2C_CEC_ADDR);

	/*hdmi_i2c_cmd_write_1(HDMI_I2C_CEC_ADDR, 0x80, 0x04);
	hdmi_i2c_cmd_write_1(HDMI_I2C_CEC_ADDR, 0x81, 0x07);
	hdmi_i2c_cmd_write_1(HDMI_I2C_CEC_ADDR, 0x7F, 1 << 6);
	hdmi_i2c_cmd_write_1(HDMI_I2C_CEC_ADDR, 0x82, 0x13);

	for (int i = 0; i < ARRAY_SIZE(adv7511_fixed_registers); i++)
		hdmi_i2c_cmd_write_1(HDMI_I2C_ADDR,
				     adv7511_fixed_registers[i].reg,
				     adv7511_fixed_registers[i].cmd);


	for (int i = 0; i < ARRAY_SIZE(adv7533_fixed_registers); i++)
		hdmi_i2c_cmd_write_1(HDMI_I2C_ADDR,
				     adv7533_fixed_registers[i].reg,
				     adv7533_fixed_registers[i].cmd);


	for (int i = 0; i < ARRAY_SIZE(adv7533_cec_fixed_registers); i++)
		hdmi_i2c_cmd_write_1(HDMI_I2C_CEC_ADDR,
				     adv7533_cec_fixed_registers[i].reg,
				     adv7533_cec_fixed_registers[i].cmd);*/

	hdmi_i2c_cmd_write_1(HDMI_I2C_ADDR,  ADV7511_REG_INT_ENABLE(0),
			     ADV7511_INT0_EDID_READY | ADV7511_INT0_HPD);
	hdmi_i2c_cmd_write_1(HDMI_I2C_ADDR, ADV7511_REG_INT_ENABLE(1),
			     ADV7511_INT1_DDC_ERROR);

	hdmi_i2c_cmd_write_1(HDMI_I2C_ADDR, 0xD6, 0b00 << 6);

	hdmi_i2c_cmd_write_1(HDMI_I2C_ADDR, 0xC9, 0x11);

	while (1) {
		unsigned char data;
		hdmi_i2c_cmd_write_read_1(HDMI_I2C_ADDR, 0x42, &data);
		LOG_HEX(data);
		hdmi_i2c_cmd_write_read_1(HDMI_I2C_ADDR, 0xD6, &data);
		LOG_HEX(data);
		hdmi_i2c_cmd_write_read_1(HDMI_I2C_ADDR, 0x96, &data);
		hdmi_i2c_cmd_write_1(HDMI_I2C_ADDR, 0x96, data);
		LOG_HEX(data);
		/*hdmi_i2c_cmd_write_read_1(HDMI_I2C_CEC_ADDR, 0x83, &data);
		LOG_HEX(data);
		hdmi_i2c_cmd_write_read_1(HDMI_I2C_ADDR, ADV7511_REG_INT(0), &data);
		LOG_HEX(data);
		hdmi_i2c_cmd_write_read_1(HDMI_I2C_CEC_ADDR, 0X4E, &data);
		LOG_HEX(data);*/
		LOG("");

		delay(400);
		/*unsigned char buff[4];
		hdmi_i2c_cmd_write_read(HDMI_I2C_CEC_ADDR, 0, &buff[0], 3);
		hdmi_i2c_cmd_write_read(HDMI_I2C_ADDR, 0, &buff[3], 1);
		LOG_HEX(*(unsigned int *)buff);*/
	}
#endif
	return 0;
}

