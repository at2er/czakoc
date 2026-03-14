/* Public domain
 * In the future, maybe it will provide different platform */
#ifndef UTILSH_PATH_MAX_H
#define UTILSH_PATH_MAX_H
#ifndef PATH_MAX

#if defined(__WIN32__)
#define PATH_MAX 260
#else
#include <limits.h>
#endif

#endif /* PATH_MAX */
#endif /* UTILSH_PATH_MAX_H */
