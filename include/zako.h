/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_ZAKO_H
#define CZAKOC_ZAKO_H
#include "darr.h"

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

	TOK_EOL,
	TOK_IDENT,
	TOK_INT,

	TOK_ERR
};

enum ZK_TYPE_BASE {
	ZK_8, ZK_16, ZK_32, ZK_64,

	ZK_FN
};

typedef darr(struct zk_stmt *) zk_stmts_t;
typedef darr(struct zk_value) zk_values_t;

struct zk_expr_op {
	int token;
};

struct zk_type {
	enum ZK_TYPE_BASE base;
	unsigned int
			_signed:1;
	union {
		struct zk_fn_type *fn;
	} u;
};

struct zk_fn_type {
	darr(struct zk_ident *) args;
	struct zk_type typ;
};

struct zk_ident {
	char *name;
	struct zk_type typ;
};

#endif
