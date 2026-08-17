#include <stdint.h>

struct Generic__int32_t {
	int32_t a;
	int32_t b;
};
static int32_t Generic__ret__int32_t(struct Generic__int32_t* self);

int32_t main(int32_t argc);
int32_t
Generic__ret__int32_t(struct Generic__int32_t* self)
{
	return self->a;
}

int32_t
main(int32_t argc)
{
	struct Generic__int32_t g0 = {.a = 123, .b = 456};
	return Generic__ret__int32_t(&g0);
}
