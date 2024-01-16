#include <string.h>
#include "sdif.h"
#include "pervasive.h"
#include "syscon.h"
#include "sysroot.h"
#include "utils.h"
#include "uart.h"

#define printf(...) uart_printf(0, __VA_ARGS__)
#define puts(...) uart_puts(0, __VA_ARGS__)
#define udelay(x) delay(x)
#define get_timer(x) ((void)x, 0)

#define SDIF_RESET_HW (1 << 0)
#define SDIF_RESET_SW (1 << 1)

#define SDHCI_CMD_MAX_TIMEOUT			3200
#define SDHCI_CMD_DEFAULT_TIMEOUT		100
#define SDHCI_READ_STATUS_TIMEOUT		1000

static uint32_t g_timeout_control[SDIF_DEVICE__MAX];
static uint32_t g_unk_18[SDIF_DEVICE__MAX];
static int g_unk_8 = 0;

// https://wiki.henkaku.xyz/vita/SDIF_Registers
typedef struct {
	uint32_t sdma_system_addr;
	uint16_t block_size;
	uint16_t block_count;
	uint32_t argument1;
	uint16_t transfer_mode;
	uint16_t command;
	uint16_t resp0;
	uint16_t resp1;
	uint16_t resp2;
	uint16_t resp3;
	uint16_t resp4;
	uint16_t resp5;
	uint16_t resp6;
	uint16_t resp7;
	uint32_t buffer_data_port;
	uint32_t present_state;
	uint8_t host_control1;
	uint8_t power_control;
	uint8_t block_gap_control;
	uint8_t wakeup_control;
	uint16_t clock_control;
	uint8_t timeout_control;
	uint8_t software_reset;
	uint16_t normal_interrupt_status_enable;
	uint16_t error_interrupt_status_enable;
	uint16_t normal_interrupt_signal_enable;
	uint16_t error_interrupt_signal_enable;
	uint16_t auto_cmd_error_status;
	uint16_t host_control_2;
	uint32_t capabilities_lo;
	uint32_t capabilities_hi;
	uint64_t maximum_current_capabilities;
	uint16_t force_event_for_auto_cmd_error_status;
	uint16_t force_event_for_error_interrupt_status;
	uint8_t adma_error_status;
	uint8_t unused1;
	uint16_t unused2;
	uint32_t adma_system_address_lo;
	uint32_t adma_system_address_hi;
	uint16_t preset_value_for_initialization;
	uint16_t preset_value_for_default_speed;
	uint16_t preset_value_for_high_speed;
	uint16_t preset_value_for_sdr12;
	uint16_t preset_value_for_sdr25;
	uint16_t preset_value_for_sdr50;
	uint16_t preset_value_for_sdr104;
	uint16_t preset_value_for_ddr50;
	uint8_t unused3[0x70];
	uint32_t shared_bus_control;
	uint8_t unused4[0x18];
	uint16_t slot_interrupt_status;
	uint16_t host_controller_version;
} sd_mmc_registers;

static volatile sd_mmc_registers *sdif_registers(enum sdif_device device)
{
	switch (device) {
	case SDIF_DEVICE_EMMC:
		return (sd_mmc_registers *)0xE0B00000;
	case SDIF_DEVICE_GC:
		return (sd_mmc_registers *)0xE0C00000;
	case SDIF_DEVICE_WLAN_BT:
		return (sd_mmc_registers *)0xE0C10000;
	case SDIF_DEVICE_MICRO_SD:
		return (sd_mmc_registers *)0xE0C20000;
	default:
		return NULL;
	}
}

static inline uint8_t sdhci_readb(enum sdif_device device, uint32_t reg)
{
	return read8((uintptr_t)sdif_registers(device) + reg);
}

static inline uint32_t sdhci_readl(enum sdif_device device, uint32_t reg)
{
	return read32((uintptr_t)sdif_registers(device) + reg);
}

static inline void sdhci_writeb(enum sdif_device device, uint8_t val, uint32_t reg)
{
	write8(val, (uintptr_t)sdif_registers(device) + reg);
}

static inline void sdhci_writew(enum sdif_device device, uint16_t val, uint32_t reg)
{
	write16(val, (uintptr_t)sdif_registers(device) + reg);
}

