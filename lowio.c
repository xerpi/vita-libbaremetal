#include "lowio.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define PERVASIVE_RESET_BASE_ADDR	0xE3101000
#define PERVASIVE_GATE_BASE_ADDR	0xE3102000
#define PERVASIVE_BASECLK_BASE_ADDR	0xE3103000
#define PERVASIVE_MISC_BASE_ADDR	0xE3100000

#define PERVASIVE_BASECLK_DSI_REGS(i)	((void *)(PERVASIVE_BASECLK_BASE_ADDR + 0x180 - (i) * 0x80))

#define GPIO0_BASE_ADDR			0xE20A0000
#define GPIO1_BASE_ADDR			0xE0100000

#define GPIO_REGS(i)			((void *)((i) == 0 ? GPIO0_BASE_ADDR : GPIO1_BASE_ADDR))

#define I2C0_BASE_ADDR			0xE0500000
#define I2C1_BASE_ADDR			0xE0510000

#define I2C_REGS(i)			((void *)((i) == 0 ? I2C0_BASE_ADDR : I2C1_BASE_ADDR))

#define DSI0_BASE_ADDR			0xE5050000
#define DSI1_BASE_ADDR			0xE5060000

#define DSI_REGS(i)			((void *)((i) == 0 ? DSI0_BASE_ADDR : DSI1_BASE_ADDR))

#define IFTU0REGA_BASE_ADDR		0xE5020000
#define IFTU0REGB_BASE_ADDR		0xE5021000
#define IFTU0CREG_BASE_ADDR		0xE5022000
#define IFTU1REGA_BASE_ADDR		0xE5030000
#define IFTU1REGB_BASE_ADDR		0xE5031000
#define IFTU1CREG_BASE_ADDR		0xE5032000

#define IFTU_REGS(i)			((void *)( \
						(i) == 0 ? IFTU0REGA_BASE_ADDR : \
						(i) == 1 ? IFTU0REGB_BASE_ADDR : \
						(i) == 2 ? IFTU1REGA_BASE_ADDR : \
						           IFTU1REGB_BASE_ADDR))

#define IFTU_CREGS(i) ((void *)(((i) & 1) == 0 ? IFTU0CREG_BASE_ADDR : IFTU1CREG_BASE_ADDR))

struct pervasive_dsi_timing_subinfo {
	unsigned int unk00;
	unsigned int unk04;
	unsigned int unk08;
};

struct pervasive_dsi_timing_info {
	unsigned int baseclk_0x24_value;
	unsigned int unk04;
	const struct pervasive_dsi_timing_subinfo *subinfo;
	unsigned int unk0C;
	unsigned int unk10;
	unsigned int unk14;
	unsigned int unk18;
	unsigned int unk1C;
	unsigned int unk20;
	unsigned int unk24;
};

static const struct pervasive_dsi_timing_subinfo stru_BD0408 = {0x17,     0xB01, 0x4180301};
static const struct pervasive_dsi_timing_subinfo stru_BD0414 = {0x4C074C, 0x904, 0x10180301};
static const struct pervasive_dsi_timing_subinfo stru_BD072C = {0x31,     0xC00, 0x4180301};
static const struct pervasive_dsi_timing_subinfo stru_BD04D0 = {0x2014F,  0xA00, 0x10000301};
static const struct pervasive_dsi_timing_subinfo stru_BD06EC = {0x20027,  0x203, 0x10000301};
static const struct pervasive_dsi_timing_subinfo stru_BD0420 = {0x60063,  0xA01, 0x10000301};
static const struct pervasive_dsi_timing_subinfo stru_BD0620 = {0x20031,  0x202, 0x10000301};
static const struct pervasive_dsi_timing_subinfo stru_BD0744 = {0x1A,     0xC01, 0x8100301};
static const struct pervasive_dsi_timing_subinfo stru_BD0850 = {0x10052,  0x800, 0x4180301};
static const struct pervasive_dsi_timing_subinfo stru_BD0778 = {0x20147,  0xA00, 0x10000301};
static const struct pervasive_dsi_timing_subinfo stru_BD0738 = {0x20143,  0xA00, 0x10000301};
static const struct pervasive_dsi_timing_subinfo stru_BD0720 = {0x34,     0xA00, 0x8100301};
static const struct pervasive_dsi_timing_subinfo stru_BD0680 = {0x31,     0xA00, 0x4180301};
static const struct pervasive_dsi_timing_subinfo stru_BD06E0 = {0x60063,  0xC01, 0x10000301};

