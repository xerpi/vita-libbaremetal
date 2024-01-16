#include <stdint.h>
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

int memcmp(const void *s1, const void *s2, size_t n)
{
	uint8_t u1, u2;

	for (; n--; s1++, s2++) {
		u1 = *(uint8_t *)s1;
		u2 = *(uint8_t *)s2;
		if (u1 != u2)
			return u1 - u2;
	}

	return 0;
}

size_t strlen(const char *str)
{
	const char *s = str;

	while (*s)
		s++;

	return s - str;
}

