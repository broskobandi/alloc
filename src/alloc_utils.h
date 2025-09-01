#ifndef ALLOC_UTILS_H
#define ALLOC_UTILS_H

#include "alloc.h"
#include <error.h>
#include <stdalign.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <string.h>

#define ARENA_SIZE 1024LU * 4
#define PTR_ALIGNED_SIZE\
	((sizeof(ptr_t) + alignof(max_align_t) - 1) & ~(alignof(max_align_t) - 1))
#define MIN_ALLOC_SIZE alignof(max_align_t)
#define NUM_ALLOC_SIZES\
	(ARENA_SIZE - MIN_ALLOC_SIZE - PTR_ALIGNED_SIZE) / MIN_ALLOC_SIZE
#define MMAP(size)\
	mmap(NULL, (size), PROT_WRITE | PROT_READ, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0)

typedef struct arena arena_t;
typedef struct ptr ptr_t;
 
struct ptr {
	void *ptr;
	size_t size;
	arena_t *arena;
	bool is_valid;
	ptr_t *next;
	ptr_t *prev;
};

struct arena {
	alignas(max_align_t) unsigned char buff[ARENA_SIZE];
	ptr_t *ptrs_tail;
	arena_t *next;
	arena_t *prev;
};

extern arena_t g_arena_head;
extern arena_t *g_arena_tail;
extern ptr_t *g_free_ptrs_tail[NUM_ALLOC_SIZES];

static inline int arena_expand() {
	g_arena_tail->next = MMAP(sizeof(arena_t));
	if (!g_arena_tail->next) ERR("Failed to allocate new arena with mmap.", 1);
	g_arena_tail->next->prev = g_arena_tail;
	g_arena_tail = g_arena_tail->next;
	g_arena_tail->next = NULL;
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

static inline int arena_reset() {
	while (g_arena_tail->prev) {
		g_arena_tail = g_arena_tail->prev;
		if (munmap(g_arena_tail->next, sizeof(arena_t)) == -1)
			ERR("Failed to unmap arena.", 1);
	}
	memset(&g_arena_head, 0, sizeof(arena_t));
	g_arena_tail = &g_arena_head;
	OK(0);
}

#endif
