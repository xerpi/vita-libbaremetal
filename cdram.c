#include "cdram.h"

void cdram_enable(void)
{
	asm volatile(
		"ldr r12, =0x117\n\t"
		"smc #0\n\t"
		: : : "r12"
	);
}
