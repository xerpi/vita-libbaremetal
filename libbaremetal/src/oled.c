#include "oled.h"
#include "pervasive.h"
#include "spi.h"
#include "gpio.h"
#include "utils.h"

/* TODO: Remove this */
#define SPI_BASE_ADDR	0xE0A00000
#define SPI_REGS(i)	((void *)(SPI_BASE_ADDR + (i) * 0x10000))

#define DELAY 0x0D
#define END 0xFF

struct oled_cmd {
	uint8_t cmd;
	uint8_t size;
	uint8_t data[];
};

static const uint8_t oled_init_cmdlist1[] = {
	0xF0, 2, 0x5A, 0x5A,
	0xF1, 2, 0x5A, 0x5A,
	0xB0, 1, 2,
	0xF3, 1, 0x3B,
	0xF4, 4, 0x33, 0x42, 0, 8,
	0xF5, 5, 0, 6, 0x26, 0x35, 3,
	0xF6, 1, 2,
	0xC6, 0xA, 0xB, 0, 0, 0x3C, 0, 0x22, 0, 0, 0, 0,
	0xF7, 1, 0x20,
	END, END
};

static const uint8_t oled_init_cmdlist2[] = {
	0xB2, 4, 0xB, 0xB, 0xB, 0xB,
	0xB1, 3, 7, 0, 0x16,
	0xF8, 0x13, 0x7F, 0x7A, 0x89, 0x67, 0x26, 0x38, 0, 0, 9, 0x67, 0x70, 0x88, 0x7A, 0x76, 5, 9, 0x23, 0x23, 0x23,
	END, END
};

static const uint8_t oled_init_cmdlist3[] = {
	0xF5, 5, 0, 6, 0x27, 0x35, 3,
	0xB2, 4, 6, 6, 6, 6,
	0xB1, 3, 7, 0, 0x10,
	0xF8, 0x13, 0x7F, 0x7A, 0x89, 0x67, 0x26, 0x38, 0, 0, 9, 0x67, 0x70, 0x88, 0x7A, 0x76, 5, 9, 0x23, 0x23, 0x23,
	END, END
};

static const uint8_t oled_disp_on_cmdlist[] = {
	DELAY, 4,
	0x11, 0,
	DELAY, 0x0D,
	0x29, 0,
	DELAY, 1,
	END, END
};

static const uint8_t oled_disp_off_cmdlist[] = {
	0x28, 0,
	DELAY, 1,
	0x10, 0,
	DELAY, 6,
	END, END
};

static const uint8_t oled_color_space_mode_0_cmdlist[] = {
	0xB3, 1, 0,
	END, END
};

static const uint8_t oled_color_space_mode_1_cmdlist[] = {
	0xB3, 1, 1,
	END, END
};

static const uint8_t oled_brightness_cmdlist[] = {
	0xF9, 0x16, 1, 0x79, 0x78, 0x8D, 0xD9, 0xDF, 0xD5, 0xCB, 0xCF, 0xC5, 0xE5, 0xE0, 0xE4, 0xDC, 0xB8, 0xD4, 0xFA, 0xED, 0xE6, 0x2F, 0, 0x2F,
	0xF9, 1, 0,
	0x26, 1, 0,
	0xB2, 1, 0x15,
	END, END
};

static void oled_write_cmd(const struct oled_cmd *cmd)
{
	uint32_t i;
	volatile uint32_t *spi_regs = SPI_REGS(2);
	uint32_t tmpbuff = 0;
	uint32_t tmpbuff_size = 0;

	pervasive_clock_enable_spi(2);

	while (spi_regs[0xA])
		spi_regs[0];

	spi_regs[2] = 0x30001;
	spi_regs[4] = 1;

	tmpbuff |= ((rbit(cmd->cmd) >> 24) << 1) << tmpbuff_size;
	tmpbuff_size += 9;

	while (tmpbuff_size > 15) {
		while (spi_regs[0xB] == 0x7F)
			;

		spi_regs[1] = tmpbuff;

		tmpbuff >>= 16;
		tmpbuff_size -= 16;
	}

	for (i = 0; i < cmd->size; i++) {
		uint8_t data = rbit(cmd->data[i]) >> 24;

		tmpbuff |= ((data << 1) | 1) << tmpbuff_size;
		tmpbuff_size += 9;

		while (tmpbuff_size > 15) {
			while (spi_regs[0xB] == 0x7F)
				;

			spi_regs[1] = tmpbuff;

			tmpbuff >>= 16;
			tmpbuff_size -= 16;
		}
	}

	while (tmpbuff_size > 0) {
		tmpbuff |= ((rbit(0) >> 24) << 1) << tmpbuff_size;
		tmpbuff_size += 9;

		while (tmpbuff_size > 15) {
			while (spi_regs[0xB] == 0x7F)
				;

			spi_regs[1] = tmpbuff;

			tmpbuff >>= 16;
			tmpbuff_size -= 16;
		};
	}

	while (spi_regs[0xB] && spi_regs[0xB])
		;

	while (spi_regs[0xA])
		spi_regs[0];

	spi_regs[4] = 0;

	while (spi_regs[4] & 1)
		;

	spi_regs[2] = 0x30001;
	spi_regs[2];
	dsb();

	pervasive_clock_disable_spi(2);
}

static void oled_write_cmdlist(const struct oled_cmd *cmdlist)
{
	const struct oled_cmd *cmd = cmdlist;
	int delay_count = 0;

	while (cmd) {
		if (delay_count <= 0) {
			if (cmd->cmd == END) {
				break;
			} else if (cmd->cmd == DELAY) {
				delay_count = cmd->size - 1;
				cmd++;
			} else {
				oled_write_cmd(cmd);
				cmd = (struct oled_cmd *)&cmd->data[cmd->size];
			}
		} else {
			delay_count--;
		}
		delay(16000 / 160);
	}
}

int oled_init(void)
{
	spi_init(2);
	pervasive_clock_disable_spi(2);

	gpio_set_port_mode(0, GPIO_PORT_OLED_LCD, GPIO_PORT_MODE_OUTPUT);
	gpio_port_set(0, GPIO_PORT_OLED_LCD);

	oled_write_cmdlist((struct oled_cmd *)oled_init_cmdlist1);
	oled_write_cmdlist((struct oled_cmd *)oled_init_cmdlist3);
	oled_write_cmdlist((struct oled_cmd *)oled_disp_on_cmdlist);
	oled_write_cmdlist((struct oled_cmd *)oled_color_space_mode_0_cmdlist);
	oled_write_cmdlist((struct oled_cmd *)oled_brightness_cmdlist);

	return 0;
}
