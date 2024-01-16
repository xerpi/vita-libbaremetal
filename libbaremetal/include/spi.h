#ifndef SPI_H
#define SPI_H

#include <stdint.h>

int spi_init(int bus);
void spi_write_start(int bus);
void spi_write_end(int bus);
void spi_write(int bus, uint32_t data);
int spi_read_available(int bus);
int spi_read(int bus);
void spi_read_end(int bus);

#endif
