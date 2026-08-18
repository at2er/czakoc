/* Simple lexer in C
 *
 * Usage:
 *     * Setup a 'struct sclexer' and pass it to 'sclexer_init',
 *       and the 'fpath' argument of 'sclexer_init' can be NULL.
 *
 */
#ifndef SCLEXER_H
#define SCLEXER_H
#include <stddef.h>
#include <stdio.h>

struct sclexer;
struct sclexer {
	int eol_tok,
	    ident_tok,
	    int_tok,
	    string_tok;

	/* Ensure keywords and puncts are continuous. It will like this:
	 * {TK_KW_0, TK_KW_1, TK_KW_COUNT, TK_PT_0, TK_PT_1, TK_PT_COUNT},
	 *
	 * sclexer_init() will set [punct_count] to TK_PT_COUNT - TK_KW_COUNT */
	int keyword_count, punct_count;

	int (*ident_reader)(struct sclexer *lexer);
	int (*int_reader)(struct sclexer *lexer);
	int (*string_reader)(struct sclexer *lexer);

	const char **tokens;
	const char **comments;

	const char *path;
	int row, col;

	const char *pos;
	const char *buf;
};

struct sclexer_tok {
	const char *path;
	const char *str;
	int row, col, len;
	int type;
};

/* always match the longest one */
int sclexer_has_match(const char **matches, int count,
		const char *buf, int blen,
		int *idx);
int sclexer_ident_reader(struct sclexer *lexer);
int sclexer_int_reader(struct sclexer *lexer);
int sclexer_init(struct sclexer *lexer, const char *buf);
int sclexer_nmatches(const char **matches);
int sclexer_next(struct sclexer *lexer, struct sclexer_tok *tok);
size_t sclexer_read_file(FILE *fp, char **buf);
int sclexer_string_reader(struct sclexer *lexer);
int sclexer_skip_space(struct sclexer *lexer, struct sclexer_tok *tok);

#endif /* SCLEXER_H */

#ifdef SCLEXER_IMPL
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
sclexer_has_match(const char **matches, int count,
		const char *buf, int blen,
		int *idx)
{
	int len, prv = 0;
	for (int i = 0; matches[i]; i++) {
		if (count && i >= count)
			break;
		len = strlen(matches[i]);
		if (blen && blen != len)
			continue;
		if (strncmp(matches[i], buf, len) == 0) {
			if (len < prv)
				continue;
			if (idx)
				*idx = i;
			prv = len;
		}
	}
	return prv;
}

int
sclexer_ident_reader(struct sclexer *lexer)
{
	const char *p = lexer->pos;
	if (!isalpha(*p) && *p != '_')
		return 0;
	p++;
	for (; *p && (isalpha(*p) || isdigit(*p) || *p == '_'); p++);
	return p - lexer->pos;
}

int
sclexer_int_reader(struct sclexer *lexer)
{
	const char *p = lexer->pos;
	if (!isdigit(*p))
		return 0;
	p++;
	for (; *p && isdigit(*p); p++);
	return p - lexer->pos;
}

int
sclexer_init(struct sclexer *lexer, const char *buf)
{
	if (!lexer->ident_reader)
		lexer->ident_reader = sclexer_ident_reader;
	if (!lexer->int_reader)
		lexer->int_reader = sclexer_int_reader;
	if (!lexer->string_reader)
		lexer->string_reader = sclexer_string_reader;
	if (!lexer->keyword_count)
		return 1;
	if (lexer->punct_count)
		lexer->punct_count -= lexer->keyword_count + 1;
	lexer->buf = lexer->pos = buf;
	return 0;
}

int
sclexer_next(struct sclexer *lexer, struct sclexer_tok *tok)
{
	const char *orig = lexer->pos;
	int ret;
	tok->path = lexer->path;

	while ((ret = sclexer_skip_space(lexer, tok)) > 0);
	if (ret < 0) /* return special tokens like EOL */
		goto end;
	tok->str = lexer->pos;
	tok->row = lexer->row;
	tok->col = lexer->col;

	if (!*lexer->pos)
		return 0;

	if (lexer->punct_count &&
	    (tok->len = sclexer_has_match(lexer->tokens + lexer->keyword_count + 1,
			lexer->punct_count,
			lexer->pos, 0, &tok->type))) {
		tok->type += lexer->keyword_count + 1;
		goto end;
	}

	if (lexer->int_tok != -1 &&
	    (tok->len = lexer->int_reader(lexer))) {
		tok->type = lexer->int_tok;
		goto end;
	}

	if (lexer->ident_tok != -1 &&
	    (tok->len = lexer->ident_reader(lexer))) {
		if ((ret = sclexer_has_match(lexer->tokens, lexer->keyword_count,
				tok->str, tok->len,
				&tok->type))) {
			tok->len = ret;
			goto end;
		}

		tok->type = lexer->ident_tok;
		goto end;
	}

	if (lexer->string_tok != -1 &&
	    (tok->len = lexer->string_reader(lexer))) {
		tok->type = lexer->string_tok;
		goto end;
	}

	return -1;
end:
	lexer->col += tok->len;
	lexer->pos += tok->len;
	return lexer->pos - orig;
}

int
sclexer_nmatches(const char **matches)
{
	int i = 0;
	for (; matches[i]; i++);
	return i;
}

size_t
sclexer_read_file(FILE *fp, char **buf)
{
	size_t len = 0, ret, siz = BUFSIZ;
	char *res = malloc(siz);

	while ((ret = fread(res + len, siz, 1, fp))) {
		len += ret;
		if ((len += ret) >= siz) {
			siz += BUFSIZ;
			res = realloc(res, siz);
		}
	}

	*buf = res;
	return len;
}

int
sclexer_string_reader(struct sclexer *lexer)
{
	int after_back_slash = 0;
	const char *p = lexer->pos + 1;

again:
	if (*p == '"' && !after_back_slash)
		goto end;
	if (after_back_slash)
		after_back_slash = 0;
	if (*p == '\\')
		after_back_slash = 1;
	p++;
	goto again;
end:
	return p - lexer->pos + 1;
}

int
sclexer_skip_space(struct sclexer *lexer, struct sclexer_tok *tok)
{
	int len = 0;
	const char *p = lexer->pos;
	if (*p == '\n') {
		lexer->row++;
		lexer->col = 0;
		if (lexer->eol_tok != -1) {
			tok->len = 1;
			tok->str = lexer->pos;
			tok->type = lexer->eol_tok;
			return -1;
		} else {
			lexer->pos = p + 1;
		}
		return 1;
	} else if ((len = sclexer_has_match(lexer->comments, 0, p, 0, NULL))) {
		for (p += len; *p && *p != '\n'; p++)
			len++;
	} else {
		for (; *p && (*p == '\t' || *p == ' '); p++)
			len++;
		if (!len)
			return 0;
	}
	lexer->col += len;
	lexer->pos = p;
	return 1;
}

#endif /* SCLEXER_IMPL */

/* License
MIT License

Copyright (c) 2025 at2er <xb0515@outlook.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE. */
