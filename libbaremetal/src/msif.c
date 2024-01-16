/*
 * For more information about the MC authentication process check:
 *   https://github.com/motoharu-gosuto/psvcmd56/blob/master/src/CMD56Reversed/SceMsif.cpp
 * and
 *   https://wiki.henkaku.xyz/vita/Memory_Card
 */

#include <string.h>
#include "msif.h"
#include "pervasive.h"
#include "sysroot.h"
#include "utils.h"
#include "mbedtls/cmac.h"
#include "mbedtls/aes.h"
#include "mbedtls/des.h"

#define MSIF_BASE_ADDR		0xE0900000

#define MSIF_COMMAND_REG	0x30
#define MSIF_DATA_REG		0x34
#define MSIF_STATUS_REG		0x38
#define MSIF_SYSTEM_REG		0x3C

#define MSIF_STATUS_TIMEOUT	0x0100
#define MSIF_STATUS_CRC_ERROR	0x0200
#define MSIF_STATUS_READY	0x1000
#define MSIF_STATUS_UNK1	0x2000
#define MSIF_STATUS_FIFO_RW	0x4000

#define MSIF_SYSTEM_RESET	0x8000

#define MS_TPC_READ_LONG_DATA	0x2000
#define MS_TPC_READ_SHORT_DATA	0x3000
#define MS_TPC_READ_REG		0x4000
#define MS_TPC_GET_INT		0x7000
#define MS_TPC_SET_RW_REG_ADRS	0x8000
#define MS_TPC_EX_SET_CMD	0x9000
#define MS_TPC_WRITE_REG	0xB000
#define MS_TPC_WRITE_SHORT_DATA	0xC000
#define MS_TPC_WRITE_LONG_DATA	0xD000
#define MS_TPC_SET_CMD		0xE000

#define MS_TPC_MSIF_AUTH_48 0x48
#define MS_TPC_MSIF_AUTH_49 0x49
#define MS_TPC_MSIF_AUTH_4A 0x4A
#define MS_TPC_MSIF_AUTH_4B 0x4B

#define MS_PARAM_REG_SYS_PARAM	0x10
#define MS_PARAM_REG_TPC_PARAM	0x17

#define MS_INT_REG_CMDNK	0x01
#define MS_INT_REG_BREQ		0x20
#define MS_INT_REG_ERR		0x40
#define MS_INT_REG_CED		0x80

#define MSPRO_CMD_READ_DATA	0x20
#define MSPRO_CMD_WRITE_DATA	0x21
#define MSPRO_CMD_READ_ATRB	0x24
#define MSPRO_CMD_STOP		0x25

#define MS_STATUS_WP		0x01

#define MS_SYS_PAR4		0x00
#define MS_SYS_PAR8		0x40
#define MS_SYS_SERIAL		0x80

struct ms_status_registers {
	uint8_t reserved0;
	uint8_t interrupt;
	uint8_t status;
	uint8_t reserved1;
	uint8_t type;
	uint8_t reserved2;
	uint8_t category;
	uint8_t class;
} __attribute__((packed));

struct msif_auth_dmac5_41_req1 {
	uint8_t f00d_1C_key[0x10];
	uint8_t card_info[0x8];
	uint8_t challenge[0x8];
	uint8_t session_id[8];
} __attribute__((packed));

struct msif_auth_dmac5_41_req2 {
	uint8_t session_id[0x8];
	uint8_t challenge[0x8];
} __attribute__((packed));

struct msif_auth_tpc_cmd48_req {
	uint8_t session_id[0x8];
	uint8_t f00d_cmd1_data;
	uint8_t reserved[0x17];
} __attribute__((packed));

struct msif_auth_tpc_cmd49_resp {
	uint8_t f00d_1C_key[0x10];
	uint8_t card_info[0x8];
	uint8_t challenge[0x8];
	uint8_t iv[0x08];
	uint8_t reserved[0x18];
} __attribute__((packed));

