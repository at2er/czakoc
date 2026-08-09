/* Public domain
 *
 * Just a unsafe container_of macro */
#ifndef UTILSH_CONTAINER_OF_H
#define UTILSH_CONTAINER_OF_H
#include <stddef.h>

#define utilsh_container_of(PTR, STRUCT, MEMBER) \
	(STRUCT*)((char*)(PTR) - offsetof(STRUCT, MEMBER))

#ifdef UTILSH_CONTAINER_OF_STRIP
#define container_of utilsh_container_of
#endif

#endif
