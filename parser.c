#include <stdbool.h>
#include <stdio.h>
#include "conv.h"
#include "csex.h"
#include "die.h"
#include "ealloc.h"
#include "macros.h"
#include "parser.h"
#include "sclexer.h"
#include "semantics.h"

#define __next(P) (sclexer_next(&(P)->lexer, &(P)->tok) ? &(P)->tok : NULL)
#define duptok(P) strndup((P)->tok.str, (P)->tok.len)
#define expect(P, T) \
	if ((P)->tok.type != (T)) \
		unexpected(&(P)->tok);
#define next(P) ((P)->peek ? ((P)->peek = 0, &(P)->tok) : __next(P))
#define peek(P) ((P)->peek = 1)
#define unexpected(TOK) die(TOK_POS_HINT"unexpected token '%.*s'\n", \
		TOK_POS_HINT_ARG(TOK), (TOK)->len, (TOK)->str)

#define STRTOK(TOK) STR((char *)(TOK)->str, (TOK)->len)
#define TOK_POS_HINT "%s:%d:%d: "
#define TOK_POS_HINT_ARG(TOK) (TOK)->path, (TOK)->row, (TOK)->col

#define throw(P, MSG) die(TOK_POS_HINT MSG "\n", TOK_POS_HINT_ARG(&(P)->tok))
#define throw_semantics_err(P, ERROR) \
		throwf((P), "semantics: %s", str_semantics_err((ERROR)))
#define throwf(P, FMT, ...) \
		die(TOK_POS_HINT FMT "\n", TOK_POS_HINT_ARG(&(P)->tok), __VA_ARGS__)

struct parser {
	struct sclexer lexer;
	struct sclexer_tok tok;

	unsigned int peek:1;

	struct zk_scope *cscope,
	                 scope;

	struct codegen codegen;

	char *name_prefix;

	enum { IN_TOP, IN_TRAIT_DEF, IN_IMPL_DEF } where;
};

static struct zk_scope *enter_scope(struct parser *p);
static void exit_scope(struct parser *p);
static enum OPERATOR get_binary_op(enum TOKEN tok);
static struct zk_expr *merge_binary_expr(struct zk_expr *parent, struct zk_expr *child);
static struct zk_expr *parse_address_of_expr(struct parser *p);
static struct zk_type *parse_arr_type(struct parser *p, struct zk_type *typ);
static zk_block_t *parse_block(struct parser *p, zk_block_t *blk);
static struct zk_expr *parse_binary_expr(struct parser *p);
static struct zk_brace_init *parse_brace_init(struct parser *p,
		struct zk_brace_init *brace_init);
static struct zk_brace_init_member *parse_brace_init_member(struct parser *p,
		struct zk_brace_init_member *member,
		unsigned int idx);
static struct zk_brace_init_member *parse_brace_init_member_idx(struct parser *p,
		struct zk_brace_init_member *member,
		unsigned int idx);
static struct zk_stmt *parse_expr_stmt(struct parser *p);
static struct zk_expr *parse_fn_call(struct parser *p);
static struct zk_top_stmt *parse_fn_def(struct parser *p, struct zk_ident *id);
static int parse_fn_def_args(struct parser *p, struct zk_fn *fn);
static struct zk_stmt *parse_ident_def_stmt(struct parser *p);
static struct zk_val *parse_ident_val(struct parser *p, struct zk_val *v);
static struct zk_stmt *parse_if_stmt(struct parser *p);
static struct zk_top_stmt *parse_impl_stmt(struct parser *p);
static struct zk_stmt *parse_return_stmt(struct parser *p);
static struct zk_val *parse_string_val(struct parser *p, struct zk_val *v);
static struct zk_struct_type *parse_struct_type(struct parser *p, struct zk_type *typ);
static struct zk_top_stmt *parse_top_ident(struct parser *p, int pub);
static struct zk_top_stmt *parse_trait_def(struct parser *p, struct zk_ident *id);
static struct zk_type *parse_type(struct parser *p, struct zk_type *typ);
static struct zk_top_stmt *parse_type_def(struct parser *p, struct zk_ident *id);
static struct zk_type *parse_type_from_ident(struct parser *p, struct zk_type *typ);
static struct zk_val *parse_val(struct parser *p);

static const char *comments[] = { "--", NULL };

