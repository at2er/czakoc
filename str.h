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
 *     - #define UTILSH_STR_REMOVE_MIN <unsigned int> (impl)
 *        default: 128
 */
#ifndef UTILSH_STR_H
#define UTILSH_STR_H
#include <stddef.h>

#define STR(CSTR, LEN) (struct str){(CSTR), (LEN), (LEN) + 1}
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
extern struct str *str_append_chr(struct str *dst, char c);

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

/* Copy [src] to [s].
 *
 * @return: [s], if it succeeds, otherwise NULL. */
extern struct str *str_from_str(struct str *s, struct str *src);

/* Insert [c] to the content of [dst] before [pos].
 *
 * @return: [dst], if it succeeds, otherwise NULL. */
extern struct str *str_insert_chr(struct str *dst, size_t pos, char c);

/* Insert [cstr] to the content of [dst] before [pos].
 * For example (not really code):
 *     s = "co"
 *          ^^
 *          01
 *     str_insert_cstr(s, 1, "iall");
 *     s = "ciallo"
 *          ^^^^^^
 *          012345
 *
 * @return: [dst], if it succeeds, otherwise NULL. */
extern struct str *str_insert_cstr(struct str *dst, size_t pos, const char *cstr);

/* Like str_insert_cstr(), but insert with [str]
 * to the content of [dst] before [pos].
 *
 * @return: [dst], if it succeeds, otherwise NULL. */
extern struct str *str_insert_str(struct str *dst, size_t pos, struct str *str);

/* Reallocate 's->s' and set 's->siz', if the [siz] > s->siz.
 * So if the [siz] is small than s->s, it won't be reallocated.
 *
 * @return: [s], if it succeeds, otherwise NULL. */
extern struct str *str_realloc(struct str *s, size_t siz);

/* Remove [len] from 's->s[pos]' (include 's->s[pos]').
 *
 * @return: [s], if it succeeds, otherwise NULL. */
extern struct str *str_remove(struct str *s, size_t pos, size_t len);

/* Like default str functions, but always call exit(1) when failed,
 * and their return value is always not NULL.
 * If you don't need this make your program fat,
 * just define this macro. */
#ifndef UTILSH_DISABLE_ESTR

#ifndef UTILSH_STR_DIE
#define UTILSH_STR_DIE abort()
#endif

/* @return: [s] */
extern struct str *estr_append_chr(struct str *dst, char c);

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

/* Copy [src] to [s].
 *
 * @return: [s] */
extern struct str *estr_from_str(struct str *s, struct str *src);

/* @return: [dst] */
extern struct str *estr_insert_chr(struct str *dst, size_t pos, char c);

/* @return: [dst] */
extern struct str *estr_insert_cstr(struct str *dst, size_t pos, const char *cstr);

/* @return: [dst] */
extern struct str *estr_insert_str(struct str *dst, size_t pos, struct str *str);

/* Reallocate 's->s' and set 's->siz', if the [siz] > s->siz.
 * So if the [siz] is small than s->s, it won't be reallocated.
 *
 * @return: [s] */
extern struct str *estr_realloc(struct str *s, size_t siz);

/* @return: [s] */
extern struct str *estr_remove(struct str *s, size_t pos, size_t len);

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

#ifndef UTILSH_STR_REMOVE_MIN
#define UTILSH_STR_REMOVE_MIN 128
#endif

/* @return: [s], if it succeeds, otherwise NULL. */
struct str *
str_append_chr(struct str *dst, char c)
{
	if (!dst)
		return NULL;
	if (!str_realloc(dst, dst->len + 2))
		return NULL;
	dst->s[dst->len] = c;
	dst->len += 1;
	dst->s[dst->len] = '\0';
	return dst;
}

struct str *
str_append_cstr(struct str *dst, const char *cstr)
{
	struct str fake;
	if (!cstr)
		return NULL;
	fake.s = (char*)cstr;
	fake.len = strlen(cstr);
	fake.siz = fake.len + 1;
	return str_append_str(dst, &fake);
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
	if (s->s)
		free(s->s);
	str_empty(s);
}

struct str *
str_from_cstr(struct str *s, const char *cstr)
{
	struct str fake;
	if (!s || !cstr)
		return NULL;
	fake.s = (char*)cstr;
	fake.siz = strlen(cstr) + 1;
	fake.len = fake.siz - 1;
	return str_from_str(s, &fake);
}

struct str *
str_from_str(struct str *s, struct str *src)
{
	if (!s || !src)
		return NULL;
	s->siz = src->len + 1;
	s->s = malloc(s->siz);
	s->len = src->len;
	strncpy(s->s, src->s, s->len);
	s->s[s->len] = '\0';
	return s;
}

struct str *
str_insert_chr(struct str *dst, size_t pos, char c)
{
	char s[2];
	struct str fake;
	s[0] = c;
	s[1] = '\0';
	fake.s = s;
	fake.len = 1;
	fake.siz = 2;
	return str_insert_str(dst, pos, &fake);
}

struct str *
str_insert_cstr(struct str *dst, size_t pos, const char *cstr)
{
	struct str fake;
	if (!cstr)
		return NULL;
	fake.s = (char*)cstr;
	fake.len = strlen(cstr);
	fake.siz = fake.len + 1;
	return str_insert_str(dst, pos, &fake);
}

struct str *
str_insert_str(struct str *dst, size_t pos, struct str *str)
{
	if (!dst || !str)
		return NULL;
	if (str->len == 0)
		return dst;
	if (pos > dst->len)
		return NULL;
	if (!str_realloc(dst, dst->len + str->len + 1))
		return NULL;
	memmove(&dst->s[pos + str->len], &dst->s[pos], dst->len - pos);
	memcpy(&dst->s[pos], str->s, str->len);
	dst->len += str->len;
	dst->s[dst->len] = '\0';
	return dst;
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

struct str *
str_remove(struct str *s, size_t pos, size_t len)
{
	/* TODO: UTILSH_STR_REMOVE_MIN */
	if (!s)
		return NULL;
	if (len == 0)
		return s;
	if (pos > s->len)
		return NULL;
	s->len -= len;
	/* helloworld
	   0123456789
	      ^^^
	   hel   orld
	   helorld
	   0123456 */
	memmove(&s->s[pos], &s->s[pos + len], s->len - pos);
	s->s[s->len] = '\0';
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
estr_append_chr(struct str *dst, char c)
	T(str_append_chr, dst, c)

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
estr_from_str(struct str *s, struct str *src)
	T(str_from_str, s, src)

struct str *
estr_insert_chr(struct str *dst, size_t pos, char c)
	T(str_insert_chr, dst, pos, c)

struct str *
estr_insert_cstr(struct str *dst, size_t pos, const char *cstr)
	T(str_insert_cstr, dst, pos, cstr)

struct str *
estr_insert_str(struct str *dst, size_t pos, struct str *str)
	T(str_insert_str, dst, pos, str)

struct str *
estr_realloc(struct str *s, size_t siz)
	T(str_realloc, s, siz)

struct str *
estr_remove(struct str *s, size_t pos, size_t len)
	T(str_remove, s, pos, len)

#undef T

#endif /* UTILSH_DISABLE_ESTR */

#ifdef _UTILSH_STR_DEFAULT_SOURCE_NOT_DEFINED
#undef _DEFAULT_SOURCE /* this macro maybe break user's '#if'? */
#endif

#endif /* UTILSH_STR_IMPL */
