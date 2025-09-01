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
		ASSERT(roundup(size) == alignof(max_align_t));
	}
	{ // size 0
		ASSERT(roundup(0) == (size_t)-1);
	}
}

void test_ptr_aligned_size() {
	{ // Normal case
		ASSERT(roundup(sizeof(ptr_t)) == PTR_ALIGNED_SIZE);
	}
	{ // size 0
		ASSERT(roundup(0) == (size_t)-1);
	}
}

void test_total_size() {
	{ // Normal case
		size_t size = alignof(max_align_t) / 2;
		size_t exp = roundup(size) + PTR_ALIGNED_SIZE;
		ASSERT(total_size(size) == exp);
	}
	{ // Normal case
		ASSERT(total_size(0) == (size_t)-1);
	}
}

void test_use_arena() {
	{ // Normal case
		size_t size = 5;
		void *data = use_arena(size);
		ASSERT(data);
		ptr_t *ptr = (ptr_t*)((unsigned char*)data - PTR_ALIGNED_SIZE);
		ASSERT(ptr->size == size);
		ASSERT(ptr->is_valid);

		size_t size2 = size * 2;
		void *data2 = use_arena(size2);
		ASSERT(data2);
		ptr_t *ptr2 = (ptr_t*)((unsigned char*)data2 - PTR_ALIGNED_SIZE);
		ASSERT(ptr2->size == size2);
		ASSERT(ptr2->is_valid);

		ASSERT(ptr2->prev == ptr);
		ASSERT(ptr->next == ptr2);
	}
}
