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

////////////////////////////////////
#include <stdarg.h>
#define SCREEN_W 960
#define SCREEN_H 544

int cns_x = 10;
int cns_y = 10;

void console_putc(char c)
{
	int last_y = cns_y;
	if (c == '\n') {
		cns_y += 20;
		cns_x = 10;
	} else if (c == '\t') {
		cns_x += 16*4;
	} else if (c >= ' ' && c <= 126) {
		font_draw_char(cns_x, cns_y, WHITE, c);
		cns_x += 16;
	}
	if (cns_x >= (SCREEN_W-16)) {
		cns_y += 20;
		cns_x = 10;
	}
	if (cns_y >= (SCREEN_H-16)) {
		cns_y = 10;
	}
	if (cns_y != last_y) {
		draw_rectangle(0, cns_y, SCREEN_W, 20, BLACK);
		if ((cns_y+20+16) < SCREEN_H) {
			draw_rectangle(0, cns_y+20, SCREEN_W, 20, BLACK);
			if ((cns_y+20+20+16) < SCREEN_H) {
				draw_rectangle(0, cns_y+40, SCREEN_W, 20, BLACK);
			}
		}
	}
}

void console_print(const char *s)
{
	while (*s) {
		console_putc(*s);
		s++;
	}
}

void console_printf(const char *s, ...)
{
	char buf[128];
	va_list argptr;
	va_start(argptr, s);
	vsnprintf(buf, sizeof(buf), s, argptr);
	va_end(argptr);
	console_print(buf);
}

void PRINT_BUF(const char *name, const void *buff, int size)
{
	console_print(name);
	for (int i = 0; i < size; i++)
		console_printf(" %02X", ((unsigned char *)buff)[i]);
	console_putc('\n');
}

