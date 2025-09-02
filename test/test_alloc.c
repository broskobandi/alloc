#include "test_utils.h"
#include "alloc_utils.h"

/**
 * alloc_utils.
 * */

void test_arena_expand() {
	{ // Normal case
		ASSERT(!g_arena_head.next);
		ASSERT(g_arena_tail == &g_arena_head);
		ASSERT(!arena_expand());
		ASSERT(g_arena_tail == g_arena_head.next);
		ASSERT(g_arena_tail->prev == &g_arena_head);
		ASSERT(!arena_expand());
		ASSERT(g_arena_tail == g_arena_head.next->next);
		ASSERT(g_arena_tail->prev->prev == &g_arena_head);
	}
}

void test_arena_reset() {
	{ // Normal case
		ASSERT(!arena_expand());
		ASSERT(!arena_expand());
		ASSERT(!arena_expand());
		ASSERT(!reset());
		ASSERT(g_arena_tail == &g_arena_head);
		ASSERT(!g_arena_head.next);
	}
}

void test_arena_del() {
	{ // Normal case
		ASSERT(!reset());
		ASSERT(!arena_expand());
		ASSERT(!arena_expand());
		ASSERT(!arena_del(g_arena_head.next));
		ASSERT(g_arena_head.next == g_arena_tail);
		ASSERT(g_arena_tail->prev == &g_arena_head);
	}
	{ // arena NULL
		ASSERT(!reset());
		ASSERT(arena_del(NULL));
	}
	{ // arena is head
		ASSERT(!reset());
		ASSERT(arena_del(g_arena_tail));
	}
}

void test_roundup() {
	{ // Normal case
		size_t size = alignof(max_align_t) / 2;
		ASSERT(ROUNDUP(size) == alignof(max_align_t));
	}
}

void test_ptr_aligned_size() {
	{ // Normal case
		ASSERT(ROUNDUP(sizeof(ptr_t)) == PTR_ALIGNED_SIZE);
	}
}

void test_total_size() {
	{ // Normal case
		size_t size = alignof(max_align_t) / 2;
		size_t exp = ROUNDUP(size) + PTR_ALIGNED_SIZE;
		ASSERT(TOTAL_SIZE(size) == exp);
	}
}

void test_arena_use() {
	{ // Normal case
		ASSERT(!reset());
		size_t size1 = ARENA_SIZE / 32;
		size_t size2 = ARENA_SIZE / 16;
		void *data1 = arena_use(size1);
		void *data2 = arena_use(size1);
		void *data3 = arena_use(size2);
		void *data4 = arena_use(size2);
		ASSERT(data1);
		ASSERT(data2);
		ASSERT(data3);
		ASSERT(data4);
		ptr_t *ptr4 = g_arena_tail->ptrs_tail;
		ptr_t *ptr3 = ptr4->prev_valid;
		ptr_t *ptr2 = ptr3->prev_valid;
		ptr_t *ptr1 = ptr2->prev_valid;
		ASSERT(ptr4->data == data4);
		ASSERT(ptr3->data == data3);
		ASSERT(ptr2->data == data2);
		ASSERT(ptr1->data == data1);
		ASSERT(ptr1->size == size1);
		ASSERT(ptr2->size == size1);
		ASSERT(ptr3->size == size2);
		ASSERT(ptr4->size == size2);
		ASSERT(ptr1->state == VALID);
		ASSERT(ptr2->state == VALID);
		ASSERT(ptr3->state == VALID);
		ASSERT(ptr4->state == VALID);
		ASSERT(ptr1->next_valid == ptr2);
		ASSERT(ptr2->prev_valid == ptr1);
		ASSERT(ptr2->next_valid == ptr3);
		ASSERT(ptr3->prev_valid == ptr2);
		ASSERT(ptr3->next_valid == ptr4);
		ASSERT(ptr4->prev_valid == ptr3);
	}
	{ // Normal case: expand arena
		ASSERT(!reset());
		g_arena_tail->offset = ARENA_SIZE - MIN_ALLOC_SIZE / 2;
		void *data = arena_use(MIN_ALLOC_SIZE);
		ASSERT(data);
		ASSERT(g_arena_tail->prev == &g_arena_head);
	}
	{ // size 0
		ASSERT(!reset());
		ASSERT(!arena_use(0));
	}
	{ // size too big
		ASSERT(!reset());
		ASSERT(!arena_use(ARENA_SIZE * 2));
	}
}

void test_free_ptr_index() {
	{ // Normal case
		ASSERT(!reset());
		ASSERT(FREE_PTR_INDEX(MIN_ALLOC_SIZE) == 0);
		ASSERT(!reset());;
		ASSERT(FREE_PTR_INDEX(MIN_ALLOC_SIZE * 2) == 1);
		ASSERT(!reset());;
		ASSERT(FREE_PTR_INDEX(MIN_ALLOC_SIZE * 3) == 2);
		ASSERT(!reset());;
	}
}

