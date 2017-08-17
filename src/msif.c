#include <string.h>
#include "msif.h"
#include "pervasive.h"
#include "sysroot.h"
#include "utils.h"

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

#define MS_PARAM_REG_SYS_PARAM	0x10

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

static void ms_read_data(unsigned char *buff, unsigned int size)
{
	while (size > 0) {
		ms_wait_fifo_rw();

		*(unsigned int *)&buff[0] = readl(MSIF_BASE_ADDR + MSIF_DATA_REG);
		*(unsigned int *)&buff[4] = readl(MSIF_BASE_ADDR + MSIF_DATA_REG);

		buff += 8;
		size -= 8;
	}

	if (size > 0) {
		unsigned char data[8];

		ms_wait_fifo_rw();

		*(unsigned int *)&data[0] = readl(MSIF_BASE_ADDR + MSIF_DATA_REG);
		*(unsigned int *)&data[4] = readl(MSIF_BASE_ADDR + MSIF_DATA_REG);

		memcpy(buff, data, size);
	}
}

static void ms_write_data(const unsigned char *buff, unsigned int size)
{
	while (size > 8) {
		ms_wait_fifo_rw();

		writel(*(unsigned int *)&buff[0], MSIF_BASE_ADDR + MSIF_DATA_REG);
		writel(*(unsigned int *)&buff[4], MSIF_BASE_ADDR + MSIF_DATA_REG);

		buff += 8;
		size -= 8;
	}

	if (size > 0) {
		unsigned char data[8];

		ms_wait_fifo_rw();

		memcpy(data, buff, size);
		memset(&data[size], 0, sizeof(data) - size);

		writel(*(unsigned int *)&data[0], MSIF_BASE_ADDR + MSIF_DATA_REG);
		writel(*(unsigned int *)&data[4], MSIF_BASE_ADDR + MSIF_DATA_REG);
	}
}

static int ms_tpc_set_rw_regs_adrs(unsigned char read_addr, unsigned int read_size,
				   unsigned char write_addr, unsigned int write_size)
{
	unsigned char buff[4];

	buff[0] = read_addr;
	buff[1] = read_size;
	buff[2] = write_addr;
	buff[3] = write_size;

	writew(MS_TPC_SET_RW_REG_ADRS | sizeof(buff), MSIF_BASE_ADDR + MSIF_COMMAND_REG);

	ms_write_data(buff, sizeof(buff));

	return ms_wait_ready();
}

static int ms_tpc_read_reg(unsigned char *buff, unsigned int size)
{
	writew(MS_TPC_READ_REG | (size & 0x7FF), MSIF_BASE_ADDR + MSIF_COMMAND_REG);

	ms_read_data(buff, size);

	return ms_wait_ready();
}

static int ms_tpc_write_reg(const unsigned char *buff, unsigned int size)
{
	writew(MS_TPC_WRITE_REG | (size & 0x7FF), MSIF_BASE_ADDR + MSIF_COMMAND_REG);

	ms_write_data(buff, size);

	return ms_wait_ready();
}

static int ms_tpc_get_reg_int(unsigned char *reg_int)
{
	unsigned char buff[8];

	writew(MS_TPC_GET_INT | 1, MSIF_BASE_ADDR + MSIF_COMMAND_REG);

	ms_wait_fifo_rw();

	ms_read_data(buff, sizeof(buff));

	if (reg_int)
		*reg_int = buff[0];

	return ms_wait_ready();
}

static void ms_reg_int_wait_ced(void)
{
	int ret;
	unsigned char reg_int;

	do {
		ret = ms_tpc_get_reg_int(&reg_int);
	} while ((ret < 0) || !(reg_int & MS_INT_REG_CED));
}

