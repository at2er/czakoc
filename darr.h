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

#ifndef DARR_FREE
#define DARR_FREE free
#include <stdlib.h>
#endif

#ifndef DARR_NUM_TYPE
#define DARR_NUM_TYPE int
#endif

#define darr(TYPE) \
	struct { \
		TYPE *e; \
		DARR_NUM_TYPE n; \
	}

#define darr_append(DARR, ELEM) \
	do { \
		darr_expand(DARR); \
		darr_last(DARR) = (ELEM); \
	} while (0)

#define darr_expand(DARR) \
	darr_resize((DARR), (DARR)->n + 1);

#define darr_free(DARR, ELEM_FREE) \
	if ((DARR)->e && (DARR)->n) { \
		for (DARR_NUM_TYPE __darr_free_i__ = 0; \
				__darr_free_i__ < (DARR)->n; __darr_free_i__++) \
			(ELEM_FREE)((DARR)->e[__darr_free_i__]); \
		(DARR)->n = 0; \
		DARR_FREE((DARR)->e); \
	}
#define darr_free_unsafe(DARR) \
	if ((DARR)->e && (DARR)->n) \
		DARR_FREE((DARR)->e); \

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
			DARR_FREE((DARR)->e); \
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