void test_ptr_free() {
	{ // Normal case
		ASSERT(!reset());
		size_t size1 = MIN_ALLOC_SIZE / 2;
		size_t size2 = MIN_ALLOC_SIZE * 2;
		size_t index1 = FREE_PTR_INDEX(size1);
		size_t index2 = FREE_PTR_INDEX(size2);
		void *data1 = arena_use(size1);
		void *data2 = arena_use(size1);
		void *data3 = arena_use(size2);
		void *data4 = arena_use(size2);
		ptr_t *ptr1 = (ptr_t*)((unsigned char*)data1 - PTR_ALIGNED_SIZE);
		ptr_t *ptr2 = (ptr_t*)((unsigned char*)data2 - PTR_ALIGNED_SIZE);
		ptr_t *ptr3 = (ptr_t*)((unsigned char*)data3 - PTR_ALIGNED_SIZE);
		ptr_t *ptr4 = (ptr_t*)((unsigned char*)data4 - PTR_ALIGNED_SIZE);
		ASSERT(!ptr_free(data1));
		ASSERT(!ptr_free(data2));
		ASSERT(!ptr_free(data3));
		ASSERT(!ptr_free(data4));
		ASSERT(g_free_ptr_tails[index1] == ptr2);
		ASSERT(g_free_ptr_tails[index1]->prev_free == ptr1);
		ASSERT(g_free_ptr_tails[index2] == ptr4);
		ASSERT(g_free_ptr_tails[index2]->prev_free == ptr3);
		ASSERT(!g_free_ptr_tails[index1]->next_free)
		ASSERT(!g_free_ptr_tails[index2]->next_free)
		ASSERT(ptr1->state == FREE);
		ASSERT(ptr2->state == FREE);
		ASSERT(ptr3->state == FREE);
		ASSERT(ptr4->state == FREE);
	}
	{ // Normal case: delete arena
		ASSERT(!reset());
		ASSERT(g_arena_tail == &g_arena_head);
		ASSERT(!arena_expand());
		void *data = arena_use(MIN_ALLOC_SIZE);
		ASSERT(data);
		ptr_t *ptr = (ptr_t*)((unsigned char*)data - PTR_ALIGNED_SIZE);
		ASSERT(ptr->arena == g_arena_head.next);
		ASSERT(g_arena_head.next == g_arena_tail);
		ASSERT(g_arena_head.offset < ARENA_SIZE - MIN_ALLOC_SIZE - PTR_ALIGNED_SIZE);
		ASSERT(g_arena_head.next->prev);
		ASSERT(!ptr_free(data));
		ASSERT(g_arena_tail == &g_arena_head);
		ASSERT(!reset());
		ASSERT(g_arena_tail == &g_arena_head);
		ASSERT(!arena_expand());
		data = arena_use(MIN_ALLOC_SIZE);
		ASSERT(data);
		ptr = (ptr_t*)((unsigned char*)data - PTR_ALIGNED_SIZE);
		ASSERT(!arena_expand());
		ASSERT(ptr->arena == g_arena_head.next);
		ASSERT(g_arena_head.next == ptr->arena);
		ASSERT(g_arena_tail->prev == ptr->arena);
		ASSERT(!ptr_free(data));
		ASSERT(g_arena_tail == g_arena_head.next);
		ASSERT(g_arena_tail->prev == &g_arena_head);
	}
	{ // Normal case: munmap
		ASSERT(!reset());
		void *data = mmap_use(ARENA_SIZE * 2);
		ASSERT(data);
		ASSERT(!PTR(data)->arena);
		ASSERT(!ptr_free(data));
	}
	{ // Data NULL
		ASSERT(!reset());
		ASSERT(ptr_free(NULL));
	}
	{ // Invalid argument
		ASSERT(!reset());
		int x = 5;
		void *dummy = &x;
		ASSERT(ptr_free(dummy));
	}
}

