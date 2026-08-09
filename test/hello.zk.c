#include <stdint.h>

struct A {
	int32_t a;
	int32_t b;
};
int32_t main(int32_t argc);
int32_t
main(int32_t argc)
{
	struct A s = {.a = 1, .b = 2};
	return 0;
}
