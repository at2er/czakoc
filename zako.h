#ifndef CZAKOC_H
#define CZAKOC_H
#include <stdbool.h>
#include <stdint.h>
#include "darr.h"
#include "macros.h"
#include "str.h"

enum TOKEN {
	TOK_AND,
	TOK_ELSE,
	TOK_ENUM,
	TOK_EXTERN,
	TOK_FN,
	TOK_FOR,
	TOK_IF,
	TOK_IMPL,
	TOK_LET,
	TOK_MUT,
	TOK_OR,
	TOK_PUB,
	TOK_RETURN,
	TOK_SELF,
	TOK_STRUCT,
	TOK_TRAIT,
	TOK_TYPE,
	TOK_UNION,

	TOK_U8, TOK_U16, TOK_U32, TOK_U64,
	TOK_I8, TOK_I16, TOK_I32, TOK_I64,

	TOK_KEYWORD_COUNT,

	TOK_EQ,
	TOK_GE,
	TOK_GT,
	TOK_LE,
	TOK_LT,
	TOK_NE,

	TOK_MINUS,
	TOK_PLUS,
	TOK_SLASH,
	TOK_STAR,

	TOK_AMP,
	TOK_COMMA,
	TOK_DOT,

	TOK_ASSIGN,
	TOK_ADD_ASSIGN,
	TOK_DIV_ASSIGN,
	TOK_MUL_ASSIGN,
	TOK_SUB_ASSIGN,

	TOK_LBRACE,
	TOK_RBRACE,
	TOK_LBRACKET,
	TOK_RBRACKET,
	TOK_LPAREN,
	TOK_RPAREN,

	TOK_PUNCT_COUNT,

	TOK_EOL,
	TOK_IDENT,
	TOK_INT,
	TOK_STRING,

	TOK_ERR
};

enum OPERATOR {
	ZK_AND,
	ZK_OR,

	ZK_ADD,
	ZK_DIV,
	ZK_MUL,
	ZK_SUB,

	ZK_EQ,
	ZK_GE,
	ZK_GT,
	ZK_LE,
	ZK_LT,
	ZK_NE,

	ZK_ASSIGN,
	ZK_ADD_ASSIGN,
	ZK_DIV_ASSIGN,
	ZK_MUL_ASSIGN,
	ZK_SUB_ASSIGN,

	ZK_DOT,

	ZK_OP_COUNT
};

enum ZK_BUILTIN_TYPE {
	ZK_U8, ZK_U16, ZK_U32, ZK_U64,
	ZK_I8, ZK_I16, ZK_I32, ZK_I64,

	ZK_CI8, ZK_CI16, ZK_CI32, ZK_CI64,

	ZK_ARR, ZK_ENUM, ZK_PTR, ZK_STRUCT, ZK_UNION,

	/* internal types */
	ZK_ANY_TYPE, ZK_CONST_STR_TYPE,
	ZK_GENERIC_TYPE, ZK_GENERIC_INSTANCE_TYPE,
	ZK_SELF_TYPE, ZK_TYPE_REF
};

#define IS_ASSIGN_EXPR(BINARY_OP) RANGE((BINARY_OP), ZK_ASSIGN, ZK_SUB_ASSIGN)

typedef darr(struct zk_expr*) zk_exprs_t;
typedef darr(struct zk_generic_instance_type*) zk_generic_instances_t;
typedef darr(struct zk_enum_member*) zk_enum_members_t;
typedef darr(struct zk_ident*) zk_idents_t;
typedef darr(struct zk_type) zk_types_t;
typedef darr(struct zk_val*) zk_vals_t;

struct zk_arr_type {
	struct zk_type *type;
	unsigned int siz;
};

struct zk_enum_member {
	struct zk_ident *id;
	struct zk_expr *val;
};

struct zk_enum_type {
	enum ZK_BUILTIN_TYPE base;
	struct zk_ident *id;
	zk_enum_members_t members;
};

struct zk_generic_instance_type {
	struct zk_type *instance;
	struct zk_type *t;
};

struct zk_scope {
	struct zk_type *expect_type;
	struct zk_scope *parent;
	zk_idents_t idents;
};

struct zk_struct_type {
	struct zk_ident *id;
	zk_idents_t members;