struct msif_auth_tpc_cmd4A_req {
	uint8_t iv[0x8];
	uint8_t reserved[0x18];
} __attribute__((packed));

static void auth_get_random_data(uint8_t *buf, int size)
{
	memset(buf, 0, size);
}

static void aes_cbc_enc(void *dst, const void *src, const void *key, void *iv,
		        uint32_t key_size, uint32_t size)
{
	mbedtls_aes_context aes;

	mbedtls_aes_init(&aes);
	mbedtls_aes_setkey_enc(&aes, key, key_size);
	mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, size, iv, src, dst);
	mbedtls_aes_free(&aes);
}

static void des3_cbc_cts_enc_iv_0(void *dst, const void *src, const void *key, uint32_t size)
{
	uint8_t iv[8];
	mbedtls_des3_context des3;

	memset(iv, 0, sizeof(iv));
	mbedtls_des3_init(&des3);
	mbedtls_des3_set3key_enc(&des3, key);
	mbedtls_des3_crypt_cbc(&des3, MBEDTLS_DES_ENCRYPT, size, iv, src, dst);
	mbedtls_des3_free(&des3);
}

static inline void ms_tpc(uint32_t tpc, uint32_t size)
{
	write16(tpc | (size & 0x7FF), MSIF_BASE_ADDR + MSIF_COMMAND_REG);
}

static int ms_wait_ready(void)
{
	uint32_t tmp;

	do {
		tmp = read16(MSIF_BASE_ADDR + MSIF_STATUS_REG);
	} while (!(tmp & (MSIF_STATUS_READY | MSIF_STATUS_TIMEOUT | MSIF_STATUS_CRC_ERROR)));

	if (tmp & (MSIF_STATUS_TIMEOUT | MSIF_STATUS_CRC_ERROR))
		return -1;

	return 0;
}

static void ms_wait_fifo_rw(void)
{
	uint32_t tmp;

	do {
		tmp = read16(MSIF_BASE_ADDR + MSIF_STATUS_REG);
	} while (!(tmp & MSIF_STATUS_FIFO_RW));
}

static void ms_wait_unk1(void)
{
	uint32_t tmp;

	do {
		tmp = read16(MSIF_BASE_ADDR + MSIF_STATUS_REG);
	} while (!(tmp & MSIF_STATUS_UNK1));
}

static void ms_wait_not_reset(void)
{
	uint32_t tmp;

	do {
		tmp = read16(MSIF_BASE_ADDR + MSIF_SYSTEM_REG);
	} while (tmp & MSIF_SYSTEM_RESET);
}

static void ms_fifo_read(void *buff, uint32_t size)
{
	uint32_t data[2];
	uint32_t *ptr = buff;

	while (size > 8) {
		ms_wait_fifo_rw();

		ptr[0] = read32(MSIF_BASE_ADDR + MSIF_DATA_REG);
		ptr[1] = read32(MSIF_BASE_ADDR + MSIF_DATA_REG);

		ptr += 2;
		size -= 8;
	}

	if (size > 0) {
		ms_wait_fifo_rw();

		data[0] = read32(MSIF_BASE_ADDR + MSIF_DATA_REG);
		data[1] = read32(MSIF_BASE_ADDR + MSIF_DATA_REG);

		memcpy(ptr, data, size);
	}
}

static void ms_fifo_write(const void *buff, uint32_t size)
{
	uint32_t data[2];
	const uint32_t *ptr = buff;

	while (size > 8) {
		ms_wait_fifo_rw();

		write32(ptr[0], MSIF_BASE_ADDR + MSIF_DATA_REG);
		write32(ptr[1], MSIF_BASE_ADDR + MSIF_DATA_REG);

		ptr += 2;
		size -= 8;
	}

	if (size > 0) {
		ms_wait_fifo_rw();

		memcpy(data, ptr, size);
		memset((char *)data + size, 0, sizeof(data) - size);

		write32(data[0], MSIF_BASE_ADDR + MSIF_DATA_REG);
		write32(data[1], MSIF_BASE_ADDR + MSIF_DATA_REG);
	}
}

