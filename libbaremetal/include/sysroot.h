#ifndef SYSROOT_H
#define SYSROOT_H

#include <stdint.h>

struct sysroot_buffer {
	uint16_t version;
	uint16_t size;
	uint32_t current_firmware_version;
	uint32_t firmware_version_shipped_from_factory;
	uint32_t unk0c[(0x2c - 0x0c) / 4];
	uint64_t bitfield_flags;
	uint32_t unk34[(0x40 - 0x34) / 4];
	uint32_t devkit_function_address_1;
	uint32_t devkit_uid;
	uint32_t devkit_function_address_2;
	uint32_t aslr_seed;
	uint32_t devkit_config_flags;
	uint32_t devkit_config_flags2;
	uint32_t devkit_config;
	uint32_t devkit_config_flags3;
	uint32_t dram_base_paddr;
	uint32_t dram_size;
	uint32_t unk68;
	uint32_t boot_type_indicator_1;
	uint8_t openpsid[0x10];
	uint32_t secure_kernel_enp_raw_data_paddr;
	uint32_t secure_kernel_enp_size;
	uint32_t unk88;
	uint32_t unk8c;
	uint32_t kprx_auth_sm_self_raw_data_paddr;
	uint32_t kprx_auth_sm_self_size;
	uint32_t prog_rvk_srvk_raw_data_paddr;
	uint32_t prog_rvk_srvk_size;
	uint16_t model;
	uint16_t device_type;
	uint16_t device_config;
	uint16_t type;
	uint16_t unka8[(0xb0 - 0xa8) / 2];
	uint8_t session_id[0x10];
	uint32_t unkc0;
	uint32_t wakeup_factor;
	uint32_t unkc8[(0xd0 - 0xc8) / 4];
	uint32_t suspend_saved_context_paddr;
	uint32_t hw_info;
	uint32_t boot_type_indicator_2;
	uint8_t unkdc[0xC];
	uint8_t hw_flags[0x10];
	uint32_t bootloader_revision;
	uint32_t sysroot_magic_value;
	uint8_t encrypted_session_key[0x20];
} __attribute__((packed));

void sysroot_init(const struct sysroot_buffer *sysroot_buffer);
uint32_t sysroot_get_hw_info(void);
int sysroot_model_is_vita(void);
int sysroot_model_is_dolce(void);
int sysroot_model_is_vita2k(void);
int sysroot_model_is_diag(void);
int sysroot_is_au_codec_ic_conexant(void);

#endif
