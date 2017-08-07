#include "libc.h"

void *memset(void *s, int c, size_t n)
{
	char *p = s;

	while (n) {
		*p++ = c;
		n--;
	}

	return s;
}

void *memcpy(void *dest, const void *src, size_t n)
{
	const char *s = src;
	char *d = dest;

	while (n) {
		*d++ = *s++;
		n--;
	}

	return dest;
}