static const char *tokens[] = {
	[TOK_ELSE] = "else",
	[TOK_EXTERN] = "extern",
	[TOK_FN] = "fn",
	[TOK_FOR] = "for",
	[TOK_IF] = "if",
	[TOK_IMPL] = "impl",
	[TOK_LET] = "let",
	[TOK_MUT] = "mut",
	[TOK_PUB] = "pub",
	[TOK_RETURN] = "return",
	[TOK_SELF] = "Self",
	[TOK_STRUCT] = "struct",
	[TOK_TRAIT] = "trait",
	[TOK_TYPE] = "type",

	[TOK_U8] = "u8", [TOK_U16] = "u16", [TOK_U32] = "u32", [TOK_U64] = "u64",
	[TOK_I8] = "i8", [TOK_I16] = "i16", [TOK_I32] = "i32", [TOK_I64] = "i64",

	[TOK_EQ] = "==",
	[TOK_GE] = ">=",
	[TOK_GT] = ">",
	[TOK_LE] = "<=",
	[TOK_LT] = "<",
	[TOK_MINUS] = "-",
	[TOK_PLUS] = "+",
	[TOK_SLASH] = "/",
	[TOK_STAR] = "*",

	[TOK_AMP] = "&",
	[TOK_COMMA] = ",",
	[TOK_DOT] = ".",

	[TOK_ASSIGN] = "=",
	[TOK_ADD_ASSIGN] = "+=",
	[TOK_DIV_ASSIGN] = "/=",
	[TOK_MUL_ASSIGN] = "*=",
	[TOK_SUB_ASSIGN] = "-=",

	[TOK_LBRACE] = "{",
	[TOK_RBRACE] = "}",
	[TOK_LBRACKET] = "[",
	[TOK_RBRACKET] = "]",
	[TOK_LPAREN] = "(",
	[TOK_RPAREN] = ")",

	NULL
};

static int op_bind_power[] = {
	[ZK_ADD] = 1,
	[ZK_DIV] = 2,
	[ZK_MUL] = 2,
	[ZK_SUB] = 1,

	[ZK_EQ] = 3,
	[ZK_GE] = 3,
	[ZK_GT] = 3,
	[ZK_LE] = 3,
	[ZK_LT] = 3,

	[ZK_ASSIGN] = 0,
	[ZK_ADD_ASSIGN] = 0,
	[ZK_DIV_ASSIGN] = 0,
	[ZK_MUL_ASSIGN] = 0,
	[ZK_SUB_ASSIGN] = 0,

	[ZK_DOT] = 9
};

struct zk_scope *
enter_scope(struct parser *p)
{
	struct zk_scope *s = ecalloc(1, sizeof(*s));
	s->parent = p->cscope;
	p->cscope = s;
	p->cscope->expect_type = s->parent->expect_type;
	return s;
}

void
exit_scope(struct parser *p)
{
	struct zk_scope *s = p->cscope;
	p->cscope = p->cscope->parent;
	// TODO: Buy more memory
	darr_free_unsafe(&s->idents);
}

enum OPERATOR
get_binary_op(enum TOKEN tok)
{
	switch (tok) {
	case TOK_EQ: return ZK_EQ;
	case TOK_GE: return ZK_GE;
	case TOK_GT: return ZK_GT;
	case TOK_LE: return ZK_LE;
	case TOK_LT: return ZK_LT;
	case TOK_MINUS: return ZK_SUB;
	case TOK_PLUS: return ZK_ADD;
	case TOK_SLASH: return ZK_DIV;
	case TOK_STAR: return ZK_MUL;
	case TOK_DOT: return ZK_DOT;
	case TOK_ASSIGN: return ZK_ASSIGN;
	case TOK_ADD_ASSIGN: return ZK_ADD_ASSIGN;
	case TOK_DIV_ASSIGN: return ZK_DIV_ASSIGN;
	case TOK_MUL_ASSIGN: return ZK_MUL_ASSIGN;
	case TOK_SUB_ASSIGN: return ZK_SUB_ASSIGN;
	default: return ZK_OP_COUNT;
	}
}

struct zk_expr *
merge_binary_expr(struct zk_expr *parent, struct zk_expr *child)
{
	struct zk_binary_expr *p = &parent->u.binary, *c = &child->u.binary;
	if (op_bind_power[p->op] < op_bind_power[c->op]) {
		c->lhs = p->rhs;
		p->rhs = ecalloc(1, sizeof(*p->rhs));
		p->rhs->k = ZK_EXPR_VAL;
		p->rhs->u.expr = child;
		return parent;
	} else {
		c->lhs = ecalloc(1, sizeof(*c->lhs));
		c->lhs->k = ZK_EXPR_VAL;
		c->lhs->u.expr = parent;
		return child;
	}
}

