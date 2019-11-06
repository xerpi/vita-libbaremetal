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

#define MSIF_BASE_ADDR		((void *)0xE0900000)

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
	unsigned char reserved0;
	unsigned char interrupt;
	unsigned char status;
	unsigned char reserved1;
	unsigned char type;
	unsigned char reserved2;
	unsigned char category;
	unsigned char class;
} __attribute__((packed));

struct msif_auth_dmac5_41_req1 {
	unsigned char f00d_1C_key[0x10];
	unsigned char card_info[0x8];
	unsigned char challenge[0x8];
	unsigned char session_id[8];
} __attribute__((packed));

struct msif_auth_dmac5_41_req2 {
	unsigned char session_id[0x8];
	unsigned char challenge[0x8];
} __attribute__((packed));

struct msif_auth_tpc_cmd48_req {
	unsigned char session_id[0x8];
	unsigned char f00d_cmd1_data;
	unsigned char reserved[0x17];
} __attribute__((packed));

struct msif_auth_tpc_cmd49_resp {
	unsigned char f00d_1C_key[0x10];
	unsigned char card_info[0x8];
	unsigned char challenge[0x8];
	unsigned char iv[0x08];
	unsigned char reserved[0x18];
} __attribute__((packed));

struct msif_auth_tpc_cmd4A_req {
	unsigned char iv[0x8];
	unsigned char reserved[0x18];
} __attribute__((packed));

static void auth_get_random_data(unsigned char *buf, int size)
{
	memset(buf, 0, size);
}

static void aes_cbc_enc(void *dst, const void *src, const void *key, void *iv,
		        unsigned int key_size, unsigned int size)
{
	mbedtls_aes_context aes;

	mbedtls_aes_init(&aes);
	mbedtls_aes_setkey_enc(&aes, key, key_size);
	mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, size, iv, src, dst);
	mbedtls_aes_free(&aes);
}

static void des3_cbc_cts_enc_iv_0(void *dst, const void *src, const void *key, unsigned int size)
{
	unsigned char iv[8];
	mbedtls_des3_context des3;

	memset(iv, 0, sizeof(iv));
	mbedtls_des3_init(&des3);
	mbedtls_des3_set3key_enc(&des3, key);
	mbedtls_des3_crypt_cbc(&des3, MBEDTLS_DES_ENCRYPT, size, iv, src, dst);
	mbedtls_des3_free(&des3);
}

static inline void ms_tpc(unsigned int tpc, unsigned int size)
{
	writew(tpc | (size & 0x7FF), MSIF_BASE_ADDR + MSIF_COMMAND_REG);
}

static int ms_wait_ready(void)
{
	unsigned int tmp;

	do {
		tmp = readw(MSIF_BASE_ADDR + MSIF_STATUS_REG);
	} while (!(tmp & (MSIF_STATUS_READY | MSIF_STATUS_TIMEOUT | MSIF_STATUS_CRC_ERROR)));

	if (tmp & (MSIF_STATUS_TIMEOUT | MSIF_STATUS_CRC_ERROR))
		return -1;

	return 0;
}

static void ms_wait_fifo_rw(void)
{
	unsigned int tmp;

	do {
		tmp = readw(MSIF_BASE_ADDR + MSIF_STATUS_REG);
	} while (!(tmp & MSIF_STATUS_FIFO_RW));
}

static void ms_wait_unk1(void)
{
	unsigned int tmp;

	do {
		tmp = readw(MSIF_BASE_ADDR + MSIF_STATUS_REG);
	} while (!(tmp & MSIF_STATUS_UNK1));
}

static void ms_wait_not_reset(void)
{
	unsigned int tmp;

	do {
		tmp = readw(MSIF_BASE_ADDR + MSIF_SYSTEM_REG);
	} while (tmp & MSIF_SYSTEM_RESET);
}

static void ms_fifo_read(void *buff, unsigned int size)
{
	unsigned int data[2];
	unsigned int *ptr = buff;

	while (size > 8) {
		ms_wait_fifo_rw();

		ptr[0] = readl(MSIF_BASE_ADDR + MSIF_DATA_REG);
		ptr[1] = readl(MSIF_BASE_ADDR + MSIF_DATA_REG);

		ptr += 2;
		size -= 8;
	}

	if (size > 0) {
		ms_wait_fifo_rw();

		data[0] = readl(MSIF_BASE_ADDR + MSIF_DATA_REG);
		data[1] = readl(MSIF_BASE_ADDR + MSIF_DATA_REG);

		memcpy(ptr, data, size);
	}
}

static void ms_fifo_write(const void *buff, unsigned int size)
{
	unsigned int data[2];
	const unsigned int *ptr = buff;

	while (size > 8) {
		ms_wait_fifo_rw();

		writel(ptr[0], MSIF_BASE_ADDR + MSIF_DATA_REG);
		writel(ptr[1], MSIF_BASE_ADDR + MSIF_DATA_REG);

		ptr += 2;
		size -= 8;
	}

	if (size > 0) {
		ms_wait_fifo_rw();

		memcpy(data, ptr, size);
		memset((char *)data + size, 0, sizeof(data) - size);

		writel(data[0], MSIF_BASE_ADDR + MSIF_DATA_REG);
		writel(data[1], MSIF_BASE_ADDR + MSIF_DATA_REG);
	}
}

