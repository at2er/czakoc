/* Public domain
 *
 * Just a dynamic array.
 *
 * Usage:
 *     // 'T' is your the type of element of the dynamic array.
 *     darr_append(T self, size_t count, T elem);
 *     darr_expand(T self, size_t count);
 *
 *     'darr_append' and 'darr_expand' is similar and will both use 'realloc' to
 *     expand the dynamic array.
 */
#ifndef UTILSH_DARR_H
#define UTILSH_DARR_H
#include <stdlib.h>

#define darr_append(SELF, COUNT, ELEM) \
	do { \
		darr_expand((SELF), (COUNT)); \
		(SELF)[(COUNT) - 1] = (ELEM); \
	} while (0)

#define darr_expand(SELF, COUNT) \
	do { \
		(COUNT)++; \
		(SELF) = realloc((SELF), (COUNT) * sizeof(*(SELF))); \
	} while (0)

#endif
