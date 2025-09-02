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
 * \file src/alloc.c
 * \brief Implementation for the alloc library.
 * \details This file contains the definitions of the public functions and 
 * initializes global variables. 
 * */

#include "alloc_utils.h"
#include <pthread.h>

/** Global instance of the arena struct that acts as the head in the linked list. */
// _Thread_local arena_t g_arena_head;
// arena_t g_arena_head = {0};
_Thread_local arena_t g_arena_head = {0};

/** Global instance of the arena struct that acts as the tail in the linked list. */
// _Thread_local arena_t *g_arena_tail = &g_arena_head;
_Thread_local arena_t *g_arena_tail = NULL;
// arena_t *g_arena_tail = NULL;

/** Global instance of an array of linked lists containing free pointers. */
_Thread_local ptr_t *g_free_ptr_tails[NUM_ALLOC_SIZES] = {0};
// ptr_t *g_free_ptr_tails[NUM_ALLOC_SIZES] = {0};

/** Global instance of a mutex object. */
// pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

/** Allocates a new block of memory.
 * \param size The size of the memory to be allocated. 
 * \return A pointer to the newly allocated memory or NULL on failure. 
 * It sets errno on failure. */
void *alloc_new(size_t size) {
	if (!size) ERR("size cannot be 0.", NULL);
	if (!g_arena_tail) g_arena_tail = &g_arena_head;
	// pthread_mutex_lock(&g_mutex);
	if (TOTAL_SIZE(size) > ARENA_SIZE) {
		void *ptr = mmap_use(size);
		// pthread_mutex_unlock(&g_mutex);
		return ptr;
	}
	if (g_free_ptr_tails[FREE_PTR_INDEX(size)] && TOTAL_SIZE(size) <= ARENA_SIZE) {
		void *ptr = free_ptr_use(size);
		// pthread_mutex_unlock(&g_mutex);
		return ptr;
	}
	if (!g_free_ptr_tails[FREE_PTR_INDEX(size)] && TOTAL_SIZE(size) <= ARENA_SIZE) {
		void *ptr = arena_use(size);
		// pthread_mutex_unlock(&g_mutex);
		return ptr;
	}
	// pthread_mutex_unlock(&g_mutex);
	ERR("Failed to allocate memory.", NULL);
}

/** Deallocates a block of memory.
 * \param ptr Pointer to the memory to be deallocated.
 * It sets errno on failure. */
void alloc_del(void *ptr) {
	if (!ptr) ERR("ptr cannot be NULL.");
	// pthread_mutex_lock(&g_mutex);
	if (ptr_free(ptr)) ERROR_SET("Failed to free pointer.");
	// pthread_mutex_unlock(&g_mutex);
}

/** Allocates a new block and copies the old content over.
 * \param ptr Pointer to the pointer that's associated with the memory block
 * to be resized.
 * \param size The size of the new block.
 * \return 0 on success and 1 on failure.
 * It sets errno on failure. */
int alloc_resize(void **ptr, size_t size) {
	if (!size) ERR("size cannot be 0.", 1);
	if (!ptr || !*ptr) ERR("ptr cannot be NULL.", 1);
	void *new_ptr = alloc_new(size);
	if (new_ptr) {
		size_t size_to_copy =
			PTR(ptr)->size > size ?
			size : PTR(ptr)->size;
		memcpy(new_ptr, *ptr, size_to_copy);
		*ptr = new_ptr;
		OK(0);
	} else {
		ERR("Failed to allocate new memory.", 1);
	}
}
