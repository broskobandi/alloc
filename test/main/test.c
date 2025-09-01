#include "test_utils.h"

TEST_INIT;

int main(void) {
	test_arena_expand();

	test_print_results();
	return 0;
}
