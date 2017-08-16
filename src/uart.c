#include <stdarg.h>
#include <stdio.h>
#include "uart.h"
#include "utils.h"

#define UART_REG_BASE_ADDR		0xE2030000
#define UARTCLKGEN_REG_BASE_ADDR	0xE3105000

#define UART_REGS(i)			((void *)(UART_REG_BASE_ADDR + (i) * 0x10000))
#define UARTCLKGEN_REGS(i)		((void *)(UARTCLKGEN_REG_BASE_ADDR + (i) * 4))

static const unsigned int baudrate_table[] = {
	300, 0x12710,
	600, 0x11388,
	1200, 0x109C4,
	2400, 0x104E2,
	4800, 0x10271,
	9600, 0x10139,
	14400, 0x100D0,
	19200, 0x1009C,
	28800, 0x10068,
	38400, 0x1004E,
	57600, 0x10034,
	115200, 0x1001A,
	230400, 0x1000D,
	250000, 0x1000C,
	460800, 0x2001A,
	921600, 0x2000D,
	3000000, 0x10001
};

static unsigned int value_for_baudrate(unsigned int baudrate)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(baudrate_table); i += 2) {
		if (baudrate == baudrate_table[i])
			return baudrate_table[i + 1];
	}

	return 0;
}

int uart_init(int bus, unsigned int baudrate)
{
	volatile unsigned int *uart_regs = UART_REGS(bus);
	volatile unsigned int *uartclkgen_regs = UARTCLKGEN_REGS(bus);
	unsigned int value;

	value = value_for_baudrate(baudrate);
	if (!value)
		return -1;

	uart_regs[1] = 0; // disable device

	*uartclkgen_regs = value; // Baudrate = 115200

	uart_regs[8] = 3;
	uart_regs[4] = 1;
	uart_regs[0xC] = 0;
	uart_regs[0x18] = 0x303;
	uart_regs[0x10] = 0;
	uart_regs[0x14] = 0;
	uart_regs[0x19] = 0x10001;

	uart_regs[1] = 1; // enable device

	uart_wait_ready(bus);

	return 0;
}

void uart_wait_ready(int bus)
{
	volatile unsigned int *uart_regs = UART_REGS(bus);

        while (!(uart_regs[0xA] & 0x200))
		dmb();
}

void uart_write(int bus, unsigned int data)
{
	volatile unsigned int *uart_regs = UART_REGS(bus);

	while (!(uart_regs[0xA] & 0x100))
		dmb();

        uart_regs[0x1C] = data;
}

unsigned int uart_read_fifo_data_available(int bus)
{
	return ((unsigned int *)UART_REGS(bus))[0x1A] & 0x3F;
}

unsigned int uart_read(int bus)
{
        unsigned int result;

	volatile unsigned int *uart_regs = UART_REGS(bus);

	while (!(uart_regs[0x1A] << 0x1A))
		dmb();

        result = uart_regs[0x1E];
        uart_regs[0x15] = 0x77F;

        return result;
}

void uart_print(int bus, const char *str)
{
	while (*str) {
		if (*str == '\n')
			uart_write(bus, '\r');

		uart_write(bus, *str++);
	}
}

void uart_printf(int bus, const char *s, ...)
{
	char buf[256];
	va_list argptr;

	va_start(argptr, s);
	vsnprintf(buf, sizeof(buf), s, argptr);
	va_end(argptr);

	uart_print(bus, buf);
}
