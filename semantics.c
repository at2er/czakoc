#include <assert.h>
#include <limits.h>
#include "die.h"
#include "ealloc.h"
#include "semantics.h"
#include "zako.h"

int semantic_error;

static const char *semantics_err_str[] = {
	[SEMANTICS_IMMUTABLE] = "immutable type",
	[SEMANTICS_INCOMPATIBLE_IDENT] = "incompatible identifier",
	[SEMANTICS_INCOMPATIBLE_TYPE] = "incompatible type",
	[SEMANTICS_INCOMPLETE_TRAIT_IMPL] = "incomplete trait implement",
	[SEMANTICS_INIT_STRUCT_WITH_INT_IDX] = "init struct with integer index",
	[SEMANTICS_LHS_CANT_BE_NULL_IN_DOT_EXPR] = "lhs can't be null in dot expr",
	[SEMANTICS_STRUCT_MEMBER_NOT_FOUND] = "struct member not found"
};

struct zk_type *
analyze_arr_brace_init_type(struct semantics_ctx *ctx,
		struct zk_brace_init *brace_init)
{
	struct zk_arr_type *arr_type = &ctx->expect->u.arr_type;
	struct zk_brace_init_member *member;
	struct zk_type *t, *expect;

	if (arr_type->siz) {
		if ((unsigned int)brace_init->members.n > arr_type->siz)
			return NULL;
	} else {
		arr_type->siz = brace_init->members.n;
	}

	ctx->expect = expect = open_type(ctx->expect);

	brace_init->type = ctx->expect;
	for (int i = 0; i < brace_init->members.n; i++) {
		member = &brace_init->members.e[i];
		ctx->expect = arr_type->type;
		t = analyze_expr_type(ctx, member->val);
		if (!t)
			return NULL;
	}

	return expect;
}

struct zk_type *
analyze_binary_expr_type(struct semantics_ctx *ctx, struct zk_expr *expr)
{
	struct zk_binary_expr *binary = &expr->u.binary;
	struct zk_type *lhst, *rhst, *t, *expect;

	ctx->expect = expect = open_type(ctx->expect);
	if (binary->type)
		return binary->type;

	if (binary->op == ZK_DOT)
		return analyze_dot_expr_type(ctx, expr);

	lhst = analyze_val_type(ctx, binary->lhs);
	if (binary->rhs) {
		rhst = analyze_val_type(ctx, binary->rhs);
		t = implicitly_convert_each_type(lhst, rhst);
		if (!t)
			return NULL;
	} else {
		t = lhst;
	}

	ctx->expect = expect;
	if (check_type(ctx, t))
		return NULL;

	return t;
}

struct zk_type *
analyze_brace_init_type(struct semantics_ctx *ctx,
		struct zk_brace_init *brace_init)
{
	assert(ctx->expect);
	ctx->expect = open_type(ctx->expect);
	switch (ctx->expect->builtin) {
	case ZK_ARR:
		return analyze_arr_brace_init_type(ctx, brace_init);
	case ZK_STRUCT:
		return analyze_struct_brace_init_type(ctx, brace_init);
	default:
		return NULL;
	}
	return ctx->expect;
}

enum ZK_BUILTIN_TYPE
analyze_cint_type(uint64_t i)
{
	if (i <= UINT8_MAX)
		return ZK_CI8;
	if (i <= UINT16_MAX)
		return ZK_CI16;
	if (i <= UINT32_MAX)
		return ZK_CI32;
	if (i <= UINT64_MAX)
		return ZK_CI64;
	die("analyze_cint_type()\n");
}

