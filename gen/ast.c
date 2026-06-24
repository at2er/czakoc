/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdio.h>
#include <string.h>

#define SCLEXER_IMPL
#define UTILSH_DIE_IMPL
#include "darr.h"
#include "die.h"
#include "sclexer.h"

#define err_eof die("%d:%d: end of file\n", tok.row, tok.col)
#define expect(TOK) if (tok.type != (TOK)) unexpected
#define next() sclexer_next(&lexer, &tok)
#define nextor if (!next())
#define nextorend nextor return 0
#define unexpected \
	die("%d:%d: unexpected token: \"%.*s\"\n", \
	    tok.row, tok.col, tok.len, tok.str)

enum TOKEN {
	TOK_ASSIGN,
	TOK_DARR,
	TOK_ENUM,
	TOK_STRUCT,
	TOK_TAG,
	TOK_TOKEN,
	TOK_TYPE,
	TOK_I64,
	TOK_U64,
	TOK_IDENT_,

	TOK_LBRACE,
	TOK_RBRACE,
	TOK_LBRACKET,
	TOK_RBRACKET,

	TOK_IDENT,

	TOK_ERR
};

static int parse(void);
static void parse_darr(void);
static int parse_enum(int indent, char *name);
static int parse_struct(int indent, char *id, char *name);
static int parse_struct_member(int indent, struct sclexer_tok *ident);
static void putindent(int level);
static void putname(char *name);

static const char *tokens[] = {
	[TOK_ASSIGN]   = "=",
	[TOK_DARR]     = "#darr",
	[TOK_ENUM]     = "enum",
	[TOK_STRUCT]   = "struct",
	[TOK_TAG]      = "#tag",
	[TOK_TOKEN]    = "TOKEN",
	[TOK_TYPE]     = "type",
	[TOK_I64]      = "i64",
	[TOK_U64]      = "u64",
	[TOK_IDENT_]   = "IDENT",
	[TOK_LBRACE]   = "{",
	[TOK_RBRACE]   = "}",
	[TOK_LBRACKET] = "[",
	[TOK_RBRACKET] = "]",
	NULL
};
static const char *comments[] = {
	"--",
	NULL
};

static struct sclexer lexer;
static struct sclexer_tok tok;

int
parse(void)
{
	char *ident;

	nextorend;
	expect(TOK_IDENT);

	ident = strndup(tok.str, tok.len);

	nextor err_eof;
	expect(TOK_TYPE);
	nextor err_eof;
	expect(TOK_ASSIGN);

	nextor err_eof;
	switch (tok.type) {
	case TOK_STRUCT:
		return parse_struct(0, NULL, ident);
	case TOK_ENUM:
		return parse_enum(0, ident);
		break;
	}
	return 1;
}

void
parse_darr(void)
{
	char *tmp;
	struct sclexer_tok type;
	nextor err_eof;
	expect(TOK_LBRACKET);
	nextor err_eof;
	type = tok;
	nextor err_eof;
	expect(TOK_RBRACKET);
	tmp = strndup(type.str, type.len);
	putname(tmp);
	fputs("s_t ", stdout);
	free(tmp);
}

int
parse_enum(int indent, char *name)
{
	struct sclexer_tok ident;
	darr(char *) members;

	nextor err_eof;
	expect(TOK_TAG);

	darr_init(&members);

	nextorend;
	expect(TOK_LBRACE);
	fputs("struct ", stdout);
	putname(name);
	puts(" {");
	putindent(indent + 1);
	puts("union {");

	while (parse_struct_member(indent + 1, &ident)) {
		darr_expand(&members);
		darr_last(&members) = strndup(ident.str, ident.len);
	}

	putindent(indent + 1);
	puts("} u;");

	putindent(indent + 1);
	puts("enum {");
	for (int i = 0; i < members.n; i++) {
		putindent(indent + 2);
		for (char *c = name; *c; c++)
			putc(toupper(*c), stdout);
		putc('_', stdout);
		for (char *c = members.e[i]; *c; c++)
			putc(toupper(*c), stdout);
		if (i == members.n - 1)
			putc('\n', stdout);
		else
			puts(",");
	}
	putindent(indent + 1);
	puts("} k;");

	putindent(indent);
	puts("};");
	return 1;
}

int
parse_struct(int indent, char *id, char *name)
{
	struct sclexer_tok ident;

	nextorend;
	expect(TOK_LBRACE);
	fputs("struct ", stdout);

	if (name) {
		putname(name);
		putc(' ', stdout);
	}
	puts("{");

	while (parse_struct_member(indent, &ident));

	putindent(indent);
	putc('}', stdout);
	if (id)
		printf(" %s", id);
	puts(";");

	return 1;
}

int
parse_struct_member(int indent, struct sclexer_tok *ident)
{
	char *name, *tmp;

	nextor err_eof;
	if (tok.type == TOK_RBRACE)
		return 0;
	expect(TOK_IDENT);
	*ident = tok;
	name = strndup(ident->str, ident->len);
	nextor err_eof;

	putindent(indent + 1);
	switch (tok.type) {
	case TOK_DARR:
		parse_darr();
		goto putname;
	case TOK_I64:
		fputs("int64_t ", stdout);
		goto putname;
	case TOK_IDENT:
		tmp = strndup(tok.str, tok.len);
		fputs("struct ", stdout);
		putname(tmp);
		putc(' ', stdout);
		free(tmp);
		goto putname;
	case TOK_IDENT_:
		fputs("struct zk_ident *", stdout);
		goto putname;
	case TOK_STRUCT:
		parse_struct(indent + 1, name, NULL);
		break;
	case TOK_TOKEN:
		fputs("enum TOKEN ", stdout);
		goto putname;
	case TOK_U64:
		fputs("uint64_t ", stdout);
		goto putname;
	default:
		unexpected;
		break;
	putname:
		printf("%s;\n", name);
		break;
	}

	free(name);

	return 1;
}

void
putindent(int level)
{
	for (int i = 0; i < level; i++)
		putc('\t', stdout);
}

void
putname(char *name)
{
	fputs("zk", stdout);
	for (char *c = name; *c; c++) {
		if (isupper(*c))
			putc('_', stdout);
		putc(tolower(*c), stdout);
	}
}

int
main()
{
	char *src;

	sclexer_read_file(stdin, &src);

	lexer.tokens = tokens;
	lexer.comments = comments;
	lexer.eol_tok = -1;
	lexer.ident_tok = TOK_IDENT;
	lexer.int_tok = -1;
	sclexer_init(&lexer, src);

#if 1
	while (parse()) {
	}
#else
	while (sclexer_next(&lexer, &tok)) {
		printf("%d:%d\t%d\t\"%.*s\"\n", tok.row, tok.col, tok.type, tok.len, tok.str);
	}
#endif

	return 0;
}
