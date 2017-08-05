#include "hdmi.h"
#include "libc.h"
#include "utils.h"
#include "i2c.h"
#include "gpio.h"
#include "pervasive.h"
#include "syscon.h"
#include "log.h"

#define HDMI_I2C_BUS	1
#define HDMI_I2C_ADDR	0x7A

/* Configurable addresses */
#define HDMI_I2C_CEC_ADDR	0x7C

/* ADV7533 Main registers */
#define ADV7533_REG_PACKET_ENABLE0		0x40
#define ADV7533_REG_POWER			0x41
#define ADV7533_REG_STATUS			0x42
#define ADV7533_REG_AVI_INFOFRAME(x)		(0x55 + (x))
#define ADV7533_REG_INT(x)			(0x96 + (x))
#define ADV7533_REG_HDCP_HDMI_CFG		0xAF
#define ADV7533_REG_CEC_I2C_ADDR		0xE1
#define ADV7533_REG_POWER2			0xD6

#define ADV7533_PACKET_ENABLE_GC		BIT(7)

#define ADV7533_POWER_POWER_DOWN		BIT(6)

#define ADV7533_INT1_BKSV			BIT(6)

#define ADV7533_STATUS_HPD			BIT(6)

#define ADV7533_REG_POWER2_HPD_SRC_MASK		0xC0
#define ADV7533_REG_POWER2_HPD_SRC_HPD		BIT(6)

/* ADV7533 CEC registers */
#define ADV7533_REG_CEC_DSI_INTERNAL_TIMING	0x27

struct reg_sequence {
	unsigned char reg;
	unsigned char cmd;
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

static void hdmi_write(unsigned char addr, unsigned char reg, unsigned char data)
{
	unsigned char buffer[2] = {
		reg, data
	};

	i2c_transfer_write(HDMI_I2C_BUS, addr, buffer, sizeof(buffer));
}

static unsigned char hdmi_read(unsigned char addr, unsigned char reg)
{
	unsigned char read_buffer;
	unsigned char write_buffer = reg;

	i2c_transfer_write_read(HDMI_I2C_BUS, addr, &write_buffer, sizeof(write_buffer),
					      addr, &read_buffer, sizeof(read_buffer));

	return read_buffer;
}

static void hdmi_set_bit(unsigned char addr, unsigned char reg, unsigned char bit)
{
	unsigned char val;
	val = hdmi_read(addr, reg);
	val |= bit;
	hdmi_write(addr, reg, val);
}

static void hdmi_clr_bit(unsigned char addr, unsigned char reg, unsigned char bit)
{
	unsigned char val;
	val = hdmi_read(addr, reg);
	val &= ~bit;
	hdmi_write(addr, reg, val);
}

static void hdmi_update_bits(unsigned char addr, unsigned char reg,
			  unsigned char mask, unsigned char val)
{
	unsigned char data = hdmi_read(addr, reg);
	data &= ~mask;
	data |= val & mask;
	hdmi_write(addr, reg, data);
}

int hdmi_init(void)
{
	static const unsigned char clock_div_by_lanes[] = { 6, 4, 3 }; /* 2, 3, 4 lanes */
	static const unsigned int lanes = 3;

	int i;
	unsigned char status;

	pervasive_hdmi_cec_set_enabled(1);

	syscon_set_hdmi_cdc_hpd(1);

	gpio_set_port_mode(0, GPIO_PORT_HDMI_BRIDGE,
			   GPIO_PORT_MODE_OUTPUT);
	gpio_port_clear(0, GPIO_PORT_HDMI_BRIDGE);

	hdmi_write(HDMI_I2C_ADDR, ADV7533_REG_CEC_I2C_ADDR,
		     HDMI_I2C_CEC_ADDR);

	do {
		status = hdmi_read(HDMI_I2C_ADDR, ADV7533_REG_STATUS);
	} while (!(status & ADV7533_STATUS_HPD));

	hdmi_update_bits(HDMI_I2C_ADDR, ADV7533_REG_POWER,
			   ADV7533_POWER_POWER_DOWN, 0);

	hdmi_update_bits(HDMI_I2C_ADDR, ADV7533_REG_POWER2,
			   ADV7533_REG_POWER2_HPD_SRC_MASK,
			   ADV7533_REG_POWER2_HPD_SRC_HPD);

	for (i = 0; i < ARRAY_SIZE(adv7533_fixed_registers); i++)
		hdmi_write(HDMI_I2C_ADDR, adv7533_fixed_registers[i].reg,
			   adv7533_fixed_registers[i].cmd);

	for (i = 0; i < ARRAY_SIZE(adv7533_cec_fixed_registers); i++)
		hdmi_write(HDMI_I2C_CEC_ADDR, adv7533_cec_fixed_registers[i].reg,
			   adv7533_cec_fixed_registers[i].cmd);

	/* Set number of dsi lanes */
	hdmi_write(HDMI_I2C_CEC_ADDR, 0x1C, lanes << 4);

	/* Set pixel clock divider mode */
	hdmi_write(HDMI_I2C_CEC_ADDR, 0x16, clock_div_by_lanes[lanes - 2] << 3);

	/* Disable internal timing generator */
	hdmi_write(HDMI_I2C_CEC_ADDR, ADV7533_REG_CEC_DSI_INTERNAL_TIMING, 0x0B);

	/* GC Packet Enable */
	hdmi_write(HDMI_I2C_ADDR, ADV7533_REG_PACKET_ENABLE0, ADV7533_PACKET_ENABLE_GC);

	/* Down Dither Output Colour Depth - 8 Bit (default) */
	hdmi_write(HDMI_I2C_ADDR, 0x49, 0x00);

	/*
	 * HDMI Output Settings
	 */
	/* Set HDMI/DVI Mode Select = HDMI Mode Enabled - bit 1*/
	hdmi_set_bit(HDMI_I2C_ADDR, ADV7533_REG_HDCP_HDMI_CFG, BIT(1));

	/* Set Color Depth to 24 Bits/Pixel */
	hdmi_clr_bit(HDMI_I2C_ADDR, 0x4C, 0x0F);
	hdmi_set_bit(HDMI_I2C_ADDR, 0x4C, 0x04);

	/* Set Active Format Aspect Ratio = 16:9 (Center) */
	hdmi_clr_bit(HDMI_I2C_ADDR, ADV7533_REG_AVI_INFOFRAME(1), 0x1F);
	hdmi_set_bit(HDMI_I2C_ADDR, ADV7533_REG_AVI_INFOFRAME(1), 0x2A);

	/* Set V1P2 Enable = +1.2V*/
	hdmi_set_bit(HDMI_I2C_ADDR, 0xE4, 0xC0);

	/* Disable test mode */
	hdmi_clr_bit(HDMI_I2C_CEC_ADDR, 0x55, BIT(7));

	/* Enable hdmi */
	hdmi_write(HDMI_I2C_CEC_ADDR, 0x03, 0x89);

	/* Set HDCP */
	hdmi_write(HDMI_I2C_ADDR, ADV7533_REG_HDCP_HDMI_CFG, 0x96);

	/* Wait for the BKSV flag */
	do {
		status = hdmi_read(HDMI_I2C_ADDR, ADV7533_REG_INT(1));
	} while (!(status & ADV7533_INT1_BKSV));

	hdmi_write(HDMI_I2C_ADDR, ADV7533_REG_INT(1),
		   (status & (unsigned char)~0xBF) | BIT(6));

	return 0;
}