static int ms_set_rw_reg_adrs(uint8_t read_addr, uint32_t read_size,
			       uint8_t write_addr, uint32_t write_size)
{
	uint8_t buff[4];

	buff[0] = read_addr;
	buff[1] = read_size;
	buff[2] = write_addr;
	buff[3] = write_size;

	ms_tpc(MS_TPC_SET_RW_REG_ADRS, sizeof(buff));
	ms_fifo_write(buff, sizeof(buff));

	return ms_wait_ready();
}

static int ms_read_reg(uint8_t addr, void *buff, uint32_t size)
{
	ms_set_rw_reg_adrs(addr, size, 0, 0);
	ms_tpc(MS_TPC_READ_REG, size);
	ms_fifo_read(buff, size);
	return ms_wait_ready();
}

static int ms_write_reg(uint8_t addr, const void *buff, uint32_t size)
{
	ms_set_rw_reg_adrs(0, 0, addr, size);
	ms_tpc(MS_TPC_WRITE_REG, size);
	ms_fifo_write(buff, size);
	return ms_wait_ready();
}

static int ms_get_reg_int(uint8_t *reg_int)
{
	ms_tpc(MS_TPC_GET_INT, sizeof(*reg_int));
	ms_fifo_read(reg_int, sizeof(*reg_int));
	return ms_wait_ready();
}

static void ms_reg_int_wait_ced(void)
{
	int ret;
	uint8_t reg_int;

	do {
		ret = ms_get_reg_int(&reg_int);
	} while ((ret < 0) || !(reg_int & MS_INT_REG_CED));
}

static void ms_reg_int_wait_breq(void)
{
	int ret;
	uint8_t reg_int;

	do {
		ret = ms_get_reg_int(&reg_int);
	} while ((ret < 0) || !(reg_int & MS_INT_REG_BREQ));
}

static int ms_set_short_data_size(uint32_t size)
{
	uint8_t tpc_param;

	if (size == 32)
		tpc_param = 0;
	else if (size == 64)
		tpc_param = 1;
	else if (size == 128)
		tpc_param = 2;
	else if (size == 256)
		tpc_param = 3;
	else
		return -1;

	ms_write_reg(MS_PARAM_REG_TPC_PARAM, &tpc_param, sizeof(tpc_param));

	return 0;
}

static void msif_set_clock_for_bus_mode(uint32_t bus_mode)
{
	uint32_t clock;

	if (bus_mode == 1)
		clock = 4;
	else if (bus_mode <= 4)
		clock = 5;
	else
		clock = 6;

	pervasive_msif_set_clock(clock);
}

static void msif_set_bus_mode(uint32_t bus_mode)
{
	uint16_t val;

	val = read16(MSIF_BASE_ADDR + MSIF_SYSTEM_REG);

	if (bus_mode == 1)
		val = (val & 0xFFBF) | 0x80;
	else if (bus_mode == 5)
		val = (val & 0xFF7F) | 0x40;
	else
		val = val & 0xFF3F;

	write16(val, MSIF_BASE_ADDR + MSIF_SYSTEM_REG);

	msif_set_clock_for_bus_mode(bus_mode);
}

static void ms_set_bus_mode(uint32_t bus_mode)
{
	uint8_t sys_param;

	switch (bus_mode) {
	case 0:
	case 2:
		sys_param = MS_SYS_PAR8;
		break;
	case 1:
		sys_param = MS_SYS_SERIAL;
		break;
	case 3:
		sys_param = MS_SYS_SERIAL | 8;
		break;
	case 4:
		sys_param = MS_SYS_PAR4;
		break;
	}

	ms_write_reg(MS_PARAM_REG_SYS_PARAM, &sys_param, sizeof(sys_param));
	msif_set_bus_mode(bus_mode);
}

