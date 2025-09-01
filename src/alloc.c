#include "alloc_utils.h"
#include <pthread.h>

arena_t g_arena_head;
arena_t *g_arena_tail;
ptr_t *g_free_ptrs_tail[NUM_ALLOC_SIZES];

pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

void *alloc_new(size_t size) {
	if (!size) ERR("size cannot be 0.", NULL);
	pthread_mutex_lock(&g_mutex);
	// try to use free ptr
	// try to use arena
	// try to use heap
	pthread_mutex_unlock(&g_mutex);
	return NULL;
}

void alloc_del(void *ptr) {
	if (!ptr) ERR("ptr cannot be NULL.");
	pthread_mutex_lock(&g_mutex);
	// free ptr
	// try to free arena node
	pthread_mutex_unlock(&g_mutex);
}

void *alloc_resize(void *ptr, size_t size) {
	if (!size) ERR("size cannot be 0.", NULL);
	if (!ptr) ERR("ptr cannot be NULL.", NULL);
	pthread_mutex_lock(&g_mutex);
	// try to expand allocation in place
	// alloc
	// copy data
	pthread_mutex_unlock(&g_mutex);
	return NULL;
}
