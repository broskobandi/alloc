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

void test_arena_use() {
	{ // Normal case
		arena_reset();
		size_t size = 5;
		void *data = arena_use(size);
		ASSERT(data);
		ptr_t *ptr = (ptr_t*)((unsigned char*)data - PTR_ALIGNED_SIZE);
		ASSERT(ptr->size == size);
		ASSERT(ptr->valid_magic == VALID_MAGIC);

		size_t size2 = size * 2;
		void *data2 = arena_use(size2);
		ASSERT(data2);
		ptr_t *ptr2 = (ptr_t*)((unsigned char*)data2 - PTR_ALIGNED_SIZE);
		ASSERT(ptr2->size == size2);
		ASSERT(ptr2->valid_magic == VALID_MAGIC);

		ASSERT(ptr2->prev == ptr);
		ASSERT(ptr->next == ptr2);
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
		ASSERT(free_ptr_index(MIN_ALLOC_SIZE) == 0);
		arena_reset();
		ASSERT(free_ptr_index(MIN_ALLOC_SIZE * 2) == 1);
		arena_reset();
		ASSERT(free_ptr_index(MIN_ALLOC_SIZE * 3) == 2);
		arena_reset();
	}
	{ // size  0
		ASSERT(free_ptr_index(0) == (size_t)-1);
	}
}

void test_ptr_free() {
	{ // Normal case
		arena_reset();
		size_t size = 5;
		void *data = arena_use(size);
		ASSERT(!ptr_free(data));
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

