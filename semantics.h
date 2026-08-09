#ifndef CZAKOC_SEMANTICS_H
#define CZAKOC_SEMANTICS_H
#include "parser.h"
#include "zako.h"

enum {
	SEMANTICS_IMMUTABLE = 1,
	SEMANTICS_INCOMPATIBLE_IDENT,
	SEMANTICS_INCOMPATIBLE_TYPE,
	SEMANTICS_INCOMPLETE_TRAIT_IMPL,
	SEMANTICS_INIT_STRUCT_WITH_INT_IDX,
	SEMANTICS_STRUCT_MEMBER_NOT_FOUND
};

extern int semantic_error;

struct zk_type *analyze_arr_brace_init_type(struct zk_type *expect,
		struct zk_brace_init *brace_init);
struct zk_type *analyze_binary_expr_type(struct zk_type *expect,
		struct zk_binary_expr *expr);
struct zk_type *analyze_brace_init_type(struct zk_type *expect,
		struct zk_brace_init *brace_init);
struct zk_type *analyze_expr_type(struct zk_type *expect,
		struct zk_expr *expr);
struct zk_type *analyze_struct_brace_init_type(struct zk_type *expect,
		struct zk_brace_init *brace_init);
enum ZK_BUILTIN_TYPE analyze_uint_type(uint64_t i);
struct zk_type *analyze_val_type(struct zk_type *expect, struct zk_val *val);
int check_binary_expr(struct zk_binary_expr *expr);
int check_fn_ident_equal(struct zk_ident *expect, struct zk_ident *src);
int check_ident_equal(struct zk_ident *expect, struct zk_ident *src);
int check_mutable(struct zk_type *type);
int check_trait_impl(struct zk_impl_stmt *stmt);
int check_type(struct zk_type *expect, struct zk_type *src);
int check_type_full_equal(struct zk_type *expect, struct zk_type *src);
struct zk_type *implicitly_convert_each_type(
		struct zk_type *lhs,
		struct zk_type *rhs);
const char *str_semantics_err(int error);

#endif
