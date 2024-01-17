#ifndef SDIF_H
#define SDIF_H

#include <stdbool.h>
#include <stdint.h>
#include "mmc.h"
#include "sdhci.h"

enum sdif_error {
	SDIF_ERROR_OK = 0,
	SDIF_ERROR_COMM = -1,
	SDIF_ERROR_TIMEOUT = -2,
	SDIF_ERROR_NOTSUPP = -3
};

enum sdif_host {
	SDIF_HOST_EMMC = 0,
	SDIF_HOST_GC = 1,
	SDIF_HOST_WLAN_BT = 2,
	SDIF_HOST_MICRO_SD = 3,
	SDIF_HOST__MAX
};

int sdif_init(enum sdif_host host);
bool sdif_is_card_inserted(enum sdif_host host);

#endif