struct zk_expr *
parse_address_of_expr(struct parser *p)
{
	struct zk_expr *expr = ecalloc(1, sizeof(*expr));
	struct zk_ident *id;
	next(p);
	id = find_ident(p->cscope, &STRTOK(&p->tok));
	if (!id)
		throwf(p, "identifier '%.*s' not found", p->tok.len, p->tok.str);
	expr->u.address_of.id = id;
	expr->u.address_of.type.builtin = ZK_PTR;
	expr->u.address_of.type.u.type = &id->type;
	expr->k = ZK_ADDRESS_OF_EXPR;
	return expr;
}

struct zk_type *
parse_arr_type(struct parser *p, struct zk_type *typ)
{
	struct zk_arr_type *arr_type = &typ->u.arr_type;
	arr_type->type = ecalloc(1, sizeof(*arr_type->type));
	parse_type(p, arr_type->type);
	next(p);
	switch (p->tok.type) {
	case TOK_COMMA:
		next(p);
		arr_type->siz = utilsh_conv_a2ui(p->tok.str, p->tok.len);
		next(p);
		expect(p, TOK_RBRACKET);
		break;
	case TOK_RBRACKET:
		break;
	default:
		unexpected(&p->tok);
		break;
	}
	return typ;
}

zk_block_t *
parse_block(struct parser *p, zk_block_t *blk)
{
	struct zk_stmt *stmt = NULL;

	darr_init(blk);

	next(p);
	expect(p, TOK_LBRACE);
	enter_scope(p);

	next(p);
	while (p->tok.type != TOK_RBRACE) {
		stmt = NULL;
		switch (p->tok.type) {
		case TOK_IF:
			stmt = parse_if_stmt(p);
			break;
		case TOK_LET:
			stmt = parse_ident_def_stmt(p);
			break;
		case TOK_RETURN:
			stmt = parse_return_stmt(p);
			break;
		case TOK_EOL:
			next(p);
			continue;
		default:
			peek(p);
			stmt = parse_expr_stmt(p);
			break;
		}
		if (!stmt)
			return NULL;
		darr_append(blk, stmt);
		next(p);
	}
	exit_scope(p);
	return blk;
}

struct zk_expr *
parse_binary_expr(struct parser *p)
{
	struct zk_binary_expr *parent, *child;
	enum OPERATOR op;
	struct zk_val *v;
	struct zk_expr *_parent = ecalloc(1, sizeof(*_parent)), *_child;

	_parent->k = ZK_BINARY_EXPR;
	parent = &_parent->u.binary;
	parent->op = ZK_OP_COUNT;

	if (!(parent->lhs = parse_val(p)))
		return NULL;

	next(p);
	while ((op = get_binary_op(p->tok.type)) != ZK_OP_COUNT) {
		if (!(v = parse_val(p)))
			return NULL;
		if (parent->rhs) {
			_child = ecalloc(1, sizeof(*_child));
			_child->k = ZK_BINARY_EXPR;
			child = &_child->u.binary;
			child->op = op;
			child->rhs = v;
			_parent = merge_binary_expr(_parent, _child);
			parent = &_parent->u.binary;
		} else {
			parent->op = op;
			parent->rhs = v;
		}

		next(p);
	}
	peek(p);

	return _parent;
}

struct zk_brace_init *
parse_brace_init(struct parser *p, struct zk_brace_init *brace_init)
{
	struct zk_brace_init_member *member;

	darr_init(&brace_init->members);
	do {
		next(p);
		if (p->tok.type == TOK_RBRACE)
			break;
		peek(p);
		darr_expand(&brace_init->members);
		member = &darr_last(&brace_init->members);
		parse_brace_init_member(p, member, brace_init->members.n - 1);
		next(p);
	} while (p->tok.type == TOK_COMMA);

	expect(p, TOK_RBRACE);

	return brace_init;
}

struct zk_brace_init_member *
parse_brace_init_member(struct parser *p,
		struct zk_brace_init_member *member,
		unsigned int idx)
{
	parse_brace_init_member_idx(p, member, idx);
	member->val = parse_binary_expr(p);
	return member;
}

