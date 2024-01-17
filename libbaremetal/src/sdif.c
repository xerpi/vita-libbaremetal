#include <string.h>
#include "sdif.h"
#include "pervasive.h"
#include "syscon.h"
#include "sysroot.h"
#include "utils.h"

#define udelay(x) delay((x) / 100)

#define SDIF_RESET_HW (1 << 0)
#define SDIF_RESET_SW (1 << 1)

static uint32_t g_unk_18[SDIF_DEVICE__MAX];
static int g_skip_sdif0_reset = 0;

static inline uintptr_t sdif_registers(enum sdif_device device)
{
	switch (device) {
	case SDIF_DEVICE_EMMC:
		return 0xE0B00000;
	case SDIF_DEVICE_GC:
		return 0xE0C00000;
	case SDIF_DEVICE_WLAN_BT:
		return 0xE0C10000;
	case SDIF_DEVICE_MICRO_SD:
		return 0xE0C20000;
	default:
		return 0;
	}
}

static inline uint8_t sdhci_readb(enum sdif_device device, uint32_t reg)
{
	return read8(sdif_registers(device) + reg);
}

static inline uint16_t sdhci_readw(enum sdif_device device, uint32_t reg)
{
	return read16(sdif_registers(device) + reg);
}

static inline uint32_t sdhci_readl(enum sdif_device device, uint32_t reg)
{
	return read32(sdif_registers(device) + reg);
}

static inline void sdhci_writeb(enum sdif_device device, uint8_t val, uint32_t reg)
{
	write8(val, sdif_registers(device) + reg);
}

static inline void sdhci_writew(enum sdif_device device, uint16_t val, uint32_t reg)
{
	write16(val, sdif_registers(device) + reg);
}

static inline void sdhci_writel(enum sdif_device device, uint32_t val, uint32_t reg)
{
	write32(val, sdif_registers(device) + reg);
}

