/* Include the header. */
#include <alloc.h>

int main(void) {
	/* Allocate a memory block of a desired size. */
	void *ptr = alloc_new(1024);

	/* As always, check if allocation was successful. */
	if (!ptr) return 1;

	/* Expand the memory if needed. */
	if (alloc_resize(&ptr, 2048)) return 1;

	/* Or shrink it. */
	if (alloc_resize(&ptr, 512)) return 1;

	/* Don't forget to free the memory when no longer needed. */
	alloc_del(ptr);

	/* And as always, don't forget to set the pointer to NULL. */
	ptr = NULL;

	return 0;
}
