#include <stdio.h>
#include <string.h>
#include "csex.h"
#include "ealloc.h"

static void call_fn(struct codegen *cg, struct zk_fn_call *call);
static void dec(struct codegen *cg, struct zk_top_stmt *stmt);
static void dec_fn(struct codegen *cg, struct zk_ident *id);
static void dec_fn_args(struct codegen *cg, zk_idents_t *args);
static void dec_ident(struct codegen *cg, struct zk_ident *id);
static void dec_impl(struct codegen *cg, struct zk_impl_stmt *impl);
static void def(struct codegen *cg, struct zk_top_stmt *stmt);
static void def_fn(struct codegen *cg, struct zk_fn_def *fn);
static void def_impl(struct codegen *cg, struct zk_impl_stmt *impl);
static void def_struct(struct codegen *cg, struct zk_ident *structure);
static void put_address_of_expr(struct codegen *cg, struct zk_address_of_expr *addrof);
static void put_arr_type(struct codegen *cg, struct zk_arr_type *arr_type);
static void put_binary_expr(struct codegen *cg, struct zk_binary_expr *expr);
static void put_blk(struct codegen *cg, zk_block_t *blk);
static void put_brace_init(struct codegen *cg, struct zk_brace_init *brace_init);
static void put_expr(struct codegen *cg, struct zk_expr *expr);
static void put_ident(struct codegen *cg, const char *name);
static void put_indent(struct codegen *cg);
static void put_stmt(struct codegen *cg, struct zk_stmt *stmt);
static void put_struct_type(struct codegen *cg, struct zk_struct_type *struct_type);
static void put_type(struct codegen *cg, struct zk_type *type);
static void put_val(struct codegen *cg, struct zk_val *val);

static const char *src_file_head =
"#include <stdint.h>\n\n";

static const char *type2ctype[] = {
	[ZK_U8] = "uint8_t",
	[ZK_U16] = "uint16_t",
	[ZK_U32] = "uint32_t",
	[ZK_U64] = "uint64_t",
	[ZK_I8] = "int8_t",
	[ZK_I16] = "int16_t",
	[ZK_I32] = "int32_t",
	[ZK_I64] = "int64_t",
	NULL
};

static const char *strop[] = {
	[ZK_ADD] = "+",
	[ZK_DIV] = "/",
	[ZK_MUL] = "*",
	[ZK_SUB] = "-",
	[ZK_EQ] = "==",
	[ZK_GE] = ">=",
	[ZK_GT] = ">",
	[ZK_LE] = "<=",
	[ZK_LT] = "<",
	[ZK_ASSIGN] = "=",
	[ZK_ADD_ASSIGN] = "+=",
	[ZK_DIV_ASSIGN] = "/=",
	[ZK_MUL_ASSIGN] = "*=",
	[ZK_SUB_ASSIGN] = "-="
};

void
call_fn(struct codegen *cg, struct zk_fn_call *call)
{
	put_ident(cg, call->fn->name);
	fputc('(', cg->out);
	for (int i = 0; i < call->args.n; i++) {
		if (i)
			fputs(", ", cg->out);
		put_expr(cg, call->args.e[i]);
	}
	fputc(')', cg->out);
}

void
dec(struct codegen *cg, struct zk_top_stmt *stmt)
{
	switch (stmt->k) {
	case ZK_FN_DEF:
		dec_fn(cg, stmt->u.fn.id);
		break;
	case ZK_IMPL_STMT:
		dec_impl(cg, &stmt->u.impl_stmt);
		break;
	default:
		return;
	}
	fputc('\n', cg->out);
}

void
dec_fn(struct codegen *cg, struct zk_ident *id)
{
	if (!id->pub)
		fputs("static ", cg->out);
	dec_ident(cg, id);
	dec_fn_args(cg, &id->u.fn.args);
	fputs(";", cg->out);
}

void
dec_fn_args(struct codegen *cg, zk_idents_t *args)
{
	fputc('(', cg->out);
	for (int i = 0; i < args->n; i++) {
		if (i != 0)
			fputs(", ", cg->out);
		dec_ident(cg, args->e[i]);
	}
	fputc(')', cg->out);
}

void
dec_ident(struct codegen *cg, struct zk_ident *id)
{
	put_type(cg, &id->type);
	fputc(' ', cg->out);
	put_ident(cg, id->name);
}