static const struct pervasive_dsi_timing_info stru_BD0458 = {0x10, 0, &stru_BD0408, 0xC0060,  1,     0x10000301, 0,          0x40592EC5, 0,          0x40392EC5};
static const struct pervasive_dsi_timing_info stru_BD0558 = {0,    0, &stru_BD0414, 0xF,      2,     0x4180301,  0,          0x405B0000, 0,          0x403B0000};
static const struct pervasive_dsi_timing_info stru_BD04E0 = {0x10, 1, &stru_BD072C, 0xA014F,  0x200, 0x10000301, 0xA0000000, 0x405F77F1, 0x40000000, 0x40392CC1};
static const struct pervasive_dsi_timing_info stru_BD0508 = {0,    1, &stru_BD0414, 0x27,     0x100, 0x4180301,  0,          0x4070E000, 0,          0x404B0000};
static const struct pervasive_dsi_timing_info stru_BD0690 = {0x10, 0, &stru_BD04D0, 0x140713, 0x100, 0x10000301, 0x40000000, 0x40637B03, 0x40000000, 0x40437B03};
static const struct pervasive_dsi_timing_info stru_BD0860 = {0x10, 0, &stru_BD06EC, 0x2001F,  0x100, 0x4180301,  0,          0x40640000, 0,          0x40440000};
static const struct pervasive_dsi_timing_info stru_BD06F8 = {0x10, 1, &stru_BD0420, 0x80163,  0,     0x10000301, 0x40000000, 0x406859C4, 0x60000000, 0x40437B03};
static const struct pervasive_dsi_timing_info stru_BD05F8 = {0x10, 1, &stru_BD0620, 0x5001F,  0,     0x4180301,  0,          0x40690000, 0,          0x40440000};
static const struct pervasive_dsi_timing_info stru_BD0828 = {0x10, 2, &stru_BD0744, 0x1001F,  0,     0x4180301,  0xA0000000, 0x406C09D8, 0x20000000, 0x4042B13B};
static const struct pervasive_dsi_timing_info stru_BD0630 = {0x10, 2, &stru_BD0850, 0xC005F,  0,     0x4180301,  0xC0000000, 0x406CBB13, 0x80000000, 0x40432762};
static const struct pervasive_dsi_timing_info stru_BD04A8 = {0x10, 2, &stru_BD0778, 0x9004F,  0,     0x8100301,  0,          0x406B0000, 0,          0x40420000};
static const struct pervasive_dsi_timing_info stru_BD0658 = {0x10, 0, &stru_BD06EC, 0x20019,  0,     0x4180301,  0,          0x40704000, 0,          0x40504000};
static const struct pervasive_dsi_timing_info stru_BD05A8 = {0x10, 0, &stru_BD0738, 0xC007B,  0,     0x10000301, 0x80000000, 0x4070957B, 0x80000000, 0x4050957B};
static const struct pervasive_dsi_timing_info stru_BD05D0 = {0x10, 0, &stru_BD06EC, 0xA0063,  0,     0x10000301, 0xE0000000, 0x40710BA2, 0xE0000000, 0x40510BA2};
static const struct pervasive_dsi_timing_info stru_BD0580 = {0x10, 0, &stru_BD072C, 0x6004F,  0,     0x8100301,  0xC0000000, 0x40728B40, 0xC0000000, 0x40528B40};
static const struct pervasive_dsi_timing_info stru_BD0750 = {0,    0, &stru_BD0414, 0x2B,     0,     0x4180301,  0,          0x40729000, 0,          0x40529000};
static const struct pervasive_dsi_timing_info stru_BD0530 = {0x10, 1, &stru_BD0720, 0x30027,  0,     0x4180301,  0,          0x407453A3, 0xC0000000, 0x405042E8};
static const struct pervasive_dsi_timing_info stru_BD0480 = {0x10, 1, &stru_BD0680, 0x80063,  0,     0x10000301, 0xA0000000, 0x40754E8B, 0xE0000000, 0x40510BA2};
static const struct pervasive_dsi_timing_info stru_BD06B8 = {0x10, 1, &stru_BD06E0, 0x31,     0,     0x4180301,  0xE0000000, 0x40772E10, 0xC0000000, 0x40528B40};
static const struct pervasive_dsi_timing_info stru_BD0430 = {0,    1, &stru_BD0414, 0x36,     0,     0x8100301,  0,          0x40773400, 0,          0x40529000};

static const struct {
	unsigned int pixelclock;
	const struct pervasive_dsi_timing_info *timing_info;
} pervasive_dsi_timing_info_lookup[] = {
	{0xF5ADD, &stru_BD0458},
	{0x107AC0, &stru_BD0558},
	{0x1334F5, &stru_BD04E0},
	{0x149970, &stru_BD0508},
	{0x17C7AA, &stru_BD0690},
	{0x186A00, &stru_BD0860},
	{0x1DB994, &stru_BD06F8},
	{0x1E8480, &stru_BD05F8},
	{0x223A1C, &stru_BD0828},
	{0x23125E, &stru_BD0630},
	{0x23F48B, &stru_BD04A8},
	{0x27AC40, &stru_BD0658},
	{0x287CF3, &stru_BD05A8},
	{0x299D69, &stru_BD05D0},
	{0x2D45F9, &stru_BD0580},
	{0x2D5190, &stru_BD0750},
	{0x31A031, &stru_BD0530},
	{0x3404C3, &stru_BD0480},
	{0x389777, &stru_BD06B8},
	{0x38A5F4, &stru_BD0430},
};

struct dsi_timing_subsubinfo {
	unsigned int unk00;
	unsigned int unk04;
	unsigned int unk08;
	unsigned int unk0C;
	unsigned int unk10;
};

struct dsi_timing_subinfo {
	unsigned int unk00;
	unsigned int unk04;
	unsigned int unk08;
	unsigned int unk0C;
	struct dsi_timing_subsubinfo subsubinfo;
};

struct dsi_timing_info {
	unsigned int flags; // bit 3 = enable CRC?
	unsigned int pixelclock_24bpp;
	const struct dsi_timing_subinfo *subinfo_24bpp;
	unsigned int pixelclock_30bpp;
	const struct dsi_timing_subinfo *subinfo_30bpp;
	unsigned int hline_time; // horizontal line time
	unsigned int vline_time;
	unsigned int mode; // mode sync pulses or sync events?
	unsigned int HBP;
	unsigned int HFP;
	unsigned int HSS;
	unsigned int VFP;
	unsigned int VBP;
	unsigned int VSS;
};

/* subinfo_24bpp */
static const struct dsi_timing_subinfo stru_A19358 = {0x2BCF, 6, 0xF05,  1, {0x806, 0x4906, 0x16, 3, 0x20}};
static const struct dsi_timing_subinfo stru_BD0C5C = {0x33E0, 7, 0x150A, 2, {0x40B, 0x40CA, 8,    4, 0x20}};
static const struct dsi_timing_subinfo stru_BD0E04 = {0x1519, 3, 0x703,  3, {3,     0x34BD, 7,    0, 0x20}};
static const struct dsi_timing_subinfo stru_BD0C00 = {0x39F5, 7, 0x1507, 5, {0x507, 0x4871, 0xB,  0, 0x20}};
static const struct dsi_timing_subinfo stru_BD0B34 = {0x3A03, 7, 0x1507, 5, {0x507, 0x4884, 0xB,  0, 0x20}};

/* subinfo_30bpp */
static const struct dsi_timing_subinfo stru_BD0AC4 = {0x1A60, 3, 0xB03,  3, {3,     0x41EC, 7,    0, 0x20}};
static const struct dsi_timing_subinfo stru_BD0CF0 = {0x4876, 9, 0x1F09, 5, {0x709, 0x4871, 0xB,  1, 0x20}};
static const struct dsi_timing_subinfo stru_BD0D4C = {0x488A, 9, 0x1F09, 5, {0x709, 0x4890, 0xB,  1, 0x20}};

