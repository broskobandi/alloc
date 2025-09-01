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
		size_t size = 5;
		void *data = arena_use(size);
		ASSERT(data);
		ptr_t *ptr = (ptr_t*)((unsigned char*)data - PTR_ALIGNED_SIZE);
		ASSERT(ptr->size == size);
		ASSERT(ptr->state == VALID);

		size_t size2 = size * 2;
		void *data2 = arena_use(size2);
		ASSERT(data2);
		ptr_t *ptr2 = (ptr_t*)((unsigned char*)data2 - PTR_ALIGNED_SIZE);
		ASSERT(ptr2->size == size2);
		ASSERT(ptr2->state == VALID);

		ASSERT(ptr2->prev_valid == ptr);
		ASSERT(ptr->next_valid == ptr2);
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
	{ // size  0
		ASSERT(FREE_PTR_INDEX(0) == (size_t)-1);
	}
}

void test_ptr_free() {
	{ // Normal case
		arena_reset();
		size_t size1 = MIN_ALLOC_SIZE / 2;
		size_t size2 = MIN_ALLOC_SIZE * 2;
		// size_t index1 = FREE_PTR_INDEX(size1);
		// size_t index2 = FREE_PTR_INDEX(size2);
		void *data1 = arena_use(size1);
		// void *data2 = arena_use(size1);
		void *data3 = arena_use(size2);
		void *data4 = arena_use(size2);
		// ptr_t *ptr1 = (ptr_t*)((unsigned char*)data1 - PTR_ALIGNED_SIZE);
		// ptr_t *ptr2 = (ptr_t*)((unsigned char*)data2 - PTR_ALIGNED_SIZE);
		// ptr_t *ptr3 = (ptr_t*)((unsigned char*)data3 - PTR_ALIGNED_SIZE);
		// ptr_t *ptr4 = (ptr_t*)((unsigned char*)data4 - PTR_ALIGNED_SIZE);
		ASSERT(!ptr_free(data1));
		// ASSERT(!ptr_free(data2));
		ASSERT(!ptr_free(data3));
		ASSERT(!ptr_free(data4));
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