struct zk_brace_init_member *
parse_brace_init_member_idx(struct parser *p,
		struct zk_brace_init_member *member,
		unsigned int idx)
{
	next(p);
	switch (p->tok.type) {
	case TOK_DOT:
		next(p);
		switch (p->tok.type) {
		case TOK_INT:
			member->k = ZK_BRACE_INIT_BY_IDX;
			member->idx.i = utilsh_conv_a2ui(p->tok.str, p->tok.len);
			break;
		case TOK_IDENT:
			member->k = ZK_BRACE_INIT_BY_IDENT;
			member->idx.ident = duptok(p);
			break;
		default:
			unexpected(&p->tok);
			break;
		}
		break;
	default:
		peek(p);
		member->k = ZK_BRACE_INIT_BY_IDX;
		member->idx.i = idx;
		return member;
	}
	next(p);
	expect(p, TOK_ASSIGN);
	return member;
}

struct zk_stmt *
parse_expr_stmt(struct parser *p)
{
	struct zk_binary_expr *binary;
	struct zk_expr *e;
	struct zk_stmt *stmt = ecalloc(1, sizeof(*stmt));
	struct zk_type *t, fake_expect;
	struct semantics_ctx sema = {0};

	stmt->k = ZK_EXPR_STMT;
	stmt->u.expr_stmt = parse_binary_expr(p);
	fake_expect.builtin = ZK_ANY_TYPE;
	sema.expect = &fake_expect;
	sema.scope = p->cscope;
	t = analyze_expr_type(&sema, stmt->u.expr_stmt);
	if (!t)
		throw_semantics_err(p, -1);

	binary = &stmt->u.expr_stmt->u.binary;
	if (binary->op == ZK_OP_COUNT &&
			binary->lhs->k == ZK_EXPR_VAL &&
			binary->lhs->u.expr->k == ZK_FN_CALL_EXPR) {
		e = binary->lhs->u.expr;
		free(stmt->u.expr_stmt);
		stmt->u.expr_stmt = e;
		return stmt;
	}
	if (!RANGE(stmt->u.expr_stmt->u.binary.op, ZK_ASSIGN, ZK_SUB_ASSIGN))
		throw(p, "not expression statement");
	return stmt;
}

struct zk_expr *
parse_fn_call(struct parser *p)
{
	struct zk_fn_call *call;
	struct zk_expr *expr = ecalloc(1, sizeof(*expr)), *e;

	expr->k = ZK_FN_CALL_EXPR;
	call = &expr->u.fn_call;

	next(p);
	expect(p, TOK_LPAREN);

	darr_init(&call->args);
	do {
		next(p);
		if (p->tok.type == TOK_RPAREN)
			break;
		peek(p);
		if (!(e = parse_binary_expr(p)))
			return NULL;
		darr_append(&call->args, e);
		next(p);
	} while (p->tok.type == TOK_COMMA);
	expect(p, TOK_RPAREN);

	return expr;
}

struct zk_top_stmt *
parse_fn_def(struct parser *p, struct zk_ident *id)
{
	struct zk_top_stmt *stmt = ecalloc(1, sizeof(*stmt));

	stmt->k = ZK_FN_DEF;
	stmt->u.fn.id = id;

	/* scope for function arguments */
	enter_scope(p);
	next(p);
	if (p->tok.type != TOK_LPAREN) {
		if (p->tok.type == TOK_ASSIGN)
			goto parse_body;
		return NULL;
	} else {
		if (parse_fn_def_args(p, &id->u.fn))
			return NULL;
	}

	if (!parse_type(p, &id->type))
		return NULL;
	next(p);
	if (p->tok.type != TOK_ASSIGN)
		goto end;

parse_body:
	p->cscope->expect_type = &id->type;
	parse_block(p, &stmt->u.fn.blk);
end:
	exit_scope(p);
	return stmt;
}

int
parse_fn_def_args(struct parser *p, struct zk_fn *fn)
{
	struct zk_ident *id;

	darr_init(&fn->args);
	do {
		next(p);
		if (p->tok.type == TOK_RPAREN)
			break;
		expect(p, TOK_IDENT);
		id = ecalloc(1, sizeof(*id));
		id->k = ZK_IDENT;
		id->name = id->realname = duptok(p);
		if (!parse_type(p, &id->type)) {
			free(id->name);
			free(id);
			return 1;
		}
		darr_append(&fn->args, id);
		darr_append(&p->cscope->idents, id);
		next(p);
	} while (p->tok.type == TOK_COMMA);

	if (p->tok.type != TOK_RPAREN)
		unexpected(&p->tok);

	return 0;
}

