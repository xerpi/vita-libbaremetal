#include "pervasive.h"
#include "cdram.h"
#include "gpio.h"
#include "i2c.h"
#include "spi.h"
#include "dsi.h"
#include "syscon.h"
#include "hdmi.h"
#include "display.h"
#include "ctrl.h"
#include "draw.h"
#include "font.h"
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

	uart_init(0, 115200);

	LOG("Baremetal payload started!");

	cdram_enable();
	i2c_init_bus(1);
	syscon_init();

	if (0)
		display_init(DISPLAY_TYPE_HDMI);
	else
		display_init(DISPLAY_TYPE_OLED);

	draw_fill_screen(WHITE);
	draw_rectangle(50, 50, 100, 100, RED);
	draw_rectangle(50 + 150, 50, 100, 100, GREEN);
	draw_rectangle(50 + 2 * 150, 50, 100, 100, BLUE);
	font_draw_string(10, 10, BLACK, "Hello world from baremetal!");

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