static int sdif_reset(enum sdif_device device, int flags)
{
	uint32_t soc, delay, enable;

	if (flags & SDIF_RESET_HW) {
		soc = pervasive_get_soc_revision();
		if ((soc & 0x1ff00) == 0) {
			if (!sysroot_model_is_dolce()) {
				if (device < SDIF_DEVICE_WLAN_BT) {
					pervasive_sdif_misc_0x110_0x11C(device, 0x200);
					if (device != SDIF_DEVICE_EMMC) {
						pervasive_reset_enter_sdif(device);
					} else {
						if (g_skip_sdif0_reset == 1) {
							//LOG("Skip sdif port[%d] reset\n", 0);
						} else {
							pervasive_reset_enter_sdif(0);
							smc(0x13C, 3, 0, 0, 0);
						}
					}
				} else {
					pervasive_reset_enter_sdif(device);
				}
			} else {
				if (device != SDIF_DEVICE_EMMC) {
					pervasive_sdif_misc_0x110_0x11C(device, 0);
					pervasive_reset_enter_sdif(device);
				} else {
					pervasive_sdif_misc_0x110_0x11C(0, 0x200);
					if (g_skip_sdif0_reset == 1) {
						//LOG("Skip sdif port[%d] reset\n", 0);
					} else {
						pervasive_reset_enter_sdif(0);
						smc(0x13C, 3, 0, 0, 0);
					}
				}
			}
		} else {
			if ((soc & 0x1ff00) == 0x100) {
				if ((syscon_get_hardware_info() & 0xff0000) < 0x800000) {
					if (device != SDIF_DEVICE_EMMC) {
						if (device < SDIF_DEVICE_MICRO_SD) {
							pervasive_sdif_misc_0x310(device, 2);
						}
						pervasive_reset_enter_sdif(device);
					} else {
						pervasive_sdif_misc_0x310(0, 4);
						if (g_skip_sdif0_reset == 1) {
							//LOG("Skip sdif port[%d] reset\n", 0);
						} else {
							pervasive_reset_enter_sdif(0);
							smc(0x13C, 3, 0, 0, 0);
						}
					}
				} else {
					if (device != SDIF_DEVICE_EMMC) {
						if (device == SDIF_DEVICE_GC) {
							if (/*sceKernelIsAllowSdCardFromMgmt()*/ 0 == 0) {
								pervasive_sdif_misc_0x310(1, 3);
							} else {
								pervasive_sdif_misc_0x310(1, 6);
							}
						} else if (device == SDIF_DEVICE_WLAN_BT) {
							pervasive_sdif_misc_0x310(2, 3);
						}
						pervasive_reset_enter_sdif(device);
					} else {
						pervasive_sdif_misc_0x310(0, 5);
						if (g_skip_sdif0_reset == 1) {
							//LOG("Skip sdif port[%d] reset\n", 0);
						} else {
							pervasive_reset_enter_sdif(0);
							smc(0x13C, 3, 0, 0, 0);
						}
					}
				}
			} else {
				if (device != SDIF_DEVICE_EMMC) {
					pervasive_reset_enter_sdif(device);
				} else {
					if (g_skip_sdif0_reset == 1) {
						//LOG("Skip sdif port[%d] reset\n", 0);
					} else {
						pervasive_reset_enter_sdif(0);
						smc(0x13C, 3, 0, 0, 0);
					}
				}
			}
		}

		pervasive_reset_exit_sdif(device);
		pervasive_clock_enable_sdif(device);

		switch(device) {
		case SDIF_DEVICE_EMMC:
			g_unk_18[device] = 0x80; //1V8
			pervasive_sdif_misc_0x124(0, 1);
			break;
		case SDIF_DEVICE_GC:
		case SDIF_DEVICE_MICRO_SD:
			g_unk_18[device] = 0x300000; // 3V3
			pervasive_sdif_misc_0x124(device, 0);
			break;
		case SDIF_DEVICE_WLAN_BT:
			g_unk_18[device] = 0x80; //1V8
			pervasive_sdif_misc_0x124(2, 1);
			break;
		default:
			g_unk_18[device] = 0;
			break;
		}
	}

	if (flags & SDIF_RESET_SW) {
		sdhci_writeb(device, SDHCI_RESET_ALL, SDHCI_SOFTWARE_RESET);
		while (sdhci_readb(device, SDHCI_SOFTWARE_RESET) & SDHCI_RESET_ALL)
			;

		sdhci_writeb(device, 14, SDHCI_TIMEOUT_CONTROL);

		delay = 102;
		do {
			if (sdhci_readl(device, SDHCI_PRESENT_STATE) & SDHCI_CARD_STATE_STABLE)
				break;
		} while ((device != SDIF_DEVICE_GC) || (delay--, delay != 0));

		sdhci_writel(device, 0, SDHCI_INT_ENABLE);
		sdhci_writel(device, 0, SDHCI_SIGNAL_ENABLE);
		sdhci_writel(device, sdhci_readl(device, SDHCI_INT_STATUS), SDHCI_INT_STATUS);

		while (sdhci_readl(device, SDHCI_INT_STATUS) != 0)
			;

		enable = SDHCI_INT_RESPONSE | SDHCI_INT_DATA_END |
				SDHCI_INT_DMA_END | SDHCI_INT_SPACE_AVAIL |
				SDHCI_INT_DATA_AVAIL | SDHCI_INT_CARD_INSERT |
				SDHCI_INT_CARD_REMOVE;
		if (device == SDIF_DEVICE_WLAN_BT)
			enable |= SDHCI_INT_CARD_INT;

		enable |= SDHCI_INT_TIMEOUT | SDHCI_INT_CRC |
			SDHCI_INT_END_BIT | SDHCI_INT_INDEX |
			SDHCI_INT_DATA_TIMEOUT | SDHCI_INT_DATA_CRC |
			SDHCI_INT_DATA_END_BIT | SDHCI_INT_ACMD12ERR |
			SDHCI_INT_ADMA_ERROR;

		sdhci_writel(device, enable, SDHCI_INT_ENABLE);
		sdhci_writel(device, enable, SDHCI_SIGNAL_ENABLE);
	}

	return 0;
}