void
dec_impl(struct codegen *cg, struct zk_impl_stmt *impl)
{
	cg->name_prefix = ecalloc(strlen(impl->struct_id->name) + 2, 1);
	strcpy(cg->name_prefix, impl->struct_id->name);
	strcat(cg->name_prefix, "__");
	for (int i = 0; i < impl->stmts.n; i++) {
		fputc('\n', cg->out);
		dec(cg, impl->stmts.e[i]);
	}
	free(cg->name_prefix);
	cg->name_prefix = NULL;
}

void
def(struct codegen *cg, struct zk_top_stmt *stmt)
{
	switch (stmt->k) {
	case ZK_FN_DEF:
		def_fn(cg, &stmt->u.fn);
		break;
	case ZK_IMPL_STMT:
		def_impl(cg, &stmt->u.impl_stmt);
		break;
	default:
		return;
	}
	fputc('\n', cg->out);
}

void
def_fn(struct codegen *cg, struct zk_fn_def *fn)
{
	put_type(cg, &fn->id->type);
	fputc('\n', cg->out);
	put_ident(cg, fn->id->name);
	dec_fn_args(cg, &fn->id->u.fn.args);
	fputc('\n', cg->out);
	put_blk(cg, &fn->blk);
}

void
def_impl(struct codegen *cg, struct zk_impl_stmt *impl)
{
	cg->name_prefix = ecalloc(strlen(impl->struct_id->name) + 2, 1);
	strcpy(cg->name_prefix, impl->struct_id->name);
	strcat(cg->name_prefix, "__");
	for (int i = 0; i < impl->stmts.n; i++) {
		fputc('\n', cg->out);
		def(cg, impl->stmts.e[i]);
	}
	free(cg->name_prefix);
	cg->name_prefix = NULL;
}

void
def_struct(struct codegen *cg, struct zk_ident *structure)
{
	struct zk_struct_type *st = &structure->type.u.struct_type;
	fputs("struct ", cg->out);
	put_ident(cg, structure->name);
	fputs(" {\n", cg->out);
	
	cg->blk_lv++;
	for (int i = 0; i < st->members.n; i++) {
		put_indent(cg);
		put_type(cg, &st->members.e[i]->type);
		fputc(' ', cg->out);
		fputs(st->members.e[i]->name, cg->out);
		fputs(";\n", cg->out);
	}
	cg->blk_lv--;
	put_indent(cg);
	fputs("};", cg->out);
}

void
put_address_of_expr(struct codegen *cg, struct zk_address_of_expr *addrof)
{
	fputc('&', cg->out);
	put_ident(cg, addrof->id->name);
}

void
put_arr_type(struct codegen *cg, struct zk_arr_type *arr_type)
{
	put_type(cg, arr_type->type);
	cg->arr_siz = arr_type->siz;
	cg->after_arr_type = 1;
}

void
put_binary_expr(struct codegen *cg, struct zk_binary_expr *expr)
{
	put_val(cg, expr->lhs);
	if (expr->op != ZK_OP_COUNT) {
		fputc(' ', cg->out);
		fputs(strop[expr->op], cg->out);
		fputc(' ', cg->out);
		if (expr->rhs)
			put_val(cg, expr->rhs);
	}
}

void
put_blk(struct codegen *cg, zk_block_t *blk)
{
	fputs("{\n", cg->out);
	cg->blk_lv++;

	for (int i = 0; i < blk->n; i++) {
		put_indent(cg);
		put_stmt(cg, blk->e[i]);
		fputc('\n', cg->out);
	}

	cg->blk_lv--;
	put_indent(cg);
	fputs("}", cg->out);
}

void
put_brace_init(struct codegen *cg, struct zk_brace_init *brace_init)
{
	struct zk_brace_init_member *member;
	fputc('{', cg->out);
	for (int i = 0; i < brace_init->members.n; i++) {
		member = &brace_init->members.e[i];
		if (i)
			fputs(", ", cg->out);
		switch (member->k) {
		case ZK_BRACE_INIT_BY_IDX:
			fputc('[', cg->out);
			fprintf(cg->out, "%u", member->idx.i);
			fputs("] = ", cg->out);
			break;
		case ZK_BRACE_INIT_BY_IDENT:
			fputc('.', cg->out);
			fputs(member->idx.ident, cg->out);
			fputs(" = ", cg->out);
			break;
		}
		put_expr(cg, member->val);
	}
	if (!brace_init->members.n)
		fputc('0', cg->out);
	fputc('}', cg->out);
}

