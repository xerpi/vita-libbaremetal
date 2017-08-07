#ifndef UTILS_H
#define UTILS_H

#define BIT(x)		(1 << (x))
#define ARRAY_SIZE(x)	(sizeof(x) / sizeof((x)[0]))

#define dmb() asm volatile("dmb\n\t")
#define dsb() asm volatile("dsb\n\t")

static inline unsigned int rbit(unsigned int x)
{
	unsigned int xrev;
	asm volatile("rbit %0, %1\n\t" : "=r"(xrev) : "r"(x));
	return xrev;
}

static inline unsigned char readb(volatile void *addr)
{
	return *(unsigned char *)addr;
}

static inline unsigned short readw(volatile void *addr)
{
	return *(unsigned short *)addr;
}

static inline unsigned int readl(volatile void *addr)
{
	return *(unsigned int *)addr;
}

static inline void writeb(unsigned char val, volatile void *addr)
{
	*(unsigned char *)addr = val;
}

static inline void writew(unsigned short val, volatile void *addr)
{
	*(unsigned short *)addr = val;
}

static inline void writel(unsigned int val, volatile void *addr)
{
	*(unsigned int *)addr = val;
}

void delay(int n);

#endif
