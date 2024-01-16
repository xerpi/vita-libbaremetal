#include <string.h>
#include "sdif.h"
#include "pervasive.h"
#include "syscon.h"
#include "sysroot.h"
#include "utils.h"

#define SDIF_RESET_HW (1 << 0)
#define SDIF_RESET_SW (1 << 1)

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
	return 0;
}

bool sdif_is_card_inserted(enum sdif_device device)
{
	volatile sd_mmc_registers *host_regs = sdif_registers(device);

	return (host_regs->present_state >> 16) & 1;
}
