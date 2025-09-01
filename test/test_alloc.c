#include "test_utils.h"
#include "alloc_utils.h"

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
		ASSERT(!arena_reset());
		ASSERT(g_arena_tail == &g_arena_head);
		ASSERT(!g_arena_head.next);
	}
}

void test_arena_del() {
	{ // Normal case
		ASSERT(!arena_reset());
		ASSERT(!arena_expand());
		ASSERT(!arena_expand());
		ASSERT(!arena_del(g_arena_head.next));
		ASSERT(g_arena_head.next == g_arena_tail);
		ASSERT(g_arena_tail->prev == &g_arena_head);
	}
	{ // arena NULL
		ASSERT(!arena_reset());
		ASSERT(arena_del(NULL));
	}
	{ // arena is head
		ASSERT(!arena_reset());
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
		arena_reset();
		size_t size1 = MIN_ALLOC_SIZE / 2;
		size_t size2 = MIN_ALLOC_SIZE * 2;
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
	{ // size 0
		arena_reset();
		ASSERT(!arena_use(0));
	}
	{ // size too big
		arena_reset();
		ASSERT(!arena_use(ARENA_SIZE * 2));
	}
}

void test_free_ptr_index() {
	{ // Normal case
		arena_reset();
		ASSERT(FREE_PTR_INDEX(MIN_ALLOC_SIZE) == 0);
		arena_reset();
		ASSERT(FREE_PTR_INDEX(MIN_ALLOC_SIZE * 2) == 1);
		arena_reset();
		ASSERT(FREE_PTR_INDEX(MIN_ALLOC_SIZE * 3) == 2);
		arena_reset();
	}
}

void test_ptr_free() {
	{ // Normal case
		arena_reset();
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
	}
	{ // Data NULL
		arena_reset();
		ASSERT(ptr_free(NULL));
	}
	{ // Invalid argument
		arena_reset();
		int x = 5;
		void *dummy = &x;
		ASSERT(ptr_free(dummy));
	}
}