struct zk_stmt *
parse_ident_def_stmt(struct parser *p)
{
	struct zk_ident *id;
	struct semantics_ctx sema = {0};
	struct zk_stmt *stmt = ecalloc(1, sizeof(*stmt));
	struct zk_type *t;

	stmt->k = ZK_IDENT_DEF_STMT;

	next(p);
	id = find_ident(p->cscope, &STRTOK(&p->tok));
	if (id)
		throwf(p, "identifier '%.*s' defined", p->tok.len, p->tok.str);

	id = ecalloc(1, sizeof(*id));
	id->k = ZK_IDENT;
	id->name = id->realname = duptok(p);
	darr_append(&p->cscope->idents, id);
	parse_type(p, &id->type);

	next(p);
	expect(p, TOK_ASSIGN);
	
	stmt->u.ident_def.id = id;
	stmt->u.ident_def.val = parse_binary_expr(p);
	sema.expect = &id->type;
	sema.scope = p->cscope;
	t = analyze_expr_type(&sema, stmt->u.ident_def.val);
	if (!t)
		throw_semantics_err(p, -1);

	return stmt;
}

struct zk_val *
parse_ident_val(struct parser *p, struct zk_val *v)
{
	struct zk_expr *expr;
	struct zk_ident *id;

	next(p);
	id = find_ident(p->cscope, &STRTOK(&p->tok));
	if (id) {
		v->k = ZK_IDENT_VAL;
		v->u.id = id;
	} else {
		v->k = ZK_UNANALYZED_IDENT_VAL;
		v->u.unanalyzed_id = duptok(p);
	}

	next(p);
	peek(p);
	if (p->tok.type != TOK_LPAREN)
		return v;
	expr = parse_fn_call(p);
	if (!expr)
		return NULL;

	if (id) {
		expr->u.fn_call.fn = id;
	} else {
		expr->u.fn_call.unanalyzed_id = v->u.unanalyzed_id;
	}

	v->k = ZK_EXPR_VAL;
	v->u.expr = expr;
	return v;
}

struct zk_stmt *
parse_if_stmt(struct parser *p)
{
	struct zk_stmt *stmt = ecalloc(1, sizeof(*stmt));
	stmt->k = ZK_IF_STMT;
	stmt->u.if_stmt.cond = parse_binary_expr(p);
	parse_block(p, &stmt->u.if_stmt.then);

	next(p);
	if (p->tok.type != TOK_ELSE) {
		peek(p);
		return stmt;
	}

	parse_block(p, &stmt->u.if_stmt.else_);
	return stmt;
}

struct zk_top_stmt *
parse_impl_stmt(struct parser *p)
{
	struct zk_top_stmt *stmt, *top_stmt = ecalloc(1, sizeof(*top_stmt));
	struct zk_ident *struct_id, *trait_id, *id;
	struct zk_struct_type *struct_type;
	struct zk_impl_stmt *impl = &top_stmt->u.impl_stmt;

	top_stmt->k = ZK_IMPL_STMT;
	darr_init(&impl->stmts);
	next(p);
	trait_id = find_ident(p->cscope, &STRTOK(&p->tok));
	if (!trait_id)
		throwf(p, "trait '%.*s' not found", p->tok.len, p->tok.str);
	if (trait_id->k != ZK_TRAIT)
		throwf(p, "identifier '%.*s' is not trait", p->tok.len, p->tok.str);
	next(p);
	expect(p, TOK_FOR);
	next(p);
	struct_id = find_ident(p->cscope, &STRTOK(&p->tok));
	if (!struct_id)
		throwf(p, "struct '%.*s' not found", p->tok.len, p->tok.str);
	struct_type = &struct_id->type.u.struct_type;
	if (struct_type->generics) {
		enter_scope(p);
		next(p);
		expect(p, TOK_LBRACKET);
		next(p);
		id = ecalloc(1, sizeof(*id));
		id->name = id->realname = duptok(p);
		id->type.builtin = ZK_GENERIC_TYPE;
		id->k = ZK_TYPE_IDENT;
		impl->generic_id = id;
		darr_append(&p->cscope->idents, id);
		next(p);
		expect(p, TOK_RBRACKET);
	}
	next(p);
	expect(p, TOK_LBRACE);
	next(p);

	impl->struct_id = struct_id;
	impl->trait_id = trait_id;

	darr_init(&impl->scope.idents);
	impl->scope.parent = p->cscope;
	p->cscope = &impl->scope;

	p->name_prefix = impl->struct_id->realname;

	while (p->tok.type != TOK_RBRACE) {
		stmt = NULL;
		switch (p->tok.type) {
		case TOK_EOL:
			break;
		case TOK_IDENT:
			peek(p);
			stmt = parse_top_ident(p, 0);
			darr_append(&struct_type->members, darr_last(&p->cscope->idents));
			break;
		}
		if (stmt)
			darr_append(&impl->stmts, stmt);
		next(p);
	}

	if (analyze_trait_impl(impl))
		throw_semantics_err(p, -1);

	impl->struct_id = struct_id;
	impl->trait_id = trait_id;
	p->cscope = p->cscope->parent;

	p->name_prefix = NULL;

	if (struct_type->generics)
		exit_scope(p);

	return top_stmt;
}