static int ms_set_rw_reg_adrs(unsigned char read_addr, unsigned int read_size,
			       unsigned char write_addr, unsigned int write_size)
{
	unsigned char buff[4];

	buff[0] = read_addr;
	buff[1] = read_size;
	buff[2] = write_addr;
	buff[3] = write_size;

	ms_tpc(MS_TPC_SET_RW_REG_ADRS, sizeof(buff));
	ms_fifo_write(buff, sizeof(buff));

	return ms_wait_ready();
}

static int ms_read_reg(unsigned char addr, void *buff, unsigned int size)
{
	ms_set_rw_reg_adrs(addr, size, 0, 0);
	ms_tpc(MS_TPC_READ_REG, size);
	ms_fifo_read(buff, size);
	return ms_wait_ready();
}

static int ms_write_reg(unsigned char addr, const void *buff, unsigned int size)
{
	ms_set_rw_reg_adrs(0, 0, addr, size);
	ms_tpc(MS_TPC_WRITE_REG, size);
	ms_fifo_write(buff, size);
	return ms_wait_ready();
}

static int ms_get_reg_int(unsigned char *reg_int)
{
	ms_tpc(MS_TPC_GET_INT, sizeof(*reg_int));
	ms_fifo_read(reg_int, sizeof(*reg_int));
	return ms_wait_ready();
}

static void ms_reg_int_wait_ced(void)
{
	int ret;
	unsigned char reg_int;

	do {
		ret = ms_get_reg_int(&reg_int);
	} while ((ret < 0) || !(reg_int & MS_INT_REG_CED));
}

static void ms_reg_int_wait_breq(void)
{
	int ret;
	unsigned char reg_int;

	do {
		ret = ms_get_reg_int(&reg_int);
	} while ((ret < 0) || !(reg_int & MS_INT_REG_BREQ));
}

static int ms_set_short_data_size(unsigned int size)
{
	unsigned char tpc_param;

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

static void msif_set_clock_for_bus_mode(unsigned int bus_mode)
{
	unsigned int clock;

	if (bus_mode == 1)
		clock = 4;
	else if (bus_mode <= 4)
		clock = 5;
	else
		clock = 6;

	pervasive_msif_set_clock(clock);
}

static void msif_set_bus_mode(unsigned int bus_mode)
{
	unsigned short val;

	val = readw(MSIF_BASE_ADDR + MSIF_SYSTEM_REG);

	if (bus_mode == 1)
		val = (val & 0xFFBF) | 0x80;
	else if (bus_mode == 5)
		val = (val & 0xFF7F) | 0x40;
	else
		val = val & 0xFF3F;

	writew(val, MSIF_BASE_ADDR + MSIF_SYSTEM_REG);

	msif_set_clock_for_bus_mode(bus_mode);
}

static void ms_set_bus_mode(unsigned int bus_mode)
{
	unsigned char sys_param;

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

static void msif_reg_0x100_mask_sub_C2868C(unsigned int val, unsigned int mask)
{
	unsigned int reg;

	reg = readl(MSIF_BASE_ADDR + 0x30 + 0x70);
	reg = (reg & mask) | (val & ~mask);
	writel(reg, MSIF_BASE_ADDR + 0x30 + 0x70);
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
	unsigned int val;
	unsigned short tmp;

	msif_set_clock_for_bus_mode(1);

	/* sub_C28A74 */
	val = readl(MSIF_BASE_ADDR + 0x30 + 0x70);

	tmp = readw(MSIF_BASE_ADDR + 0x30 - 0x2C);
	tmp |= 1;
	writew(tmp, MSIF_BASE_ADDR + 0x30 - 0x2C);

	while (1) {
		tmp = readw(MSIF_BASE_ADDR + 0x30 - 0x2C);
		if (!(tmp & 1)) {
			writel(val, MSIF_BASE_ADDR + 0x30 + 0x70);

			tmp = readw(MSIF_BASE_ADDR + 0x30 - 0x2C);
			tmp |= 4;
			writew(tmp, MSIF_BASE_ADDR + 0x30 - 0x2C);
			break;
		}
	}

	/* sub_C2AD1C */
	tmp = readw(MSIF_BASE_ADDR + MSIF_SYSTEM_REG);
	tmp |= MSIF_SYSTEM_RESET;
	writew(tmp, MSIF_BASE_ADDR + MSIF_SYSTEM_REG);

	ms_wait_not_reset();

	tmp = readw(MSIF_BASE_ADDR + MSIF_SYSTEM_REG);
	/*
	 * Enabling this doesn't work. Default value = 0x20A5.
	tmp &= 0xFFF8;
	tmp |= 0x4005;
	*/
	writew(tmp, MSIF_BASE_ADDR + MSIF_SYSTEM_REG);

	ms_reg_int_wait_ced();
}

static void ms_get_info(unsigned int *model_name_type, unsigned int *unkC20,
			unsigned int *write_protected)
{
	unsigned int type;
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
			unsigned int v15;

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
					unsigned int v16 = 0xA820;
					if (!(readw(MSIF_BASE_ADDR + MSIF_STATUS_REG) & 0x400))
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
				if (!(readw(MSIF_BASE_ADDR + MSIF_STATUS_REG) & 0x400))
					type = 0xE880;
			}
		}
	} else {
		type = 0;
	}

	*model_name_type = type;
}

