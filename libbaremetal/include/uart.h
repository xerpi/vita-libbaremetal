#ifndef UART_H
#define UART_H

int uart_init(int bus, uint32_t baudrate);
void uart_wait_ready(int bus);
void uart_write(int bus, uint32_t data);
uint32_t uart_read_fifo_data_available(int bus);
uint32_t uart_read(int bus);
void uart_print(int bus, const char *str);
void uart_printf(int bus, const char *s, ...) __attribute__((format(printf, 2, 3)));

#endif
