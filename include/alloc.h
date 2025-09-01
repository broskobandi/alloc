#ifndef ALLOC_H
#define ALLOC_H

#include <stddef.h>

void *alloc_new(size_t size);
void alloc_del(void *ptr);
void *alloc_resize(void *ptr, size_t size);

#endif