extern void console_printf(const char *s, ...);
#define PRINT(...) console_printf(__VA_ARGS__)
//////////////////////////////////

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

	LOG("Init done!\n");

	draw_fill_screen(BLACK);
	/*draw_rectangle(50, 50, 100, 100, RED);
	draw_rectangle(50 + 150, 50, 100, 100, GREEN);
	draw_rectangle(50 + 2 * 150, 50, 100, 100, BLUE);
	font_draw_string(10, 10, WHITE, "Hello world from baremetal!");*/

	//PRINT("Init\n\n");

	touch_init();
	touch_enable2();
	touch_enable4();

	// Enable analog sticks sampling
	ctrl_set_analog_sampling(1);

	//unsigned char buffer0[64];
	//memset(buffer0, 0, sizeof(buffer0));
	//touch_read(buffer0);
	//touch_enable3();

	while (1) {
		static int i = 0;
		struct ctrl_data ctrl;

		ctrl_read(&ctrl);

		/*font_draw_stringf(50, 60, WHITE, "Model: 0x%04X", sysroot->model);
		font_draw_stringf(50, 80, WHITE, "Device type: 0x%04X", sysroot->device_type);
		font_draw_stringf(50, 100, WHITE, "Device config: 0x%04X", sysroot->device_config);
		font_draw_stringf(50, 120, WHITE, "Type: 0x%04X", sysroot->type);
		font_draw_stringf(50, 140, WHITE, "HW Info: 0x%08X", sysroot->hw_info);

		font_draw_stringf(50, 180, WHITE, "Memory Card inserted: %d",
			pervasive_msif_get_card_insert_state());*/

		//font_draw_stringf(10, 200, WHITE, "Buttons: 0x%08X", ctrl);

		/*struct syscon_touchpanel_device_info info;
		memset(&info, 0, sizeof(info));
		syscon_get_touchpanel_device_info(&info);

		PRINT("Info 0x%04X 0x%04X 0x%04X 0x%04X\n",
			info.front_vendor_id, info.front_fw_version,
			info.back_vendor_id, info.back_fw_version);*/

		draw_rectangle(960/2 + ctrl.lx - 128, 544/2 + ctrl.ly - 128, 10, 10, RED);
		draw_rectangle(960/2 + ctrl.rx - 128, 544/2 + ctrl.ry - 128, 10, 10, BLUE);

		unsigned char buffer1[128];
		memset(buffer1, 0, sizeof(buffer1));
		touch_read(buffer1);

		static struct touch_report reports[20];
		for (int i = 0, offset = 0; i < 10 + 10; i++, offset = i * 6) {
			reports[i].id = buffer1[offset + 0];
			reports[i].force = buffer1[offset + 5];
			reports[i].x = buffer1[offset + 1] | (((uint16_t)buffer1[offset + 2] & 7) << 8);
			reports[i].y = buffer1[offset + 3] | (((uint16_t)buffer1[offset + 4] & 7) << 8);
		}

		static const uint32_t colors[] = {
			RED,
			GREEN,
			BLUE,
			YELLOW,
			CYAN,
			PINK,
			LIME,
			PURP,
			WHITE,
			BLACK,
		};

		for (int i = 0; i < 6; i++) {
			if (!(reports[i].id & 0x80)) {
				draw_rectangle((reports[i].x * 960) / 1920, (reports[i].y * 544) / 1088, 5, 5,
					colors[i % (sizeof(colors) / sizeof (*colors))]);
			}
		}
		for (int i = 10; i < 14; i++) {
			if (!(reports[i].id & 0x80)) {
				draw_rectangle((reports[i].x * 960) / 1920, 108 + (reports[i].y * 544) / (890 - 108), 5, 5,
					colors[(6 + i) % (sizeof(colors) / sizeof (*colors))]);
			}
		}

		font_draw_stringf(50, 80, WHITE, "%02X %02X %04X %04X", reports[0].id,
			reports[0].force, reports[0].x, reports[0].y);

#if 0

		uint8_t id = buffer1[0];
		uint8_t force = buffer1[5];
		unsigned short x = buffer1[1] | ((uint16_t)(buffer1[2] & 7) << 8);
		unsigned short y = buffer1[3] | ((uint16_t)(buffer1[4] & 7) << 8);
		//if (!(id & 0x80))
		//	PRINT("(%02X, %04X, %04X)\n", id, x, y);
		//PRINT_BUF("", buffer1, 6);
		//touch_enable4();

		uint32_t color = WHITE;
		if (CTRL_BUTTON_HELD(ctrl, CTRL_L))
			color = RED;
		else if (CTRL_BUTTON_HELD(ctrl, CTRL_R))
			color = GREEN;

		font_draw_stringf(50, 80, WHITE, "%02X %02X %04X %04X", id, force, x, y);
		/* Touchscreen: 1920x1088 */
		if (!(id & 0x80))
			draw_rectangle((x * 960) / 1920, (y * 544) / 1088, 5, 5, color);
#endif

#if 0
		unsigned char buffer2[64];
		memset(buffer2, 0, sizeof(buffer2));
		touch_read2(buffer2);

		for (int i = 0; i < 24; i++)
			font_draw_stringf(10 + (32 + 8) * i, 300, WHITE, "%02X", buffer2[i]);

		for (int i = 24; i < 48; i++)
			font_draw_stringf(10 + (32 + 8) * (i-24), 320, WHITE, "%02X", buffer2[i]);


		unsigned char buffer3[64];
		memset(buffer3, 0, sizeof(buffer3));
		touch_read4(buffer3);

		for (int i = 0; i < 24; i++)
			font_draw_stringf(10 + (32 + 8) * i, 360, WHITE, "%02X", buffer3[i]);

		for (int i = 24; i < 48; i++)
			font_draw_stringf(10 + (32 + 8) * (i-24), 380, WHITE, "%02X", buffer3[i]);
#endif

#if 0

		struct syscon_packet packet;
		unsigned int touch;

		//ksceSysconCtrlDeviceReset(0xC, 0);
		ksceSysconCtrlDeviceReset(0xc, 1);

		syscon_common_write(0, 0x03a7, 1);

		syscon_common_write(0x0101, 0x088e, 3);

		//syscon_common_write(0, 0x0301, 1);
		syscon_common_write(0, 0x0302, 1);

		//syscon_common_read(&touch, 0x387);
		memset(&packet, 0, sizeof(packet));
		syscon_packet_send(&packet, 0x04000600, 0x0381, 5);
		for (int i = 0; i < 10; i++)
			font_draw_stringf(50 + (32 + 8) * i, 220, WHITE, "%02X", packet.rx[i]);
#endif
		if (CTRL_BUTTON_HELD(ctrl.buttons, CTRL_POWER))
			syscon_reset_device(SYSCON_RESET_TYPE_COLD_RESET, 0);

		if (i++ % 10 < 5)
			gpio_port_set(0, GPIO_PORT_GAMECARD_LED);
		else
			gpio_port_clear(0, GPIO_PORT_GAMECARD_LED);
	}

	return 0;
}
