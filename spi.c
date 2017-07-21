#include "spi.h"
#include "utils.h"

#define SPI0_BASE_ADDR			0xE0A00000
#define SPI1_BASE_ADDR			0xE0A10000
#define SPI2_BASE_ADDR			0xE0A20000

#define SPI_REGS(i)			((void *)( \
						(i) == 0 ? SPI0_BASE_ADDR : \
						(i) == 1 ? SPI1_BASE_ADDR : \
						           SPI2_BASE_ADDR))

int spi_init(int bus)
{
	volatile unsigned int *spi_regs = SPI_REGS(bus);

	spi_regs[8] = 0;
	dmb();

	return 0;
}

void spi_write_start(int bus)
{
	volatile unsigned int *spi_regs = SPI_REGS(bus);

	/*
	 * Flush pending data to be read from the FIFO
	 */
	while (spi_regs[0xA])
		spi_regs[0];

	spi_regs[0xB];
	spi_regs[9] = 0x600;
}

void spi_write_end(int bus)
{
	volatile unsigned int *spi_regs = SPI_REGS(bus);

	spi_regs[2] = 0;
	spi_regs[4] = 1;
	spi_regs[4];
	dsb();
}

void spi_write(int bus, unsigned int data)
{
	volatile unsigned int *spi_regs = SPI_REGS(bus);

	spi_regs[1] = data;
}

int spi_read_avaiable(int bus)
{
	volatile unsigned int *spi_regs = SPI_REGS(bus);

	return spi_regs[0xA];
}

int spi_read(int bus)
{
	volatile unsigned int *spi_regs = SPI_REGS(bus);

	return spi_regs[0];
}

void spi_read_end(int bus)
{
	volatile unsigned int *spi_regs = SPI_REGS(bus);

	spi_regs[4] = 0;
	spi_regs[4];
	dsb();
}