static void msif_reg_0x100_mask_sub_C2868C(uint32_t val, uint32_t mask)
{
	uint32_t reg;

	reg = read32(MSIF_BASE_ADDR + 0x30 + 0x70);
	reg = (reg & mask) | (val & ~mask);
	write32(reg, MSIF_BASE_ADDR + 0x30 + 0x70);
}

void msif_init(void)
{
	pervasive_reset_enter_msif();
	pervasive_msif_set_clock(4);
	pervasive_reset_exit_msif();
	pervasive_clock_enable_msif();
}

static void msif_reset_sub_C286C4(void)
{
	uint32_t val;
	uint16_t tmp;

	msif_set_clock_for_bus_mode(1);

	/* sub_C28A74 */
	val = read32(MSIF_BASE_ADDR + 0x30 + 0x70);

	tmp = read16(MSIF_BASE_ADDR + 0x30 - 0x2C);
	tmp |= 1;
	write16(tmp, MSIF_BASE_ADDR + 0x30 - 0x2C);

	while (1) {
		tmp = read16(MSIF_BASE_ADDR + 0x30 - 0x2C);
		if (!(tmp & 1)) {
			write32(val, MSIF_BASE_ADDR + 0x30 + 0x70);

			tmp = read16(MSIF_BASE_ADDR + 0x30 - 0x2C);
			tmp |= 4;
			write16(tmp, MSIF_BASE_ADDR + 0x30 - 0x2C);
			break;
		}
	}

	/* sub_C2AD1C */
	tmp = read16(MSIF_BASE_ADDR + MSIF_SYSTEM_REG);
	tmp |= MSIF_SYSTEM_RESET;
	write16(tmp, MSIF_BASE_ADDR + MSIF_SYSTEM_REG);

	ms_wait_not_reset();

	tmp = read16(MSIF_BASE_ADDR + MSIF_SYSTEM_REG);
	/*
	 * Enabling this doesn't work. Default value = 0x20A5.
	tmp &= 0xFFF8;
	tmp |= 0x4005;
	*/
	write16(tmp, MSIF_BASE_ADDR + MSIF_SYSTEM_REG);

	ms_reg_int_wait_ced();
}

static void ms_get_info(uint32_t *model_name_type, uint32_t *unkC20,
			uint32_t *write_protected)
{
	uint32_t type = 0;
	struct ms_status_registers status;

	ms_read_reg(0, &status, sizeof(status));

	if (status.status & MS_STATUS_WP) {
		*write_protected = 1;
		*unkC20 = 0;
	}

	if (status.type + 1 <= 1) {
		if (status.type != status.category) {
			if (status.category - 1 > 0x7E)
				type = 0;
			else
				type = 0x40;
		} else if ((status.type == status.class) || (status.class - 1 <= 2)) {
			uint32_t v15;

			if (status.type == 0) {
				v15 = 0x2010;
				type = 0x2011;
			} else {
				v15 = 0x10;
				type = 0x11;
			}

			if (status.class != 0xFF)
				type = (1 << status.class) | v15;
		}
	} else if (status.type == 1) {
		if (status.category == 0) {
			if (status.class <= 3) {
				if (status.reserved2 == 0) {
					type = (1 << status.class) | 0x2020;
				} else if (status.reserved2 == 7) {
					uint32_t v16 = 0xA820;
					if (!(read16(MSIF_BASE_ADDR + MSIF_STATUS_REG) & 0x400))
						v16 = 0xE820;
					type = (1 << status.class) | v16;
				}
			} else {
				type = 0;
			}
		} else if (status.category == 0x10) {
			if (status.reserved2)
				type = 0;
			else
				type = 0x2080;

			if (status.reserved2 == 7) {
				type = 0xA880;
				if (!(read16(MSIF_BASE_ADDR + MSIF_STATUS_REG) & 0x400))
					type = 0xE880;
			}
		}
	} else {
		type = 0;
	}

	*model_name_type = type;
}