static void msif_auth_derive_iv_tweak(const unsigned char *tweak_seed,
				      unsigned char *tweak_key0,
				      unsigned char *tweak_key1)
{
	uint64_t tmp;

	tmp = be_uint64_t_load(tweak_seed) * 2;
	be_uint64_t_store(tweak_key0, tmp);
	tweak_key0[7] = ((tweak_seed[0] & 0x80) > 0) ? (tweak_key0[7] ^ 0x1B) : tweak_key0[7];

	tmp = be_uint64_t_load(tweak_key0) * 2;
	be_uint64_t_store(tweak_key1, tmp);
	tweak_key1[7] = ((tweak_key0[0] & 0x80) > 0) ? (tweak_key1[7] ^ 0x1B) : tweak_key1[7];
}

static void msif_auth_derive_iv(void *result, const void *data, const void *key, unsigned int size)
{
	unsigned char tweak_seed_enc[8];
	unsigned char tweak_seed[8];
	memset(tweak_seed, 0, sizeof(tweak_seed));
	des3_cbc_cts_enc_iv_0(tweak_seed_enc, tweak_seed, key, 8);

	unsigned int tweak_key0[2];
	unsigned int tweak_key1[2];
	msif_auth_derive_iv_tweak(tweak_seed_enc, (unsigned char *)tweak_key0,
				  (unsigned char *)tweak_key1);
	if (size <= 8)
		return;

	const unsigned int *current_ptr = data;
	unsigned int current_size = size;

	unsigned int IV[2];
	memset(IV, 0, sizeof(IV));

	while (current_size > 8) {
		int current_round[2];
		current_round[0] = current_ptr[0] ^ IV[0];
		current_round[1] = current_ptr[1] ^ IV[1];

		des3_cbc_cts_enc_iv_0(IV, current_round, key, 8);

		current_ptr = current_ptr + 2;
		current_size = current_size - 8;
	}

	unsigned int IV_mod[2];
	unsigned int tail_data[2];

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

	unsigned int final_round[2];
	final_round[0] = tail_data[0] ^ IV_mod[0];
	final_round[1] = tail_data[1] ^ IV_mod[1];

	des3_cbc_cts_enc_iv_0(result, final_round, key, 8);
}

static void rmauth_cmd_0x1(unsigned int *res)
{
	*res = 0;
}

static void rmauth_cmd_0x2(const unsigned char key[32],
			   const unsigned char seed[32],
			   unsigned char out_key[32])
{
	unsigned char iv[MBEDTLS_AES_BLOCK_SIZE];
	unsigned char seed_trunc[16];

	memcpy(seed_trunc, seed, 16);

	memset(iv, 0, MBEDTLS_AES_BLOCK_SIZE);
	aes_cbc_enc(&out_key[0], seed_trunc, &key[0], iv, 128, 16);

	memset(iv, 0, MBEDTLS_AES_BLOCK_SIZE);
	aes_cbc_enc(&out_key[16], seed_trunc, &key[16], iv, 128, 16);
}

static void msif_auth(const unsigned char key[32], unsigned int auth_val)
{
	unsigned char key_1C[32];

	unsigned int f00d_cmd1_res;
	rmauth_cmd_0x1(&f00d_cmd1_res);

	unsigned char session_id[8];
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

void msif_setup(const unsigned char key[32])
{
	unsigned int misc_0x0000;
	unsigned int hw_info_masked;
	unsigned int model_name_type, unkC20, write_protected;

	msif_reset_sub_C286C4();
	ms_get_info(&model_name_type, &unkC20, &write_protected);

	misc_0x0000 = pervasive_read_misc(0);
	hw_info_masked = sysroot_get_hw_info() & 0xFF0000;

	if (misc_0x0000) {
		if (misc_0x0000 == 0x100) {
			unsigned int val;
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

void msif_read_sector(unsigned int sector, void *buff)
{
	const unsigned int count = 1;
	unsigned char data[7];

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

void msif_read_atrb(unsigned int address, void *buff)
{
	const unsigned int count = 1;
	unsigned char data[7];

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

void msif_read_short_data(unsigned char cmd, void *buff, unsigned int size)
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

void msif_write_short_data(unsigned char cmd, const void *buff, unsigned int size)
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
