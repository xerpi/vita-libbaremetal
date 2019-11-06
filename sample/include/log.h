#ifndef LOG_H
#define LOG_H

#include <baremetal/uart.h>

#define LOG(...)	uart_printf(0, __VA_ARGS__)
#define LOG_HEX(x)	uart_printf(0, "0x%08X\n", x)

static inline void LOG_BUFFER(const char *name, const void *buff, int size)
{
	int i;

	LOG(name);
	for (i = 0; i < size; i++)
		LOG(" %02X", ((unsigned char *)buff)[i]);
	LOG("\n");
}

#endif
