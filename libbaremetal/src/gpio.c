#include "gpio.h"
#include "utils.h"

#define GPIO0_BASE_ADDR			0xE20A0000
#define GPIO1_BASE_ADDR			0xE0100000

#define GPIO_REGS(i)			((void *)((i) == 0 ? GPIO0_BASE_ADDR : GPIO1_BASE_ADDR))

#define GPIO_DIRECTION		0x00
#define GPIO_READ		0x04
#define GPIO_SET		0x08
#define GPIO_CLEAR		0x0C
#define GPIO_INT_MODE_0_15	0x14
#define GPIO_INT_MODE_16_31	0x18
#define GPIO_INT_MASK_GATE0	0x1C
#define GPIO_INT_MASK_GATE1	0x20
#define GPIO_INT_MASK_GATE2	0x24
#define GPIO_INT_MASK_GATE3	0x28
#define GPIO_INT_MASK_GATE4	0x2C
#define GPIO_READ_LATCH		0x34
#define GPIO_INT_STATUS_GATE0	0x38
#define GPIO_INT_STATUS_GATE1	0x3C
#define GPIO_INT_STATUS_GATE2	0x40
#define GPIO_INT_STATUS_GATE3	0x44
#define GPIO_INT_STATUS_GATE4	0x48

void gpio_set_port_mode(int bus, int port, int mode)
{
	volatile uint32_t *gpio_regs = GPIO_REGS(bus);

	gpio_regs[0] = (gpio_regs[0] & ~(1 << port)) | (mode << port);

	dmb();
}

int gpio_port_read(int bus, int port)
{
	volatile uint32_t *gpio_regs = GPIO_REGS(bus);

	return (gpio_regs[1] >> port) & 1;
}

void gpio_port_set(int bus, int port)
{
	volatile uint32_t *gpio_regs = GPIO_REGS(bus);

	gpio_regs[2] |= 1 << port;

	gpio_regs[0xD];

	dsb();
}

void gpio_port_clear(int bus, int port)
{
	volatile uint32_t *gpio_regs = GPIO_REGS(bus);

	gpio_regs[3] |= 1 << port;

	gpio_regs[0xD];

	dsb();
}

void gpio_set_intr_mode(int bus, int port, int mode)
{
	volatile uint32_t *gpio_regs = GPIO_REGS(bus);
	uint32_t reg = 5 + port / 16;
	uint32_t off = 2 * (port % 16);

	gpio_regs[reg] |= (gpio_regs[reg] & ~(3 << off)) | (mode << off);

	dmb();
}

int gpio_query_intr(int bus, int port)
{
	volatile uint32_t *gpio_regs = GPIO_REGS(bus);

	return (1 << port) & ((gpio_regs[0x0E] & ~gpio_regs[0x07]) |
			      (gpio_regs[0x0F] & ~gpio_regs[0x08]) |
			      (gpio_regs[0x10] & ~gpio_regs[0x09]) |
			      (gpio_regs[0x11] & ~gpio_regs[0x0A]) |
			      (gpio_regs[0x12] & ~gpio_regs[0x0B]));
}

int gpio_acquire_intr(int bus, int port)
{
	uint32_t ret;
	uint32_t mask = 1 << port;
	volatile uint32_t *gpio_regs = GPIO_REGS(bus);

	ret = mask & ((gpio_regs[0x0E] & ~gpio_regs[0x07]) |
		      (gpio_regs[0x0F] & ~gpio_regs[0x08]) |
		      (gpio_regs[0x10] & ~gpio_regs[0x09]) |
		      (gpio_regs[0x11] & ~gpio_regs[0x0A]) |
		      (gpio_regs[0x12] & ~gpio_regs[0x0B]));

	gpio_regs[0x0E] = mask;
	gpio_regs[0x0F] = mask;
	gpio_regs[0x10] = mask;
	gpio_regs[0x11] = mask;
	gpio_regs[0x12] = mask;
	dsb();

	return ret;
}
