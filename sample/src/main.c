#include <stdio.h>
#include <string.h>
#include <baremetal/pervasive.h>
#include <baremetal/cdram.h>
#include <baremetal/gpio.h>
#include <baremetal/i2c.h>
#include <baremetal/syscon.h>
#include <baremetal/sysroot.h>
#include <baremetal/display.h>
#include <baremetal/ctrl.h>
#include <baremetal/touch.h>
#include <baremetal/msif.h>
#include <baremetal/draw.h>
#include <baremetal/font.h>
#include <baremetal/utils.h>
#include "log.h"

static const unsigned char msif_key[32] = {
	0 /* Bring your own keys */
};

int main(struct sysroot_buffer *sysroot, unsigned int cpu_id)
{
	if (cpu_id != 0) {
		while (1)
			wfe();
	}

	sysroot_init(sysroot);

	pervasive_clock_enable_gpio();
	pervasive_reset_exit_gpio();
	pervasive_clock_enable_uart(0);
	pervasive_reset_exit_uart(0);
	pervasive_clock_enable_i2c(1);
	pervasive_reset_exit_i2c(1);

	uart_init(0, 115200);

	LOG("Baremetal payload started!\n");

	cdram_enable();
	i2c_init_bus(1);
	syscon_init();

	if (sysroot_model_is_dolce())
		display_init(DISPLAY_TYPE_HDMI);
	else if (sysroot_model_is_vita2k())
		display_init(DISPLAY_TYPE_LCD);
	else
		display_init(DISPLAY_TYPE_OLED);

	if (pervasive_msif_get_card_insert_state() && (msif_key[0] != 0)) {
		msif_init();
		syscon_msif_set_power(1);
		msif_setup(msif_key);
		LOG("MS auth done!\n");

		unsigned char sector[MS_SECTOR_SIZE];
		msif_read_sector(0, sector);
		LOG_BUFFER("MBR:", sector, MS_SECTOR_SIZE);
	}

	gpio_set_port_mode(0, GPIO_PORT_GAMECARD_LED, GPIO_PORT_MODE_OUTPUT);
	gpio_port_set(0, GPIO_PORT_GAMECARD_LED);

	ctrl_set_analog_sampling(1);

	touch_init();
	touch_configure(TOUCH_PORT_FRONT | TOUCH_PORT_BACK,
	                TOUCH_MAX_REPORT_FRONT,
	                TOUCH_MAX_REPORT_BACK);
	touch_set_sampling_cycle(TOUCH_PORT_FRONT | TOUCH_PORT_BACK, 0xFF, 0xFF);

	LOG("Init done!\n");

	draw_fill_screen(BLACK);

	draw_rectangle(50, 50, 100, 100, RED);
	draw_rectangle(50 + 150, 50, 100, 100, GREEN);
	draw_rectangle(50 + 2 * 150, 50, 100, 100, BLUE);
	font_draw_string(10, 10, WHITE, "Hello world from baremetal!");
	font_draw_stringf(50, 60, WHITE, "Model: 0x%04X", sysroot->model);
	font_draw_stringf(50, 80, WHITE, "Device type: 0x%04X", sysroot->device_type);
	font_draw_stringf(50, 100, WHITE, "Device config: 0x%04X", sysroot->device_config);
	font_draw_stringf(50, 120, WHITE, "Type: 0x%04X", sysroot->type);
	font_draw_stringf(50, 140, WHITE, "HW Info: 0x%08X", sysroot->hw_info);

	font_draw_stringf(50, 180, WHITE, "Memory Card inserted: %d",
		pervasive_msif_get_card_insert_state());

	while (1) {
		static int i = 0;

		struct ctrl_data ctrl;
		ctrl_read(&ctrl);

		draw_rectangle(960/2 + ctrl.lx - 128, 544/2 + ctrl.ly - 128 - 4, 8, 8, ORANGE);
		draw_rectangle(960/2 + ctrl.rx - 128, 544/2 + ctrl.ry - 128 - 4, 8, 8, CYAN);

		static const uint32_t colors[] = {
			RED, GREEN, BLUE, YELLOW, WHITE, CYAN, ORANGE, PINK, LIME, PURPLE,
		};

		struct touch_data touch;
		touch_read(TOUCH_PORT_FRONT | TOUCH_PORT_BACK, &touch);

		for (int i = 0; i < touch.num_front; i++) {
			draw_rectangle((touch.front[i].x * 960) / 1920,
			               (touch.front[i].y * 544) / 1088,
			               6, 6,
				       colors[i % (sizeof(colors) / sizeof (*colors))]);
		}

		for (int i = 0; i < touch.num_back; i++) {
			draw_rectangle((touch.back[i].x * 960) / 1920,
			               (touch.back[i].y * 544) / 1088,
			               6, 6,
			               colors[(touch.num_front + i) % (sizeof(colors) / sizeof (*colors))]);
		}

		font_draw_stringf(670, 10, WHITE, "%02d %03d %04d %04d",
			touch.front[0].id, touch.front[0].force, touch.front[0].x, touch.front[0].y);

		if (CTRL_BUTTON_HELD(ctrl.buttons, CTRL_POWER))
			break;

		if (i++ % 10 < 5)
			gpio_port_set(0, GPIO_PORT_GAMECARD_LED);
		else
			gpio_port_clear(0, GPIO_PORT_GAMECARD_LED);
	}

	syscon_reset_device(SYSCON_RESET_TYPE_COLD_RESET, 0);

	return 0;
}
