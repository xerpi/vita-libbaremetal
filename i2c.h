#ifndef I2C_H
#define I2C_H

void i2c_init_bus(int bus);
void i2c_transfer_write(int bus, unsigned char addr, const unsigned char *buffer, int size);
void i2c_transfer_read(int bus, unsigned char addr, unsigned char *buffer, int size);
void i2c_transfer_write_read(int bus, unsigned char write_addr, const unsigned char *write_buffer, int write_size,
				      unsigned char read_addr, unsigned char *read_buffer, int read_size);

#endif
