/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdio.h>
#include <string.h>

#include "czakoc.h"

#define SCLEXER_IMPL
#include "sclexer.h"

#define expect(P, TOK) if ((P)->tok.type != (TOK)) unexpected(P)
#define next(P) sclexer_next(&(P)->lexer, &(P)->tok)
#define nextor(P) if (!next(P))
#define unexpected(P) \
	die("%d:%d: unexpected token: \"%.*s\"\n", \
	    (P)->tok.row, (P)->tok.col, (P)->tok.len, (P)->tok.str)

enum TOKEN {
	TOK_ASSIGN,
	TOK_COMMA,

	TOK_LBRACE,
	TOK_RBRACE,
	TOK_LPAREN,
	TOK_RPAREN,

	TOK_FN,
	TOK_RETURN,

	TOK_U8, TOK_U16, TOK_U32, TOK_U64,
	TOK_I8, TOK_I16, TOK_I32, TOK_I64,

	TOK_IDENT,
	TOK_INT,

	TOK_ERR
};

struct parser {
	struct sclexer lexer;
	struct sclexer_tok tok;
};

static void parse_top_ident(struct parser *p);

static const char *tokens[] = {
	[TOK_ASSIGN] = "=",
	[TOK_COMMA]  = ",",
	[TOK_LBRACE] = "{",
	[TOK_RBRACE] = "}",
	[TOK_LPAREN] = "(",
	[TOK_RPAREN] = ")",
	[TOK_FN]     = "fn",
	[TOK_RETURN] = "return",
	[TOK_U8]     = "u8",
	[TOK_U16]    = "u16",
	[TOK_U32]    = "u32",
	[TOK_U64]    = "u64",
	[TOK_I8]     = "i8",
	[TOK_I16]    = "i16",
	[TOK_I32]    = "i32",
	[TOK_I64]    = "i64",
	NULL
};
static const char *comments[] = {
	"--",
	NULL
};

void
parse_top_ident(struct parser *p)
{
	struct zk_top_stmt *id;
}

int
parse_file(const char *path)
{
	FILE *fp;
	struct parser p = {0};
	char *src;

	if (!(fp = fopen(path, "r")))
		return 1;

	sclexer_read_file(fp, &src);
	p.lexer.comments = comments;
	p.lexer.tokens = tokens;
	p.lexer.ident_tok = TOK_IDENT;
	p.lexer.int_tok = TOK_INT;	
	sclexer_init(&p.lexer, src);
#if 0
	while (1) {
		nextor(&p) break;
		if (p.tok == TOK_IDENT) {
			parse_top_ident(&p);
			continue;
		}
	}
#else
	while (sclexer_next(&p.lexer, &p.tok)) {
		printf("%d:%d\t%d\t\"%.*s\"\n",
				p.tok.row, p.tok.col, p.tok.type,
				p.tok.len, p.tok.str);
	}
#endif
	return 0;
}
