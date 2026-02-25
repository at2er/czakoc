/* Simple lexer in C
 *
 * Usage:
 *     * Setup a 'struct sclexer' and pass it to 'sclexer_init',
 *       and the 'fpath' argument of 'sclexer_init' can be NULL.
 *     * Give a file content to 'src' and 'src_siz' of 'struct sclexer',
 *       If you want to read a file and get the content of it,
 *       use 'sclexer_read_file'.
 *     * Parse the 'src' by 'sclexer_get_tok' or just use 'sclexer_get_tokens'
 *       to parse all content of 'src'.
 *
 * Options:
 *     SCLEXER_DISABLE_MSG_COLOR:    disable message color.
 *     SCLEXER_DISABLE_MORE_LOC_MSG: disable location information of message.
 *
 * MIT License
 *
 * Copyright (c) 2025 at2er <xb0515@outlook.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. */
#ifndef SCLEXER_H
#define SCLEXER_H
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

enum SCLEXER_TOK_KIND {
	SCLEXER_UNKNOWN_TOK,
	SCLEXER_EOF,
	SCLEXER_EOL,

	SCLEXER_IDENT,
	SCLEXER_INT,
	SCLEXER_INT_NEG,
	SCLEXER_KEYWORD,
	SCLEXER_STRING,
	SCLEXER_SYMBOL,

	SCLEXER_INDENT_BLOCK_BEGIN,
	SCLEXER_INDENT_BLOCK_END,

	SCLEXER_TOK_KIND_COUNT
};

struct sclexer_str_slice {
	const char *begin;
	size_t len;
};

struct sclexer_loc {
	const char *fpath;
	size_t line, column;
};

union sclexer_tok_data {
	int64_t  sint;
	uint64_t uint;

	size_t keyword;
	struct sclexer_str_slice str;
	size_t symbol;

	/* it shouldn't be used by user */
	void *_v;
};

struct sclexer_tok {
	union sclexer_tok_data data;
	enum SCLEXER_TOK_KIND kind;

	struct sclexer_str_slice src;
	struct sclexer_loc loc;
};

struct sclexer {
	bool enable_indent;
	bool (*is_ident)(char c, bool begin);

	/* Single line comments, such as ";" and "//",
	 * so I think you will know what means of it.
	 * (It won't be setup by 'sclexer_init')
	 */
	const char **comments;
	size_t comments_count;

	/* Like "~!@#$%^&*()-_+=" or setup else by yourself?
	 * (It won't be setup by 'sclexer_init')
	 */
	const char **symbols;
	size_t symbols_count;

	/* Like "enum", "struct", "define",
	 * or just call it "special ientifier"
	 * (It won't be setup by 'sclexer_init')
	 */
	const char **keywords;
	size_t keywords_count;

	/* Preparing for parsing string */
	const char *src;
	size_t src_siz;

	bool _after_endl;
	const char *_cur;
	size_t _last_indent;
	struct sclexer_loc _loc;
};

bool sclexer_default_is_ident(char c, bool begin);

/**
 * Just copy the content of 'dst' to 'src' without any memory allocate.
 * So, 'src' and 'dst' must be a correct memory.
 */
void sclexer_dup_tok(struct sclexer_tok *dst, struct sclexer_tok *src);

/**
 * @return: true when 'src' not end, otherwise false.
 */
bool sclexer_get_tok(struct sclexer *self,
		struct sclexer_tok *tok);

/**
 * Before calling this function, setup all options without '_xxx' in 'lexer'.
 * If you need read a file, use 'sclexer_read_file'
 * and pass the result to this function.
 *
 * @param self: initialized 'struct sclexer'
 * @param fpath: NULL | file path
 */
void sclexer_init(struct sclexer *self, const char *fpath);

const char *sclexer_kind_names(enum SCLEXER_TOK_KIND kind);

size_t sclexer_get_tokens(struct sclexer *self, struct sclexer_tok **result);

void sclexer_print_tok(struct sclexer *self, struct sclexer_tok *tok);

/**
 * Read file context from 'fpath' to 'result' and returns length of it.
 *
 * @param result: store the file content, you need free by yourself.
 * @return: length of 'result'.
 */
size_t sclexer_read_file(char **result, const char *fpath);

#endif

#ifdef SCLEXER_IMPL
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _SCLEXER_DEFAULT_TOKENS_CAPACITY 64
#define _SCLEXER_ERR_FMT "libsclexer: %s:%s:%d: "
#define _SCLEXER_ERR_FMT_ARG __FILE__, __func__, __LINE__

