#include "i2c.h"
#include "utils.h"

#define I2C0_BASE_ADDR			0xE0500000
#define I2C1_BASE_ADDR			0xE0510000

#define I2C_REGS(i)			((void *)((i) == 0 ? I2C0_BASE_ADDR : I2C1_BASE_ADDR))

static inline void i2c_wait_busy(volatile uint32_t *i2c_regs)
{
	while (i2c_regs[7])
		;
}

void i2c_init_bus(int bus)
{
	volatile uint32_t *i2c_regs = I2C_REGS(bus);

	i2c_regs[0xB] = 0x100F70F;
	i2c_regs[2] = 1;
	i2c_regs[3] = 1;
	i2c_regs[5] = 7;
	dsb();

	i2c_wait_busy(i2c_regs);

	i2c_regs[0xA] = i2c_regs[0xA];
	i2c_regs[0xB] = 0x1000000;

	i2c_regs[6] = 4; // or 5?
}

void i2c_transfer_write(int bus, uint8_t addr, const uint8_t *buffer, int size)
{
	int i;
	volatile uint32_t *i2c_regs = I2C_REGS(bus);

	i2c_regs[2] = 1;
	i2c_regs[3] = 1;
	i2c_regs[4] = addr >> 1;

	for (i = 0; i < size; i++)
		i2c_regs[0] = buffer[i];

	i2c_regs[5] = (size << 8) | 2;

	i2c_wait_busy(i2c_regs);

	i2c_regs[5] = 4;

	i2c_wait_busy(i2c_regs);
}

void i2c_transfer_read(int bus, uint8_t addr, uint8_t *buffer, int size)
{
	int i;
	volatile uint32_t *i2c_regs = I2C_REGS(bus);

	i2c_regs[2] = 1;
	i2c_regs[3] = 1;
	i2c_regs[4] = addr >> 1;
	i2c_regs[5] = (size << 8) | 0x13;

	i2c_wait_busy(i2c_regs);

	for (i = 0; i < size; i++)
		buffer[i] = i2c_regs[1];

	i2c_regs[5] = 4;

	i2c_wait_busy(i2c_regs);
}

void i2c_transfer_write_read(int bus, uint8_t write_addr, const uint8_t *write_buffer, int write_size,
			     uint8_t read_addr, uint8_t *read_buffer, int read_size)
{
	int i;
	volatile uint32_t *i2c_regs = I2C_REGS(bus);

	i2c_regs[2] = 1;
	i2c_regs[3] = 1;
	i2c_regs[4] = write_addr >> 1;

	for (i = 0; i < write_size; i++)
		i2c_regs[0] = write_buffer[i];

	i2c_regs[5] = (write_size << 8) | 2;

	i2c_wait_busy(i2c_regs);

	i2c_regs[5] = 5;

	i2c_wait_busy(i2c_regs);

	i2c_regs[5] = (read_size << 8) | 0x13;

	i2c_wait_busy(i2c_regs);

	for (i = 0; i < read_size; i++)
		read_buffer[i] = i2c_regs[1];

	i2c_regs[5] = 4;

	i2c_wait_busy(i2c_regs);
}
