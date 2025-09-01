#ifndef ALLOC_UTILS_H
#define ALLOC_UTILS_H

#include "alloc.h"
#include <error.h>
#include <stdalign.h>
#include <stdbool.h>
#include <sys/mman.h>

#define ARENA_SIZE 1024LU * 4
#define PTR_ALIGNED_SIZE\
	((sizeof(ptr_t) + alignof(max_align_t) - 1) & ~(alignof(max_align_t) - 1))
#define MIN_ALLOC_SIZE alignof(max_align_t)
#define NUM_ALLOC_SIZES\
	(ARENA_SIZE - MIN_ALLOC_SIZE - PTR_ALIGNED_SIZE) / MIN_ALLOC_SIZE

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

static inline void arena_reset() {
	arena_t *cur = g_arena_tail;
	while (cur) {
		if (cur->prev) {
			cur = cur->prev;
			if (munmap(cur->next, sizeof(arena_t)) == -1)
				ERR("Failed to unmap arena.");
		}
	}
}

#endif