void test_free_ptr_use() {
	{ // Normal case
		ASSERT(!reset());
		size_t size1 = MIN_ALLOC_SIZE / 2;
		size_t size2 = MIN_ALLOC_SIZE * 2;
		void *data1 = arena_use(size1);
		void *data2 = arena_use(size1);
		void *data3 = arena_use(size2);
		void *data4 = arena_use(size2);
		ptr_t *ptr1 = (ptr_t*)((unsigned char*)data1 - PTR_ALIGNED_SIZE);
		ptr_t *ptr2 = (ptr_t*)((unsigned char*)data2 - PTR_ALIGNED_SIZE);
		ptr_t *ptr3 = (ptr_t*)((unsigned char*)data3 - PTR_ALIGNED_SIZE);
		ptr_t *ptr4 = (ptr_t*)((unsigned char*)data4 - PTR_ALIGNED_SIZE);
		ASSERT(!ptr_free(data1));
		ASSERT(!ptr_free(data2));
		ASSERT(!ptr_free(data3));
		ASSERT(!ptr_free(data4));

		void *data5 = free_ptr_use(size1);
		void *data6 = free_ptr_use(size1);
		void *data7 = free_ptr_use(size2);
		void *data8 = free_ptr_use(size2);
		ptr_t *ptr5 = (ptr_t*)((unsigned char*)data5 - PTR_ALIGNED_SIZE);
		ptr_t *ptr6 = (ptr_t*)((unsigned char*)data6 - PTR_ALIGNED_SIZE);
		ptr_t *ptr7 = (ptr_t*)((unsigned char*)data7 - PTR_ALIGNED_SIZE);
		ptr_t *ptr8 = (ptr_t*)((unsigned char*)data8 - PTR_ALIGNED_SIZE);
		ASSERT(ptr5 == ptr2);
		ASSERT(ptr6 == ptr1);
		ASSERT(ptr7 == ptr4);
		ASSERT(ptr8 == ptr3);
		ASSERT(ptr5->state == VALID);
		ASSERT(ptr6->state == VALID);
		ASSERT(ptr7->state == VALID);
		ASSERT(ptr8->state == VALID);
		ASSERT(!ptr5->next_free);
		ASSERT(!ptr6->next_free);
		ASSERT(!ptr7->next_free);
		ASSERT(!ptr8->next_free);
		ASSERT(!ptr5->prev_free);
		ASSERT(!ptr6->prev_free);
		ASSERT(!ptr7->prev_free);
		ASSERT(!ptr8->prev_free);
		ASSERT(ptr5->size == size1);
		ASSERT(ptr6->size == size1);
		ASSERT(ptr7->size == size2);
		ASSERT(ptr8->size == size2);
	}
	{ // size 0
		ASSERT(!reset());
		ASSERT(!free_ptr_use(0));
	}
	{ // no matching free pointer
		ASSERT(!reset());
		size_t size = ARENA_SIZE / 32;
		ASSERT(!free_ptr_use(size));
	}
}

void test_mmap_use() {
	{ // Normal case
		ASSERT(!reset());
		void *data = mmap_use(ARENA_SIZE * 10);
		ASSERT(data);
		ptr_t *ptr = (ptr_t*)((unsigned char*)data - PTR_ALIGNED_SIZE);
		ASSERT(ptr->state == VALID);
		ASSERT(ptr->size == ARENA_SIZE * 10);
		ASSERT(ptr->data == data);
		ASSERT(munmap(PTR(data), TOTAL_SIZE(ARENA_SIZE * 10)) != -1);
	}
	{ // size 0
		void *data = mmap_use(0);
		ASSERT(!data);
	}
	{ // size too small
		void *data = mmap_use(MIN_ALLOC_SIZE);
		ASSERT(!data);
	}
}

/** 
 * alloc.c
 * */

void test_alloc_new() {
	{ // Normal case
		ASSERT(!reset());
		int *data = alloc_new(sizeof(int));
		ASSERT(data);
	}
	{ // Normal case: use free list
		ASSERT(!reset());
		int *data = alloc_new(sizeof(int));
		ASSERT(!g_free_ptr_tails[FREE_PTR_INDEX(sizeof(int))]);
		ASSERT(!ptr_free(data));
		ASSERT(g_free_ptr_tails[FREE_PTR_INDEX(sizeof(int))]);
		int *data2 = alloc_new(sizeof(int));
		ASSERT(data2);
		ASSERT(!g_free_ptr_tails[FREE_PTR_INDEX(sizeof(int))]);
	} 
	{ // Normal case: use arena
		ASSERT(!reset());
		int *data = alloc_new(sizeof(int));
		ASSERT(data);
		ASSERT(g_arena_tail->ptrs_tail == PTR(data));
	}
	{ // Normal case: use mmap
		ASSERT(!reset());
		typedef struct obj {
			char buff[ARENA_SIZE * 10];
		} obj_t;
		obj_t *obj = alloc_new(sizeof(obj_t));
		ASSERT(obj);
		ASSERT(munmap(PTR(obj), TOTAL_SIZE(ARENA_SIZE * 10)) != -1);
	}
	{ // size is 0
		ASSERT(!reset());
		ASSERT(!alloc_new(0));
	}
}

void test_alloc_del() {
	{ // Normal case
		ASSERT(!reset());
		void *data = alloc_new(MIN_ALLOC_SIZE);
		alloc_del(data);
		ASSERT(g_free_ptr_tails[FREE_PTR_INDEX(MIN_ALLOC_SIZE)]->data == data);
	}
}

void test_alloc_resize() {
	{ // Normal case
		ASSERT(!reset());
		int *data = alloc_new(sizeof(int));
		*data = 5;
		ASSERT(PTR(data)->size == sizeof(int));
		ASSERT(!alloc_resize((void**)&data, sizeof(int) * 2));
		ASSERT(PTR(data)->size == sizeof(int) * 2);
		ASSERT(*data == 5);
	}
	{ // size is 0
		int x = 5;
		void *ptr = &x;
		ASSERT(alloc_resize(&ptr, 0));
	}
	{ // ptr is NULL
		ASSERT(alloc_resize(NULL, 4));
	}
}