/*                                              flags pixelclk24 subinfo_24bpp pixelclk30 subinfo_30bpp hline  vline mode HBP   HFP   HSS   VFP  VBP VSS */
static const struct dsi_timing_info stru_BD0D14 = {5,   0x223A1C, &stru_A19358, 0,        0,            0x41A, 0x252, 0, 0x14,  0x42,  4,    4,   4, 0x2A};
static const struct dsi_timing_info stru_BD0BC8 = {5,   0x223A1C, &stru_A19358, 0,        0,            0x4E2, 0x252, 0, 0x14,  0x10A, 4,    4,   4, 0x2E};
static const struct dsi_timing_info stru_BD0ED0 = {5,   0x223A1C, &stru_A19358, 0,        0,            0x55F, 0x237, 0, 0x14,  0x187, 4,    4,   4, 0xF};
static const struct dsi_timing_info stru_BD0E60 = {0xF, 0x287CF3, &stru_BD0C5C, 0,        0,            0x594, 0x307, 0, 0x72,  8,     0x18, 4,   4, 0x2F};
static const struct dsi_timing_info stru_BD0E98 = {1,   0x107AC0, &stru_BD0E04, 0x149970, &stru_BD0AC4, 0x35A, 0x20D, 0, 0x10,  0x3E,  0x3C, 9,   6, 0x1E};
static const struct dsi_timing_info stru_BD0E28 = {1,   0x107AC0, &stru_BD0E04, 0x149970, &stru_BD0AC4, 0x35A, 0x20E, 0, 0x10,  0x3E,  0x3C, 0xA, 6, 0x1E};
static const struct dsi_timing_info stru_BD0DCC = {1,   0x107AC0, &stru_BD0E04, 0x149970, &stru_BD0AC4, 0x360, 0x271, 0, 0xC,   0x40,  0x44, 5,   5, 0x27};
static const struct dsi_timing_info stru_BD0B90 = {1,   0x107AC0, &stru_BD0E04, 0x149970, &stru_BD0AC4, 0x360, 0x272, 0, 0xC,   0x40,  0x44, 6,   5, 0x28};
static const struct dsi_timing_info stru_BD0CB8 = {0,   0x2D45F9, &stru_BD0C00, 0x389777, &stru_BD0CF0, 0x898, 0x465, 1, 0x58,  0x2C,  0x94, 2,   5, 0x10};
static const struct dsi_timing_info stru_BD0B58 = {0,   0x2D5190, &stru_BD0B34, 0x38A5F4, &stru_BD0D4C, 0xA50, 0x465, 1, 0x210, 0x2C,  0x94, 2,   5, 0x10};
static const struct dsi_timing_info stru_BD0F68 = {0,   0x2D45F9, &stru_BD0C00, 0x389777, &stru_BD0CF0, 0x672, 0x2EE, 0, 0x6E,  0x28,  0xDC, 5,   5, 0x14};
static const struct dsi_timing_info stru_BD0D70 = {0,   0x2D5190, &stru_BD0B34, 0x38A5F4, &stru_BD0D4C, 0x7BC, 0x2EE, 0, 0x1B8, 0x28,  0xDC, 5,   5, 0x14};
static const struct dsi_timing_info stru_BD0C80 = {0,   0x2D45F9, &stru_BD0C00, 0x389777, &stru_BD0CF0, 0x898, 0x465, 0, 0x58,  0x2C,  0x94, 4,   5, 0x24};
static const struct dsi_timing_info stru_BD0A8C = {0,   0x2D5190, &stru_BD0B34, 0x38A5F4, &stru_BD0D4C, 0xA50, 0x465, 0, 0x210, 0x2C,  0x94, 4,   5, 0x24};
static const struct dsi_timing_info stru_BD0AE8 = {0,   0x2D45F9, &stru_BD0C00, 0x389777, &stru_BD0CF0, 0xABE, 0x465, 0, 0x27E, 0x2C,  0x94, 4,   5, 0x24};

static const struct {
	unsigned int value;
	const struct dsi_timing_info *timing_info;
} dsi_timing_info_lookup[] = {
	{0, &stru_BD0D14},
	{0x80, &stru_BD0BC8},
	{0x20, &stru_BD0ED0},
	{0x8900, &stru_BD0E60},
	{0x8300, &stru_BD0E98},
	{0x8370, &stru_BD0E28},
	{0x8480, &stru_BD0DCC},
	{0x84F0, &stru_BD0B90},
	{0x8500, &stru_BD0CB8},
	{0x8580, &stru_BD0B58},
	{0x8600, &stru_BD0F68},
	{0x8680, &stru_BD0D70},
	{0x8710, &stru_BD0C80},
	{0x8790, &stru_BD0A8C},
	{0x8730, &stru_BD0AE8},
};

void delay(int n)
{
	volatile int i, j;

	for (i = 0; i < n; i++)
		for (j = 0; j < 1000; j++)
			;
}

static inline void pervasive_mask_or(unsigned int addr, unsigned int val)
{
	volatile unsigned long tmp;

	asm volatile(
		"ldr %0, [%1]\n\t"
		"orr %0, %2\n\t"
		"str %0, [%1]\n\t"
		"dmb\n\t"
		"ldr %0, [%1]\n\t"
		"dsb\n\t"
		: "=&r"(tmp)
		: "r"(addr), "r"(val)
	);
}

static inline void pervasive_mask_and_not(unsigned int addr, unsigned int val)
{
	volatile unsigned long tmp;

	asm volatile(
		"ldr %0, [%1]\n\t"
		"bic %0, %2\n\t"
		"str %0, [%1]\n\t"
		"dmb\n\t"
		"ldr %0, [%1]\n\t"
		"dsb\n\t"
		: "=&r"(tmp)
		: "r"(addr), "r"(val)
	);
}

unsigned int pervasive_read_misc(unsigned int offset)
{
	return *(unsigned int *)(PERVASIVE_MISC_BASE_ADDR + offset);
}

void pervasive_clock_enable_uart(int bus)
{
	pervasive_mask_or(PERVASIVE_GATE_BASE_ADDR + 0x120 + 4 * bus, 1);
}

void pervasive_reset_exit_uart(int bus)
{
	pervasive_mask_and_not(PERVASIVE_RESET_BASE_ADDR + 0x120 + 4 * bus, 1);
}

