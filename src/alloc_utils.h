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
 * \file src/alloc_utils.h
 * \brief Private header file for the alloc library.
 * \details This file contains the forward declarations of global variables, 
 * the definitions of inline static helper functions, typedefs and
 * macros for the alloc libary.
 * */

#ifndef ALLOC_UTILS_H
#define ALLOC_UTILS_H

#include "alloc.h"
#include <error.h>
#include <stdalign.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <string.h>
#include <stddef.h>

#define ARENA_SIZE 1024LU * 4
// #define ARENA_SIZE 1024LU * 32
// #define ARENA_SIZE 1024LU * 128
#define ROUNDUP(size)\
	(size_t)(((size) + alignof(max_align_t) - 1) & ~(alignof(max_align_t) - 1))
#define PTR_ALIGNED_SIZE\
	(size_t)ROUNDUP(sizeof(ptr_t))
#define TOTAL_SIZE(size)\
	(size_t)(PTR_ALIGNED_SIZE + ROUNDUP(size))
#define MIN_ALLOC_SIZE alignof(max_align_t)
#define NUM_ALLOC_SIZES\
	(size_t)((ARENA_SIZE - PTR_ALIGNED_SIZE) / MIN_ALLOC_SIZE)
#define FREE_PTR_INDEX(size)\
	(size_t)(((TOTAL_SIZE((size)) - PTR_ALIGNED_SIZE) / MIN_ALLOC_SIZE) - 1)
#define MMAP(size)\
	mmap(NULL, (size), PROT_WRITE | PROT_READ, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0)
#define PTR(data)\
	((ptr_t*)((unsigned char*)data - PTR_ALIGNED_SIZE))

/** Enum containing the possible pointer states. */
typedef enum ptr_state {
	FREE,
	VALID,
} ptr_state_t;

/** Arena struct tontaining the main memory buffer and metadata.
 * Forward declaration. */
typedef struct arena arena_t;

/** Pointer struct containing a raw pointer and metadata.
 * Forward declaration. */
typedef struct ptr ptr_t;
 
/** Pointer struct containing a raw pointer and metadata. */
struct ptr {
	void *data;
	size_t size;
	arena_t *arena;
	ptr_t *next_valid;
	ptr_t *prev_valid;
	ptr_t *next_free;
	ptr_t *prev_free;
	ptr_state_t state;
};

/** Arena struct tontaining the main memory buffer and metadata. */
struct arena {
	alignas(max_align_t) unsigned char buff[ARENA_SIZE];
	size_t offset;
	ptr_t *ptrs_tail;
	arena_t *next;
	arena_t *prev;
};

/** Global instance of the arena struct that acts as the head in the linked list.
 * Forward declaration. */
extern _Thread_local arena_t g_arena_head;
// extern arena_t g_arena_head;

/** Global instance of the arena struct that acts as the tail in the linked list.
 * Forward declaration. */
extern _Thread_local arena_t *g_arena_tail;
// extern arena_t *g_arena_tail;

/** Global instance of an array of linked lists containing free pointers.
 * Forward declaration. */
extern _Thread_local ptr_t *g_free_ptr_tails[NUM_ALLOC_SIZES];
// extern ptr_t *g_free_ptr_tails[NUM_ALLOC_SIZES];

/** Expands the arena by allocating a new node with mmap().
 * \return 0 on success or 1 on failure. */
static inline int arena_expand() {
	g_arena_tail->next = (arena_t*)MMAP(sizeof(arena_t));
	if (!g_arena_tail->next) RET_ERR("Failed to allocate new arena with mmap.", 1);
	g_arena_tail->next->prev = g_arena_tail;
	g_arena_tail = g_arena_tail->next;
	g_arena_tail->next = NULL;
	g_arena_tail->offset = 0;
	g_arena_tail->ptrs_tail = NULL;
	RET_OK(0);
}

/** Removes an arena node from the linked list.
 * \param arena The arena node to be deleted.
 * \return 0 on success or 1 on failure. */
static inline int arena_del(arena_t *arena) {
	if (!arena) RET_ERR("arena cannot be NULL.", 1);
	if (arena == &g_arena_head) RET_ERR("arena head cannot be deleted.", 1);
	if (arena->prev && arena->next) {
		arena->prev->next = arena->next;
		arena->next->prev = arena->prev;
		if (munmap(arena, sizeof(arena_t))) RET_ERR("Failed to unmap arena.", 1);
	} else if (!arena->next) {
		g_arena_tail = arena->prev;
		if (munmap(g_arena_tail->next, sizeof(arena_t)))
			RET_ERR("Failed to unmap arena.", 1);
		g_arena_tail->next = NULL;
	}
	RET_OK(0);
}

/** Resets all global variables. Unmaps heap memory.
 * \return 0 on success or 1 on failure. */
static inline int reset() {
	error_reset();
	while (g_arena_tail && g_arena_tail->prev) {
		g_arena_tail = g_arena_tail->prev;
		if (munmap(g_arena_tail->next, sizeof(arena_t)) == -1)
			RET_ERR("Failed to unmap arena.", 1);
	}
	memset(&g_arena_head, 0, sizeof(arena_t));
	g_arena_tail = &g_arena_head;
	memset(g_free_ptr_tails, 0, sizeof(g_free_ptr_tails));
	RET_OK(0);
}