int sdif_init(enum sdif_device device)
{
	sdif_reset(device, SDIF_RESET_HW | SDIF_RESET_SW);

	if (sdif_is_card_inserted(device)) {
		uint8_t voltage;

		if (g_unk_18[device] & 0x300000)
			voltage = SDHCI_POWER_330;
		else if (g_unk_18[device] & 0x60000)
			voltage = SDHCI_POWER_300;
		else if (g_unk_18[device] & (1 << 7))
			voltage = SDHCI_POWER_180;
		else
			voltage = 0;

		sdif_bus_voltage_select(device, voltage);
	}

	return 0;
}

bool sdif_is_card_inserted(enum sdif_device device)
{
	return !!(sdhci_readl(device, SDHCI_PRESENT_STATE) & SDHCI_CARD_PRESENT);
}

void sdif_bus_voltage_select(enum sdif_device device, uint8_t voltage)
{
	uint8_t tmp8;
	uint16_t clockctrl, tmp16;
	const uint16_t sdclk_freq = 128;

	uint32_t delay = 102;
	do {
		if (sdhci_readl(device, SDHCI_PRESENT_STATE) & SDHCI_CARD_STATE_STABLE)
			break;
	} while ((device != SDIF_DEVICE_GC) || (delay--, delay != 0));

	if (sdhci_readl(device, SDHCI_PRESENT_STATE) & SDHCI_CARD_PRESENT) {
		tmp8 = sdhci_readb(device, SDHCI_POWER_CONTROL);
		tmp8 &= 0xfe;
		sdhci_writeb(device, tmp8, SDHCI_POWER_CONTROL);

		tmp8 = sdhci_readb(device, SDHCI_POWER_CONTROL);
		tmp8 |= voltage;
		sdhci_writeb(device, tmp8, SDHCI_POWER_CONTROL);

		tmp8 = sdhci_readb(device, SDHCI_POWER_CONTROL);
		tmp8 |= SDHCI_POWER_ON;
		sdhci_writeb(device, tmp8, SDHCI_POWER_CONTROL);

		sdhci_writew(device, 0, SDHCI_CLOCK_CONTROL);

		clockctrl = SDHCI_CLOCK_INT_EN |
			(sdclk_freq & SDHCI_DIV_MASK) << SDHCI_DIVIDER_SHIFT |
			((sdclk_freq & SDHCI_DIV_HI_MASK) >> SDHCI_DIV_MASK_LEN)
				<< SDHCI_DIVIDER_HI_SHIFT;

		sdhci_writew(device, clockctrl, SDHCI_CLOCK_CONTROL);

		while (!(sdhci_readw(device, SDHCI_CLOCK_CONTROL) & SDHCI_CLOCK_INT_STABLE))
			;

		/* Vita OS turns this on/off if !WLAN/BT when sending commands.
		 * Here we just leave it enabled */
		if (/* device == SDIF_DEVICE_WLAN_BT*/ 1) {
			tmp16 = sdhci_readw(device, SDHCI_CLOCK_CONTROL);
			tmp16 |= SDHCI_CLOCK_CARD_EN;
			sdhci_writew(device, tmp16, SDHCI_CLOCK_CONTROL);
		}

		sdhci_writeb(device, 0, SDHCI_HOST_CONTROL);
	}
}

static void sdhci_cmd_done(enum sdif_device host, struct mmc_cmd *cmd)
{
	int i;
	if (cmd->resp_type & MMC_RSP_136) {
		/* CRC is stripped so we need to do some shifting. */
		for (i = 0; i < 4; i++) {
			cmd->response[i] = sdhci_readl(host,
					SDHCI_RESPONSE + (3-i)*4) << 8;
			if (i != 3)
				cmd->response[i] |= sdhci_readb(host,
						SDHCI_RESPONSE + (3-i)*4-1);
		}
	} else {
		cmd->response[0] = sdhci_readl(host, SDHCI_RESPONSE);
	}
}

