/* Public domain */
#ifndef UTILSH_CONV_H
#define UTILSH_CONV_H
#include <stddef.h>
#include <stdint.h>
#include <string.h>

uint64_t utilsh_conv_a2ui(const char *str, size_t len);

#endif

#ifdef UTILSH_CONV_IMPL

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

#endif /* UTILSH_CONV_IMPL */
