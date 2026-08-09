#include <assert.h>
#include <limits.h>
#include "die.h"
#include "semantics.h"
#include "zako.h"

int semantic_error;

static const char *semantics_err_str[] = {
	[SEMANTICS_IMMUTABLE] = "immutable type",
	[SEMANTICS_INCOMPATIBLE_IDENT] = "incompatible identifier",
	[SEMANTICS_INCOMPATIBLE_TYPE] = "incompatible type",
	[SEMANTICS_INCOMPLETE_TRAIT_IMPL] = "incomplete trait implement",
	[SEMANTICS_INIT_STRUCT_WITH_INT_IDX] = "init struct with integer index",
	[SEMANTICS_STRUCT_MEMBER_NOT_FOUND] = "struct member not found"
};

struct zk_type *
analyze_arr_brace_init_type(struct zk_type *expect,
		struct zk_brace_init *brace_init)
{
	struct zk_arr_type *arr_type = &expect->u.arr_type;
	struct zk_brace_init_member *member;
	struct zk_type *t;

	if (arr_type->siz) {
		if ((unsigned int)brace_init->members.n > arr_type->siz)
			return NULL;
	} else {
		arr_type->siz = brace_init->members.n;
	}

	brace_init->type = expect;
	for (int i = 0; i < brace_init->members.n; i++) {
		member = &brace_init->members.e[i];
		t = analyze_expr_type(arr_type->type, member->val);
		if (!t)
			return NULL;
	}

	return expect;
}

struct zk_type *
analyze_binary_expr_type(struct zk_type *expect, struct zk_binary_expr *expr)
{
	struct zk_type *lhst, *rhst, *t;

	if (expr->type)
		return expr->type;
	lhst = analyze_val_type(expect, expr->lhs);
	if (expr->rhs) {
		rhst = analyze_val_type(expect, expr->rhs);
		t = implicitly_convert_each_type(lhst, rhst);
		if (!t)
			return NULL;
	} else {
		t = lhst;
	}

	if (check_type(expect, t))
		return NULL;

	return t;
}

struct zk_type *
analyze_brace_init_type(struct zk_type *expect,
		struct zk_brace_init *brace_init)
{
	assert(expect);
	switch (expect->builtin) {
	case ZK_ARR:
		return analyze_arr_brace_init_type(expect, brace_init);
	case ZK_STRUCT:
		return analyze_struct_brace_init_type(expect, brace_init);
	default:
		return NULL;
	}
	return expect;
}

struct zk_type *
analyze_expr_type(struct zk_type *expect, struct zk_expr *expr)
{
	switch (expr->k) {
	case ZK_ADDRESS_OF_EXPR:
		return &expr->u.address_of.type;
	case ZK_BINARY_EXPR:
		return analyze_binary_expr_type(expect, &expr->u.binary);
	case ZK_FN_CALL_EXPR:
		return &expr->u.fn_call.fn->type;
	case ZK_IF_EXPR:
		return expr->u.if_expr.type;
	}
	return NULL;
}

struct zk_type *
analyze_struct_brace_init_type(struct zk_type *expect,
		struct zk_brace_init *brace_init)
{
	struct zk_brace_init_member *member;
	struct zk_ident *struct_member;
	struct zk_struct_type *struct_type = &expect->u.struct_type;
	struct zk_type *t;

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
		t = analyze_expr_type(&struct_member->type, member->val);
		if (!t)
			return NULL;
	}
	return expect;
}

enum ZK_BUILTIN_TYPE
analyze_uint_type(uint64_t i)
{
	if (i <= UINT8_MAX)
		return ZK_U8;
	if (i <= UINT16_MAX)
		return ZK_U16;
	if (i <= UINT32_MAX)
		return ZK_U32;
	if (i <= UINT64_MAX)
		return ZK_U64;
	die("analyze_uint_type()\n");
}

struct zk_type *
analyze_val_type(struct zk_type *expect, struct zk_val *val)
{
	switch (val->k) {
	case ZK_BRACE_INIT_VAL:
		return analyze_brace_init_type(expect, &val->u.brace_init);
	case ZK_EXPR_VAL:
		return analyze_expr_type(expect, val->u.expr);
	case ZK_IDENT_VAL:
		return &val->u.id->type;
	case ZK_INT_VAL:
		if (expect && IS_SIGNED_INTEGER(expect->builtin))
			val->u.i.type.builtin += 4;
		return &val->u.i.type;
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
	int ret;
	if (expect->k != src->k)
		return (semantic_error = SEMANTICS_INCOMPATIBLE_IDENT);
	if ((ret = check_type_full_equal(&expect->type, &src->type)))
		return ret;
	switch (expect->k) {
	case ZK_FN:
		ret = check_fn_ident_equal(expect, src);
		if (ret)
			return ret;
		break;
	case ZK_IDENT:
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
	if (!type->mutable)
		return (semantic_error = SEMANTICS_IMMUTABLE);
	return 0;
}

int
check_trait_impl(struct zk_impl_stmt *stmt)
{
	struct zk_ident *trait_id, *impl_id;
	int ret;
	struct zk_trait *trait = &stmt->trait_id->u.trait;

	if (trait->scope.idents.n != stmt->scope.idents.n)
		return (semantic_error = SEMANTICS_INCOMPLETE_TRAIT_IMPL);

	for (int i = 0; i < trait->scope.idents.n; i++) {
		trait_id = trait->scope.idents.e[i];
		impl_id = find_ident(&stmt->scope, &STR(trait_id->name, strlen(trait_id->name)));
		if (!impl_id)
			return (semantic_error = SEMANTICS_INCOMPLETE_TRAIT_IMPL);
		if ((ret = check_ident_equal(trait_id, impl_id)))
			return ret;
	}
	return 0;
}

int
check_type(struct zk_type *expect, struct zk_type *src)
{
	if (expect->builtin == src->builtin)
		return 0;
	if (!implicitly_convert_each_type(expect, src))
		return (semantic_error = SEMANTICS_INCOMPATIBLE_TYPE);
	return 0;
}

int
check_type_full_equal(struct zk_type *expect, struct zk_type *src)
{
	if (expect->builtin != src->builtin)
		goto err;
	if (IS_INTEGER(expect->builtin))
		return 0;
	switch (expect->builtin) {
	case ZK_PTR:
		return check_type_full_equal(expect->u.type, src->u.type);
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