int sdif_send_cmd(enum sdif_device host, struct mmc_cmd *cmd, struct mmc_data *data)
{
	unsigned int stat = 0;
	int ret = 0;
	uint32_t mask, flags, mode = 0;

	mask = SDHCI_CMD_INHIBIT | SDHCI_DATA_INHIBIT;

	/* We shouldn't wait for data inihibit for stop commands, even
	   though they might use busy signaling */
	if (cmd->cmdidx == MMC_CMD_STOP_TRANSMISSION ||
	    ((cmd->cmdidx == MMC_CMD_SEND_TUNING_BLOCK ||
	      cmd->cmdidx == MMC_CMD_SEND_TUNING_BLOCK_HS200) && !data))
		mask &= ~SDHCI_DATA_INHIBIT;

	while (sdhci_readl(host, SDHCI_PRESENT_STATE) & mask)
		udelay(1000);

	mask = SDHCI_INT_RESPONSE;
	if ((cmd->cmdidx == MMC_CMD_SEND_TUNING_BLOCK ||
	     cmd->cmdidx == MMC_CMD_SEND_TUNING_BLOCK_HS200) && !data)
		mask = SDHCI_INT_DATA_AVAIL;

	if (!(cmd->resp_type & MMC_RSP_PRESENT))
		flags = SDHCI_CMD_RESP_NONE;
	else if (cmd->resp_type & MMC_RSP_136)
		flags = SDHCI_CMD_RESP_LONG;
	else if (cmd->resp_type & MMC_RSP_BUSY) {
		flags = SDHCI_CMD_RESP_SHORT_BUSY;
		mask |= SDHCI_INT_DATA_END;
	} else
		flags = SDHCI_CMD_RESP_SHORT;

	if (cmd->resp_type & MMC_RSP_CRC)
		flags |= SDHCI_CMD_CRC;
	if (cmd->resp_type & MMC_RSP_OPCODE)
		flags |= SDHCI_CMD_INDEX;
	if (data || cmd->cmdidx ==  MMC_CMD_SEND_TUNING_BLOCK ||
	    cmd->cmdidx == MMC_CMD_SEND_TUNING_BLOCK_HS200)
		flags |= SDHCI_CMD_DATA;

	/* Set Transfer mode regarding to data flag */
	if (data) {
		if (data->blocks > 1)
			mode |= SDHCI_TRNS_MULTI | SDHCI_TRNS_BLK_CNT_EN;

		if (data->flags == MMC_DATA_READ)
			mode |= SDHCI_TRNS_READ;

		sdhci_writew(host, SDHCI_MAKE_BLKSZ(SDHCI_DEFAULT_BOUNDARY_ARG,
				data->blocksize), SDHCI_BLOCK_SIZE);
		sdhci_writew(host, data->blocks, SDHCI_BLOCK_COUNT);
		sdhci_writew(host, mode, SDHCI_TRANSFER_MODE);
	}

	sdhci_writel(host, cmd->cmdarg, SDHCI_ARGUMENT);
	sdhci_writew(host, SDHCI_MAKE_CMD(cmd->cmdidx, flags), SDHCI_COMMAND);
	do {
		stat = sdhci_readl(host, SDHCI_INT_STATUS);
		if (stat & SDHCI_INT_ERROR)
			break;
	} while ((stat & mask) != mask);

	if ((stat & (SDHCI_INT_ERROR | mask)) == mask) {
		sdhci_cmd_done(host, cmd);
		sdhci_writel(host, mask, SDHCI_INT_STATUS);
	} else
		ret = -1;

	/*if (!ret && data)
		ret = sdhci_transfer_data(host, data);*/

	stat = sdhci_readl(host, SDHCI_INT_STATUS);
	sdhci_writel(host, SDHCI_INT_ALL_MASK, SDHCI_INT_STATUS);
	if (!ret)
		return 0;

	if (stat & SDHCI_INT_TIMEOUT)
		return SDIF_ERROR_TIMEOUT;
	else
		return SDIF_ERROR_COMM;
}

int sdif_mmc_go_idle(enum sdif_device device)
{
	struct mmc_cmd cmd;

	cmd.cmdidx = MMC_CMD_GO_IDLE_STATE;
	cmd.cmdarg = 0;
	cmd.resp_type = MMC_RSP_NONE;

	return sdif_send_cmd(device, &cmd, NULL);
}
