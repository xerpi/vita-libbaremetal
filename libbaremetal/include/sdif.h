#ifndef SDIF_H
#define SDIF_H

#include <stdbool.h>
#include <stdint.h>
#include "mmc.h"
#include "sdhci.h"

enum sdif_error {
	SDIF_ERROR_OK = 0,
	SDIF_ERROR_COMM = -1,
	SDIF_ERROR_TIMEOUT = -2
};

enum sdif_device {
	SDIF_DEVICE_EMMC = 0,
	SDIF_DEVICE_GC = 1,
	SDIF_DEVICE_WLAN_BT = 2,
	SDIF_DEVICE_MICRO_SD = 3,
	SDIF_DEVICE__MAX
};

int sdif_init(enum sdif_device device);
bool sdif_is_card_inserted(enum sdif_device device);
void sdif_bus_voltage_select(enum sdif_device device, uint8_t voltage);

int sdif_send_cmd(enum sdif_device device, struct mmc_cmd *cmd, struct mmc_data *data);
int sdif_mmc_go_idle(enum sdif_device device);

#endif