struct zk_stmt *
parse_return_stmt(struct parser *p)
{
	struct zk_stmt *stmt = ecalloc(1, sizeof(*stmt));
	struct zk_type *type;
	struct semantics_ctx sema = {0};

	stmt->k = ZK_RETURN_STMT;
	stmt->u.return_stmt = parse_binary_expr(p);

	sema.expect = p->cscope->expect_type;
	sema.scope = p->cscope;
	type = analyze_expr_type(&sema, stmt->u.return_stmt);
	if (!type)
		throw_semantics_err(p, -1);
	if (!stmt->u.return_stmt) {
		free(stmt);
		return NULL;
	}
	return stmt;
}

struct zk_val *
parse_string_val(struct parser *p, struct zk_val *v)
{
	struct zk_arr_type *arr_type = &v->u.str.type.u.arr_type;
	next(p);
	v->k = ZK_STRING_VAL;
	v->u.str.s = duptok(p);
	v->u.str.type.builtin = ZK_CONST_STR_TYPE;
	arr_type->type = ecalloc(1, sizeof(*arr_type));
	arr_type->type->builtin = ZK_U8;
	arr_type->siz = strlen(v->u.str.s);
	return v;
}

struct zk_struct_type *
parse_struct_type(struct parser *p, struct zk_type *typ)
{
	struct zk_ident *id;
	struct zk_struct_type *st = &typ->u.struct_type;

	next(p);
	darr_init(&st->generic_instances);
	if (p->tok.type == TOK_LBRACKET) {
		enter_scope(p);
		next(p);
		id = ecalloc(1, sizeof(*id));
		id->name = id->realname = duptok(p);
		id->type.builtin = ZK_GENERIC_TYPE;
		id->k = ZK_TYPE_IDENT;
		st->generics = 1;
		darr_append(&p->cscope->idents, id);
		next(p);
		expect(p, TOK_RBRACKET);
		next(p);
	}
	expect(p, TOK_LBRACE);
	next(p);
	darr_init(&st->members);
	while (p->tok.type != TOK_RBRACE) {
		switch (p->tok.type) {
		case TOK_EOL:
			break;;
		case TOK_IDENT:
			id = ecalloc(1, sizeof(*id));
			id->k = ZK_IDENT;
			id->name = id->realname = duptok(p);
			parse_type(p, &id->type);
			darr_append(&st->members, id);
			break;
		}
		next(p);
	}

	if (st->generic_instances.n)
		exit_scope(p);

	return st;
}

struct zk_top_stmt *
parse_top_ident(struct parser *p, int pub)
{
	struct zk_ident *id = ecalloc(1, sizeof(*id));
	struct zk_top_stmt *stmt;

	next(p);
	id->name = id->realname = duptok(p);
	id->pub = pub;
	if (p->name_prefix)
		id->realname = codegen_get_realname(p->name_prefix, id->name);
	switch (pub) {
	case 0:
		darr_append(&p->cscope->idents, id);
		break;
	case 1:
		darr_append(&p->cscope->parent->idents, id);
		break;
	case 2:
		id->extern_ = 1;
		darr_append(&p->cscope->idents, id);
		break;
	}

	next(p);
	switch (p->tok.type) {
	case TOK_TRAIT:
		stmt = parse_trait_def(p, id);
		break;
	case TOK_TYPE:
		stmt = parse_type_def(p, id);
		break;
	case TOK_FN:
		stmt = parse_fn_def(p, id);
		break;
	default:
		unexpected(&p->tok);
		break;
	}

	return stmt;
}

