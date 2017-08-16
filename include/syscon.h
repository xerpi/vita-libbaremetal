#ifndef SYSCON_H
#define SYSCON_H

#define SYSCON_RESET_POWEROFF	0
#define SYSCON_RESET_SUSPEND	1
#define SYSCON_RESET_COLD_RESET	2
#define SYSCON_RESET_SOFT_RESET	17

int syscon_init(void);
void syscon_reset_device(int type, int unk);
void syscon_set_hdmi_cdc_hpd(int enable);
void syscon_ctrl_read(unsigned int *data);
void syscon_msif_set_power(int enable);

#endif
