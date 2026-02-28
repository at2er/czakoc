/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "lexer.h"
#include "macros.h"

static const char *comments[1] = {";"};

static const char *keywords[] = {
	[KEYWORD_FN]     = "fn",
	[KEYWORD_IF]     = "if",
	[KEYWORD_LET]    = "let",
	[KEYWORD_MUT]    = "mut",
	[KEYWORD_PUB]    = "pub",
	[KEYWORD_RETURN] = "return",
	[KEYWORD_THEN]   = "then",

	[KEYWORD_I8]  = "i8",
	[KEYWORD_I16] = "i16",
	[KEYWORD_I32] = "i32",
	[KEYWORD_I64] = "i64",
	[KEYWORD_U8]  = "u8",
	[KEYWORD_U16] = "u16",
	[KEYWORD_U32] = "u32",
	[KEYWORD_U64] = "u64"
};

static const char *symbols[] = {
	[SYM_BRACE_L]          = "{",
	[SYM_BRACE_R]          = "}",
	[SYM_BRACKET_L]        = "[",
	[SYM_BRACKET_R]        = "]",
	[SYM_PAREN_L]          = "(",
	[SYM_PAREN_R]          = ")",
	[SYM_ASSIGN]           = "=",
	[SYM_COMMA]            = ",",
	[SYM_INFIX_LESS_EQUAL] = "<=",
	[SYM_INFIX_ADD]        = "+",
	[SYM_INFIX_DIV]        = "/",
	[SYM_INFIX_MUL]        = "*",
	[SYM_INFIX_SUB]        = "-"
};

const char *
cstr_keyword(enum ZAKO_KEYWORD kw)
{
	return keywords[kw];
}

const char *
cstr_symbol(enum ZAKO_SYMBOL sym)
{
	return symbols[sym];
}

char *
init_lexer(struct sclexer *lexer, const char *path)
{
	char *src = NULL;
	lexer->enable_indent = true;
	lexer->comments = comments;
	lexer->comments_count = LENGTH(comments);
	lexer->keywords = keywords;
	lexer->keywords_count = LENGTH(keywords);
	lexer->symbols = symbols;
	lexer->symbols_count = LENGTH(symbols);
	lexer->src_siz = sclexer_read_file(&src, path);
	lexer->src = src;
	sclexer_init(lexer, path);
	return src;
}
