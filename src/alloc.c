#include "alloc_utils.h"
#include <pthread.h>

arena_t g_arena_head;
arena_t *g_arena_tail = &g_arena_head;
ptr_t *g_free_ptr_tails[NUM_ALLOC_SIZES] = {0};

pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

void *alloc_new(size_t size) {
	if (!size) ERR("size cannot be 0.", NULL);
	pthread_mutex_lock(&g_mutex);
	if (TOTAL_SIZE(size) > ARENA_SIZE) {
		void *ptr = mmap_use(size);
		pthread_mutex_unlock(&g_mutex);
		return ptr;
	}
	if (g_free_ptr_tails[FREE_PTR_INDEX(size)] && TOTAL_SIZE(size) <= ARENA_SIZE) {
		void *ptr = free_ptr_use(size);
		pthread_mutex_unlock(&g_mutex);
		return ptr;
	}
	if (!g_free_ptr_tails[FREE_PTR_INDEX(size)] && TOTAL_SIZE(size) <= ARENA_SIZE) {
		void *ptr = arena_use(size);
		pthread_mutex_unlock(&g_mutex);
		return ptr;
	}
	pthread_mutex_unlock(&g_mutex);
	ERR("Failed to allocate memory.", NULL);
}

void alloc_del(void *ptr) {
	if (!ptr) ERR("ptr cannot be NULL.");
	pthread_mutex_lock(&g_mutex);
	if (ptr_free(ptr)) ERROR_SET("Failed to free pointer.");
	pthread_mutex_unlock(&g_mutex);
}

void *alloc_resize(void *ptr, size_t size) {
	if (!size) ERR("size cannot be 0.", NULL);
	if (!ptr) ERR("ptr cannot be NULL.", NULL);
	// void *new_ptr = alloc_new(size);
	// pthread_mutex_lock(&g_mutex);
	// printf("segg\n");
	// if (!new_ptr) {
	// 	pthread_mutex_unlock(&g_mutex);
	// 	ERR("Failed to allocate new memory.", NULL);
	// }
	// size_t size_to_copy =
	// 	PTR(ptr)->size > size ?
	// 	size : PTR(ptr)->size;
	// memcpy(new_ptr, ptr, size_to_copy);
	pthread_mutex_unlock(&g_mutex);
	// return new_ptr;
	return NULL;
}
