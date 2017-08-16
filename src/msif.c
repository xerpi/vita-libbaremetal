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

#define MS_INT_REG_CMDNK	0x01
#define MS_INT_REG_BREQ		0x20
#define MS_INT_REG_ERR		0x40
#define MS_INT_REG_CED		0x80

static int ms_wait_ready(void)
{
	unsigned int tmp;

	do {
		tmp = readw(MSIF_BASE_ADDR + MSIF_STATUS_REG);
	} while (!(tmp & (MSIF_STATUS_READY | MSIF_STATUS_TIMEOUT | MSIF_STATUS_CRC_ERROR)));

	if (tmp & (MSIF_STATUS_TIMEOUT | MSIF_STATUS_CRC_ERROR))
		return 0;

	return 0;
}

static void ms_wait_fifo_rw(void)
{
	unsigned int tmp;

	do {
		tmp = readw(MSIF_BASE_ADDR + MSIF_STATUS_REG);
	} while (!(tmp & MSIF_STATUS_FIFO_RW));
}

static void ms_wait_not_reset(void)
{
	unsigned int tmp;

	do {
		tmp = readw(MSIF_BASE_ADDR + MSIF_SYSTEM_REG);
	} while (tmp & MSIF_SYSTEM_RESET);
}

static void msif_set_clock(unsigned int clock)
{
	unsigned int val;

	if (clock == 1)
		val = 4;
	else if (clock <= 4)
		val = 5;
	else
		val = 6;

	pervasive_msif_set_clock(val);
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

	msif_set_clock(1);

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
	 * Enabling this doesn't work.
	tmp &= 0xFFF8;
	tmp |= 0x4005;
	*/
	writew(tmp, MSIF_BASE_ADDR + MSIF_SYSTEM_REG);
}

static int msif_sub_C2AD68(unsigned char read_addr, unsigned int read_size,
			    unsigned char write_addr, unsigned int write_size)
{
	unsigned int data = (read_addr << 0) | (read_size << 8) |
			    (write_addr << 16) | (write_size << 24);

	writew(MS_TPC_SET_RW_REG_ADRS | 4, MSIF_BASE_ADDR + MSIF_COMMAND_REG);

	ms_wait_fifo_rw();

	writel(data, MSIF_BASE_ADDR + MSIF_DATA_REG);
	writel(0, MSIF_BASE_ADDR + MSIF_DATA_REG);

	return ms_wait_ready();
}

static void msif_sub_C2A940(unsigned int size, unsigned char *buff)
{
	while (size > 0) {
		ms_wait_fifo_rw();

		*(unsigned int *)(buff + 0) = readl(MSIF_BASE_ADDR + MSIF_DATA_REG);
		*(unsigned int *)(buff + 4) = readl(MSIF_BASE_ADDR + MSIF_DATA_REG);

		buff += 8;
		size -= 8;
	}
}

static int msif_sub_C2AB7C(unsigned int size, unsigned char *buff)
{
	writew(MS_TPC_READ_REG | (size & 0x7FF), MSIF_BASE_ADDR + MSIF_COMMAND_REG);

	msif_sub_C2A940(size, buff);

	return ms_wait_ready();
}

static void msif_sub_C28CDC(unsigned char read_addr, unsigned int read_size, unsigned char *buff)
{
	if (read_size - 1 > 0xFF)
		return;

	msif_sub_C2AD68(read_addr, read_size, 0x10, 0x0F);
	msif_sub_C2AB7C(read_size, buff);
}

static void msif_init1_sub_C28700(unsigned int *unkC08, unsigned int *unkC20, unsigned int *unkC24)
{
	unsigned int v13;
	unsigned char buff[8];

	msif_sub_C28CDC(0, sizeof(buff), buff);

	if ((buff[2] << 0x1F) < 0) {
		*unkC24 = 1;
		*unkC20 = 0;
	}

	if (buff[4] + 1 <= 1) {
		if (buff[4] != buff[6]) {
			if (buff[6] - 1 > 0x7E)
				v13 = 0;
			else
				v13 = 0x40;
		} else if ((buff[4] == buff[7]) || (buff[7] - 1 <= 2)) {
			unsigned int v15;

			if (buff[4] == 0) {
				v15 = 0x2010;
				v13 = 0x2011;
			} else {
				v15 = 0x10;
				v13 = 0x11;
			}

			if (buff[7] != 0xFF)
				v13 = (1 << buff[7]) | v15;
		}
	} else if (buff[4] == 1) {
		if (buff[6] == 0) {
			if (buff[7] <= 3) {
				if (buff[5] == 0) {
					v13 = (1 << buff[7]) | 0x2020;
				} else if (buff[5] == 7) {
					unsigned int v16 = 0xA820;
					if (!(readw(MSIF_BASE_ADDR + 0x30 + 0x08) & 0x400))
						v16 = 0xE820;
					v13 = (1 << buff[7]) | v16;
				}
			} else {
				v13 = 0;
			}
		} else if (buff[6] == 0x10) {
			if (buff[5])
				v13 = 0;
			else
				v13 = 0x2080;

			if (buff[5] == 7) {
				v13 = 0xA880;
				if (!(readw(MSIF_BASE_ADDR + 0x30 + 0x08) & 0x400))
					v13 = 0xE880;
			}
		}
	} else {
		v13 = 0;
	}

	*unkC08 = v13;
}

void msif_init1(void)
{
	unsigned int misc_0x0000;
	unsigned int hw_info_masked;
	unsigned int unkC08, unkC20, unkC24;
	unsigned int tmp1, tmp2, tmp3;

	msif_reset_sub_C286C4();
	msif_init1_sub_C28700(&unkC08, &unkC20, &unkC24);

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

	msif_init1_sub_C28700(&tmp1, &tmp2, &tmp3);

	if (((tmp1 & 0xF0) == 0x10) || ((tmp1 & 0xF0) == 0x20)) {
		/* TODO: sub_C280CC */
	}
}

void msif_init2(struct msif_init2_arg *arg)
{
	/* TODO */
}