struct zk_type *
analyze_dot_expr_type(struct semantics_ctx *ctx, struct zk_expr *expr)
{
	struct zk_binary_expr *binary = &expr->u.binary;
	struct zk_fn_call *fn_call = NULL;
	struct zk_ident *lhsid, *id;
	struct zk_type *lhst, *t, *expect;
	struct zk_struct_type *struct_type;
	char *name = NULL;

	ctx->expect = expect = open_type(ctx->expect);
	if (binary->lhs->k == ZK_UNANALYZED_IDENT_VAL) {
		semantic_error = SEMANTICS_LHS_CANT_BE_NULL_IN_DOT_EXPR;
		return NULL;
	} else if (binary->lhs->k != ZK_IDENT_VAL) {
		return NULL;
	}
	lhsid = binary->lhs->u.id;

	if (!(lhst = analyze_val_type(ctx, binary->lhs)))
		return NULL;
	lhst = open_type(lhst);
	if (!binary->rhs)
		return NULL;
	if (lhsid->k != ZK_IDENT)
		return NULL;
	if (lhst->builtin == ZK_PTR)
		lhst = lhst->u.type;
	lhst = open_type(lhst);
	switch (lhst->builtin) {
	case ZK_STRUCT:
		struct_type = &lhst->u.struct_type;
		break;
	default:
		return NULL;
	}

	if (binary->rhs->k == ZK_UNANALYZED_IDENT_VAL) {
		name = binary->rhs->u.unanalyzed_id;
		binary->rhs->k = ZK_IDENT_VAL;
	} else if (binary->rhs->k == ZK_IDENT_VAL) {
		name = binary->rhs->u.id->name;
	} else if (binary->rhs->k == ZK_EXPR_VAL) {
		if (binary->rhs->u.expr->k != ZK_FN_CALL_EXPR)
			return NULL;
		fn_call = &binary->rhs->u.expr->u.fn_call;
		name = fn_call->unanalyzed_id;
		fn_call->unanalyzed_id = NULL;
	} else {
		return NULL;
	}

	id = find_struct_member(struct_type, &STR(name, strlen(name)));
	if (!id)
		return NULL;
	t = open_type(&id->type);

	if (fn_call) {
		fn_call->fn = id;
		darr_expand(&fn_call->args);
		memmove(fn_call->args.e + 1, fn_call->args.e,
				sizeof(*fn_call->args.e) * (fn_call->args.n - 1));
		fn_call->args.e[0] = ecalloc(1, sizeof(struct zk_expr));
		fn_call->args.e[0]->k = ZK_ADDRESS_OF_EXPR;
		fn_call->args.e[0]->u.address_of.id = lhsid;
		fn_call->args.e[0]->u.address_of.type.builtin = ZK_PTR;
		fn_call->args.e[0]->u.address_of.type.u.type = &id->type;
	} else {
		binary->rhs->u.id = id;
	}

	ctx->expect = expect;
	if (check_type(ctx, t))
		return NULL;

	return t;
}

struct zk_type *
analyze_expr_type(struct semantics_ctx *ctx, struct zk_expr *expr)
{
	ctx->expect = open_type(ctx->expect);
	switch (expr->k) {
	case ZK_ADDRESS_OF_EXPR:
		return &expr->u.address_of.type;
	case ZK_BINARY_EXPR:
		return analyze_binary_expr_type(ctx, expr);
	case ZK_FN_CALL_EXPR:
		return &expr->u.fn_call.fn->type;
	case ZK_IF_EXPR:
		return expr->u.if_expr.type;
	}
	return NULL;
}

struct zk_type *
analyze_struct_brace_init_type(struct semantics_ctx *ctx,
		struct zk_brace_init *brace_init)
{
	struct zk_brace_init_member *member;
	struct zk_ident *struct_member;
	struct zk_struct_type *struct_type = &ctx->expect->u.struct_type;
	struct zk_type *t, *expect;

	ctx->expect = expect = open_type(ctx->expect);

	for (int i = 0; i < brace_init->members.n; i++) {
		member = &brace_init->members.e[i];
		if (member->k != ZK_BRACE_INIT_BY_IDENT) {
			semantic_error = SEMANTICS_INIT_STRUCT_WITH_INT_IDX;
			return NULL;
		}
		struct_member = find_struct_member(struct_type,
				&STR(member->idx.ident, strlen(member->idx.ident)));
		if (!struct_member) {
			semantic_error = SEMANTICS_STRUCT_MEMBER_NOT_FOUND;
			return NULL;
		}
		ctx->expect = open_type(&struct_member->type);
		t = analyze_expr_type(ctx, member->val);
		if (!t)
			return NULL;
	}
	return expect;
}

int
analyze_trait_impl(struct zk_impl_stmt *impl)
{
	struct zk_ident *trait_id, *impl_id;
	int ret;
	struct zk_trait *trait = &impl->trait_id->u.trait;

	if (trait->scope.idents.n != impl->scope.idents.n)
		return (semantic_error = SEMANTICS_INCOMPLETE_TRAIT_IMPL);

	for (int i = 0; i < trait->scope.idents.n; i++) {
		trait_id = trait->scope.idents.e[i];
		impl_id = find_ident(&impl->scope, &STR(trait_id->name, strlen(trait_id->name)));
		if (!impl_id)
			return (semantic_error = SEMANTICS_INCOMPLETE_TRAIT_IMPL);
		if ((ret = check_ident_equal(trait_id, impl_id)))
			return ret;
	}
	return 0;
}

