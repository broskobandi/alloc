#include "test_utils.h"

TEST_INIT;

int main(void) {
	test_arena_expand();
	test_arena_reset();
	test_arena_del();
	test_roundup();
	test_ptr_aligned_size();

	test_print_results();
	return 0;
}
