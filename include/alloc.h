/*
MIT License

Copyright (c) 2025 broskobandi

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/**
 * \file include/alloc.h
 * \brief Public header file for the alloc library.
 * \details This file contains the public function declarations for the 
 * alloc library.
 * */

#ifndef ALLOC_H
#define ALLOC_H

#include <stddef.h>

/** Allocates a new block of memory.
 * \param size The size of the memory to be allocated. 
 * \return A pointer to the newly allocated memory or NULL on failure. 
 * It sets errno on failure. */
void *alloc_new(size_t size);

/** Deallocates a block of memory.
 * \param ptr Pointer to the memory to be deallocated.
 * It sets errno on failure. */
void alloc_del(void *ptr);

/** Allocates a new block and copies the old content over.
 * \param ptr Pointer to the pointer that's associated with the memory block
 * to be resized.
 * \param size The size of the new block.
 * \return 0 on success and 1 on failure.
 * It sets errno on failure. */
int alloc_resize(void **ptr, size_t size);

#endif
