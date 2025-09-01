#ifndef ALLOC_UTILS_H
#define ALLOC_UTILS_H

#include "alloc.h"
#include <error.h>
#include <stdalign.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <string.h>

#define VALID_MAGIC (size_t)0x900DF00D
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
	void *data;
	size_t size;
	arena_t *arena;
	size_t valid_magic;
	ptr_t *next;
	ptr_t *prev;
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

static inline size_t roundup(size_t size) {
	if (!size) ERR("size cannot be 0.", (size_t)-1);
	OK(((size + alignof(max_align_t) - 1) & ~(alignof(max_align_t) - 1)));
}

static inline size_t total_size(size_t size) {
	if (!size) ERR("size cannot be 0.", (size_t)-1);
	OK(PTR_ALIGNED_SIZE + roundup(size));
}

static inline void *arena_use(size_t size) {
	if (!size) ERR("size cannot be 0.", NULL);
	if (total_size(size) > ARENA_SIZE) ERR("size is too big.", NULL);
	if (g_arena_tail->offset + total_size(size) > ARENA_SIZE)
		if (arena_expand()) ERR("Failed to expand arena.", NULL);
	if (!g_arena_tail->ptrs_tail) {
		g_arena_tail->ptrs_tail = (ptr_t*)&g_arena_tail->buff[g_arena_tail->offset];
	} else {
		g_arena_tail->ptrs_tail->next = (ptr_t*)&g_arena_tail->buff[g_arena_tail->offset];
		g_arena_tail->ptrs_tail->next->prev = g_arena_tail->ptrs_tail;
		g_arena_tail->ptrs_tail = g_arena_tail->ptrs_tail->next;
	}
	g_arena_tail->ptrs_tail->next = NULL;
	g_arena_tail->ptrs_tail->size = size;
	g_arena_tail->ptrs_tail->valid_magic = VALID_MAGIC;
	g_arena_tail->ptrs_tail->arena = g_arena_tail;
	g_arena_tail->ptrs_tail->data = 
		(unsigned char*)g_arena_tail->ptrs_tail + PTR_ALIGNED_SIZE;
	g_arena_tail->offset = total_size(size);
	OK(g_arena_tail->ptrs_tail->data);
}

static inline size_t free_ptr_index(size_t size) {
	if (!size) ERR("size cannto be 0.", (size_t)-1);
	OK(((total_size(size) - PTR_ALIGNED_SIZE) / MIN_ALLOC_SIZE) - 1);
}

static inline int ptr_free(void *data) {
	if (!data) ERR("data cannot be NULL.", 1);
	ptr_t *ptr = (ptr_t*)((unsigned char*)data - PTR_ALIGNED_SIZE);
	if (ptr->valid_magic != VALID_MAGIC) ERR("Invalid argument.", 1);
	// if (!g_free_ptr_tails)
	OK(0);
}

#endif
