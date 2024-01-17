#include <string.h>
#include "sdif.h"
#include "pervasive.h"
#include "syscon.h"
#include "sysroot.h"
#include "utils.h"

#define udelay(x) delay((x) / 100)

#define SDIF_RESET_HW (1 << 0)
#define SDIF_RESET_SW (1 << 1)

static struct sdif_host_status {
	uint32_t version;
	uint32_t voltages;
} g_sdif_host_status[SDIF_HOST__MAX];

static int g_skip_sdif0_reset = 0;

static inline uintptr_t sdif_registers(enum sdif_host host)
{
	switch (host) {
	case SDIF_HOST_EMMC:
		return 0xE0B00000;
	case SDIF_HOST_GC:
		return 0xE0C00000;
	case SDIF_HOST_WLAN_BT:
		return 0xE0C10000;
	case SDIF_HOST_MICRO_SD:
		return 0xE0C20000;
	default:
		return 0;
	}
}

static inline uint8_t sdhci_readb(enum sdif_host host, uint32_t reg)
{
	return read8(sdif_registers(host) + reg);
}

static inline uint16_t sdhci_readw(enum sdif_host host, uint32_t reg)
{
	return read16(sdif_registers(host) + reg);
}

static inline uint32_t sdhci_readl(enum sdif_host host, uint32_t reg)
{
	return read32(sdif_registers(host) + reg);
}

static inline void sdhci_writeb(enum sdif_host host, uint8_t val, uint32_t reg)
{
	write8(val, sdif_registers(host) + reg);
}

static inline void sdhci_writew(enum sdif_host host, uint16_t val, uint32_t reg)
{
	write16(val, sdif_registers(host) + reg);
}

static inline void sdhci_writel(enum sdif_host host, uint32_t val, uint32_t reg)
{
	write32(val, sdif_registers(host) + reg);
}

