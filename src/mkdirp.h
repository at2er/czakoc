/* Public domain
 *
 * Just a 'mkdir -p'. */
#ifndef UTILSH_MKDIRP_H
#define UTILSH_MKDIRP_H
#include <sys/stat.h>

extern int mkdirp(const char *path, mode_t mode);

#endif

#ifdef UTILSH_MKDIRP_IMPL
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

int
mkdirp(const char *path, mode_t mode)
{
	struct stat st;
	char *tmp;

	if (stat(path, &st) == 0) {
		if (S_ISDIR(st.st_mode))
			return 0;
		errno = ENOTDIR;
		return 1;
	}

	tmp = calloc(strlen(path) + 1, sizeof(char));
	strcpy(tmp, path);

	for (char *cur = (tmp[0] == '/' ? &tmp[1] : tmp); *cur; cur++) {
		if (*cur != '/')
			continue;
		*cur = '\0';
		if (mkdir(tmp, mode) < 0 && errno != EEXIST)
			goto err_free_tmp;
		*cur = '/';
	}

	if (mkdir(tmp, mode) < 0 && errno != EEXIST)
		goto err_free_tmp;

	free(tmp);

	return 0;
err_free_tmp:
	free(tmp);
	return 1;
}

#endif
