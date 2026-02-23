/* Public domain
 *
 * Just a string.
 *
 * Usage: see function declarations.
 *     - #define UTILSH_DISABLE_ESTR
 *               Don't generate auto error handle functions.
 *
 *     - #define UTILSH_STR_DIE <your error handle function call>
 *        default: abort()
 *
 */
#ifndef UTILSH_STR_H
#define UTILSH_STR_H
#include <stddef.h>

/* Use this format macro for printf or else string format function
 * with STR_ARG() */
#define STR_FMT "%.*s"
#define STR_ARG(S)  (int)((S).len),  (S).s
#define STR_ARGR(S) (int)((S)->len), (S)->s

/* Allocate it with your favourite way.
 * If you allocated it with malloc() or else,
 * you must free it by yourself. */
struct str {
	char *s;
	size_t len, siz;
};

/* @return: [s], if it succeeds, otherwise NULL. */
extern struct str *str_append_cstr(struct str *dst, const char *cstr);

/* @return: [s], if it succeeds, otherwise NULL. */
extern struct str *str_append_str(struct str *dst, const struct str *src);

/* Set '\0' at 's->s[0]' and set 's->len' to 0.
 * Like str_empty() but won't reset 's->s' to NULL.
 * You can use [s] again after calling this str_clean().
 *
 * @return: [s], if it succeeds, otherwise NULL. */
extern struct str *str_clean(struct str *s);

/* Reset the member of [s] by zero, but won't free allocated 's->s'.
 *
 * @return: [s], if it succeeds, otherwise NULL. */
extern struct str *str_empty(struct str *s);

/* Expand 's->s' with str_realloc().
 *
 * @return: [s], if it succeeds, otherwise NULL. */
extern struct str *str_expand_siz(struct str *s, size_t more);

/* Free content inside [s], but won't free itself. */
extern void str_free(struct str *s);

/* Copy [cstr] to the content of [s].
 *
 * @return: [s], if it succeeds, otherwise NULL. */
extern struct str *str_from_cstr(struct str *s, const char *cstr);

/* Reallocate 's->s' and set 's->siz', if the [siz] > s->siz.
 * So if the [siz] is small than s->s, it won't be reallocated.
 *
 * @return: [s], if it succeeds, otherwise NULL. */
extern struct str *str_realloc(struct str *s, size_t siz);

/* Like default str functions, but always call exit(1) when failed,
 * and their return value is always not NULL.
 * If you don't need this make your program fat,
 * just define this macro. */
#ifndef UTILSH_DISABLE_ESTR

#ifndef UTILSH_STR_DIE
#define UTILSH_STR_DIE abort()
#endif

/* @return: [s] */
extern struct str *estr_append_cstr(struct str *dst, const char *cstr);

/* @return: [s] */
extern struct str *estr_append_str(struct str *dst, const struct str *src);

/* Set '\0' at 's->s[0]' and set 's->len' to 0.
 * Like str_empty() but won't reset 's->s' to NULL.
 * You can use [s] again after calling this str_clean().
 *
 * @return: [s] */
extern struct str *estr_clean(struct str *s);

/* Reset the member of [s] by zero, but won't free allocated 's->s'.
 *
 * @return: [s] */
extern struct str *estr_empty(struct str *s);

/* Expand 's->s' with str_realloc().
 *
 * @return: [s] */
extern struct str *estr_expand_siz(struct str *s, size_t more);

/* Copy [cstr] to the content of [s].
 *
 * @return: [s] */
extern struct str *estr_from_cstr(struct str *s, const char *cstr);

/* Reallocate 's->s' and set 's->siz', if the [siz] > s->siz.
 * So if the [siz] is small than s->s, it won't be reallocated.
 *
 * @return: [s] */
extern struct str *estr_realloc(struct str *s, size_t siz);

#endif /* UTILSH_DISABLE_ESTR */

#endif /* UTILSH_STR_H */

#ifdef UTILSH_STR_IMPL

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#define _UTILSH_STR_DEFAULT_SOURCE_NOT_DEFINED
#endif

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

struct str *
str_append_cstr(struct str *dst, const char *cstr)
{
	size_t cstr_len;
	if (!dst || !cstr)
		return NULL;
	cstr_len = strlen(cstr);
	if (!str_realloc(dst, dst->len + cstr_len + 1))
		return NULL;
	memcpy(&dst->s[dst->len], cstr, cstr_len);
	dst->len += cstr_len;
	dst->s[dst->len] = '\0';
	return dst;
}

struct str *
str_append_str(struct str *dst, const struct str *s1)
{
	if (!dst || !s1)
		return NULL;
	if (!str_realloc(dst, dst->len + s1->len + 1))
		return NULL;
	memcpy(&dst->s[dst->len], s1->s, s1->len);
	dst->len += s1->len;
	dst->s[dst->len] = '\0';
	return dst;
}

struct str *
str_clean(struct str *s)
{
	if (!s)
		return NULL;
	s->len = 0;
	if (s->siz > 0)
		s->s[0] = '\0';
	return s;
}

struct str *
str_empty(struct str *s)
{
	if (!s)
		return NULL;
	s->s = NULL;
	s->siz = s->len = 0;
	return s;
}

struct str *
str_expand_siz(struct str *s, size_t more)
{
	if (!s)
		return NULL;
	if (!str_realloc(s, s->len + more + 1))
		return NULL;
	return s;
}

void
str_free(struct str *s)
{
	free(s->s);
	str_empty(s);
}

struct str *
str_from_cstr(struct str *s, const char *cstr)
{
	if (!s || !cstr)
		return NULL;
	s->siz = strlen(cstr) + 1;
	s->s   = malloc(s->siz);
	s->len = s->siz - 1;
	strcpy(s->s, cstr);
	return s;
}

struct str *
str_realloc(struct str *s, size_t siz)
{
	if (!s)
		return NULL;
	if (s->siz >= siz)
		return s;
	s->s = realloc(s->s, siz);
	s->siz = siz;
	return s;
}

#ifndef UTILSH_DISABLE_ESTR
#include <stdio.h>

#define T(DEST_FN_NAME, ...) \
	{ \
		struct str *ret = DEST_FN_NAME (__VA_ARGS__); \
		if (!ret) { \
			perror("[str.h]: " #DEST_FN_NAME "()"); \
			UTILSH_STR_DIE; \
		} \
		return ret; \
	}

struct str *
estr_append_cstr(struct str *dst, const char *cstr)
	T(str_append_cstr, dst, cstr)

struct str *
estr_append_str(struct str *dst, const struct str *src)
	T(str_append_str, dst, src)

struct str *
estr_clean(struct str *s)
	T(str_clean, s)

struct str *
estr_empty(struct str *s)
	T(str_empty, s)

struct str *
estr_expand_siz(struct str *s, size_t more)
	T(str_expand_siz, s, more)

struct str *
estr_from_cstr(struct str *s, const char *cstr)
	T(str_from_cstr, s, cstr)

struct str *
estr_realloc(struct str *s, size_t siz)
	T(str_realloc, s, siz)

#undef T

#endif /* UTILSH_DISABLE_ESTR */

#ifdef _UTILSH_STR_DEFAULT_SOURCE_NOT_DEFINED
#undef _DEFAULT_SOURCE /* this macro maybe break user's '#if'? */
#endif

#endif /* UTILSH_STR_IMPL */
