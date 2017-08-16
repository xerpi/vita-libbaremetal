#ifndef LOG_H
#define LOG_H

#include "uart.h"

#define LOG(...)	uart_printf(0, __VA_ARGS__)
#define LOG_HEX(x)	uart_printf(0, "0x%08X\n", x)

#endif
