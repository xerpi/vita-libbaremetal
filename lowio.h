#ifndef LOWIO_H
#define LOWIO_H

#define dmb() asm volatile("dmb\n\t")
#define dsb() asm volatile("dsb\n\t")

void delay(int n);

unsigned int pervasive_read_misc(unsigned int offset);
void pervasive_clock_enable_uart(int bus);
void pervasive_reset_exit_uart(int bus);
void pervasive_clock_enable_gpio(void);
void pervasive_reset_exit_gpio(void);
void pervasive_clock_enable_i2c(int bus);
void pervasive_reset_exit_i2c(int bus);
void pervasive_clock_enable_spi(int bus);
void pervasive_reset_exit_spi(int bus);
void pervasive_clock_enable_dsi(int bus, int value);
void pervasive_reset_exit_dsi(int bus, int value);
void pervasive_dsi_set_pixeclock(int bus, int pixelclock);

#define GPIO_PORT_MODE_INPUT	0
#define GPIO_PORT_MODE_OUTPUT	1

#define GPIO_PORT_GAMECARD_LED	6
#define GPIO_PORT_PS_LED	7
#define GPIO_PORT_HDMI_BRIDGE	15

void gpio_set_port_mode(int bus, int port, int mode);
int gpio_port_read(int bus, int port);
void gpio_port_set(int bus, int port);
void gpio_port_clear(int bus, int port);
int gpio_query_intr(int bus, int port);
int gpio_acquire_intr(int bus, int port);

void i2c_init_bus(int bus);
void i2c_transfer_write(int bus, unsigned char addr, const unsigned char *buffer, int size);
void i2c_transfer_read(int bus, unsigned char addr, unsigned char *buffer, int size);
void i2c_transfer_write_read(int bus, unsigned char write_addr, const unsigned char *write_buffer, int write_size,
				      unsigned char read_addr, unsigned char *read_buffer, int read_size);

void dsi_enable_bus(int bus);

struct iftu_plane_info {
	unsigned int pixelformat;
	unsigned int width;
	unsigned int height;
	unsigned int leftover_stride;
	unsigned int unk10; // always 0
	unsigned int paddr;
	unsigned int unk18; // always 0
	unsigned int unk1C; // always 0
	unsigned int unk20; // always 0
	unsigned int src_x; // in (0x100000 / 960) multiples
	unsigned int src_y; // in (0x100000 / 544) multiples
	unsigned int src_w; // in (0x100000 / 960) multiples
	unsigned int src_h; // in (0x100000 / 544) multiples
	unsigned int dst_x;
	unsigned int dst_y;
	unsigned int dst_w;
	unsigned int dst_h;
	unsigned int vfront_porch;
	unsigned int vback_porch;
	unsigned int hfront_porch;
	unsigned int hback_porch;
};

void iftu_crtc_enable(int crtc);
void iftu_set_plane(int plane, struct iftu_plane_info *info);

#endif
