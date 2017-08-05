#include "utils.h"

void delay(int n)
{
	volatile int i, j;

	for (i = 0; i < n; i++)
		for (j = 0; j < 200; j++)
			;
}
