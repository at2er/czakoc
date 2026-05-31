/* Public domain
 *
 * Just a dynamic array. */
#ifndef UTILSH_DARR_H
#define UTILSH_DARR_H
#include <stddef.h>
#include <string.h>

#ifndef darr_realloc
#define darr_realloc realloc
#include <stdlib.h>
#endif

#ifndef darr_free
#define darr_free free
#include <stdlib.h>
#endif

#define darr(TYPE) \
	struct { \
		TYPE *e; \
		int n; \
	}

#define darr_append(DARR, ELEM) \
	do { \
		darr_expand(DARR); \
		darr_last(DARR) = (ELEM); \
	} while (0)

#define darr_expand(DARR) \
	darr_resize((DARR), (DARR)->n + 1);

#define darr_init(DARR) \
	do { \
		(DARR)->n = 0; \
		(DARR)->e = NULL; \
	} while (0)

#define darr_last(DARR) ((DARR)->e[(DARR)->n - 1])

#define darr_reduce(DARR) \
	do { \
		if ((DARR)->n <= 0) \
			break; \
		darr_resize((DARR), (DARR)->n - 1); \
	} while (0)

#define darr_remove(DARR, POS) \
	do { \
		if ((DARR)->n == 1) { \
			darr_free((DARR)->e); \
			(DARR)->e = NULL; \
			(DARR)->n = 0; \
			break; \
		} \
		if ((DARR)->n <= 0) \
			break; \
		memmove((DARR)->e + (POS), \
				(DARR)->e + (POS) + 1, \
				(DARR)->n - (POS)); \
		darr_resize((DARR), (DARR)->n - 1); \
	} while (0)

#define darr_resize(DARR, N) \
	do { \
		if ((DARR)->n == (N)) \
			break; \
		(DARR)->n = (N); \
		(DARR)->e = darr_realloc((DARR)->e, \
				(DARR)->n * sizeof(*(DARR)->e)); \
	} while (0)

#endif