void pervasive_clock_enable_gpio(void)
{
	pervasive_mask_or(PERVASIVE_GATE_BASE_ADDR + 0x100, 1);
}

void pervasive_reset_exit_gpio(void)
{
	pervasive_mask_and_not(PERVASIVE_RESET_BASE_ADDR + 0x100, 1);
}

void pervasive_clock_enable_i2c(int bus)
{
	pervasive_mask_or(PERVASIVE_GATE_BASE_ADDR + 0x110 + 4 * bus, 1);
}

void pervasive_reset_exit_i2c(int bus)
{
	pervasive_mask_and_not(PERVASIVE_RESET_BASE_ADDR + 0x110 + 4 * bus, 1);
}

void pervasive_clock_enable_spi(int bus)
{
	pervasive_mask_or(PERVASIVE_GATE_BASE_ADDR + 0x104 + 4 * bus, 1);
}

void pervasive_reset_exit_spi(int bus)
{
	pervasive_mask_and_not(PERVASIVE_RESET_BASE_ADDR + 0x104 + 4 * bus, 1);
}

void pervasive_clock_enable_dsi(int bus, int value)
{
	pervasive_mask_or(PERVASIVE_GATE_BASE_ADDR + 0x80 + 4 * bus, value);
}

void pervasive_reset_exit_dsi(int bus, int value)
{
	pervasive_mask_and_not(PERVASIVE_RESET_BASE_ADDR + 0x80 + 4 * bus, value);
}

void pervasive_dsi_set_pixeclock(int bus, int pixelclock)
{
	int i;
	volatile unsigned int *baseclk_dsi_regs;
	const struct pervasive_dsi_timing_info *timing_info;

	for (i = 0; i < ARRAY_SIZE(pervasive_dsi_timing_info_lookup); i++) {
		if (pervasive_dsi_timing_info_lookup[i].pixelclock == pixelclock)
			break;
	}

	if (i >= ARRAY_SIZE(pervasive_dsi_timing_info_lookup))
		return;

	timing_info = pervasive_dsi_timing_info_lookup[i].timing_info;
	baseclk_dsi_regs = PERVASIVE_BASECLK_DSI_REGS(bus);

	baseclk_dsi_regs[2] = 1;
	baseclk_dsi_regs[1] = 1;
	baseclk_dsi_regs[1];
	dsb();

	baseclk_dsi_regs[9] = timing_info->baseclk_0x24_value;
	baseclk_dsi_regs[0] = 1;
	baseclk_dsi_regs[0];
	dsb();

	if (timing_info->baseclk_0x24_value & 0x10) {
		const struct pervasive_dsi_timing_subinfo *timing_subinfo =
			timing_info->subinfo;

		baseclk_dsi_regs[0xC] = timing_subinfo->unk00;
		baseclk_dsi_regs[0xD] = timing_subinfo->unk04;
		baseclk_dsi_regs[0xE] = timing_subinfo->unk08;
		baseclk_dsi_regs[0] = 0;
		baseclk_dsi_regs[0];
		dsb();

		delay(1000);
	}

	baseclk_dsi_regs[0x10] = timing_info->unk0C;
	baseclk_dsi_regs[0x11] = timing_info->unk10;
	baseclk_dsi_regs[0x12] = timing_info->unk14;
	baseclk_dsi_regs[8] = timing_info->unk04;
	baseclk_dsi_regs[1] = 0;
	baseclk_dsi_regs[1];
	dsb();

	delay(1000);

	baseclk_dsi_regs[2] = 0;
	baseclk_dsi_regs[2];
	dsb();
}

void gpio_set_port_mode(int bus, int port, int mode)
{
	volatile unsigned int *gpio_regs = GPIO_REGS(bus);

	gpio_regs[0] = (gpio_regs[0] & ~(1 << port)) | (mode << port);

	dmb();
}

int gpio_port_read(int bus, int port)
{
	volatile unsigned int *gpio_regs = GPIO_REGS(bus);

	return (gpio_regs[1] >> port) & 1;
}

void gpio_port_set(int bus, int port)
{
	volatile unsigned int *gpio_regs = GPIO_REGS(bus);

	gpio_regs[2] |= 1 << port;

	gpio_regs[0xD];

	dsb();
}

void gpio_port_clear(int bus, int port)
{
	volatile unsigned int *gpio_regs = GPIO_REGS(bus);

	gpio_regs[3] |= 1 << port;

	gpio_regs[0xD];

	dsb();
}

int gpio_query_intr(int bus, int port)
{
	volatile unsigned int *gpio_regs = GPIO_REGS(bus);

	return (1 << port) & ((gpio_regs[0x0E] & ~gpio_regs[0x07]) |
			      (gpio_regs[0x0F] & ~gpio_regs[0x08]) |
			      (gpio_regs[0x10] & ~gpio_regs[0x09]) |
			      (gpio_regs[0x11] & ~gpio_regs[0x0A]) |
			      (gpio_regs[0x12] & ~gpio_regs[0x0B]));
}

int gpio_acquire_intr(int bus, int port)
{
	unsigned int ret;
	unsigned int mask = 1 << port;
	volatile unsigned int *gpio_regs = GPIO_REGS(bus);

	ret = mask & ((gpio_regs[0x0E] & ~gpio_regs[0x07]) |
		      (gpio_regs[0x0F] & ~gpio_regs[0x08]) |
		      (gpio_regs[0x10] & ~gpio_regs[0x09]) |
		      (gpio_regs[0x11] & ~gpio_regs[0x0A]) |
		      (gpio_regs[0x12] & ~gpio_regs[0x0B]));

	gpio_regs[0x0E] = mask;
	gpio_regs[0x0F] = mask;
	gpio_regs[0x10] = mask;
	gpio_regs[0x11] = mask;
	gpio_regs[0x12] = mask;
	dsb();

	return ret;
}

static inline void i2c_wait_busy(volatile unsigned int *i2c_regs)
{
	while (i2c_regs[7])
		;
}

void i2c_init_bus(int bus)
{
	volatile unsigned int *i2c_regs = I2C_REGS(bus);

	i2c_regs[0xB] = 0x100F70F;
	i2c_regs[2] = 1;
	i2c_regs[3] = 1;
	i2c_regs[5] = 7;
	dsb();

	i2c_wait_busy(i2c_regs);

	i2c_regs[0xA] = i2c_regs[0xA];
	i2c_regs[0xB] = 0x1000000;

	i2c_regs[6] = 4; // or 5?
}

