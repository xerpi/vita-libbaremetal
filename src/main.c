#include <stdio.h>
#include "pervasive.h"
#include "cdram.h"
#include "gpio.h"
#include "i2c.h"
#include "syscon.h"
#include "sysroot.h"
#include "display.h"
#include "ctrl.h"
#include "draw.h"
#include "font.h"
#include "utils.h"
#include "log.h"

int main(struct sysroot_buffer *sysroot)
{
	if (get_cpu_id() != 0) {
		while (1)
			wfe();
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

	if (sysroot_model_is_dolce(sysroot))
		display_init(DISPLAY_TYPE_HDMI);
	else if (sysroot_model_is_vita2k(sysroot))
		display_init(DISPLAY_TYPE_LCD);
	else
		display_init(DISPLAY_TYPE_OLED);

	draw_fill_screen(BLACK);
	draw_rectangle(50, 50, 100, 100, RED);
	draw_rectangle(50 + 150, 50, 100, 100, GREEN);
	draw_rectangle(50 + 2 * 150, 50, 100, 100, BLUE);
	font_draw_string(10, 10, WHITE, "Hello world from baremetal!");

	gpio_set_port_mode(0, GPIO_PORT_GAMECARD_LED, GPIO_PORT_MODE_OUTPUT);
	gpio_port_set(0, GPIO_PORT_GAMECARD_LED);

	LOG("Init done!");

	while (1) {
		static int i = 0;
		unsigned int ctrl_data;

		font_draw_stringf(50, 60, WHITE, "Model: 0x%04X\n", sysroot->model);
		font_draw_stringf(50, 80, WHITE, "Device type: 0x%04X\n", sysroot->device_type);
		font_draw_stringf(50, 100, WHITE, "Device config: 0x%04X\n", sysroot->device_config);
		font_draw_stringf(50, 120, WHITE, "Type: 0x%04X\n", sysroot->type);
		font_draw_stringf(50, 140, WHITE, "UnkD4: 0x%08X\n", sysroot->unkd4[0]);

		ctrl_read(&ctrl_data);
		if (CTRL_BUTTON_HELD(ctrl_data, CTRL_POWER))
			syscon_reset_device(SYSCON_RESET_COLD_RESET, 0);

		if (i++ % 10 < 5)
			gpio_port_set(0, GPIO_PORT_GAMECARD_LED);
		else
			gpio_port_clear(0, GPIO_PORT_GAMECARD_LED);
	}

	return 0;
}