static int sdif_reset(enum sdif_host host, int flags)
{
	uint32_t soc, delay, enable;

	if (flags & SDIF_RESET_HW) {
		soc = pervasive_get_soc_revision();
		if ((soc & 0x1ff00) == 0) {
			if (!sysroot_model_is_dolce()) {
				if (host < SDIF_HOST_WLAN_BT) {
					pervasive_sdif_misc_0x110_0x11C(host, 0x200);
					if (host != SDIF_HOST_EMMC) {
						pervasive_reset_enter_sdif(host);
					} else {
						if (g_skip_sdif0_reset == 1) {
							//LOG("Skip sdif port[%d] reset\n", 0);
						} else {
							pervasive_reset_enter_sdif(0);
							smc(0x13C, 3, 0, 0, 0);
						}
					}
				} else {
					pervasive_reset_enter_sdif(host);
				}
			} else {
				if (host != SDIF_HOST_EMMC) {
					pervasive_sdif_misc_0x110_0x11C(host, 0);
					pervasive_reset_enter_sdif(host);
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
					if (host != SDIF_HOST_EMMC) {
						if (host < SDIF_HOST_MICRO_SD) {
							pervasive_sdif_misc_0x310(host, 2);
						}
						pervasive_reset_enter_sdif(host);
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
					if (host != SDIF_HOST_EMMC) {
						if (host == SDIF_HOST_GC) {
							if (/*sceKernelIsAllowSdCardFromMgmt()*/ 0 == 0) {
								pervasive_sdif_misc_0x310(1, 3);
							} else {
								pervasive_sdif_misc_0x310(1, 6);
							}
						} else if (host == SDIF_HOST_WLAN_BT) {
							pervasive_sdif_misc_0x310(2, 3);
						}
						pervasive_reset_enter_sdif(host);
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
				if (host != SDIF_HOST_EMMC) {
					pervasive_reset_enter_sdif(host);
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

		pervasive_reset_exit_sdif(host);
		pervasive_clock_enable_sdif(host);

		switch(host) {
		case SDIF_HOST_EMMC:
			g_sdif_host_status[host].voltages = MMC_VDD_165_195;
			pervasive_sdif_misc_0x124(0, 1);
			break;
		case SDIF_HOST_GC:
		case SDIF_HOST_MICRO_SD:
			g_sdif_host_status[host].voltages = MMC_VDD_32_33 | MMC_VDD_33_34;
			pervasive_sdif_misc_0x124(host, 0);
			break;
		case SDIF_HOST_WLAN_BT:
			g_sdif_host_status[host].voltages = MMC_VDD_165_195;
			pervasive_sdif_misc_0x124(2, 1);
			break;
		default:
			g_sdif_host_status[host].voltages = 0;
			break;
		}
	}

	if (flags & SDIF_RESET_SW) {
		sdhci_writeb(host, SDHCI_RESET_ALL, SDHCI_SOFTWARE_RESET);
		while (sdhci_readb(host, SDHCI_SOFTWARE_RESET) & SDHCI_RESET_ALL)
			;

		sdhci_writeb(host, 14, SDHCI_TIMEOUT_CONTROL);

		delay = 102;
		do {
			if (sdhci_readl(host, SDHCI_PRESENT_STATE) & SDHCI_CARD_STATE_STABLE)
				break;
		} while ((host != SDIF_HOST_GC) || (delay--, delay != 0));

		sdhci_writel(host, 0, SDHCI_INT_ENABLE);
		sdhci_writel(host, 0, SDHCI_SIGNAL_ENABLE);
		sdhci_writel(host, sdhci_readl(host, SDHCI_INT_STATUS), SDHCI_INT_STATUS);

		while (sdhci_readl(host, SDHCI_INT_STATUS) != 0)
			;

		enable = SDHCI_INT_RESPONSE | SDHCI_INT_DATA_END |
				SDHCI_INT_DMA_END | SDHCI_INT_SPACE_AVAIL |
				SDHCI_INT_DATA_AVAIL | SDHCI_INT_CARD_INSERT |
				SDHCI_INT_CARD_REMOVE;
		if (host == SDIF_HOST_WLAN_BT)
			enable |= SDHCI_INT_CARD_INT;

		enable |= SDHCI_INT_TIMEOUT | SDHCI_INT_CRC |
			SDHCI_INT_END_BIT | SDHCI_INT_INDEX |
			SDHCI_INT_DATA_TIMEOUT | SDHCI_INT_DATA_CRC |
			SDHCI_INT_DATA_END_BIT | SDHCI_INT_ACMD12ERR |
			SDHCI_INT_ADMA_ERROR;

		sdhci_writel(host, enable, SDHCI_INT_ENABLE);
		sdhci_writel(host, enable, SDHCI_SIGNAL_ENABLE);
	}

	return 0;
}

static int sdif_bus_voltage_select(enum sdif_host host, uint8_t voltage)
{
	uint8_t tmp8;
	uint16_t clockctrl, tmp16;
	const uint16_t sdclk_freq = 128;

	uint32_t delay = 102;
	do {
		if (sdhci_readl(host, SDHCI_PRESENT_STATE) & SDHCI_CARD_STATE_STABLE)
			break;
	} while ((host != SDIF_HOST_GC) || (delay--, delay != 0));

	if (sdhci_readl(host, SDHCI_PRESENT_STATE) & SDHCI_CARD_PRESENT) {
		tmp8 = sdhci_readb(host, SDHCI_POWER_CONTROL);
		tmp8 &= 0xfe;
		sdhci_writeb(host, tmp8, SDHCI_POWER_CONTROL);

		tmp8 = sdhci_readb(host, SDHCI_POWER_CONTROL);
		tmp8 |= voltage;
		sdhci_writeb(host, tmp8, SDHCI_POWER_CONTROL);

		tmp8 = sdhci_readb(host, SDHCI_POWER_CONTROL);
		tmp8 |= SDHCI_POWER_ON;
		sdhci_writeb(host, tmp8, SDHCI_POWER_CONTROL);

		sdhci_writew(host, 0, SDHCI_CLOCK_CONTROL);

		clockctrl = SDHCI_CLOCK_INT_EN |
			(sdclk_freq & SDHCI_DIV_MASK) << SDHCI_DIVIDER_SHIFT |
			((sdclk_freq & SDHCI_DIV_HI_MASK) >> SDHCI_DIV_MASK_LEN)
				<< SDHCI_DIVIDER_HI_SHIFT;

		sdhci_writew(host, clockctrl, SDHCI_CLOCK_CONTROL);

		while (!(sdhci_readw(host, SDHCI_CLOCK_CONTROL) & SDHCI_CLOCK_INT_STABLE))
			;

		/* Vita OS turns this on/off if !WLAN/BT when sending commands.
		 * Here we just leave it enabled */
		if (/* host == SDIF_HOST_WLAN_BT*/ 1) {
			tmp16 = sdhci_readw(host, SDHCI_CLOCK_CONTROL);
			tmp16 |= SDHCI_CLOCK_CARD_EN;
			sdhci_writew(host, tmp16, SDHCI_CLOCK_CONTROL);
		}

		sdhci_writeb(host, 0, SDHCI_HOST_CONTROL);
	}

	return 0;
}

static void sdhci_cmd_done(enum sdif_host host, struct mmc_cmd *cmd)
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

static int sdif_send_cmd(enum sdif_host host, struct mmc_cmd *cmd, struct mmc_data *data)
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

static int sdif_mmc_go_idle(enum sdif_host host)
{
	struct mmc_cmd cmd;

	cmd.cmdidx = MMC_CMD_GO_IDLE_STATE;
	cmd.cmdarg = 0;
	cmd.resp_type = MMC_RSP_NONE;

	return sdif_send_cmd(host, &cmd, NULL);
}

static int sdif_mmc_send_if_cond(enum sdif_host host)
{
	struct mmc_cmd cmd;
	int err;

	cmd.cmdidx = SD_CMD_SEND_IF_COND;
	/* We set the bit if the host supports voltages between 2.7 and 3.6 V */
	cmd.cmdarg = ((g_sdif_host_status[host].voltages & 0xff8000) != 0) << 8 | 0xaa;
	cmd.resp_type = MMC_RSP_R7;

	err = sdif_send_cmd(host, &cmd, NULL);
	if (err)
		return err;

	if ((cmd.response[0] & 0xff) != 0xaa)
		return SDIF_ERROR_NOTSUPP;
	else
		g_sdif_host_status[host].version = SD_VERSION_2;

	return 0;
}

static int sdif_init_cmd_sequence(enum sdif_host host)
{
	int ret;

	ret = sdif_mmc_go_idle(host);
	if (ret)
		return ret;

	ret = sdif_mmc_send_if_cond(host);
	if (ret)
		return ret;

	return 0;
}

int sdif_init(enum sdif_host host)
{
	int ret;

	ret = sdif_reset(host, SDIF_RESET_HW | SDIF_RESET_SW);
	if (ret)
		return ret;

	if (sdif_is_card_inserted(host)) {
		uint8_t voltage;

		if (g_sdif_host_status[host].voltages & (MMC_VDD_32_33 | MMC_VDD_33_34))
			voltage = SDHCI_POWER_330;
		else if (g_sdif_host_status[host].voltages & (MMC_VDD_29_30 | MMC_VDD_30_31))
			voltage = SDHCI_POWER_300;
		else if (g_sdif_host_status[host].voltages & MMC_VDD_165_195)
			voltage = SDHCI_POWER_180;
		else
			voltage = 0;

		ret = sdif_bus_voltage_select(host, voltage);
		if (ret)
			return ret;

		ret = sdif_init_cmd_sequence(host);
		if (ret)
			return ret;
	}

	return 0;
}

bool sdif_is_card_inserted(enum sdif_host host)
{
	return !!(sdhci_readl(host, SDHCI_PRESENT_STATE) & SDHCI_CARD_PRESENT);
}