#ifndef SCLEXER_DISABLE_MSG_COLOR
#define _SCLEXER_MSG_COLOR(MSG, COLOR) COLOR MSG _SCLEXER_MSG_COLOR_RESET
#define _SCLEXER_MSG_COLOR_LOC(MSG) _SCLEXER_MSG_COLOR(MSG, "\x1b[31m")
#define _SCLEXER_MSG_COLOR_RESET "\x1b[0m"
#else
#define _SCLEXER_MSG_COLOR(MSG, ...) MSG
#define _SCLEXER_MSG_COLOR_LOC(MSG) MSG
#define _SCLEXER_MSG_COLOR_RESET
#endif // SCLEXER_DISABLE_MSG_COLOR

#ifndef SCLEXER_DISABLE_MORE_LOC_MSG
#define _SCLEXER_TOK_LOC_FMT _SCLEXER_MSG_COLOR_LOC("f:%s,l:%lu,c:%lu")
#define _SCLEXER_TOK_LOC_UNWRAP(TOK) \
	(TOK)->loc.fpath, (TOK)->loc.line, (TOK)->loc.column
#else
#define _SCLEXER_TOK_LOC_FMT _SCLEXER_MSG_COLOR_LOC("l:%lu,c:%lu")
#define _SCLEXER_TOK_LOC_UNWRAP(TOK) (TOK)->loc.line, (TOK)->loc.column
#endif // SCLEXER_DISABLE_MORE_LOC_MSG

#define _SCLEXER_TOK_KIND_FMT _SCLEXER_MSG_COLOR("%s", "\x1b[32m")
#define _SCLEXER_TOK_KIND_FMT_ARG(TOK) kind_names[(TOK)->kind]

