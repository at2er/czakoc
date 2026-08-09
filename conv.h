/* Public domain */
#ifndef UTILSH_CONV_H
#define UTILSH_CONV_H
#include <stddef.h>
#include <stdint.h>
#include <string.h>

int64_t utilsh_conv_a2i(const char *str, size_t len);
uint64_t utilsh_conv_a2ui(const char *str, size_t len);

char *utilsh_conv_i2a(char *str, size_t len, int64_t i);

#endif

#ifdef UTILSH_CONV_IMPL
#include <string.h>

int64_t
utilsh_conv_a2i(const char *str, size_t len)
{
	size_t i = 0;
	int64_t result = 0;
	if (!len)
		len = strlen(str);
	if (*str == '-')
		i = 1;
	for (; i < len; i++) {
		result *= 10;
		result += str[i] - '0';
	}
	if (*str == '-')
		return -result;
	return result;
}

uint64_t
utilsh_conv_a2ui(const char *str, size_t len)
{
	uint64_t result = 0;
	if (!len)
		len = strlen(str);
	for (size_t i = 0; i < len; i++) {
		result *= 10;
		result += str[i] - '0';
	}
	return result;
}

char *
utilsh_conv_i2a(char *str, size_t len, int64_t i)
{
	size_t idx;
	/* TODO: negative. 奇异搞笑 itoa */
	memset(str, 0, len);
	for (idx = len - 1; i; idx--) {
		str[idx] = '0' + i % 10;
		i /= 10;
	}
	if (idx)
		memmove(str, str + idx, len - idx);
	return str;
}

#endif /* UTILSH_CONV_IMPL */
