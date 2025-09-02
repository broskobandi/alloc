# alloc
Memory allocator written in C.

## Features
- Thread safety.
- Global static buffer.
- Free list.
- Resizability.

## Installation
```bash
git clone git@github.com:broskobandi/alloc.git &&
cd alloc &&
make &&
sudo make install
```

## Uninstallation
```bash
cd alloc &&
sudo make uninstall
```

## Usage
```c
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
```
Use -L/usr/local/bin -lalloc as compiler flags.

## Documentation
```bash
cd alloc &&
make doc
```
Then open index.html generated in doc/html.

## Testing
```bash
cd alloc &&
make clean &&
make test &&
make clean
```