/* For public functions from `sclexer.h` */
#define _sclexer_check(E) \
	if (!(E)) { \
		_sclexer_eprintf(_SCLEXER_ERR_FMT"failed to _sclexer_check '%s'\n", \
				_SCLEXER_ERR_FMT_ARG, #E); \
	}

/* https://github.com/Gottox/smu
 * See `LICENSE.smu` under this file for the license. {{{ */
static void _sclexer_eprintf(const char *format, ...);
static void *_sclexer_ereallocz(void *p, size_t siz);

void _sclexer_eprintf(const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
	exit(EXIT_FAILURE);
}

void *_sclexer_ereallocz(void *p, size_t siz)
{
	void *r;
	if (p) {
		r = realloc(p, siz);
	} else {
		r = calloc(1, siz);
	}
	if (r)
		return r;
	_sclexer_eprintf(_SCLEXER_ERR_FMT"failed to malloc() %u bytes\n",
			_SCLEXER_ERR_FMT_ARG, siz);
	return NULL;
}
/* }}} */

static void _sclexer_advance(struct sclexer *self, size_t readed);
/**
 * @return: 0 on compare failed, otherwise compared string length.
 */
static size_t _sclexer_cmp_src_with_cstr(const char *cur, const char *cstr);
static bool _sclexer_do_eof(struct sclexer *self, struct sclexer_tok *tok);
static size_t _sclexer_do_ident(struct sclexer *self, struct sclexer_tok *tok);
static void _sclexer_drop_space(struct sclexer *self);
static size_t _sclexer_drop_until_endl(struct sclexer *self);
static bool _sclexer_is_prev_eol(struct sclexer_tok *tokens,
		struct sclexer_tok *cur_tok,
		size_t count);
static void _sclexer_next_line(struct sclexer *self);
static size_t _sclexer_try_comment(struct sclexer *self);
static size_t _sclexer_try_digit(struct sclexer *self, struct sclexer_tok *tok);
static bool _sclexer_try_endl(struct sclexer *self, struct sclexer_tok *tok);
static bool _sclexer_try_indent(struct sclexer *self, struct sclexer_tok *tok);
static void _sclexer_try_keyword(struct sclexer *self, struct sclexer_tok *tok);
static size_t _sclexer_try_string(struct sclexer *self, struct sclexer_tok *tok);
static size_t _sclexer_try_symbol(struct sclexer *self, struct sclexer_tok *tok);

static const char *kind_names[SCLEXER_TOK_KIND_COUNT] = {
	[SCLEXER_UNKNOWN_TOK] = "UNKNOWN_TOK",
	[SCLEXER_EOF]         = "EOF",
	[SCLEXER_EOL]         = "EOL",
	[SCLEXER_IDENT]       = "IDENT",
	[SCLEXER_INT]         = "INT",
	[SCLEXER_INT_NEG]     = "INT_NEG",
	[SCLEXER_KEYWORD]     = "KEYWORD",
	[SCLEXER_STRING]      = "STRING",
	[SCLEXER_SYMBOL]      = "SYMBOL",

	[SCLEXER_INDENT_BLOCK_BEGIN] = "INDENT_BLOCK_BEGIN",
	[SCLEXER_INDENT_BLOCK_END]   = "INDENT_BLOCK_END"
};

void _sclexer_advance(struct sclexer *self, size_t readed)
{
	self->_cur = &self->_cur[readed];
	self->_loc.column += readed;
}

size_t _sclexer_cmp_src_with_cstr(const char *cur, const char *cstr)
{
	size_t i = 0;
	for (; cur[i] != '\0' && cstr[i] != '\0'; i++) {
		if (cur[i] != cstr[i])
			return 0;
	}
	return i;
}

bool _sclexer_do_eof(struct sclexer *self, struct sclexer_tok *tok)
{
	if (self->_last_indent) {
		tok->kind = SCLEXER_INDENT_BLOCK_END;
		self->_last_indent--;
		return true;
	}
	tok->kind = SCLEXER_EOF;
	return false;
}

size_t _sclexer_do_ident(struct sclexer *self, struct sclexer_tok *tok)
{
	size_t readed = 0;
	if (!self->is_ident(self->_cur[0], true))
		return 0;
	readed = 1;
	for (; self->_cur[readed] != '\0'; readed++) {
		if (!self->is_ident(self->_cur[readed], false))
			goto end;
	}
end:
	tok->kind = SCLEXER_IDENT;
	tok->src.len = readed;
	tok->data.str.begin = tok->src.begin;
	tok->data.str.len = readed;
	return readed;
}

void _sclexer_drop_space(struct sclexer *self)
{
	size_t readed = 0;
	if (self->_cur[readed] == '\n')
		return;
	while (isspace(self->_cur[readed]))
		readed++;
	_sclexer_advance(self, readed);
}

size_t _sclexer_drop_until_endl(struct sclexer *self)
{
	size_t readed = 0;
	for (; self->_cur[readed] != '\0'; readed++) {
		if (self->_cur[readed] == '\n')
			return readed + 1;
	}
	return 0;
}

bool _sclexer_is_prev_eol(struct sclexer_tok *tokens,
		struct sclexer_tok *cur_tok,
		size_t count)
{
	if (count == 0)
		return false;
	if (cur_tok->kind != SCLEXER_EOL)
		return false;
	if (tokens[count - 1].kind != SCLEXER_EOL)
		return false;
	return true;
}

void _sclexer_next_line(struct sclexer *self)
{
	self->_after_endl = true;
	self->_loc.line++;
	self->_loc.column = 1;
}

size_t _sclexer_try_comment(struct sclexer *self)
{
	for (size_t i = 0; i < self->comments_count; i++) {
		if (_sclexer_cmp_src_with_cstr(self->_cur, self->comments[i]))
			return _sclexer_drop_until_endl(self);
	}
	return 0;
}

size_t _sclexer_try_digit(struct sclexer *self, struct sclexer_tok *tok)
{
	size_t readed = 0;
	tok->data.uint = 0;
	if (self->_cur[0] == '-') {
		if (!isdigit(self->_cur[1]))
			return 0;
		readed = 1;
	}
	if (!isdigit(self->_cur[0]))
		return 0;
	for (; isdigit(self->_cur[readed]); readed++) {
		tok->data.uint *= 10;
		tok->data.uint += self->_cur[readed] - '0';
	}
	tok->kind = SCLEXER_INT;
	if (self->_cur[0] == '-') {
		tok->data.sint = -(tok->data.uint);
		tok->kind = SCLEXER_INT_NEG;
	}
	return readed;
}

bool _sclexer_try_endl(struct sclexer *self, struct sclexer_tok *tok)
{
	size_t readed = 1;
	if (self->_cur[0] != '\n' && (readed = _sclexer_try_comment(self)) == 0) {
		return false;
	}
	tok->kind = SCLEXER_EOL;
	tok->src.len = readed;
	_sclexer_advance(self, readed);
	_sclexer_next_line(self);
	return true;
}

bool _sclexer_try_indent(struct sclexer *self, struct sclexer_tok *tok)
{
	size_t readed = 0;
	if (!self->enable_indent)
		return false;
	if (!self->_after_endl)
		return false;
	while (self->_cur[readed] == '\t')
		readed++;
	if (readed > self->_last_indent) {
		tok->kind = SCLEXER_INDENT_BLOCK_BEGIN;
		self->_last_indent++;
	} else if (readed < self->_last_indent) {
		tok->kind = SCLEXER_INDENT_BLOCK_END;
		self->_last_indent--;
	} else {
		return false;
	}
	return true;
}

void _sclexer_try_keyword(struct sclexer *self, struct sclexer_tok *tok)
{
	for (size_t i = 0; i < self->keywords_count; i++) {
		if (strlen(self->keywords[i]) != tok->src.len)
			continue;
		if (strncmp(self->keywords[i],
					tok->src.begin,
					tok->src.len) == 0) {
			tok->data.keyword = i;
			tok->kind = SCLEXER_KEYWORD;
			return;
		}
	}
}

size_t _sclexer_try_string(struct sclexer *self, struct sclexer_tok *tok)
{
	size_t readed = 0;
	if (self->_cur[0] != '"')
		return 0;
	readed = 1;
	for (; self->_cur[readed] != '"'; readed++) {
		switch (self->_cur[readed]) {
		case '\0':
		case '\n':
			return 0;
		}
	}
	readed++;
	tok->data.str.begin = &self->_cur[1];
	tok->data.str.len = readed - 2;
	tok->kind = SCLEXER_STRING;
	return readed;
}

size_t _sclexer_try_symbol(struct sclexer *self, struct sclexer_tok *tok)
{
	size_t prev = 0, readed = 0;
	for (size_t i = 0; i < self->symbols_count; i++) {
		readed = _sclexer_cmp_src_with_cstr(self->_cur, self->symbols[i]);
		if (readed == 0)
			continue;
		if (readed < prev)
			continue;
		tok->data.symbol = i;
		tok->kind = SCLEXER_SYMBOL;
		prev = readed;
	}
	return prev;
}

bool sclexer_default_is_ident(char c, bool begin)
{
	if (begin && isdigit(c))
		return false;
	if (c == '_')
		return true;
	if (isalnum(c))
		return true;
	return false;
}

void sclexer_dup_tok(struct sclexer_tok *dst, struct sclexer_tok *src)
{
	_sclexer_check(dst && src);
	memcpy(dst, src, sizeof(*src));
}

bool sclexer_get_tok(struct sclexer *self,
		struct sclexer_tok *tok)
{
	size_t readed = 0;
	_sclexer_check(self && tok);
	_sclexer_check(self->src && self->_cur);

	tok->src.begin = self->_cur;
	tok->loc = self->_loc;
	tok->kind = SCLEXER_UNKNOWN_TOK;

	if (_sclexer_try_indent(self, tok)) {
		self->_after_endl = false;
		return true;
	}

	if (self->_cur[0] == '\0') {
		if (!self->_after_endl) {
			tok->kind = SCLEXER_EOL;
			self->_after_endl = true;
			return true;
		}
		return false;
	}
	if (self->_after_endl)
		self->_after_endl = false;

	_sclexer_drop_space(self);
	if (self->_cur[0] == '\0')
		return _sclexer_do_eof(self, tok);

	tok->src.begin = self->_cur;
	tok->loc = self->_loc;

	if (_sclexer_try_endl(self, tok))
		return true;
	if ((readed = _sclexer_try_digit(self, tok)))
		goto end;
	if ((readed = _sclexer_try_string(self, tok)))
		goto end;
	if ((readed = _sclexer_try_symbol(self, tok)))
		goto end;
	if ((readed = _sclexer_do_ident(self, tok))) {
		_sclexer_try_keyword(self, tok);
		goto end;
	}
	_sclexer_eprintf(_SCLEXER_ERR_FMT"unknown token '%c' "_SCLEXER_TOK_LOC_FMT"\n",
			_SCLEXER_ERR_FMT_ARG,
			self->_cur[0],
			_SCLEXER_TOK_LOC_UNWRAP(tok));
	return false;
end:
	_sclexer_advance(self, readed);
	tok->src.len = readed;
	return true;
}

void sclexer_init(struct sclexer *self, const char *fpath)
{
	_sclexer_check(self)
	_sclexer_check(self->src)
	_sclexer_check(fpath)
	if (!self->comments
			&& !self->symbols
			&& !self->keywords) {
		_sclexer_eprintf(_SCLEXER_ERR_FMT"need more parameters\n",
				_SCLEXER_ERR_FMT_ARG);
	}
	if (!self->is_ident)
		self->is_ident = sclexer_default_is_ident;
	self->_cur = self->src;
	self->_last_indent = 0;
	self->_loc.fpath  = fpath;
	self->_loc.line   = 1;
	self->_loc.column = 1;
}

const char *sclexer_kind_names(enum SCLEXER_TOK_KIND kind)
{
	if (kind >= SCLEXER_TOK_KIND_COUNT)
		return NULL;
	return kind_names[kind];
}

size_t sclexer_get_tokens(struct sclexer *self, struct sclexer_tok **result)
{
	size_t count = 0, capacity = _SCLEXER_DEFAULT_TOKENS_CAPACITY;
	struct sclexer_tok cur_tok = {0};
	struct sclexer_tok *tokens = NULL;
	tokens = _sclexer_ereallocz(tokens, sizeof(*tokens) * capacity);
	for (; sclexer_get_tok(self, &cur_tok); count++) {
		if (_sclexer_is_prev_eol(tokens, &cur_tok, count)) {
			count--;
			continue;
		}
		if (count == capacity) {
			capacity += _SCLEXER_DEFAULT_TOKENS_CAPACITY;
			tokens = _sclexer_ereallocz(tokens, sizeof(*tokens) * capacity);
		}
		sclexer_dup_tok(&tokens[count], &cur_tok);
	}
	*result = tokens;
	return count;
}

/* shits, is's cannot be readed. */
void sclexer_print_tok(struct sclexer *self, struct sclexer_tok *tok)
{
	switch (tok->kind) {
	case SCLEXER_IDENT:
	case SCLEXER_STRING:
		printf(_SCLEXER_TOK_KIND_FMT"(len=%lu, '%.*s', "_SCLEXER_TOK_LOC_FMT")\n",
				_SCLEXER_TOK_KIND_FMT_ARG(tok),
				tok->data.str.len,
				(int)tok->data.str.len,
				tok->data.str.begin,
				_SCLEXER_TOK_LOC_UNWRAP(tok));
		break;
	case SCLEXER_INT:
		printf(_SCLEXER_TOK_KIND_FMT"(%lu, "_SCLEXER_TOK_LOC_FMT")\n",
				_SCLEXER_TOK_KIND_FMT_ARG(tok),
				tok->data.uint,
				_SCLEXER_TOK_LOC_UNWRAP(tok));
		break;
	case SCLEXER_INT_NEG:
		printf(_SCLEXER_TOK_KIND_FMT"(%ld, "_SCLEXER_TOK_LOC_FMT")\n",
				_SCLEXER_TOK_KIND_FMT_ARG(tok),
				tok->data.sint,
				_SCLEXER_TOK_LOC_UNWRAP(tok));
		break;
	case SCLEXER_KEYWORD:
		printf(_SCLEXER_TOK_KIND_FMT"('%s', "_SCLEXER_TOK_LOC_FMT")\n",
				_SCLEXER_TOK_KIND_FMT_ARG(tok),
				self->keywords[tok->data.keyword],
				_SCLEXER_TOK_LOC_UNWRAP(tok));
		break;
	case SCLEXER_SYMBOL:
		printf(_SCLEXER_TOK_KIND_FMT"('%s', "_SCLEXER_TOK_LOC_FMT")\n",
				_SCLEXER_TOK_KIND_FMT_ARG(tok),
				self->symbols[tok->data.symbol],
				_SCLEXER_TOK_LOC_UNWRAP(tok));
		break;
	default:
		printf(_SCLEXER_TOK_KIND_FMT"("_SCLEXER_TOK_LOC_FMT")\n",
				_SCLEXER_TOK_KIND_FMT_ARG(tok),
				_SCLEXER_TOK_LOC_UNWRAP(tok));
		break;
	}
}

size_t sclexer_read_file(char **result, const char *fpath)
{
	/* https://github.com/Gottox/smu */
	char *buf = NULL;
	size_t bsiz, len = 0, readed = 0;
	FILE *fp;
	if (!(fp = fopen(fpath, "r")))
		_sclexer_eprintf(_SCLEXER_ERR_FMT"no such file or directory: %s\n",
				_SCLEXER_ERR_FMT_ARG, fpath);
	bsiz = 2 * BUFSIZ;
	buf = _sclexer_ereallocz(buf, bsiz);
	while ((readed = fread(buf + len, 1, BUFSIZ, fp))) {
		len += readed;
		if (BUFSIZ + len + 1 > bsiz) {
			bsiz += BUFSIZ;
			buf = _sclexer_ereallocz(buf, bsiz);
		}
	}
	buf[len] = '\0';
	fclose(fp);
	*result = buf;
	return len;
}
#endif

/* LICENSE.smu
 *
 * MIT/X Consortium License

 * (c) 2007-2014 Enno Boland <g s01 de>

 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software. 

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL 
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE. */
