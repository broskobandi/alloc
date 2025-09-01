#ifndef ALLOC_UTILS_H
#define ALLOC_UTILS_H

#include "alloc.h"
#include <error.h>
#include <stdalign.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <string.h>

#define ARENA_SIZE 1024LU * 4
#define ROUNDUP(size)\
	(((size) + alignof(max_align_t) - 1) & ~(alignof(max_align_t) - 1))
#define PTR_ALIGNED_SIZE\
	ROUNDUP(sizeof(ptr_t))
#define TOTAL_SIZE(size)\
	(PTR_ALIGNED_SIZE + ROUNDUP(size))
#define MIN_ALLOC_SIZE alignof(max_align_t)
#define NUM_ALLOC_SIZES\
	((ARENA_SIZE - PTR_ALIGNED_SIZE) / MIN_ALLOC_SIZE)
#define FREE_PTR_INDEX(size)\
	(((TOTAL_SIZE((size)) - PTR_ALIGNED_SIZE) / MIN_ALLOC_SIZE) - 1)
#define MMAP(size)\
	mmap(NULL, (size), PROT_WRITE | PROT_READ, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0)

typedef enum ptr_state {
	FREE,
	VALID,
} ptr_state_t;

typedef struct arena arena_t;
typedef struct ptr ptr_t;
 
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

struct arena {
	alignas(max_align_t) unsigned char buff[ARENA_SIZE];
	size_t offset;
	ptr_t *ptrs_tail;
	arena_t *next;
	arena_t *prev;
};

extern arena_t g_arena_head;
extern arena_t *g_arena_tail;
extern ptr_t *g_free_ptr_tails[NUM_ALLOC_SIZES];

static inline int arena_expand() {
	g_arena_tail->next = MMAP(sizeof(arena_t));
	if (!g_arena_tail->next) ERR("Failed to allocate new arena with mmap.", 1);
	g_arena_tail->next->prev = g_arena_tail;
	g_arena_tail = g_arena_tail->next;
	g_arena_tail->next = NULL;
	g_arena_tail->offset = 0;
	g_arena_tail->ptrs_tail = NULL;
	OK(0);
}

static inline int arena_del(arena_t *arena) {
	if (!arena) ERR("arena cannot be NULL.", 1);
	if (arena == &g_arena_head) ERR("arena head cannot be deleted.", 1);
	if (arena->next) {
		arena->prev->next = arena->next;
		arena->next->prev = arena->prev;
		if (munmap(arena, sizeof(arena_t))) ERR("Failed to unmap arena.", 1);
	} else {
		arena = arena->prev;
		if (munmap(arena->next, sizeof(arena_t))) ERR("Failed to unmap arena.", 1);
		arena->next = NULL;
	}
	OK(0);
}

static inline int reset() {
	while (g_arena_tail->prev) {
		g_arena_tail = g_arena_tail->prev;
		if (munmap(g_arena_tail->next, sizeof(arena_t)) == -1)
			ERR("Failed to unmap arena.", 1);
	}
	memset(&g_arena_head, 0, sizeof(arena_t));
	g_arena_tail = &g_arena_head;
	memset(g_free_ptr_tails, 0, NUM_ALLOC_SIZES * sizeof(ptr_t*));
	OK(0);
}

static inline void *arena_use(size_t size) {
	if (!size) ERR("size cannot be 0.", NULL);
	if (TOTAL_SIZE(size) > ARENA_SIZE) ERR("size is too big.", NULL);
	if (g_arena_tail->offset + TOTAL_SIZE(size) > ARENA_SIZE)
		if (arena_expand()) ERR("Failed to expand arena.", NULL);
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
	OK(g_arena_tail->ptrs_tail->data);
}

static inline int ptr_free(void *data) {
	if (!data) ERR("data cannot be NULL.", 1);
	ptr_t *ptr = (ptr_t*)((unsigned char*)data - PTR_ALIGNED_SIZE);
	if (ptr->state != VALID) ERR("Invalid argument.", 1);
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
	OK(0);
}

static inline void *free_ptr_use(size_t size) {
	if (!size) ERR("size cannot be 0.", NULL);
	ptr_t *ptr = g_free_ptr_tails[FREE_PTR_INDEX(size)];
	if (!g_free_ptr_tails[FREE_PTR_INDEX(size)])
		ERR("No matching free pointer found.", NULL);
	if (ptr->prev_free) {
		ptr->prev_free->next_free = NULL;
		g_free_ptr_tails[FREE_PTR_INDEX(size)] = ptr->prev_free;
	} else {
		g_free_ptr_tails[FREE_PTR_INDEX(size)] = NULL;
	}
	ptr->next_free = NULL;
	ptr->prev_free = NULL;
	ptr->state = VALID;
	OK(ptr->data);
}

// static inline void *mmap_use(size_t size) {
// 	if (!size) ERR("size cannot be 0.", NULL);
// 	if (TOTAL_SIZE(size) <= ARENA_SIZE) ERR("size is too small.", NULL);
//
// }

#endif
