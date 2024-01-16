#include <stdint.h>
#include "utils.h"

void delay(uint32_t n)
{
	volatile uint32_t i, j;

	for (i = 0; i < n; i++)
		for (j = 0; j < 200; j++)
			;
}

uint32_t get_cpu_id(void)
{
	uint32_t mpidr;
	asm volatile("mrc p15, 0, %0, c0, c0, 5\n\t" : "=r"(mpidr));
	return mpidr & 0xF;
}

