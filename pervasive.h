#ifndef PERVASIVE_H
#define PERVASIVE_H

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
void pervasive_dsi_set_pixelclock(int bus, int pixelclock);
void pervasive_dsi_misc_unk(int bus);

void pervasive_hdmi_cec_set_enabled(int enable);

#endif
