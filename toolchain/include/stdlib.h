#ifndef GIRAFFE_TOOLCHAIN_STDLIB_H
#define GIRAFFE_TOOLCHAIN_STDLIB_H

#include <stddef.h>

void *malloc(size_t size);
void *calloc(size_t count, size_t size);
void *realloc(void *pointer, size_t size);
void free(void *pointer);
int abs(int value);
long strtol(const char *text, char **end, int base);

#endif
