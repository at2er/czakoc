#include <stdint.h>

struct List__i32 {
	int32_t data;
	struct List__i32 *next;
	struct List__i32 *prev;
};

struct ListIterator__i32 {
};

int
main()
{
	struct List__i32 int_list = {0};

	List__i32__append(&int_list, 123);
	List__i32__append(&int_list, 456);

	struct ListIterator__i32 int_list_iter = List__i32__iter(&int_list);
	for (struct List__i32 *i = NULL; ListIterator__i32__next(&int_list_iter);) {
		// ...
	}

	return 0;
}
