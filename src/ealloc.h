/* Public domain
 *
 * Just some memory allocation function with error handling.
 * If you need different error handling for each call,
 * you should use Xalloc() from stdlib.h, not this.
 *
 * #define UTILSH_EALLOC_DIE <your error handle function call>
 *  default: abort()
 */
#ifndef UTILSH_EALLOC_H
#define UTILSH_EALLOC_H
#include <stddef.h>

#ifndef UTILSH_EALLOC_DIE
#define UTILSH_EALLOC_DIE abort()
#endif

extern void *ecalloc(size_t n, size_t siz);

#endif /* UTILSH_EALLOC_H */

#ifdef UTILSH_EALLOC_IMPL
#include <stdio.h>
#include <stdlib.h>

void *
ecalloc(size_t n, size_t siz)
{
	void *p;
	if ((p = calloc(n, siz)) == NULL) {
		perror("[ealloc.h]: calloc()");
		UTILSH_EALLOC_DIE;
		return NULL;
	}
	return p;
}
#endif /* UTILSH_EALLOC_IMPL */
