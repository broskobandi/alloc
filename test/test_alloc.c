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

	}
	{ // arena NULL

	}
	{ // arena is head

	}
}
