#include "pervasive.h"
#include "cdram.h"
#include "gpio.h"
#include "i2c.h"
#include "spi.h"
#include "dsi.h"
#include "iftu.h"
#include "syscon.h"
#include "hdmi.h"
#include "utils.h"
#include "log.h"

static unsigned int get_cpu_id(void);

#if 0
static int hdmi_dsi_setup()
{
	pervasive_dsi_set_pixeclock(1, 0x223A1C);
	pervasive_clock_enable_dsi(1, 0xF);
	pervasive_reset_exit_dsi(1, 7);

	dsi_enable_bus(1);

	static struct iftu_plane_info plane = {
		.pixelformat = 0x10, // A8B8G8R8
		.width = 960,
		.height = 544,
		.leftover_stride = 0,
		.unk10 = 0, // always 0
		.paddr = 0x40200000,
		.unk18 = 0, // always 0
		.unk1C = 0, // always 0
		.unk20 = 0, // always 0
		.src_x = 0, // in (0x100000 / 960) multiples
		.src_y = 0, // in (0x100000 / 544) multiples
		.src_w = 0x100000, // in (0x100000 / 960) multiples
		.src_h = 0x100000, // in (0x100000 / 544) multiples
		.dst_x = 0,
		.dst_y = 0,
		.dst_w = 960,
		.dst_h = 544,
		.vfront_porch = 0,
		.vback_porch = 0,
		.hfront_porch = 0,
		.hback_porch = 0,
	};

	iftu_set_plane(0, &plane);

	iftu_crtc_enable(1);

	hdmi_init();
}
#endif

int main(void)
{
	if (get_cpu_id() != 0) {
		while (1)
			;
	}

	pervasive_clock_enable_gpio();
	pervasive_reset_exit_gpio();
	pervasive_clock_enable_uart(0);
	pervasive_reset_exit_uart(0);
	pervasive_clock_enable_i2c(1);
	pervasive_reset_exit_i2c(1);
	pervasive_clock_enable_spi(0);
	pervasive_reset_exit_spi(0);

	uart_init(0, 115200);

	LOG("Baremetal payload started!");

	LOG_HEX(*(unsigned int *)(0xE3103000 + 0x124));
	LOG_HEX(pervasive_read_misc(0x0000));

	cdram_enable();
	i2c_init_bus(1);
	spi_init(0);
	syscon_init();

	static const struct iftu_csc_params csc1_plane2 = {0, 0, 0x3FF, 0,    0,     0,    0x254, 0, 0x395, 0x254, 0xF93, 0xEF0, 0x254, 0x439, 0};
	static const struct iftu_csc_params csc1_plane3 = {0x40, 0x202, 0x3FF, 0,    0,     0,    0x254, 0, 0x395, 0x254, 0xF93, 0xEF0, 0x254, 0x439, 0};
	static const struct iftu_csc_params csc2 = {0x40, 0x40,  0x3AC, 0x40, 0x3AC, 0x40, 0x1B7, 0, 0,     0,     0x1B7, 0,     0,     0,     0x1B7};

	iftu_set_csc1(2, &csc1_plane2);
	iftu_set_csc1(3, &csc1_plane3);
	iftu_set_csc2(2, &csc2);
	iftu_set_csc2(3, &csc2);
	iftu_set_control_value(3, 4);
	iftu_set_alpha(2, 0);
	iftu_set_alpha(3, 0xFF);

	static struct iftu_plane_source_fb_info plane_fb = {
		.pixelformat = 0x10, // A8B8G8R8
		.width = 1080,
		.height = 720,
		.leftover_stride = 0,
		.unk10 = 0, // always 0
		.paddr = 0x21000000,
		.unk18 = 0, // always 0
		.unk1C = 0, // always 0
		.unk20 = 0, // always 0
		.src_x = 0, // in (0x10000 / 960) multiples
		.src_y = 0, // in (0x10000 / 544) multiples
		.src_w = 0x10000, // in (0x10000 / 960) multiples
		.src_h = 0x10000, // in (0x10000 / 544) multiples
		.dst_x = 0,
		.dst_y = 0,
		.dst_w = 1080,
		.dst_h = 720,
		.vfront_porch = 0,
		.vback_porch = 0,
		.hfront_porch = 0,
		.hback_porch = 0,
	};

	memset((void *)0x21000000, 0xFF, 1080*720*4);

	//iftu_set_plane(0, &plane_fb);
	//iftu_set_plane(1, &plane_fb);
	iftu_set_source_fb(2, &plane_fb);
	iftu_set_source_fb(3, &plane_fb);
	iftu_crtc_enable(1);

	delay(1000);

	pervasive_dsi_set_pixelclock(1, 0x2D45F9);
	pervasive_clock_enable_dsi(1, 0xF);
	pervasive_reset_exit_dsi(1, 7);

	dsi_init();
	dsi_enable_bus(1, 0x8600);

	//iftu_set_dst_conversion(2, 1080, 720, 0);
	//iftu_set_dst_conversion(3, 1080, 720, 0);
	iftu_init_plane(2);
	iftu_init_plane(3);

	dsi_unk(1, 0x8600, 0);

	hdmi_init();

	gpio_set_port_mode(0, GPIO_PORT_GAMECARD_LED, GPIO_PORT_MODE_OUTPUT);
	gpio_port_set(0, GPIO_PORT_GAMECARD_LED);

	LOG("Init done!");

	while (1) {
		static unsigned int i = 0;
		/*uart_write(0, '0' + (i % 10));
		uart_write(0, '\n');
		uart_write(0, '\r');*/

		if (i++ % 100000 < 50000)
			gpio_port_set(0, GPIO_PORT_GAMECARD_LED);
		else
			gpio_port_clear(0, GPIO_PORT_GAMECARD_LED);
	}

	return 0;
}

static unsigned int get_cpu_id(void)
{
	unsigned int mpidr;
	asm volatile("mrc p15, 0, %0, c0, c0, 5\n\t" : "=r"(mpidr));
	return mpidr & 0xF;
}
