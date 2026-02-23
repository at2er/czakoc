/* Public domain
 *
 * Just a implmentation of 'die' function.
 *
 * Usage:
 *     void die(const char *fmt, ...);
 *         Just like 'printf' but it will call 'exit(1)' to end your program.
 *
 *     - #define UTILSH_DIE <function call>
 *        Define it in implement file.
 *        default: exit(1)
 */
#ifndef UTILSH_DIE_H
#define UTILSH_DIE_H

extern void die(const char *fmt, ...);

#endif

#ifdef UTILSH_DIE_IMPL

#ifndef UTILSH_DIE
#define UTILSH_DIE exit(1)
#endif

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void
die(const char *msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	vfprintf(stderr, msg, ap);
	va_end(ap);

	UTILSH_DIE;
}
#endif /* UTILSH_DIE_IMPL */