	struct zk_type *cur_instance;
	zk_generic_instances_t generic_instances;
	bool generics;
};

struct zk_type {
	enum ZK_BUILTIN_TYPE builtin;

	union {
		struct zk_arr_type arr_type;
		struct zk_enum_type enum_type;
		struct zk_generic_instance_type *generic_instance_type;
		struct zk_struct_type struct_type;
		struct zk_type *type;
	} u;

	unsigned int mutable:1;
};

struct zk_address_of_expr {
	struct zk_ident *id;
	struct zk_type type;
};

struct zk_binary_expr {
	struct zk_val *lhs, *rhs;
	enum OPERATOR op;
	struct zk_type *type;
};

struct zk_fn_call {
	struct zk_ident *fn;
	zk_exprs_t args;
	char *unanalyzed_id;
};

struct zk_if_expr {
	struct zk_expr *cond;
	zk_exprs_t then;
	struct zk_type *type;
};

struct zk_expr {
	enum {
		ZK_ADDRESS_OF_EXPR,
		ZK_BINARY_EXPR,
		ZK_FN_CALL_EXPR,
		ZK_IF_EXPR
	} k;
	union {
		struct zk_address_of_expr address_of;
		struct zk_binary_expr binary;
		struct zk_fn_call fn_call;
		struct zk_if_expr if_expr;
	} u;
};

struct zk_fn {
	zk_idents_t args;
};

struct zk_trait {
	struct zk_scope scope;
};

struct zk_ident {
	char *name, *realname;
	struct zk_type type;
	unsigned int pub:1, extern_:1;

	enum ZK_IDENT_KIND {
		ZK_FN,
		ZK_TRAIT,
		ZK_TYPE_IDENT,
		ZK_IDENT
	} k;
	union {
		struct zk_fn fn;
		struct zk_trait trait;
	} u;
};

struct zk_mod {
	struct zk_scope scope;
};

typedef darr(struct zk_brace_init_member) zk_brace_init_members_t;
struct zk_brace_init {
	zk_brace_init_members_t members;
	struct zk_type *type;
};

struct zk_brace_init_member {
	enum {
		ZK_BRACE_INIT_BY_IDX,
		ZK_BRACE_INIT_BY_IDENT
	} k;
	union {
		unsigned int i;
		char *ident;
	} idx;
	struct zk_expr *val;
};

/* WTF */
struct zk_int_val {
	int64_t i;
	struct zk_type type;
};

struct zk_string_val {
	char *s;
	struct zk_type type;
};

struct zk_val {
	enum {
		ZK_BRACE_INIT_VAL,
		ZK_EXPR_VAL,
		ZK_IDENT_VAL,
		ZK_INT_VAL,
		ZK_STRING_VAL,

		ZK_UNANALYZED_IDENT_VAL
	} k;
	union {
		struct zk_brace_init brace_init;
		struct zk_expr *expr;
		struct zk_int_val i; /* shit */
		struct zk_ident *id;
		struct zk_string_val str;
		char *unanalyzed_id;
	} u;
};

#define IS_INTEGER(BUILTIN_TYPE) (RANGE((BUILTIN_TYPE), ZK_U8, ZK_CI64))
#define IS_SIGNED_INTEGER(BUILTIN_TYPE) ( \
		RANGE((BUILTIN_TYPE), ZK_I8, ZK_I64) || \
		RANGE((BUILTIN_TYPE), ZK_CI8, ZK_CI64))

struct zk_type *deref_type(struct zk_type *t);
struct zk_type *dup_type(struct zk_type *t);
struct zk_ident *find_enum_member(struct zk_enum_type *type, const struct str *name);
struct zk_ident *find_ident(struct zk_scope *scope, const struct str *name);
struct zk_ident *find_struct_member(struct zk_struct_type *type, const struct str *name);
struct zk_type *monomorphize(struct zk_type *instance, struct zk_type *t);
struct zk_arr_type *monomorphize_arr(struct zk_arr_type *instance,
		struct zk_type *t);
struct zk_struct_type *monomorphize_struct(struct zk_struct_type *instance,
		struct zk_type *t);
struct zk_type *open_type(struct zk_type *type);

#endif