void i2c_transfer_write(int bus, unsigned char addr, const unsigned char *buffer, int size)
{
	int i;
	volatile unsigned int *i2c_regs = I2C_REGS(bus);

	i2c_regs[2] = 1;
	i2c_regs[3] = 1;
	i2c_regs[4] = addr >> 1;

	for (i = 0; i < size; i++)
		i2c_regs[0] = buffer[i];

	i2c_regs[5] = (size << 8) | 2;

	i2c_wait_busy(i2c_regs);

	i2c_regs[5] = 4;

	i2c_wait_busy(i2c_regs);
}

void i2c_transfer_read(int bus, unsigned char addr, unsigned char *buffer, int size)
{
	int i;
	volatile unsigned int *i2c_regs = I2C_REGS(bus);

	i2c_regs[2] = 1;
	i2c_regs[3] = 1;
	i2c_regs[4] = addr >> 1;
	i2c_regs[5] = (size << 8) | 0x13;

	i2c_wait_busy(i2c_regs);

	for (i = 0; i < size; i++)
		buffer[i] = i2c_regs[1];

	i2c_regs[5] = 4;

	i2c_wait_busy(i2c_regs);
}

void i2c_transfer_write_read(int bus, unsigned char write_addr, const unsigned char *write_buffer, int write_size,
			              unsigned char read_addr, unsigned char *read_buffer, int read_size)
{
	int i;
	volatile unsigned int *i2c_regs = I2C_REGS(bus);

	i2c_regs[2] = 1;
	i2c_regs[3] = 1;
	i2c_regs[4] = write_addr >> 1;

	for (i = 0; i < write_size; i++)
		i2c_regs[0] = write_buffer[i];

	i2c_regs[5] = (write_size << 8) | 2;

	i2c_wait_busy(i2c_regs);

	i2c_regs[5] = 5;

	i2c_wait_busy(i2c_regs);

	i2c_regs[5] = (read_size << 8) | 0x13;

	i2c_wait_busy(i2c_regs);

	for (i = 0; i < read_size; i++)
		read_buffer[i] = i2c_regs[1];

	i2c_regs[5] = 4;

	i2c_wait_busy(i2c_regs);
}

