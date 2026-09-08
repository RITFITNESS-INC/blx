#ifndef GIRAFFE_TOOLCHAIN_STDIO_H
#define GIRAFFE_TOOLCHAIN_STDIO_H

#include <stddef.h>

int printf(const char *format, ...);
int sprintf(char *buffer, const char *format, ...);
int snprintf(char *buffer, size_t capacity, const char *format, ...);

#endif