static inline void sdhci_writel(enum sdif_device device, uint32_t val, uint32_t reg)
{
	write32(val, (uintptr_t)sdif_registers(device) + reg);
}

static int sdif_reset(enum sdif_device device, int flags)
{
	volatile sd_mmc_registers *host_regs = sdif_registers(device);

	if (flags & SDIF_RESET_HW) {
		uint32_t soc = pervasive_get_soc_revision();
		if ((soc & 0x1ff00) == 0) {
			if (!sysroot_model_is_dolce()) {
				if (device < SDIF_DEVICE_WLAN_BT) {
					pervasive_sdif_misc_0x110_0x11C(device, 0x200);
					if (device != SDIF_DEVICE_EMMC) {
						pervasive_reset_enter_sdif(device);
					} else {
						if (g_unk_8 == 1) {
							//ksceDebugPrintf2(0xf, &unk_table, "Skip sdif port[%d] reset\n",0);
						} else {
							pervasive_reset_enter_sdif(0);
							//ksceSblSmSchedProxyExecuteF00DCommand(3, 0, 0, 0);
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
					if (g_unk_8 == 1) {
						//ksceDebugPrintf2(0xf, &unk_table, "Skip sdif port[%d] reset\n",0);
					} else {
						pervasive_reset_enter_sdif(0);
						//ksceSblSmSchedProxyExecuteF00DCommand(3, 0, 0, 0);
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
						if (g_unk_8 == 1) {
							//ksceDebugPrintf2(0xf, &unk_table, "Skip sdif port[%d] reset\n",0);
						} else {
							pervasive_reset_enter_sdif(0);
							//ksceSblSmSchedProxyExecuteF00DCommand(3, 0, 0, 0);
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
						if (g_unk_8 == 1) {
							//ksceDebugPrintf2(0xf, &unk_table, "Skip sdif port[%d] reset\n",0);
						} else {
							pervasive_reset_enter_sdif(0);
							//ksceSblSmSchedProxyExecuteF00DCommand(3, 0, 0, 0);
						}
					}
				}
			} else {
				if (device != SDIF_DEVICE_EMMC) {
					pervasive_reset_enter_sdif(device);
				} else {
					if (g_unk_8 == 1) {
						//ksceDebugPrintf2(0xf, &unk_table, "Skip sdif port[%d] reset\n",0);
					} else {
						pervasive_reset_enter_sdif(0);
						//ksceSblSmSchedProxyExecuteF00DCommand(3, 0, 0, 0);
					}
				}
			}
		}

		pervasive_reset_exit_sdif(device);
		pervasive_clock_enable_sdif(device);

		g_timeout_control[device] = 0xe;
		switch(device) {
		case SDIF_DEVICE_EMMC:
			g_unk_18[device] = 0x80;
			pervasive_sdif_misc_0x124(0, 1);
			break;
		case SDIF_DEVICE_GC:
		case SDIF_DEVICE_MICRO_SD:
			g_unk_18[device] = 0x300000;
			pervasive_sdif_misc_0x124(device, 0);
			break;
		case SDIF_DEVICE_WLAN_BT:
			g_unk_18[device] = 0x80;
			pervasive_sdif_misc_0x124(2, 1);
			break;
		default:
			g_unk_18[device] = 0;
			break;
		}
	}

	if (flags & SDIF_RESET_SW) {
		/* Bit 0 – SWRSTALL Software reset for All */
		host_regs->software_reset = 1;
		do {
		} while (host_regs->software_reset & 1);

		host_regs->timeout_control = g_timeout_control[device];

		uint32_t delay = 102;
		do {
			/* Bit 17 – CARDSS Card State Stable */
			if (host_regs->present_state & (1 << 17)) {
				break;
			}
		} while ((device != SDIF_DEVICE_GC) || (delay--, delay != 0));

		host_regs->normal_interrupt_signal_enable = 0;
		host_regs->error_interrupt_signal_enable = 0;
		host_regs->auto_cmd_error_status = 0;
		host_regs->host_control_2 = 0;
		host_regs->normal_interrupt_status_enable = host_regs->normal_interrupt_status_enable;
		host_regs->error_interrupt_status_enable = host_regs->error_interrupt_status_enable;

		do {
		} while (host_regs->normal_interrupt_status_enable != 0);

		do {
		} while (host_regs->error_interrupt_status_enable != 0);

		uint16_t enable;
		if (device == SDIF_DEVICE_WLAN_BT) {
			enable = 0x1fb;
		} else {
			enable = 0xfb;
		}

		host_regs->normal_interrupt_signal_enable = enable;
		host_regs->error_interrupt_signal_enable = 0x37f;
		host_regs->auto_cmd_error_status = enable;
		host_regs->host_control_2 = 0x37f;
	}

	return 0;
}

int sdif_init(enum sdif_device device)
{
	sdif_reset(device, SDIF_RESET_HW | SDIF_RESET_SW);

	if (sdif_is_card_inserted(device)) {
		enum sdif_bus_voltage voltage;

		if ((g_unk_18[device] & 0x300000) == 0) {
			if ((g_unk_18[device] & 0x60000) == 0) {
				if (g_unk_18[device] & (1 << 7)) {
					voltage = SDIF_BUS_VOLTAGE_1V8;
				} else {
					voltage = 0;
				}
			} else {
				voltage = SDIF_BUS_VOLTAGE_3V0;
			}
		} else {
			voltage = SDIF_BUS_VOLTAGE_3V3;
		}

		sdif_bus_voltage_select(device, voltage);
	}

	return 0;
}

bool sdif_is_card_inserted(enum sdif_device device)
{
	volatile sd_mmc_registers *host_regs = sdif_registers(device);

	return (host_regs->present_state >> 16) & 1;
}

void sdif_bus_voltage_select(enum sdif_device device, enum sdif_bus_voltage voltage)
{
	volatile sd_mmc_registers *host_regs = sdif_registers(device);

	uint32_t delay = 102;
	do {
		/* Bit 17 – CARDSS Card State Stable */
		if ((host_regs->present_state >> 17) & 1)
			break;
	} while ((device != SDIF_DEVICE_GC) || (delay--, delay != 0));


	/* Bit 16 – CARDINS Card Inserted */
	if ((host_regs->present_state >> 16) & 1) {
		host_regs->power_control &= 0xfe;
		host_regs->power_control |= voltage << 1;
		host_regs->power_control |= 1;
		host_regs->clock_control = 0;
		host_regs->clock_control = 0x8001;

		do {
			/* Bit 1 – INTCLKS Internal Clock Stable */
		} while (!(host_regs->clock_control & 2));

		if (device == SDIF_DEVICE_WLAN_BT)
			host_regs->clock_control |= 4;

		host_regs->host_control1 = 0;
	}
}

static void sdhci_reset(enum sdif_device host, uint8_t mask)
{
	unsigned long timeout;

	/* Wait max 100 ms */
	timeout = 100;
	sdhci_writeb(host, mask, SDHCI_SOFTWARE_RESET);
	while (sdhci_readb(host, SDHCI_SOFTWARE_RESET) & mask) {
		if (timeout == 0) {
			printf("%s: Reset 0x%x never completed.\n",
			       __func__, (int)mask);
			return;
		}
		timeout--;
		udelay(1000);
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
	int trans_bytes = 0, is_aligned = 1;
	uint32_t mask, flags, mode = 0;
	uint32_t time = 0;
	uint64_t start = get_timer(0);
	/* Timeout unit - ms */
	static unsigned int cmd_timeout = SDHCI_CMD_DEFAULT_TIMEOUT;

	mask = SDHCI_CMD_INHIBIT | SDHCI_DATA_INHIBIT;

	/* We shouldn't wait for data inihibit for stop commands, even
	   though they might use busy signaling */
	if (cmd->cmdidx == MMC_CMD_STOP_TRANSMISSION ||
	    ((cmd->cmdidx == MMC_CMD_SEND_TUNING_BLOCK ||
	      cmd->cmdidx == MMC_CMD_SEND_TUNING_BLOCK_HS200) && !data))
		mask &= ~SDHCI_DATA_INHIBIT;

	while (sdhci_readl(host, SDHCI_PRESENT_STATE) & mask) {
		if (time >= cmd_timeout) {
			printf("%s: MMC: %d busy ", __func__, host);
			if (2 * cmd_timeout <= SDHCI_CMD_MAX_TIMEOUT) {
				cmd_timeout += cmd_timeout;
				printf("timeout increasing to: %u ms.\n",
				       cmd_timeout);
			} else {
				puts("timeout.\n");
				return -1;
			}
		}
		time++;
		udelay(1000);
	}

	sdhci_writel(host, SDHCI_INT_ALL_MASK, SDHCI_INT_STATUS);

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
		sdhci_writeb(host, 0xe, SDHCI_TIMEOUT_CONTROL);

		//if (!(host->quirks & SDHCI_QUIRK_SUPPORT_SINGLE))
		//	mode = SDHCI_TRNS_BLK_CNT_EN;
		trans_bytes = data->blocks * data->blocksize;
		if (data->blocks > 1)
			mode |= SDHCI_TRNS_MULTI | SDHCI_TRNS_BLK_CNT_EN;

		if (data->flags == MMC_DATA_READ)
			mode |= SDHCI_TRNS_READ;

		/*if (host->flags & USE_DMA) {
			mode |= SDHCI_TRNS_DMA;
			sdhci_prepare_dma(host, data, &is_aligned, trans_bytes);
		}*/

		sdhci_writew(host, SDHCI_MAKE_BLKSZ(SDHCI_DEFAULT_BOUNDARY_ARG,
				data->blocksize),
				SDHCI_BLOCK_SIZE);
		sdhci_writew(host, data->blocks, SDHCI_BLOCK_COUNT);
		sdhci_writew(host, mode, SDHCI_TRANSFER_MODE);
	} else if (cmd->resp_type & MMC_RSP_BUSY) {
		sdhci_writeb(host, 0xe, SDHCI_TIMEOUT_CONTROL);
	}

	sdhci_writel(host, cmd->cmdarg, SDHCI_ARGUMENT);
	sdhci_writew(host, SDHCI_MAKE_CMD(cmd->cmdidx, flags), SDHCI_COMMAND);
	start = get_timer(0);
	do {
		stat = sdhci_readl(host, SDHCI_INT_STATUS);
		if (stat & SDHCI_INT_ERROR)
			break;

		/*if (host->quirks & SDHCI_QUIRK_BROKEN_R1B &&
		    cmd->resp_type & MMC_RSP_BUSY && !data) {
			unsigned int state =
				sdhci_readl(host, SDHCI_PRESENT_STATE);

			if (!(state & SDHCI_DAT_ACTIVE))
				return 0;
		}*/

		if (get_timer(start) >= SDHCI_READ_STATUS_TIMEOUT) {
			printf("%s: Timeout for status update: %08x %08x\n",
			       __func__, stat, mask);
			return -2;
		}
	} while ((stat & mask) != mask);

	if ((stat & (SDHCI_INT_ERROR | mask)) == mask) {
		sdhci_cmd_done(host, cmd);
		sdhci_writel(host, mask, SDHCI_INT_STATUS);
	} else
		ret = -1;

	/*if (!ret && data)
		ret = sdhci_transfer_data(host, data);*/

	/*if (host->quirks & SDHCI_QUIRK_WAIT_SEND_CMD)
		udelay(1000);*/

	stat = sdhci_readl(host, SDHCI_INT_STATUS);
	sdhci_writel(host, SDHCI_INT_ALL_MASK, SDHCI_INT_STATUS);
	if (!ret) {
		/*if ((host->quirks & SDHCI_QUIRK_32BIT_DMA_ADDR) &&
				!is_aligned && (data->flags == MMC_DATA_READ))
			memcpy(data->dest, host->align_buffer, trans_bytes);*/
		return 0;
	}

	sdhci_reset(host, SDHCI_RESET_CMD);
	sdhci_reset(host, SDHCI_RESET_DATA);
	if (stat & SDHCI_INT_TIMEOUT)
		return -2;
	else
		return -1;

	return 0;
}

int sdif_mmc_go_idle(enum sdif_device device)
{
	struct mmc_cmd cmd;

	cmd.cmdidx = MMC_CMD_GO_IDLE_STATE;
	cmd.cmdarg = 0;
	cmd.resp_type = MMC_RSP_NONE;

	return sdif_send_cmd(device, &cmd, NULL);
}