void dsi_enable_bus(int bus)
{
	static const int pixel_size = 24;
	static const int unk06 = 3;
	static const int unk07 = 1;

	unsigned int packet[40];
	unsigned int packet_size;
	const struct dsi_timing_subinfo *subinfo;
	const struct dsi_timing_info *timing_info = dsi_timing_info_lookup[0].timing_info;
	volatile unsigned int *dsi_regs = DSI_REGS(bus);

	dsi_regs[0x15] = 0;
	dsi_regs[0x146] = 1;

	if ((pervasive_read_misc(0x0000) & 0x1FF00) > 0xFF) {
		dsi_regs[0x240] = (dsi_regs[0x240] & 0xFFFFFFFC) | 2;
		dsi_regs[0x241] = (dsi_regs[0x241] & 0xFFFFFFFC) | 2;
		dsi_regs[0x242] = (dsi_regs[0x242] & 0xFFFFFFFC) | 2;
		if (unk06 == 3)
			dsi_regs[0x243] = (dsi_regs[0x243] & 0xFFFFFFFC) | 2;
	}

	dsi_regs[0x250] = 0x200;
	dsi_regs[0x251] = 0x200;
	dsi_regs[0x252] = 0x200;

	if (unk06 == 3)
		dsi_regs[0x253] = 0x200;

	if (pixel_size == 24)
		subinfo = timing_info->subinfo_24bpp;
	else
		subinfo = timing_info->subinfo_30bpp;

	if (subinfo) {
		dsi_regs[0x204] = subinfo->unk00;
		dsi_regs[0x205] = subinfo->unk04;
		dsi_regs[0x206] = subinfo->unk08;
		dsi_regs[0x207] = subinfo->unk0C;
		dsi_regs[0x208] = subinfo->subsubinfo.unk00;
		dsi_regs[0x209] = subinfo->subsubinfo.unk04;
		dsi_regs[0x20A] = subinfo->subsubinfo.unk08;
		dsi_regs[0x20B] = subinfo->subsubinfo.unk0C;
		dsi_regs[0x20C] = subinfo->subsubinfo.unk10;
		dsi_regs[0x20F] = subinfo->unk04 | (subinfo->unk04 << 0x10);
	}

	unsigned int flags = timing_info->flags;
	unsigned int HBP = timing_info->HBP;
	unsigned int HSS = timing_info->HSS;
	unsigned int HFP = timing_info->HFP;
	unsigned int VFP = timing_info->VFP;
	unsigned int VBP = timing_info->VBP;
	unsigned int VSS = timing_info->VSS;

	if (bus == 1) {
		dsi_regs[0x20D] = 0xF;
		dsi_regs[0x20E] = 0;
		dsi_regs[0x201] = 1;
		dsi_regs[0x145] = 0xFFFFFFFF;

		if (unk07 == 1) {
			unsigned int v81 = 0xA30000A2;
			unsigned int v82 = timing_info->hline_time - (timing_info->HSS + timing_info->HFP) - timing_info->HBP;

			if (((timing_info->flags << 0x1C) & 0x80000000) != 0)
				v82 -= 2;

			unsigned int v86 = timing_info->vline_time - 2 - VBP - VSS - VFP;
			unsigned int v89 = (0x300 * v82) & 0xFFFFFF;

			if (unk06 == 3)
				v81 = 0xA30000A4;

			packet[1] = 0x40EFFFF;
			packet[2] = v81;
			packet[3] = 0x28000004;
			packet[4] = 0x10000001;
			packet[5] = 0x28000001;
			packet[6] = 0x10000021;
			packet[7] = (VSS + VBP - 3) | 0x4010000;
			packet[8] = 0x28000001;
			packet[9] = 0x10000021;
			packet[0xA] = 0x28000001;
			packet[0xB] = 0x10000021;
			packet[0xC] = 0x40005619;
			packet[0xD] = v89 | 0x4000003E;
			packet[0xE] = 0x40015019;
			packet[0xF] = (v86 | 0x4000000) | 0x30000;
			packet[0x10] = 0x10000021;
			packet[0x11] = 0x40005619;
			packet[0x12] = v89 | 0x4000003E;
			packet[0x13] = 0x40015019;
			packet[0x14] = 0x10000021;
			packet[0x15] = (VFP - 2) | 0x4010000;
			packet[0x16] = 0x28000001;
			packet[0x17] = 0x10000021;
			packet[0x18] = 0xC000000;

			packet_size = 0x18;
		} else {
			if (timing_info->mode) {
				unsigned int v61 = (unsigned int)(timing_info->vline_time + 1 - 2 * (VBP + timing_info->VFP + VSS)) >> 1;
				unsigned int v62 = timing_info->hline_time - (HFP + HSS) - timing_info->HBP;
				unsigned int v63 = timing_info->flags & 2;
				unsigned int v64 = timing_info->flags & 4;
				unsigned int v65;
				unsigned int v70;
				unsigned int v98;

				if (v63)
					v63 = pixel_size * HFP;
				if (timing_info->flags & 2)
					v63 = ((v63 >> 3) - 0xA) | 0x80000000;

				if (v64)
					v64 = pixel_size * HSS;

				if (timing_info->flags & 4)
					v64 = ((v64 >> 3) - 0xA) | 0x80000000;

				if (unk06 == 3)
					v65 = 0xA30000A4;
				else
					v65 = 0xA30000A2;
				packet[2] = v65;
				packet[1] = 0x415FFFF;
				packet[3] = 0x28800005;
				packet[4] = 0x10000081;
				if (v63) {
					v98 = (v61 - 1) | 0x4050000;
					packet[0x15] = (v61 - 1) | 0x4050000;
					packet[5] = ((v63 << 8) & 0xFFFFFF) | 0x40000099;
					packet[0xA] = ((v63 << 8) & 0xFFFFFF) | 0x40000099;
					packet[0xE] = ((v63 << 8) & 0xFFFFFF) | 0x40000099;
					packet[0x13] = ((v63 << 8) & 0xFFFFFF) | 0x40000099;
					packet[0x18] = ((v63 << 8) & 0xFFFFFF) | 0x40000099;
					packet[7] = (VBP - 2) | 0x4030000;
					packet[6] = 0x100000B1;
					packet[0xB] = 0x100000B1;
					packet[0xF] = 0x100000B1;
					packet[0x14] = 0x100000B1;
					packet[0x10] = (VSS - 3) | 0x4030000;
					packet[8] = 0x28000001;
					packet[0x11] = 0x28000001;
					packet[0x16] = 0x28000001;
					packet[9] = 0x100000A1;
					packet[0x12] = 0x100000A1;
					packet[0x17] = 0x100000A1;
					packet[0xC] = 0x28000008;
					packet[0xD] = 0x10000091;
				} else {
					v98 = (v61 - 1) | 0x4050000;
					packet[0x15] = (v61 - 1) | 0x4050000;
					packet[0x10] = (VSS - 3) | 0x4030000;
					packet[5] = 0x28000002;
					packet[7] = (VBP - 2) | 0x4030000;
					packet[0xA] = 0x28000002;
					packet[0xE] = 0x28000002;
					packet[0x13] = 0x28000002;
					packet[0x18] = 0x28000002;
					packet[6] = 0x100000B1;
					packet[0xB] = 0x100000B1;
					packet[0xF] = 0x100000B1;
					packet[0x14] = 0x100000B1;
					packet[8] = 0x28000001;
					packet[0x11] = 0x28000001;
					packet[0x16] = 0x28000001;
					packet[9] = 0x100000A1;
					packet[0x12] = 0x100000A1;
					packet[0x17] = 0x100000A1;
					packet[0xC] = 0x28000008;
					packet[0xD] = 0x10000091;
				}
				packet[0x19] = 0x100000B1;

				unsigned int v66 = 0x28000010;
				if (v64)
					v66 = ((v64 << 8) & 0xFFFFFF) | 0x40000099;
				packet[0x1A] = v66;

				unsigned int v67;
				if (pixel_size == 0x18)
					v67 = ((0x300 * v62) & 0xFFFFFF) | 0x400000BE;
				else
					v67 = ((0x3C0 * v62) & 0xFFFF00) | 0x400000A9;

				packet[0x1B] = v67;
				packet[0x1C] = VFP | 0x4030000;
				packet[0x1D] = 0x28000001;
				packet[0x1E] = 0x100000A1;

				if (v63) {
					packet[0x24] = 0x28000001;
					packet[0x25] = 0x100000A1;
					packet[0x2B] = 0x28000001;
					packet[0x2C] = 0x100000A1;
					packet[0x23] = (VBP - 1) | 0x4030000;
					packet[0x2A] = (VSS - 2) | 0x4030000;
					packet[0x2F] = v98;
					packet[0x30] = 0x28000001;
					packet[0x31] = 0x100000A1;
					packet[0x1F] = ((v63 << 8) & 0xFFFFFF) | 0x40000099;
					packet[0x26] = ((v63 << 8) & 0xFFFFFF) | 0x40000099;
					packet[0x2D] = ((v63 << 8) & 0xFFFFFF) | 0x40000099;
					packet[0x32] = ((v63 << 8) & 0xFFFFFF) | 0x40000099;
					packet[0x20] = 0x100000B1;
					packet[0x27] = 0x100000B1;
					packet[0x2E] = 0x100000B1;
					packet[0x21] = 0x28000004;
					packet[0x22] = 0x100000C1;
					packet[0x28] = 0x28000008;
					packet[0x29] = 0x100000D1;
				} else {
					packet[0x24] = 0x28000001;
					packet[0x25] = 0x100000A1;
					packet[0x2B] = 0x28000001;
					packet[0x2C] = 0x100000A1;
					packet[0x23] = (VBP - 1) | 0x4030000;
					packet[0x2A] = (VSS - 2) | 0x4030000;
					packet[0x2F] = v98;
					packet[0x30] = 0x28000001;
					packet[0x31] = 0x100000A1;
					packet[0x1F] = 0x28000002;
					packet[0x26] = 0x28000002;
					packet[0x2D] = 0x28000002;
					packet[0x32] = 0x28000002;
					packet[0x20] = 0x100000B1;
					packet[0x27] = 0x100000B1;
					packet[0x2E] = 0x100000B1;
					packet[0x21] = 0x28000004;
					packet[0x22] = 0x100000C1;
					packet[0x28] = 0x28000008;
					packet[0x29] = 0x100000D1;
				}

				packet[0x33] = 0x100000B1;

				unsigned int v69 = 0x28000010;
				if (v64)
					v69 = ((v64 << 8) & 0xFFFFFF) | 0x40000099;

				packet[0x34] = v69;

				if (pixel_size == 0x18)
					v70 = ((0x300 * v62) & 0xFFFFFF) | 0x400000BE;
				else
					v70 = ((0x3C0 * v62) & 0xFFFF00) | 0x400000A9;

				packet[0x35] = v70;
				packet[0x36] = (VFP - 1) | 0x4030000;
				packet[0x37] = 0x28000001;
				packet[0x38] = 0x100000A1;

				unsigned int v72 = 0x28000002;
				if (v63)
					v72 = ((v63 << 8) & 0xFFFFFF) | 0x40000099;

				packet[0x39] = v72;
				packet[0x3A] = 0x100000B1;
				packet[0x3B] = 0xC000000;

				//packet_size = &packet[0x3C] - &packet[1];
				packet_size = 0xEC;
			} else {
				char *v57;
				unsigned int v38 = timing_info->HBP;
				unsigned int v40 = timing_info->vline_time - (VFP + timing_info->VSS);
				unsigned int v41 = VBP;
				unsigned int v42 = timing_info->HFP;
				unsigned int v44 = timing_info->hline_time - (HSS + HBP) - HFP;
				unsigned int v45 = flags & 1;
				unsigned int v46 = 0x40DFFFF;

				packet[0] = v40 - VBP;

				if (flags & 1)
					v41 = pixel_size;
				if (flags & 1)
					v38 *= v41;
				if (flags & 1)
					v45 = ((v38 >> 3) - 0xC) | 0x80000000;

				unsigned int v47 = flags & 2;

				if (flags & 2)
					v38 = pixel_size;
				else
					v42 = 0;

				if (flags & 2)
					v42 *= v38;
				if (flags & 2)
					v42 = ((v42 >> 3) - 0xA) | 0x80000000;

				unsigned int v49 = flags & 4;
				unsigned int v48 = (v49 == 0);

				unsigned int v97;

				if (v49)
					v47 = pixel_size;
				else
					v97 = 0;

				unsigned int v50 = 0xA30000A2;
				if (unk06 == 3)
					v50 = 0xA30000A4;

				if (!v48)
					v47 *= HSS;
				if (!v48)
					v97 = ((v47 >> 3) - 0xA) | 0x80000000;

				if (v45)
					v46 = 0x417FFFF;

				packet[1] = v46;
				packet[2] = v50;
				packet[3] = 0x28000004;
				packet[4] = 0x10000001;
				if (v42) {
					packet[5] = ((v42 << 8) & 0xFFFFFF) | 0x40000019;
					packet[7] = (VBP - 2) | 0x4030000;
					packet[0x10] = (VSS - 2) | 0x4030000;
					packet[0xA] = ((v42 << 8) & 0xFFFFFF) | 0x40000019;
					packet[0xE] = ((v42 << 8) & 0xFFFFFF) | 0x40000019;
					packet[0x13] = ((v42 << 8) & 0xFFFFFF) | 0x40000019;
					packet[6] = 0x10000031;
					packet[0xB] = 0x10000031;
					packet[0xF] = 0x10000031;
					packet[8] = 0x28000001;
					packet[0x11] = 0x28000001;
					packet[9] = 0x10000021;
					packet[0x12] = 0x10000021;
					packet[0xC] = 0x28000008;
					packet[0xD] = 0x10000011;
				} else {
					packet[5] = 0x28000002;
					packet[0xA] = 0x28000002;
					packet[0xE] = 0x28000002;
					packet[7] = (VBP - 2) | 0x4030000;
					packet[0x10] = (VSS - 2) | 0x4030000;
					packet[0x13] = 0x28000002;
					packet[6] = 0x10000031;
					packet[0xB] = 0x10000031;
					packet[0xF] = 0x10000031;
					packet[8] = 0x28000001;
					packet[0x11] = 0x28000001;
					packet[9] = 0x10000021;
					packet[0x12] = 0x10000021;
					packet[0xC] = 0x28000008;
					packet[0xD] = 0x10000011;
				}
				packet[0x14] = 0x10000031;
				if (v45) {
					packet[0x15] = 0x28000001;
					packet[0x16] = 0x10000021;

					unsigned int v75 = 0x28000002;
					if (v42)
						v75 = ((v42 << 8) & 0xFFFFFF) | 0x40000019;
					packet[0x17] = v75;

					packet[0x18] = 0x10000031;

					unsigned int v76 = 0x28000010;
					if (v97)
						v76 = ((v97 << 8) & 0xFFFFFF) | 0x40000019;
					packet[0x19] = v76;

					unsigned int v77;
					if (pixel_size == 0x18)
						v77 = ((0x300 * v44) & 0xFFFFFF) | 0x4000003E;
					else
						v77 = ((0x3C0 * v44) & 0xFFFF00) | 0x40000029;
					packet[0x1A] = v77;

					packet[0x1B] = ((v45 << 8) & 0xFFFFFF) | 0x40000019;
					packet[0x1C] = ((packet[0] & 0xFFFF) - 2) | 0x4050000;
					packet[0x1D] = 0x10000021;

					unsigned int  v78 = 0x28000002;
					if (v42)
						v78 = ((v42 << 8) & 0xFFFFFF) | 0x40000019;
					packet[0x1E] = v78;

					packet[0x1F] = 0x10000031;

					unsigned int v79;
					if (v97)
						v79 = ((v97 << 8) & 0xFFFFFF) | 0x40000019;
					else
						v79 = 0x28000010;
					packet[0x20] = v79;

					unsigned int v80;
					if (pixel_size == 0x18)
						v80 = ((0x300 * v44) & 0xFFFFFF) | 0x4000003E;
					else
						v80 = ((0x3C0 * v44) & 0xFFFF00) | 0x40000029;
					packet[0x21] = v80;

					packet[0x22] = ((v45 << 8) & 0xFFFFFF) | 0x40000019;
					packet[0x23] = 0x10000021;

					if (v42) {
						packet[0x28] = 0x10000021;
						packet[0x24] = ((v42 << 8) & 0xFFFFFF) | 0x40000019;
						packet[0x29] = ((v42 << 8) & 0xFFFFFF) | 0x40000019;
						packet[0x25] = 0x10000031;
						packet[0x26] = (VFP - 2) | 0x4030000;
						packet[0x27] = 0x28000001;
					} else {
						packet[0x28] = 0x10000021;
						packet[0x24] = 0x28000002;
						packet[0x29] = 0x28000002;
						packet[0x25] = 0x10000031;
						packet[0x26] = (VFP - 2) | 0x4030000;
						packet[0x27] = 0x28000001;
					}
					packet[0x2A] = 0x10000031;

					v57 = (char *)&packet[0x2B];
				} else {
					packet[0x15] = ((packet[0] & 0xFFFF) - 1) | 0x4050000;
					packet[0x16] = 0x28000001;
					packet[0x17] = 0x10000021;

					unsigned int v53 = 0x28000002;
					if (v42)
						v53 = ((v42 << 8) & 0xFFFFFF) | 0x40000019;
					packet[0x18] = v53;

					packet[0x19] = 0x10000031;

					unsigned int v54 = 0x28000010;
					if (v97)
						v54 = ((v97 << 8) & 0xFFFFFF) | 0x40000019;
					packet[0x1A] = v54;

					unsigned int v55;
					if (pixel_size == 0x18)
						v55 = ((0x300 * v44) & 0xFFFFFF) | 0x4000003E;
					else
						v55 = ((0x3C0 * v44) & 0xFFFF00) | 0x40000029;
					packet[0x1B] = v55;

					packet[0x1C] = (VFP - 1) | 0x4030000;
					packet[0x1D] = 0x28000001;
					packet[0x1E] = 0x10000021;

					unsigned int v56 = 0x28000002;
					if (v42)
						v56 = ((v42 << 8) & 0xFFFFFF) | 0x40000019;
					packet[0x1F] = v56;

					packet[0x20] = 0x10000031;

					v57 = (char *)&packet[0x21];
				}
				*(unsigned int *)v57 = 0xC000000;
				packet_size = (v57 + 4 - (char *)&packet[1]) >> 2;
			}
			if (packet_size <= 0)
				goto LABEL_18;
		}
	} else {
		unsigned int v32;

		unsigned int hline_time = timing_info->hline_time;
		unsigned int horizontal_porches = timing_info->HBP + timing_info->HFP;
		unsigned int vertical_porches = VBP + VSS;
		unsigned int vline_time = timing_info->vline_time;
		unsigned int horizontal_pixels = hline_time - horizontal_porches;

		if (unk07)
			v32 = 0x400B7819;
		else
			v32 = 0x400B4019;

		dsi_regs[0x20D] = 7;
		dsi_regs[0x20E] = 0;
		dsi_regs[0x201] = 1;
		dsi_regs[0x145] = 0xFFFFFFFF;

		packet[1] = 0x407FFFF;
		packet[2] = 0xA3000082;
		packet[3] = 0x28000004;
		packet[4] = 0x10000001;
		packet[5] = v32;
		packet[6] = (vertical_porches - 2) | 0x4020000;
		packet[7] = 0x28000001;
		packet[8] = 0x10000021;
		packet[9] = v32;
		packet[0xA] = ((vline_time - 1) - VBP - VSS - VFP) | 0x4030000;
		packet[0xB] = 0x28000001;
		packet[0xC] = 0x10000021;
		packet[0xD] = ((0x300 * HSS - 0xA00) & 0xFFFFFF) | 0x40000019;
		packet[0xE] = ((0x300 * (horizontal_pixels - HSS)) & 0xFFFFFF) | 0x4000003E;
		packet[0xF] = (VFP - 1) | 0x4020000;
		packet[0x10] = 0x28000001;
		packet[0x11] = 0x10000021;
		packet[0x12] = v32;
		packet[0x13] = 0xC000000;

		packet_size = 0x13;
	}

	unsigned int *packer_ptr = packet;
	unsigned int i = 0;
	do {
		unsigned int data = packer_ptr[1];
		packer_ptr++;
		i++;
		dsi_regs[0x140] = data;
	} while (i < packet_size);

LABEL_18:
	dsi_regs[0x147] = 1;
}