static void msif_auth_derive_iv_tweak(const uint8_t *tweak_seed,
				      uint8_t *tweak_key0,
				      uint8_t *tweak_key1)
{
	uint64_t tmp;

	tmp = be_uint64_t_load(tweak_seed) * 2;
	be_uint64_t_store(tweak_key0, tmp);
	tweak_key0[7] = ((tweak_seed[0] & 0x80) > 0) ? (tweak_key0[7] ^ 0x1B) : tweak_key0[7];

	tmp = be_uint64_t_load(tweak_key0) * 2;
	be_uint64_t_store(tweak_key1, tmp);
	tweak_key1[7] = ((tweak_key0[0] & 0x80) > 0) ? (tweak_key1[7] ^ 0x1B) : tweak_key1[7];
}

static void msif_auth_derive_iv(void *result, const void *data, const void *key, uint32_t size)
{
	uint8_t tweak_seed_enc[8];
	uint8_t tweak_seed[8];
	memset(tweak_seed, 0, sizeof(tweak_seed));
	des3_cbc_cts_enc_iv_0(tweak_seed_enc, tweak_seed, key, 8);

	uint32_t tweak_key0[2];
	uint32_t tweak_key1[2];
	msif_auth_derive_iv_tweak(tweak_seed_enc, (uint8_t *)tweak_key0,
				  (uint8_t *)tweak_key1);
	if (size <= 8)
		return;

	const uint32_t *current_ptr = data;
	uint32_t current_size = size;

	uint32_t IV[2];
	memset(IV, 0, sizeof(IV));

	while (current_size > 8) {
		int current_round[2];
		current_round[0] = current_ptr[0] ^ IV[0];
		current_round[1] = current_ptr[1] ^ IV[1];

		des3_cbc_cts_enc_iv_0(IV, current_round, key, 8);

		current_ptr = current_ptr + 2;
		current_size = current_size - 8;
	}

	uint32_t IV_mod[2];
	uint32_t tail_data[2];

	if (current_size == 8) {
		tail_data[0] = current_ptr[0];
		tail_data[1] = current_ptr[1];
		IV_mod[0] = IV[0] ^ tweak_key0[0];
		IV_mod[1] = IV[1] ^ tweak_key0[1];
	} else {
		tail_data[0] = current_ptr[0];
		tail_data[1] = current_ptr[1];
		memset(((char *)tail_data) + current_size, 0, 8 - current_size);
		IV_mod[0] = IV[0] ^ tweak_key1[0];
		IV_mod[1] = IV[1] ^ tweak_key1[1];
	}

	uint32_t final_round[2];
	final_round[0] = tail_data[0] ^ IV_mod[0];
	final_round[1] = tail_data[1] ^ IV_mod[1];

	des3_cbc_cts_enc_iv_0(result, final_round, key, 8);
}

static void rmauth_cmd_0x1(uint32_t *res)
{
	*res = 0;
}

static void rmauth_cmd_0x2(const uint8_t key[32],
			   const uint8_t seed[32],
			   uint8_t out_key[32])
{
	uint8_t iv[MBEDTLS_AES_BLOCK_SIZE];
	uint8_t seed_trunc[16];

	memcpy(seed_trunc, seed, 16);

	memset(iv, 0, MBEDTLS_AES_BLOCK_SIZE);
	aes_cbc_enc(&out_key[0], seed_trunc, &key[0], iv, 128, 16);

	memset(iv, 0, MBEDTLS_AES_BLOCK_SIZE);
	aes_cbc_enc(&out_key[16], seed_trunc, &key[16], iv, 128, 16);
}