struct zk_top_stmt *
parse_trait_def(struct parser *p, struct zk_ident *id)
{
	struct zk_top_stmt *stmt, *top_stmt = ecalloc(1, sizeof(*top_stmt));
	struct zk_trait *trait;
	if (p->where == IN_TRAIT_DEF)
		throw(p, "double trait define!");
	next(p);
	expect(p, TOK_ASSIGN);
	next(p);
	expect(p, TOK_LBRACE);
	next(p);

	id->k = ZK_TRAIT;
	top_stmt->k = ZK_TRAIT_DEF;
	top_stmt->u.trait_def.id = id;
	trait = &id->u.trait;
	darr_init(&top_stmt->u.trait_def.stmts);
	darr_init(&trait->scope.idents);

	trait->scope.parent = p->cscope;
	p->cscope = &trait->scope;
	p->where = IN_TRAIT_DEF;
	while (p->tok.type != TOK_RBRACE) {
		switch (p->tok.type) {
		case TOK_IDENT:
			peek(p);
			stmt = parse_top_ident(p, 0);
			darr_append(&top_stmt->u.trait_def.stmts, stmt);
			break;
		case TOK_EOL:
			break;
		}
		next(p);
	}
	p->cscope = p->cscope->parent;
	p->where = IN_TOP;
	return top_stmt;
}

struct zk_type *
parse_type(struct parser *p, struct zk_type *typ)
{
	enum ZK_BUILTIN_TYPE builtin;
again:
	next(p);
	switch (p->tok.type) {
	case TOK_MUT:
		typ->mutable = 1;
		goto again;
	case TOK_STAR:
		builtin = ZK_PTR;
		typ->u.type = ecalloc(1, sizeof(*typ->u.type));
		parse_type(p, typ->u.type);
		break;
	case TOK_SELF:
		builtin = ZK_SELF_TYPE;
		break;
	case TOK_STRUCT:
		builtin = ZK_STRUCT;
		parse_struct_type(p, typ);
		break;
	case TOK_U8: builtin = ZK_U8; break;
	case TOK_U16: builtin = ZK_U16; break;
	case TOK_U32: builtin = ZK_U32; break;
	case TOK_U64: builtin = ZK_U64; break;
	case TOK_I8: builtin = ZK_I8; break;
	case TOK_I16: builtin = ZK_I16; break;
	case TOK_I32: builtin = ZK_I32; break;
	case TOK_I64: builtin = ZK_I64; break;
	case TOK_LBRACKET:
		builtin = ZK_ARR;
		parse_arr_type(p, typ);
		break;
	case TOK_IDENT:
		peek(p);
		return parse_type_from_ident(p, typ);
	default:
		unexpected(&p->tok);
	}

	typ->builtin = builtin;
	return typ;
}

struct zk_top_stmt *
parse_type_def(struct parser *p, struct zk_ident *id)
{
	struct zk_top_stmt *stmt = ecalloc(1, sizeof(*stmt));

	id->k = ZK_TYPE_IDENT;

	next(p);
	if (p->tok.type != TOK_ASSIGN && p->where == IN_TRAIT_DEF) {
		id->type.builtin = ZK_ANY_TYPE;
		peek(p);
		return stmt;
	} else {
		expect(p, TOK_ASSIGN);
	}
	parse_type(p, &id->type);
	switch (id->type.builtin) {
	case ZK_STRUCT:
		stmt->k = ZK_STRUCT_DEF;
		stmt->u.struct_def = id;
		id->type.u.struct_type.id = id;
		break;
	case ZK_TYPE_REF:
		stmt->k = ZK_TYPE_ALIAS_STMT;
		break;
	default:
		throw(p, "unsupport");
	}

	return stmt;
}

