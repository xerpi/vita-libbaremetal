#ifndef PERVASIVE_H
#define PERVASIVE_H

#include <stdint.h>

uint32_t pervasive_read_misc(uint32_t offset);

void pervasive_clock_enable_uart(int bus);
void pervasive_reset_exit_uart(int bus);

void pervasive_clock_enable_gpio(void);
void pervasive_reset_exit_gpio(void);

void pervasive_clock_enable_i2c(int bus);
void pervasive_reset_exit_i2c(int bus);

void pervasive_clock_enable_spi(int bus);
void pervasive_clock_disable_spi(int bus);
void pervasive_reset_exit_spi(int bus);

void pervasive_clock_enable_dsi(int bus, int value);
void pervasive_reset_exit_dsi(int bus, int value);

void pervasive_clock_enable_msif(void);
void pervasive_clock_disable_msif(void);
void pervasive_reset_exit_msif(void);
void pervasive_reset_enter_msif(void);

void pervasive_dsi_set_pixelclock(int bus, int pixelclock);
void pervasive_dsi_misc_unk_enable(int bus);
void pervasive_dsi_misc_unk_disable(int bus);

void pervasive_hdmi_cec_set_enabled(int enable);

int pervasive_msif_get_card_insert_state(void);
uint32_t pervasive_msif_unk(void);
void pervasive_msif_set_clock(uint32_t clock);

#endif
