#ifndef UTILS_H
#define UTILS_H

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define dmb() asm volatile("dmb\n\t")
#define dsb() asm volatile("dsb\n\t")

void delay(int n);

#endif