struct zk_type *
analyze_val_type(struct semantics_ctx *ctx, struct zk_val *val)
{
	switch (val->k) {
	case ZK_BRACE_INIT_VAL:
		return analyze_brace_init_type(ctx, &val->u.brace_init);
	case ZK_EXPR_VAL:
		return analyze_expr_type(ctx, val->u.expr);
	case ZK_IDENT_VAL:
		return &val->u.id->type;
	case ZK_INT_VAL:
		return &val->u.i.type;
	case ZK_UNANALYZED_IDENT_VAL:
		return NULL;
	}
	return NULL;
}

int
check_binary_expr(struct zk_binary_expr *expr)
{
	struct zk_type *lhst = analyze_val_type(NULL, expr->lhs);
	if (IS_ASSIGN_EXPR(expr->op))
		return check_mutable(lhst);
	return 0;
}

int
check_fn_ident_equal(struct zk_ident *expect, struct zk_ident *src)
{
	struct zk_ident *expect_arg, *src_arg;
	struct zk_fn *expect_fn = &expect->u.fn, *src_fn = &src->u.fn;
	int ret;

	if (expect_fn->args.n != src_fn->args.n)
		return (semantic_error = SEMANTICS_INCOMPATIBLE_IDENT);
	for (int i = 0; i < expect_fn->args.n; i++) {
		expect_arg = expect_fn->args.e[i];
		src_arg = src_fn->args.e[i];
		if ((ret = check_ident_equal(expect_arg, src_arg)))
			return ret;
	}
	return 0;
}

int
check_ident_equal(struct zk_ident *expect, struct zk_ident *src)
{
	struct semantics_ctx ctx = {0};
	int ret;

	if (expect->k != src->k)
		return (semantic_error = SEMANTICS_INCOMPATIBLE_IDENT);
	ctx.expect = &expect->type;
	if ((ret = check_type_full_equal(&ctx, &src->type)))
		return ret;
	switch (expect->k) {
	case ZK_FN:
		ret = check_fn_ident_equal(expect, src);
		if (ret)
			return ret;
		break;
	case ZK_IDENT:
		break;
	case ZK_TYPE_IDENT:
		if (expect->type.builtin != ZK_ANY_TYPE)
			abort();
		break;
	default: /* unexpected path */
		abort();
		break;
	}
	return 0;
}

int
check_mutable(struct zk_type *type)
{
	type = open_type(type);
	if (!type->mutable)
		return (semantic_error = SEMANTICS_IMMUTABLE);
	return 0;
}

int
check_type(struct semantics_ctx *ctx, struct zk_type *src)
{
	struct zk_type *expect = open_type(ctx->expect);
	src = open_type(src);
	switch (expect->builtin) {
	case ZK_ANY_TYPE:
		return 0;
	case ZK_SELF_TYPE:
		return 0;
	default: break;
	}
	if (expect->builtin == src->builtin)
		return 0;
	if (!implicitly_convert_each_type(expect, src))
		return (semantic_error = SEMANTICS_INCOMPATIBLE_TYPE);
	return 0;
}

int
check_type_full_equal(struct semantics_ctx *ctx, struct zk_type *src)
{
	struct zk_type *expect = open_type(ctx->expect);
	src = open_type(src);
	switch (expect->builtin) {
	case ZK_ANY_TYPE:
		return 0;
	case ZK_SELF_TYPE:
		return 0;
	default: break;
	}
	if (IS_INTEGER(expect->builtin) && RANGE(src->builtin, ZK_CI8, ZK_CI64))
		return 0;
	if (expect->builtin != src->builtin)
		goto err;
	if (IS_INTEGER(expect->builtin))
		return 0;
	switch (expect->builtin) {
	case ZK_PTR:
		ctx->expect = expect->u.type;
		return check_type_full_equal(ctx, src->u.type);
	case ZK_STRUCT:
		die("TODO\n");
		break;
	default:
		goto err;
	}
err:
	return (semantic_error = SEMANTICS_INCOMPATIBLE_TYPE);
}

struct zk_type *
implicitly_convert_each_type(
		struct zk_type *lhs,
		struct zk_type *rhs)
{
	lhs = open_type(lhs);
	rhs = open_type(rhs);
	if (!IS_INTEGER(lhs->builtin) || !IS_INTEGER(rhs->builtin))
		return NULL;

	if (IS_SIGNED_INTEGER(lhs->builtin)) {
		if (!IS_SIGNED_INTEGER(rhs->builtin))
			return NULL;
		if (lhs->builtin >= rhs->builtin)
			return lhs;
		else
			return rhs;
	} else {
		if (IS_SIGNED_INTEGER(rhs->builtin))
			return NULL;
		if (lhs->builtin >= rhs->builtin)
			return lhs;
		else
			return rhs;
	}

	return NULL;
}

const char *
str_semantics_err(int error)
{
	if (error == -1)
		return semantics_err_str[semantic_error];
	return semantics_err_str[error];
}