void
put_expr(struct codegen *cg, struct zk_expr *expr)
{
	switch (expr->k) {
	case ZK_ADDRESS_OF_EXPR:
		put_address_of_expr(cg, &expr->u.address_of);
		break;
	case ZK_BINARY_EXPR:
		put_binary_expr(cg, &expr->u.binary);
		break;
	case ZK_FN_CALL_EXPR:
		call_fn(cg, &expr->u.fn_call);
		break;
	}
}

void
put_ident(struct codegen *cg, const char *name)
{
	if (cg->blk_lv == 0 && cg->name_prefix)
		fputs(cg->name_prefix, cg->out);
	fputs(name, cg->out);
	if (cg->after_arr_type) {
		fputc('[', cg->out);
		if (cg->arr_siz)
			fprintf(cg->out, "%u", cg->arr_siz);
		fputc(']', cg->out);
		cg->after_arr_type = 0;
	}
}

void
put_indent(struct codegen *cg)
{
	for (int i = 0; i < cg->blk_lv; i++)
		fputc('\t', cg->out);
}

void
put_stmt(struct codegen *cg, struct zk_stmt *stmt)
{
	switch (stmt->k) {
	case ZK_EXPR_STMT:
		put_expr(cg, stmt->u.expr_stmt);
		break;
	case ZK_IDENT_DEF_STMT:
		put_type(cg, &stmt->u.ident_def.id->type);
		fputc(' ', cg->out);
		put_ident(cg, stmt->u.ident_def.id->name);
		fputs(" = ", cg->out);
		put_expr(cg, stmt->u.ident_def.val);
		break;
	case ZK_IF_STMT:
		fputs("if (", cg->out);
		put_expr(cg, stmt->u.if_stmt.cond);
		fputs(") ", cg->out);
		put_blk(cg, &stmt->u.if_stmt.then);
		if (stmt->u.if_stmt.else_.n) {
			fputs(" else ", cg->out);
			put_blk(cg, &stmt->u.if_stmt.else_);
		}
		return;
	case ZK_RETURN_STMT:
		fputs("return ", cg->out);
		put_expr(cg, stmt->u.return_stmt);
		break;
	}
	fputc(';', cg->out);
}

void
put_struct_type(struct codegen *cg, struct zk_struct_type *struct_type)
{
	fputs("struct ", cg->out);
	put_ident(cg, struct_type->id->name);
}

void
put_type(struct codegen *cg, struct zk_type *type)
{
	switch (type->builtin) {
	case ZK_ARR:
		put_arr_type(cg, &type->u.arr_type);
		break;
	case ZK_PTR:
		put_type(cg, type->u.type);
		fputc('*', cg->out);
		break;
	case ZK_STRUCT:
		put_struct_type(cg, &type->u.struct_type);
		break;
	default:
		fputs(type2ctype[type->builtin], cg->out);
		break;
	}
}

void
put_val(struct codegen *cg, struct zk_val *val)
{
	switch (val->k) {
	case ZK_BRACE_INIT_VAL:
		put_brace_init(cg, &val->u.brace_init);
		break;
	case ZK_EXPR_VAL:
		fputc('(', cg->out);
		put_expr(cg, val->u.expr);
		fputc(')', cg->out);
		break;
	case ZK_IDENT_VAL:
		put_ident(cg, val->u.id->name);
		break;
	case ZK_INT_VAL: // use fprintf(), baka
		fprintf(cg->out, "%ld", val->u.i.i);
		break;
	}
}

void
codegen(struct codegen *cg, FILE *out)
{
	struct zk_top_stmt *stmt;
	cg->out = out;

	fputs(src_file_head, out);

	for (int i = 0; i < cg->stmts.n; i++) {
		stmt = cg->stmts.e[i];
		if (stmt->k == ZK_STRUCT_DEF) {
			def_struct(cg, stmt->u.struct_def);
			fputc('\n', out);
		}
	}

	for (int i = 0; i < cg->stmts.n; i++)
		dec(cg, cg->stmts.e[i]);

	for (int i = 0; i < cg->stmts.n; i++)
		def(cg, cg->stmts.e[i]);
}
