#ifndef LOG_H
#define LOG_H

#include "uart.h"

#define LOG(str) \
	do { \
		uart_print(0, str); \
		uart_write(0, '\n'); \
		uart_write(0, '\r'); \
	} while (0)

#define LOG_HEX(x) \
	do { \
		uart_write(0, '0'); \
		uart_write(0, 'x'); \
		for (int __i = 28; __i >= 0; __i -= 4) \
			uart_write(0, "0123456789ABCDEF"[((x) >> __i) & 0xF]); \
		uart_write(0, '\n'); \
		uart_write(0, '\r'); \
	} while (0)

#endif