void iftu_crtc_enable(int crtc)
{
	volatile unsigned int *iftu_cregs = IFTU_CREGS(crtc);

	iftu_cregs[0] = 1;
}

void iftu_set_plane(int plane, struct iftu_plane_info *info)
{
	static const unsigned int crtc_mask = 0b10;
	const unsigned int plane_regs_offset = 0x200 + ((crtc_mask >> 1) & 1) * 0x100;

	volatile unsigned int *iftu_plane_regs = IFTU_REGS(plane) + plane_regs_offset;

	iftu_plane_regs[0x00] = info->paddr;
	iftu_plane_regs[0x01] = info->unk18;
	iftu_plane_regs[0x02] = info->unk1C;

	iftu_plane_regs[0x08] = info->unk20;
	iftu_plane_regs[0x09] = info->src_x;
	iftu_plane_regs[0x0A] = info->src_y;

	iftu_plane_regs[0x10] = info->pixelformat;
	iftu_plane_regs[0x11] = info->width;
	iftu_plane_regs[0x12] = info->height;
	iftu_plane_regs[0x13] = 1; // Control reg

	iftu_plane_regs[0x15] = info->leftover_stride;
	iftu_plane_regs[0x16] = info->unk10;

	iftu_plane_regs[0x18] = info->vfront_porch;
	iftu_plane_regs[0x19] = info->vback_porch;
	iftu_plane_regs[0x1A] = info->hfront_porch;
	iftu_plane_regs[0x1B] = info->hback_porch;

	iftu_plane_regs[0x30] = info->src_w;
	iftu_plane_regs[0x31] = info->src_h;
	iftu_plane_regs[0x32] = info->dst_x;
	iftu_plane_regs[0x33] = info->dst_y;
	iftu_plane_regs[0x34] = info->dst_w;
	iftu_plane_regs[0x35] = info->dst_h;

}
