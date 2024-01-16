#ifndef I2C_H
#define I2C_H

#include <stdint.h>

void i2c_init_bus(int bus);
void i2c_transfer_write(int bus, uint8_t addr, const uint8_t *buffer, int size);
void i2c_transfer_read(int bus, uint8_t addr, uint8_t *buffer, int size);
void i2c_transfer_write_read(int bus, uint8_t write_addr, const uint8_t *write_buffer, int write_size,
			     uint8_t read_addr, uint8_t *read_buffer, int read_size);

#endif
