#include "pervasive.h"
#include "cdram.h"
#include "gpio.h"
#include "i2c.h"
#include "spi.h"
#include "dsi.h"
#include "display.h"
#include "syscon.h"
#include "hdmi.h"
#include "ctrl.h"
#include "utils.h"
#include "log.h"

static unsigned int get_cpu_id(void);

int main(void)
{
	if (get_cpu_id() != 0) {
		while (1)
			asm volatile("wfe\n\t");
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

	cdram_enable();
	i2c_init_bus(1);
	spi_init(0);
	syscon_init();

	pervasive_dsi_set_pixelclock(1, 0x2D45F9);
	pervasive_clock_enable_dsi(1, 0xF);
	pervasive_reset_exit_dsi(1, 7);

	dsi_init();
	dsi_enable_bus(1, 0x8600);
	delay(1000);

	display_init(1);

	dsi_unk(1, 0x8600, 0);

	hdmi_init();

	gpio_set_port_mode(0, GPIO_PORT_GAMECARD_LED, GPIO_PORT_MODE_OUTPUT);
	gpio_port_set(0, GPIO_PORT_GAMECARD_LED);

	LOG("Init done!");

	while (1) {
		static int i = 0;
		unsigned int ctrl_data;

		ctrl_read(&ctrl_data);
		if (CTRL_BUTTON_HELD(ctrl_data, CTRL_PSBUTTON))
			syscon_reset_device(SYSCON_RESET_COLD_RESET, 0);

		if (i++ % 1000 < 500)
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