static void ms_reg_int_wait_breq(void)
{
	int ret;
	unsigned char reg_int;

	do {
		ret = ms_tpc_get_reg_int(&reg_int);
	} while ((ret < 0) || !(reg_int & MS_INT_REG_BREQ));
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
	unsigned char system;

	switch (bus_mode) {
	case 0:
	case 2:
		system = MS_SYS_PAR8;
		break;
	case 1:
		system = MS_SYS_SERIAL;
		break;
	case 3:
		system = MS_SYS_SERIAL | 8;
		break;
	case 4:
		system = MS_SYS_PAR4;
		break;
	}

	ms_tpc_set_rw_regs_adrs(0, 0, MS_PARAM_REG_SYS_PARAM, 1);
	ms_tpc_write_reg(&system, sizeof(system));
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

static void ms_read_reg(unsigned char read_addr, unsigned int read_size, unsigned char *buff)
{
	if (read_size - 1 > 0xFF)
		return;

	ms_tpc_set_rw_regs_adrs(read_addr, read_size, 0x10, 0x0F);
	ms_tpc_read_reg(buff, read_size);
}

static void ms_get_info(unsigned int *model_name_type, unsigned int *unkC20,
			unsigned int *write_protected)
{
	unsigned int type;
	struct ms_status_registers status;

	ms_read_reg(0, sizeof(status), (unsigned char *)&status);

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

void msif_init1(void)
{
	unsigned int misc_0x0000;
	unsigned int hw_info_masked;
	unsigned int model_name_type;
	unsigned int unkC20 = 0;
	unsigned int write_protected = 0;
	unsigned int tmp1, tmp2 = 0, tmp3 = 0;

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

	ms_get_info(&tmp1, &tmp2, &tmp3);

	if (((tmp1 & 0xF0) == 0x10) || ((tmp1 & 0xF0) == 0x20)) {
		/* TODO: sub_C280CC */
	}

	/*
	 * Switch to parallel 4 mode.
	 */
	ms_set_bus_mode(4);
}

void msif_init2(struct msif_init2_arg *arg)
{
	/* TODO */
}

void msif_read_sector(unsigned int sector, unsigned char *buff)
{
	const unsigned int count = 1;
	unsigned char data[7];

	writew(MS_TPC_EX_SET_CMD | sizeof(data), MSIF_BASE_ADDR + MSIF_COMMAND_REG);

	data[0] = MSPRO_CMD_READ_DATA;
	data[1] = (count >> 8) & 0xFF;
	data[2] = count & 0xFF;
	data[3] = (sector >> 24) & 0xFF;
	data[4] = (sector >> 16) & 0xFF;
	data[5] = (sector >> 8) & 0xFF;
	data[6] = sector & 0xFF;

	ms_write_data(data, sizeof(data));

	ms_wait_ready();
	ms_wait_unk1();
	ms_reg_int_wait_breq();

	writew(MS_TPC_READ_LONG_DATA | (count * MS_SECTOR_SIZE),
	       MSIF_BASE_ADDR + MSIF_COMMAND_REG);

	ms_read_data(buff, count * MS_SECTOR_SIZE);

	ms_wait_ready();
	ms_wait_unk1();
	ms_reg_int_wait_ced();
}

void msif_read_atrb(unsigned int address, unsigned char *buff)
{
	const unsigned int count = 1;
	unsigned char data[7];

	writew(MS_TPC_EX_SET_CMD | sizeof(data), MSIF_BASE_ADDR + MSIF_COMMAND_REG);

	data[0] = MSPRO_CMD_READ_ATRB;
	data[1] = (count >> 8) & 0xFF;
	data[2] = count & 0xFF;
	data[3] = (address >> 24) & 0xFF;
	data[4] = (address >> 16) & 0xFF;
	data[5] = (address >> 8) & 0xFF;
	data[6] = address & 0xFF;

	ms_write_data(data, sizeof(data));

	ms_wait_ready();
	ms_wait_unk1();
	ms_reg_int_wait_breq();

	writew(MS_TPC_READ_LONG_DATA | (count * MS_SECTOR_SIZE),
	       MSIF_BASE_ADDR + MSIF_COMMAND_REG);

	ms_read_data(buff, count * MS_SECTOR_SIZE);

	ms_wait_ready();
	ms_wait_unk1();
	ms_reg_int_wait_ced();
}
