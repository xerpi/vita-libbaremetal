#ifndef SDIF_H
#define SDIF_H

#include <stdbool.h>
#include <stdint.h>
#include "mmc.h"
#include "sdhci.h"

enum sdif_device {
	SDIF_DEVICE_EMMC = 0,
	SDIF_DEVICE_GC = 1,
	SDIF_DEVICE_WLAN_BT = 2,
	SDIF_DEVICE_MICRO_SD = 3,
	SDIF_DEVICE__MAX
};

enum sdif_bus_voltage {
	SDIF_BUS_VOLTAGE_1V8 = 0x5,
	SDIF_BUS_VOLTAGE_3V0 = 0x6,
	SDIF_BUS_VOLTAGE_3V3 = 0x7
};

int sdif_init(enum sdif_device device);
bool sdif_is_card_inserted(enum sdif_device device);
void sdif_bus_voltage_select(enum sdif_device device, enum sdif_bus_voltage voltage);

#endif
