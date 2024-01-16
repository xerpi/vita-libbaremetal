#ifndef SDIF_H
#define SDIF_H

#include <stdint.h>

enum sdif_device {
	SDIF_DEVICE_EMMC = 0,
	SDIF_DEVICE_GC = 1,
	SDIF_DEVICE_WLAN_BT = 2,
	SDIF_DEVICE_MICRO_SD = 3,
	SDIF_DEVICE__MAX
};

int sdif_init(enum sdif_device device);

#endif
