#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "parsetree.h"
#include "parser.h"
#include "memotable.h"
#include "uint_set.h"
#include "token_store.h"
#include "scanner.h"

#define UIS_MAX_IN_BOUND(out, set, bound) \
	for (int i = bound; i >= j; i--) { \
		if (uis_ret_true == uint_set_contains(&set, i)) { \
			out = i; \
			break; \
		} \
	}

#define UIS_MIN_IN_BOUND(out, set, bound) \
	for (int i = j; i <= bound; i++) { \
		if (uis_ret_true == uint_set_contains(&set, i)) { \
			out = i; \
			break; \
		} \
	}

#define ALLOC_NODE(out, type) \
	type *out = malloc(sizeof(type)); \
	if (NULL == out) { \
		puts("Failed to allocate space for type"); \
		pt_failed = 1; \
		return NULL; \
	}

#define GET_NEEDED_MEMO(out, the_table, idx) \
	switch (memotable_get_val(&the_table, idx, &out)) { \
	case mt_ret_alloc_fail: \
		puts("Failed to allocate space for copy of memo!"); \
		pt_failed = 1; \
		return NULL; \
	case mt_ret_not_found: \
		printf("Memotable " #the_table " did not contain a needed memo at %d\n", j); \
		pt_failed = 1; \
		return NULL; \
	}
	

#define CHECK_FAIL if (pt_failed) { return NULL; } else { return result; }

/* Failure indicator */
int pt_failed;

/* Pre-declarations
 * The third int* parameter on some of these routines lets the caller know how
 * many tokens were absorbed in building the node so it can pick back up at the
 * right index
 */
struct node_program         *build_program(int, int);
struct node_proc_def        *build_proc_def(int, int);
struct node_var_def_list    *build_var_def_list(int, int, int*);
struct node_ident_list      *build_ident_list(int, int);
struct node_ident           *build_ident(int);
struct node_stmt_list       *build_stmt_list(int, int, int*);
struct node_stmt            *build_stmt(int, int);
struct node_proc_call       *build_proc_call(int, int);
struct node_assignment      *build_assignment(int, int);
struct node_if_stmt         *build_if_stmt(int, int);
struct node_return          *build_return(int, int);
struct node_arg_list        *build_arg_list(int, int);
struct node_logical_exp     *build_logical_exp(int, int, int*);
struct node_relational_exp  *build_relational_exp(int, int);
struct node_arithmetic_exp  *build_arithmetic_exp(int, int, int*);
struct node_term            *build_term(int, int);
struct node_factor          *build_factor(int, int);
struct node_value           *build_value(int);

struct node_program *pt_create() {
	return build_program(0, token_count);
}

struct node_program *build_program(int j, int bound) {
#ifdef PARSETREE_DEBUG
	printf("build_program called, j = %d, bound = %d\n", j, bound);
#endif

	if (j == token_count) {
		return NULL;
	}

	struct uint_set memo;
	GET_NEEDED_MEMO(memo, PROGRAM_MT, j)

	ALLOC_NODE(result, struct node_program)

	int proc_bound;
	UIS_MIN_IN_BOUND(proc_bound, memo, bound)
	result->proc = build_proc_def(j, proc_bound);

	result->more = build_program(proc_bound, bound);
	result->variation = NULL == result->more ? nv_last : nv_partial;

	CHECK_FAIL
}

struct node_proc_def *build_proc_def(int j, int bound) {
#ifdef PARSETREE_DEBUG
	printf("build_proc_def called, j = %d, bound = %d\n", j, bound);
#endif

	ALLOC_NODE(result, struct node_proc_def)

	result->name = build_ident(j + 1);
	struct node_ident *end_name = build_ident(bound - 2);
	if (0 != strcmp(result->name->name, end_name->name)) {
		printf("End identifier (%s) does not match declared name for procedure '%s'!\n",
				end_name->name, result->name->name);
		pt_failed = 1;
		return NULL;
	}

	int has_params = 0, has_decls = 0;
	int resume_at = j + 3;

	enum token_type maybe_paren;
	token_store_get(j + 2, &maybe_paren, NULL, NULL);
	if (token_type_lparen == maybe_paren) {
		result->param_list = build_var_def_list(j + 3, bound - 5, &resume_at);
		resume_at += 2; /* skip closing paren and "is" keyword */
		has_params = 1;
	}

	enum token_type maybe_begin;
	token_store_get(resume_at, &maybe_begin, NULL, NULL);
	if (token_type_keyword_begin != maybe_begin) {
		result->var_decls = build_var_def_list(resume_at, bound - 3, &resume_at);
		has_decls = 1;
	}

	result->instructions = build_stmt_list(resume_at + 1, bound - 3, NULL);

	if (has_params) {
		result->variation |= nv_withparams;
	}
	if (has_decls) {
		result->variation |= nv_withdecls;
	}

	CHECK_FAIL
}

struct node_var_def_list *build_var_def_list(int j, int bound, int *resume_out) {
#ifdef PARSETREE_DEBUG
	printf("build_var_def_list called, j = %d, bound = %d\n", j, bound);
#endif

	if (j == bound) {
		return NULL;
	}

	ALLOC_NODE(result, struct node_var_def_list)

	struct uint_set memo;
	GET_NEEDED_MEMO(memo, VAR_DEF_LIST_MT, j)

	/* Tighten bound */
	if (resume_out != NULL) {
		UIS_MAX_IN_BOUND(bound, memo, bound)
		*resume_out = bound;
	}

	int this_bound;
	UIS_MIN_IN_BOUND(this_bound, memo, bound)
	result->type_name = build_ident(this_bound - 2);
	result->var_names = build_ident_list(j, this_bound - 3);

	result->more = build_var_def_list(this_bound, bound, NULL);
	result->variation = NULL == result->more ? nv_last : nv_partial;

	CHECK_FAIL
}

struct node_ident_list *build_ident_list(int j, int bound) {
#ifdef PARSETREE_DEBUG
	printf("build_ident_list called, j = %d, bound = %d\n", j, bound);
#endif

	ALLOC_NODE(result, struct node_ident_list)
	struct node_ident_list *cur = result;
	for (int i = j; i < bound - 1; i += 2) {
		cur->ident = build_ident(i);
		ALLOC_NODE(more, struct node_ident_list)
		cur->more = more;
		cur->variation = nv_partial;
		cur = more;
	}
	cur->ident = build_ident(bound - 1);
	cur->variation = nv_last;

	return result;
}

struct node_ident *build_ident(int j) {
#ifdef PARSETREE_DEBUG
	printf("build_ident called, j = %d\n", j);
#endif

	ALLOC_NODE(result, struct node_ident)

	union lexeme_value ident_name;
	token_store_get(j, NULL, NULL, &ident_name);
	int name_len = strlen(ident_name.val_str);
	result->name = malloc(sizeof(char) * (name_len + 1));
	strcpy(result->name, ident_name.val_str);

	return result;
}

struct node_stmt_list *build_stmt_list(int j, int bound, int *resume_out) {
#ifdef PARSETREE_DEBUG
	printf("build_stmt_list called, j = %d, bound = %d\n", j, bound);
#endif

	if (j == bound) {
		return NULL;
	}

	struct uint_set memo;
	GET_NEEDED_MEMO(memo, STMT_LIST_MT, j)

	/* Tighten bound */
	if (NULL != resume_out) {
		UIS_MAX_IN_BOUND(bound, memo, bound)
		*resume_out = bound;
	}

	ALLOC_NODE(result, struct node_stmt_list)

	int stmt_bound;
	UIS_MIN_IN_BOUND(stmt_bound, memo, bound)
	result->stmt = build_stmt(j, stmt_bound - 1);

	result->more = build_stmt_list(stmt_bound, bound, NULL);
	result->variation = NULL == result->more ? nv_last : nv_partial;

	CHECK_FAIL
}

struct node_stmt *build_stmt(int j, int bound) {
#ifdef PARSETREE_DEBUG
	printf("build_stmt called, j = %d, bound = %d\n", j, bound);
#endif

	struct uint_set memo;
	GET_NEEDED_MEMO(memo, STMT_MT, j)

	int stmt_end;
	UIS_MAX_IN_BOUND(stmt_end, memo, bound)

	if (stmt_end != bound) {
		printf("Disagreement between statement list bound and statement end (%d vs %d)!\n",
				bound, stmt_end);
		pt_failed = 1;
		return NULL;
	}

	ALLOC_NODE(result, struct node_stmt)

	/* Try each statement type */
	if (NULL != (result->stmt = build_proc_call(j, bound))) {
		result->variation = nv_call_stmt;
		CHECK_FAIL
	}

	if (NULL != (result->stmt = build_assignment(j, bound))) {
		result->variation = nv_assign_stmt;
		CHECK_FAIL
	}

	if (NULL != (result->stmt = build_return(j, bound))) {
		result->variation = nv_ret_stmt;
		CHECK_FAIL
	}

	if (NULL != (result->stmt = build_if_stmt(j, bound))) {
		result->variation = nv_if_stmt;
		CHECK_FAIL
	}

	printf("Could not create any valid statement node at j = %d!\n", j);
	pt_failed = 1;
	return NULL;
}

struct node_proc_call *build_proc_call(int j, int bound) {
#ifdef PARSETREE_DEBUG
	printf("build_proc_call called, j = %d, bound = %d\n", j, bound);
#endif

	struct uint_set memo;
	GET_NEEDED_MEMO(memo, PROC_CALL_MT, j)
	if (uis_ret_true == uint_set_isempty(&memo)) {
		return NULL;
	}

	int stmt_end;
	UIS_MAX_IN_BOUND(stmt_end, memo, bound)

	if (stmt_end != bound) {
		printf("Disagreement between general statement bound and procedure call statement end (%d vs %d)!\n",
				bound, stmt_end);
		pt_failed = 1;
		return NULL;
	}

	ALLOC_NODE(result, struct node_proc_call)

	result->proc_name = build_ident(j);
	result->arguments = build_arg_list(j + 2, bound - 1);

	CHECK_FAIL
}

struct node_assignment *build_assignment(int j, int bound) {
#ifdef PARSETREE_DEBUG
	printf("build_assignment called, j = %d, bound = %d\n", j, bound);
#endif

	struct uint_set memo;
	GET_NEEDED_MEMO(memo, ASSIGN_MT, j)

	if (uis_ret_true == uint_set_isempty(&memo)) {
		return NULL;
	}

	int stmt_end;
	UIS_MAX_IN_BOUND(stmt_end, memo, bound)
	if (stmt_end != bound) {
		printf("Disagreement between general statement bound and assignment statement end (%d vs %d)!\n",
				bound, stmt_end);
		pt_failed = 1;
		return NULL;
	}

	ALLOC_NODE(result, struct node_assignment)

	result->lhs = build_ident(j);
	
	int logical_exp_end, arithmetic_exp_end;
	result->rhs_log = build_logical_exp(j + 2, bound, &logical_exp_end);
	result->rhs_arith = build_arithmetic_exp(j + 2, bound, &arithmetic_exp_end);

	/* TODO deallocate the portion of the tree used for the type of expression we're not using */
	if (arithmetic_exp_end == bound) {
		result->variation = nv_arithmetic;
		CHECK_FAIL
	}
	else if (logical_exp_end == bound) {
		result->variation = nv_logical;
		CHECK_FAIL
	}
	else {
		printf("Assignment statement rhs could not be filled by logical nor arithmetic expression at j = %d!\n", j);
		pt_failed = 1;
		return NULL;
	}
}

struct node_return *build_return(int j, int bound) {
#ifdef PARSETREE_DEBUG
	printf("build_return called, j = %d, bound = %d\n", j, bound);
#endif

	struct uint_set memo;
	GET_NEEDED_MEMO(memo, RETURN_STMT_MT, j)
	if (uis_ret_true == uint_set_isempty(&memo)) {
		return NULL;
	}

	int stmt_end;
	UIS_MAX_IN_BOUND(stmt_end, memo, bound)

	if (stmt_end != bound) {
		printf("Disagreement between general statement bound and return statement end (%d vs %d)!\n",
				bound, stmt_end);
		pt_failed = 1;
		return NULL;
	}

	ALLOC_NODE(result, struct node_return)

	if (j + 1 == bound) {
		result->variation = nv_ret_noval;
		CHECK_FAIL
	}

	int logical_exp_end, arithmetic_exp_end;
	result->retval_log = build_logical_exp(j + 1, bound, &logical_exp_end);
	result->retval_arith = build_arithmetic_exp(j + 1, bound, &arithmetic_exp_end);

	/* TODO deallocate the portion of the tree used for the type of expression we're not using */
	if (arithmetic_exp_end == bound) {
		result->variation = nv_arithmetic;
		CHECK_FAIL;
	}
	else if (logical_exp_end == bound) {
		result->variation = nv_logical;
		CHECK_FAIL
	}
	else {
		printf("Return value could not be filled by logical nor arithmetic expression at j = %d!\n", j);
		pt_failed = 1;
		return NULL;
	}
}

struct node_if_stmt *build_if_stmt(int j, int bound) {
#ifdef PARSETREE_DEBUG
	printf("build_if_stmt called, j = %d, bound = %d\n", j, bound);
#endif

	struct uint_set memo;
	GET_NEEDED_MEMO(memo, IF_STMT_MT, j)

	if (uis_ret_true == uint_set_isempty(&memo)) {
		return NULL;
	}

	int stmt_end;
	UIS_MAX_IN_BOUND(stmt_end, memo, bound)
	if (stmt_end != bound) {
		printf("Disagreement between general statement bound and if statement end (%d vs %d)!\n",
				bound, stmt_end);
		pt_failed = 1;
		return NULL;
	}

	ALLOC_NODE(result, struct node_if_stmt)

	int cond_end;
	result->cond = build_logical_exp(j + 1, bound - 4, &cond_end);
	if (NULL == result->cond) {
		printf("Could not find condition in expected location (j = %d)!\n", j + 1);
		pt_failed = 1;
		return NULL;
	}

	int pos_end, neg_end;
	result->stmts_pos = build_stmt_list(cond_end + 1, bound - 2, &pos_end);

	if (pos_end == bound - 2) {
		result->variation = nv_if_noelse;
		CHECK_FAIL
	}

	result->stmts_neg = build_stmt_list(pos_end + 1, bound - 2, &neg_end);
	CHECK_FAIL
}

struct node_arg_list *build_arg_list(int j, int bound) {
#ifdef PARSETREE_DEBUG
	printf("build_arg_list called, j = %d, bound = %d\n", j, bound);
#endif

	if (j >= bound) {
		return NULL;
	}

	struct uint_set memo;
	GET_NEEDED_MEMO(memo, ARG_LIST_MT, j)

	ALLOC_NODE(result, struct node_arg_list)

	int logical_exp_end, arithmetic_exp_end, larger_exp_end;
	result->arg_log = build_logical_exp(j, bound, &logical_exp_end);
	result->arg_arith = build_arithmetic_exp(j, bound, &arithmetic_exp_end);
	
	if (logical_exp_end > arithmetic_exp_end) {
		result->variation = nv_logical;
		larger_exp_end = logical_exp_end;
	}
	else {
		result->variation = nv_arithmetic;
		larger_exp_end = arithmetic_exp_end;
	}

	result->more = build_arg_list(arithmetic_exp_end + 1, bound);
	if (NULL == result->more) {
		result->variation |= nv_last;
	}
	else {
		result->variation |= nv_partial;
	}

	CHECK_FAIL
}

struct node_logical_exp *build_logical_exp(int j, int bound, int *resume_out) {
#ifdef PARSETREE_DEBUG
	printf("build_logical_exp called, j = %d, bound = %d\n", j, bound);
#endif

	struct uint_set memo;
	GET_NEEDED_MEMO(memo, LOGICAL_EXP_MT, j)

	int new_bound = -1;
	UIS_MAX_IN_BOUND(new_bound, memo, bound)
	if (-1 == new_bound) {
		return NULL;
	}
	bound = new_bound;

	if (NULL != resume_out) {
		*resume_out = bound;
	}

	ALLOC_NODE(result, struct node_logical_exp)
	
	enum token_type maybe_lparen, maybe_rparen;
	token_store_get(j, &maybe_lparen, NULL, NULL);
	token_store_get(bound - 1, &maybe_rparen, NULL, NULL);
	if (token_type_lparen == maybe_lparen && token_type_rparen == maybe_rparen) {
		result->inner_exp = build_logical_exp(j + 1, bound - 1, NULL);
		if (NULL == result->inner_exp) {
			printf("Could not find logical expression inside parentheses (j = %d)!\n", j + 1);
			pt_failed = 1;
			return NULL;
		}
		result->variation = nv_parenthesized;
		CHECK_FAIL
	}

	struct uint_set rel_memo;
	if (mt_ret_success == memotable_get_val(&RELATIONAL_EXP_MT, j, &rel_memo)) {
		if (uis_ret_true == uint_set_contains(&rel_memo, bound)) {
			result->rel_exp = build_relational_exp(j, bound);
			result->variation = nv_simple;
			CHECK_FAIL
		}
		uint_set_destroy(&rel_memo);
	}

	for (int i = j + 2; i < bound; i++) {
		if (mt_ret_success == memotable_get_val(&RELATIONAL_EXP_MT, i, &rel_memo)) {
			if (uis_ret_true == uint_set_contains(&rel_memo, bound)) {
				result->rel_exp = build_relational_exp(i, bound);
				result->inner_exp = build_logical_exp(j, i - 1, NULL);
				token_store_get(i - 1, NULL, &result->op, NULL);
				result->variation = nv_binary;
				uint_set_destroy(&rel_memo);
				CHECK_FAIL
			}
			uint_set_destroy(&rel_memo);
		}
	}

	puts("Failed to build logical expression as parenthesized, as simple, and as binary!");
	pt_failed = 1;
	return NULL;
}

struct node_relational_exp *build_relational_exp(int j, int bound) {
#ifdef PARSETREE_DEBUG
	printf("build_relational_exp called, j = %d, bound = %d\n", j, bound);
#endif

	ALLOC_NODE(result, struct node_relational_exp)
	if (j + 1 == bound) {
		result->variation = nv_simple;
		if (NULL == (result->val = build_value(j))) {
			printf("Could not build value inside relational exp at j = %d!\n", j);
			pt_failed = 1;
			return NULL;
		}
		CHECK_FAIL
	}

	struct uint_set memo;
	GET_NEEDED_MEMO(memo, RELATIONAL_EXP_MT, j)

	result->variation = nv_binary;
	int resume;
	result->lhs = build_arithmetic_exp(j, bound, &resume);
	token_store_get(resume, NULL, &result->op, NULL);
	result->rhs = build_arithmetic_exp(resume + 1, bound, NULL);

	if (NULL == result->lhs || NULL == result->rhs) {
		printf("Either right- or left-hand side of binary relational expression failed to be created!\n");
		pt_failed = 1;
		return NULL;
	}

	CHECK_FAIL
}

struct node_arithmetic_exp *build_arithmetic_exp(int j, int bound, int *resume_out) {
#ifdef PARSETREE_DEBUG
	printf("build_arithmetic_exp called, j = %d, bound = %d\n", j, bound);
#endif

	struct uint_set memo;
	GET_NEEDED_MEMO(memo, ARITHMETIC_EXP_MT, j)

	int new_bound = -1;
	UIS_MAX_IN_BOUND(new_bound, memo, bound)
	if (-1 == new_bound) {
		return NULL;
	}
	bound = new_bound;

	if (NULL != resume_out) {
		*resume_out = bound;
	}

	ALLOC_NODE(result, struct node_arithmetic_exp)

	struct uint_set term_memo;
	if (mt_ret_success == memotable_get_val(&TERM_MT, j, &term_memo)) {
		if (uis_ret_true == uint_set_contains(&term_memo, bound)) {
			result->term = build_term(j, bound);
			result->variation = nv_simple;
			uint_set_destroy(&term_memo);
			CHECK_FAIL
		}
		uint_set_destroy(&term_memo);
	}

	for (int i = j + 2; i < bound; i++) {
		if (mt_ret_success == memotable_get_val(&TERM_MT, i, &term_memo)) {
			if (uis_ret_true == uint_set_contains(&term_memo, bound)) {
				result->term = build_term(i, bound);
				result->inner_exp = build_arithmetic_exp(j, i - 1, NULL);
				token_store_get(i - 1, NULL, &result->op, NULL);
				printf("Arith op = %d\n", result->op);
				result->variation = nv_binary;
				CHECK_FAIL
			}
			uint_set_destroy(&term_memo);
		}
	}

	puts("Failed to build arithmetic expression as simple and as binary!");
	pt_failed = 1;
	return NULL;
}

struct node_term *build_term(int j, int bound) {
#ifdef PARSETREE_DEBUG
	printf("build_term called, j = %d, bound = %d\n", j, bound);
#endif

	struct uint_set memo;
	GET_NEEDED_MEMO(memo, TERM_MT, j)

	int new_bound = -1;
	UIS_MAX_IN_BOUND(new_bound, memo, bound)
	if (-1 == new_bound) {
		return NULL;
	}
	bound = new_bound;

	ALLOC_NODE(result, struct node_term)

	struct uint_set fac_memo;
	if (mt_ret_success == memotable_get_val(&FACTOR_MT, j, &fac_memo)) {
		if (uis_ret_true == uint_set_contains(&fac_memo, bound)) {
			result->factor = build_factor(j, bound);
			result->variation = nv_simple;
			uint_set_destroy(&fac_memo);
			CHECK_FAIL
		}
		uint_set_destroy(&fac_memo);
	}

	for (int i = j + 2; i < bound; i++) {
		if (mt_ret_success == memotable_get_val(&FACTOR_MT, i, &fac_memo)) {
			if (uis_ret_true == uint_set_contains(&fac_memo, bound)) {
				result->factor = build_factor(i, bound);
				token_store_get(i - 1, NULL, &result->op, NULL);
				result->inner_term = build_term(j, i - 1);
				result->variation = nv_binary;
				CHECK_FAIL
			}
			uint_set_destroy(&fac_memo);
		}
	}

	puts("Failed to build term as simple and as binary!");
	pt_failed = 1;
	return NULL;
}

struct node_factor *build_factor(int j, int bound) {
#ifdef PARSETREE_DEBUG
	printf("build_factor called, j = %d, bound = %d\n", j, bound);
#endif

	struct uint_set memo;
	GET_NEEDED_MEMO(memo, FACTOR_MT, j)

	ALLOC_NODE(result, struct node_factor)

	enum token_type maybe_lparen, maybe_rparen;
	enum token_subtype maybe_minus;
	token_store_get(j, &maybe_lparen, &maybe_minus, NULL);
	token_store_get(bound - 1, &maybe_rparen, NULL, NULL);
	if (token_type_lparen == maybe_lparen && token_type_rparen == maybe_rparen) {
		result->inner_exp = build_arithmetic_exp(j + 1, bound - 1, NULL);
		if (NULL == result->inner_exp) {
			printf("Could not find arithmetic expression inside parentheses (j = %d)!\n", j + 1);
			pt_failed = 1;
			return NULL;
		}
		result->variation = nv_parenthesized;
		CHECK_FAIL
	}

	if (token_subtype_minus == maybe_minus) {
		result->neg_factor = build_factor(j + 1, bound);
		result->variation = nv_fac_negative;
		CHECK_FAIL
	}

	result->val = build_value(j);
	result->variation = nv_simple;
	CHECK_FAIL
}

struct node_value *build_value(int j) {
#ifdef PARSETREE_DEBUG
	printf("build_value called, j = %d\n", j);
#endif

	struct uint_set memo;
	GET_NEEDED_MEMO(memo, VALUE_MT, j)
	if (uis_ret_true == uint_set_isempty(&memo)) {
		printf("No value at j = %d!\n", j);
		pt_failed = 1;
		return NULL;
	}

	ALLOC_NODE(result, struct node_value)

	token_store_get(j, &result->lit_type, NULL, &result->val);
	if (token_type_identifier == result->lit_type) {
		result->ident = build_ident(j);
		result->variation = nv_val_ident;
		CHECK_FAIL
	}

	result->variation = nv_val_literal;
	CHECK_FAIL
}
