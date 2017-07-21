#include "lowio.h"
#include "spi.h"
#include "syscon.h"
#include "hdmi.h"
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

	gpio_set_port_mode(0, GPIO_PORT_HDMI_BRIDGE, GPIO_PORT_MODE_OUTPUT);
	gpio_port_set(0, GPIO_PORT_HDMI_BRIDGE);
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

	i2c_init_bus(1);
	spi_init(0);
	syscon_init();

	gpio_set_port_mode(0, GPIO_PORT_GAMECARD_LED, GPIO_PORT_MODE_OUTPUT);
	gpio_port_set(0, GPIO_PORT_GAMECARD_LED);

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