struct zk_type *
parse_type_from_ident(struct parser *p, struct zk_type *typ)
{
	struct zk_ident *id;
	struct zk_generic_instance_type *instance;
	struct zk_struct_type *struct_type;
	struct semantics_ctx sema = {0};
	struct zk_type t;

	next(p);
	expect(p, TOK_IDENT);
	id = find_ident(p->cscope, &STRTOK(&p->tok));
	if (!id)
		throwf(p, "type identifier '%.*s' not found", p->tok.len, p->tok.str);
	if (id->k != ZK_TYPE_IDENT)
		throwf(p, "identifier '%.*s' is not a type identifier", p->tok.len, p->tok.str);

	typ->builtin = ZK_TYPE_REF;
	typ->u.type = &id->type;
	if (id->type.builtin != ZK_STRUCT)
		return typ;

	struct_type = &id->type.u.struct_type;
	if (!struct_type->generics)
		return typ;

	next(p);
	expect(p, TOK_LBRACKET);
	parse_type(p, &t);
	next(p);
	expect(p, TOK_RBRACKET);

	if (deref_type(&t)->builtin == ZK_GENERIC_TYPE) {
		typ->builtin = ZK_GENERIC_INSTANCE_TYPE;
		instance = ecalloc(1, sizeof(*instance));
		instance->instance = typ->u.type;
		instance->t = ecalloc(1, sizeof(*instance->t));
		instance->t->builtin = ZK_TYPE_REF;
		instance->t->u.type = deref_type(&t);
		typ->u.generic_instance_type = instance;
		return typ;
	}

	for (int i = 0; i < struct_type->generic_instances.n; i++) {
		sema.expect = struct_type->generic_instances.e[i]->t;
		if (check_type_full_equal(&sema, &t) == 0) {
			typ->builtin = ZK_TYPE_REF;
			typ->u.type = struct_type->generic_instances.e[i]->instance;
			return typ;
		}
	}

	instance = ecalloc(1, sizeof(*instance));
	instance->t = dup_type(&t);
	instance->instance = dup_type(&id->type);
	instance->instance->u.struct_type.cur_instance = instance->t;
	darr_append(&struct_type->generic_instances, instance);
	
	monomorphize_struct(&instance->instance->u.struct_type, instance->t);

	typ->builtin = ZK_GENERIC_INSTANCE_TYPE;
	typ->u.generic_instance_type = instance;
	return typ;
}

struct zk_val *
parse_val(struct parser *p)
{
	struct zk_val *v = ecalloc(1, sizeof(*v));
	next(p);
	switch (p->tok.type) {
	case TOK_AMP:
		v->k = ZK_EXPR_VAL;
		v->u.expr = parse_address_of_expr(p);
		break;
	case TOK_LBRACE:
		v->k = ZK_BRACE_INIT_VAL;
		parse_brace_init(p, &v->u.brace_init);
		break;
	case TOK_LPAREN:
		v->k = ZK_EXPR_VAL;
		v->u.expr = parse_binary_expr(p);
		next(p);
		expect(p, TOK_RPAREN);
		break;
	case TOK_IDENT:
		peek(p);
		return parse_ident_val(p, v);
	case TOK_INT:
		v->k = ZK_INT_VAL;
		if (p->tok.str[0] == '-') {
			v->u.i.i = utilsh_conv_a2i(p->tok.str, p->tok.len);
			v->u.i.type.builtin = analyze_cint_type(v->u.i.i);
		} else {
			v->u.i.i = utilsh_conv_a2ui(p->tok.str, p->tok.len);
			v->u.i.type.builtin = analyze_cint_type(v->u.i.i);
		}
		break;
	case TOK_STRING:
		peek(p);
		return parse_string_val(p, v);
	default:
		unexpected(&p->tok);
		break;
	}
	return v;
}

struct zk_mod *
parse(const char *path)
{
	struct codegen cg = {0};
	FILE *fp;
	struct zk_mod *mod;
	struct parser p = {0};
	int pub = 0;
	char *src = NULL;
	size_t src_siz = 0;
	struct zk_top_stmt *stmt;

	if (!(fp = fopen(path, "r")))
		die("fopen()\n");

	p.lexer.path = path;
	p.lexer.eol_tok = TOK_EOL;
	p.lexer.ident_tok = TOK_IDENT;
	p.lexer.int_tok = TOK_INT;
	p.lexer.string_tok = TOK_STRING;
	p.lexer.tokens = tokens;
	p.lexer.comments = comments;
	src_siz = sclexer_read_file(fp, &src);
	sclexer_init(&p.lexer, src);

	mod = ecalloc(1, sizeof(*mod));
	p.cscope = &p.scope;
	p.cscope->parent = &mod->scope;

	while (next(&p)) {
		stmt = NULL;
		switch (p.tok.type) {
		case TOK_EXTERN:
			stmt = parse_top_ident(&p, 2);
			break;
		case TOK_IMPL:
			stmt = parse_impl_stmt(&p);
			break;
		case TOK_PUB:
			if (pub)
				throw(&p, "double pub");
			pub = 1;
			break;
		case TOK_EOL:
			break;
		case TOK_IDENT:
			peek(&p);
			stmt = parse_top_ident(&p, pub);
			pub = 0;
			break;
		}
		if (stmt)
			darr_append(&cg.stmts, stmt);
	}

	codegen(&cg, stdout);

	return mod;
}

#define SCLEXER_IMPL
#include "sclexer.h"