/** Returns a pointer to a memory block allocated in the arena.
 * \param size The size of the block to be allocated. 
 * \return The pointer to the allocated data or NULL on failure. */
static inline void *arena_use(size_t size) {
	if (!size) RET_ERR("size cannot be 0.", NULL);
	if (TOTAL_SIZE(size) > ARENA_SIZE) RET_ERR("size is too big.", NULL);
	if (g_arena_tail->offset + TOTAL_SIZE(size) > ARENA_SIZE)
		if (arena_expand()) RET_ERR("Failed to expand arena.", NULL);
	if (!g_arena_tail->ptrs_tail) {
		g_arena_tail->ptrs_tail = (ptr_t*)&g_arena_tail->buff[g_arena_tail->offset];
	} else {
		g_arena_tail->ptrs_tail->next_valid =
			(ptr_t*)&g_arena_tail->buff[g_arena_tail->offset];
		g_arena_tail->ptrs_tail->next_valid->prev_valid =
			g_arena_tail->ptrs_tail;
		g_arena_tail->ptrs_tail =
			g_arena_tail->ptrs_tail->next_valid;
	}
	g_arena_tail->ptrs_tail->next_valid = NULL;
	g_arena_tail->ptrs_tail->size = size;
	g_arena_tail->ptrs_tail->state = VALID;
	g_arena_tail->ptrs_tail->arena = g_arena_tail;
	g_arena_tail->ptrs_tail->data = 
		(unsigned char*)g_arena_tail->ptrs_tail + PTR_ALIGNED_SIZE;
	g_arena_tail->offset += TOTAL_SIZE(size);
	RET_OK(g_arena_tail->ptrs_tail->data);
}

/** Marks a pointer and its associated data free.
 * \param data The poiter to the data to be freed.
 * \return 0 on sucecss or 1 on failure. */
static inline int ptr_free(void *data) {
	if (!data) RET_ERR("data cannot be NULL.", 1);
	ptr_t *ptr = (ptr_t*)((unsigned char*)data - PTR_ALIGNED_SIZE);
	if (ptr->state != VALID) RET_ERR("Invalid argument.", 1);
	if (!ptr->arena) {
		if (munmap(ptr, TOTAL_SIZE(ptr->size)) == -1)
			RET_ERR("Failed to unmap memory with munmap().", 1);
		RET_OK(0);
	}
	if (
		!ptr->arena->ptrs_tail->prev_valid &&
		ptr->arena->prev &&
		ptr->arena->prev->offset < ARENA_SIZE - MIN_ALLOC_SIZE - PTR_ALIGNED_SIZE
	) {
		arena_del(ptr->arena);
		RET_OK(0);
	}
	size_t i = FREE_PTR_INDEX(ptr->size);
	if (!g_free_ptr_tails[i]) {
		g_free_ptr_tails[i] = ptr;
		ptr->prev_free = NULL;
	} else {
		g_free_ptr_tails[i]->next_free = ptr;
		g_free_ptr_tails[i]->next_free->prev_free = g_free_ptr_tails[i];
		g_free_ptr_tails[i] = g_free_ptr_tails[i]->next_free;
	}
	g_free_ptr_tails[i]->next_free = NULL;
	g_free_ptr_tails[i]->state = FREE;
	RET_OK(0);
}

/** Reuses a pointer and its associated data that was previously marked free.
 * \param size The size of the memory block to be reused. 
 * \return A pointer to the memory block or NULL on failure. */
static inline void *free_ptr_use(size_t size) {
	if (!size) RET_ERR("size cannot be 0.", NULL);
	ptr_t *ptr = g_free_ptr_tails[FREE_PTR_INDEX(size)];
	if (!g_free_ptr_tails[FREE_PTR_INDEX(size)])
		RET_ERR("No matching free pointer found.", NULL);
	if (ptr->prev_free) {
		ptr->prev_free->next_free = NULL;
		g_free_ptr_tails[FREE_PTR_INDEX(size)] = ptr->prev_free;
	} else {
		g_free_ptr_tails[FREE_PTR_INDEX(size)] = NULL;
	}
	ptr->next_free = NULL;
	ptr->prev_free = NULL;
	ptr->state = VALID;
	RET_OK(ptr->data);
}

/** Allocates a block of memory in the heap using mmap().
 * \param size The size of the memory block to be allocated. 
 * \return A pointer to the memory block or NULL on failure. */
static inline void *mmap_use(size_t size) {
	if (!size) RET_ERR("size cannot be 0.", NULL);
	if (TOTAL_SIZE(size) <= ARENA_SIZE) RET_ERR("size is too small.", NULL);
	ptr_t *ptr = (ptr_t*)MMAP(TOTAL_SIZE(size));
	if (!ptr) RET_ERR("Failed to allocate ptr with mmap().", NULL);
	ptr->data = (unsigned char*)ptr + PTR_ALIGNED_SIZE;
	ptr->state = VALID;
	ptr->arena = NULL;
	ptr->prev_valid = NULL;
	ptr->next_valid = NULL;
	ptr->size = size;
	RET_OK(ptr->data);
}

#endif
