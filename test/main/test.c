#include "test_utils.h"

TEST_INIT;

int main(void) {
	test_arena_expand();
	test_arena_reset();
	test_arena_del();
	test_roundup();
	test_ptr_aligned_size();
	test_total_size();
	test_arena_use();
	test_free_ptr_index();
	test_ptr_free();
	test_free_ptr_use();
	test_mmap_use();

	test_alloc_new();

	test_print_results();
	return 0;
}
