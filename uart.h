#ifndef UART_H
#define UART_H

int uart_init(int bus, unsigned int baudrate);
void uart_wait_ready(int bus);
void uart_write(int bus, unsigned int data);
unsigned int uart_read_fifo_data_available(int bus);
unsigned int uart_read(int bus);
void uart_print(int bus, const char *str);

#endif
