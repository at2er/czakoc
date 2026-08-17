#ifndef CZAKOC_PARSER_H
#define CZAKOC_PARSER_H
#include "darr.h"
#include "zako.h"

typedef darr(struct zk_stmt*) zk_block_t;
typedef darr(struct zk_top_stmt*) zk_top_stmts_t;

struct zk_fn_def {
	zk_block_t blk;
	struct zk_ident *id;
};

struct zk_ident_def_stmt {
	struct zk_ident *id;
	struct zk_expr *val;
};

struct zk_if_stmt {
	zk_block_t then, else_;
	struct zk_expr *cond;
};

struct zk_impl_stmt {
	struct zk_scope scope;
	zk_top_stmts_t stmts;
	struct zk_ident *struct_id, *trait_id;

	struct zk_ident *generic_id;
};

struct zk_stmt {
	enum ZK_STMT_KIND {
		ZK_EXPR_STMT,
		ZK_IDENT_DEF_STMT,
		ZK_IF_STMT,
		ZK_RETURN_STMT
	} k;
	union {
		struct zk_expr *expr_stmt;
		struct zk_ident_def_stmt ident_def;
		struct zk_if_stmt if_stmt;
		struct zk_expr *return_stmt;
	} u;
};

struct zk_trait_def {
	struct zk_ident *id;
	zk_top_stmts_t stmts;
};

struct zk_top_stmt {
	enum {
		ZK_FN_DEF,
		ZK_IMPL_STMT,
		ZK_STRUCT_DEF,
		ZK_TRAIT_DEF,
		ZK_TYPE_ALIAS_STMT
	} k;
	union {
		struct zk_fn_def fn;
		struct zk_ident *struct_def;
		struct zk_impl_stmt impl_stmt;
		struct zk_trait_def trait_def;
	} u;
};

struct zk_mod *parse(const char *path);

#endif
