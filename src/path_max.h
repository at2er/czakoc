/* Public domain
 * In the future, maybe it will provide different platform */
#ifndef UTILSH_PATH_MAX_H
#define UTILSH_PATH_MAX_H
#ifndef PATH_MAX

#if defined(__linux__)
#include <linux/limits.h>
#elif defined(__WIN32__)
#define PATH_MAX 260
#else
#define PATH_MAX 4096
#endif

#endif /* PATH_MAX */
#endif /* UTILSH_PATH_MAX_H */