static void msif_auth(const uint8_t key[32], uint32_t auth_val)
{
	uint8_t key_1C[32];

	uint32_t f00d_cmd1_res;
	rmauth_cmd_0x1(&f00d_cmd1_res);

	uint8_t session_id[8];
	auth_get_random_data(session_id, sizeof(session_id));

	/* Execute TPC cmd 0x48 - send request - establish session with memory card */
	struct msif_auth_tpc_cmd48_req cmd48_req;
	memcpy(cmd48_req.session_id, session_id, sizeof(session_id));
	cmd48_req.f00d_cmd1_data = f00d_cmd1_res;
	memset(cmd48_req.reserved, 0, 0x17);
	msif_write_short_data(MS_TPC_MSIF_AUTH_48, &cmd48_req, 0x20);

	/* Execute TPC cmd 0x49 - get response - result of establishing session with memory card */
	struct msif_auth_tpc_cmd49_resp cmd49_resp;
	memset(&cmd49_resp, 0, sizeof(cmd49_resp));
	msif_read_short_data(MS_TPC_MSIF_AUTH_49, &cmd49_resp, 0x40);

	/* Execute f00d rm_auth cmd 0x2, scrambles and sets the key into slot 0x1C */
	rmauth_cmd_0x2(key, cmd49_resp.f00d_1C_key, key_1C);

	/* Prepare 3des-cbc-cts (dmac5 cmd 0x41 request) data */
	struct msif_auth_dmac5_41_req1 d5req1;
	memcpy(d5req1.f00d_1C_key, cmd49_resp.f00d_1C_key, 0x10);
	memcpy(d5req1.card_info, cmd49_resp.card_info, 0x8);
	memcpy(d5req1.challenge, cmd49_resp.challenge, 0x8);
	memcpy(d5req1.session_id, session_id, 0x8);

	/* Encrypt prepared buffer with 3des-cbc-cts and obtain IV */
	char des3_iv_1[0x8];
	memset(des3_iv_1, 0, sizeof(des3_iv_1));
	msif_auth_derive_iv(des3_iv_1, &d5req1, key_1C, 0x28);

	/* Verify that IV matches to the one, given in TPC 0x49 response */
	/* assert(memcmp(des3_iv_1, cmd49_resp.iv, 8) == 0) */

	/* Prepare 3des-cbc-cts (dmac5 cmd 0x41 request) data */
	struct msif_auth_dmac5_41_req2 d5req2;
	memcpy(d5req2.session_id, d5req1.session_id, 8);
	memcpy(d5req2.challenge, cmd49_resp.challenge, 8);

	/* Encrypt prepared buffer with 3des-cbc-cts and obtain IV */
	char des3_iv_2[0x8];
	memset(des3_iv_2, 0, sizeof(des3_iv_2));
	msif_auth_derive_iv(des3_iv_2, &d5req2, key_1C, 0x10);

	/* Execute TPC cmd 0x4A - send request - complete auth */
	struct msif_auth_tpc_cmd4A_req cmd4A_req;
	memcpy(cmd4A_req.iv, des3_iv_2, 8);
	memset(cmd4A_req.reserved, 0, 0x18);
	msif_write_short_data(MS_TPC_MSIF_AUTH_4A, &cmd4A_req, 0x20);
}

void msif_setup(const uint8_t key[32])
{
	uint32_t soc_revision;
	uint32_t hw_info_masked;
	uint32_t model_name_type, unkC20, write_protected;

	msif_reset_sub_C286C4();
	ms_get_info(&model_name_type, &unkC20, &write_protected);

	soc_revision = pervasive_get_soc_revision();
	hw_info_masked = sysroot_get_hw_info() & 0xFF0000;

	if (soc_revision) {
		if (soc_revision == 0x100) {
			uint32_t val;
			if (hw_info_masked < 0x800000)
				val = 0x35000C;
			else
				val = 0x7000A;
			msif_reg_0x100_mask_sub_C2868C(val, 0xFF00FFF0);
		}
	} else if (sysroot_model_is_dolce()) {
		msif_reg_0x100_mask_sub_C2868C(0, 0xFFFF80FF);
	} else {
		msif_reg_0x100_mask_sub_C2868C(0x300, 0xFFFF80FF);
	}

	/*
	 * Switch to parallel 4 mode.
	 */
	ms_set_bus_mode(4);

	if ((model_name_type & 0xF0) == 0x10)
		msif_auth(key, 2);
	else if ((model_name_type & 0xF0) == 0x20)
		msif_auth(key, 5);
}

void msif_get_info(struct msif_info *info)
{
	/* TODO */
}

void msif_read_sector(uint32_t sector, void *buff)
{
	const uint32_t count = 1;
	uint8_t data[7];

	data[0] = MSPRO_CMD_READ_DATA;
	data[1] = (count >> 8) & 0xFF;
	data[2] = count & 0xFF;
	data[3] = (sector >> 24) & 0xFF;
	data[4] = (sector >> 16) & 0xFF;
	data[5] = (sector >> 8) & 0xFF;
	data[6] = sector & 0xFF;

	ms_tpc(MS_TPC_EX_SET_CMD, sizeof(data));
	ms_fifo_write(data, sizeof(data));

	ms_wait_ready();
	ms_wait_unk1();
	ms_reg_int_wait_breq();

	ms_tpc(MS_TPC_READ_LONG_DATA, count * MS_SECTOR_SIZE);
	ms_fifo_read(buff, count * MS_SECTOR_SIZE);

	ms_wait_ready();
	ms_wait_unk1();
	ms_reg_int_wait_ced();
}

void msif_read_atrb(uint32_t address, void *buff)
{
	const uint32_t count = 1;
	uint8_t data[7];

	data[0] = MSPRO_CMD_READ_ATRB;
	data[1] = (count >> 8) & 0xFF;
	data[2] = count & 0xFF;
	data[3] = (address >> 24) & 0xFF;
	data[4] = (address >> 16) & 0xFF;
	data[5] = (address >> 8) & 0xFF;
	data[6] = address & 0xFF;

	ms_tpc(MS_TPC_EX_SET_CMD, sizeof(data));
	ms_fifo_write(data, sizeof(data));

	ms_wait_ready();
	ms_wait_unk1();
	ms_reg_int_wait_breq();

	ms_tpc(MS_TPC_READ_LONG_DATA, count * MS_SECTOR_SIZE);
	ms_fifo_read(buff, count * MS_SECTOR_SIZE);

	ms_wait_ready();
	ms_wait_unk1();
	ms_reg_int_wait_ced();
}

void msif_read_short_data(uint8_t cmd, void *buff, uint32_t size)
{
	if (ms_set_short_data_size(size))
		return;

	ms_tpc(MS_TPC_SET_CMD, sizeof(cmd));
	ms_fifo_write(&cmd, sizeof(cmd));

	ms_wait_ready();
	ms_wait_unk1();
	ms_reg_int_wait_breq();

	ms_tpc(MS_TPC_READ_SHORT_DATA, size);
	ms_fifo_read(buff, size);

	ms_wait_ready();
	ms_wait_unk1();
	ms_reg_int_wait_ced();
}

void msif_write_short_data(uint8_t cmd, const void *buff, uint32_t size)
{
	if (ms_set_short_data_size(size))
		return;

	ms_tpc(MS_TPC_SET_CMD, sizeof(cmd));
	ms_fifo_write(&cmd, sizeof(cmd));

	ms_wait_ready();
	ms_wait_unk1();
	ms_reg_int_wait_breq();

	ms_tpc(MS_TPC_WRITE_SHORT_DATA, size);
	ms_fifo_write(buff, size);

	ms_wait_ready();
	ms_wait_unk1();
	ms_reg_int_wait_ced();
}
