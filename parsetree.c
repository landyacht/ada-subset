#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "parsetree.h"
#include "parser.h"
#include "memotable.h"
#include "uint_set.h"
#include "token_store.h"
#include "scanner.h"

#define UIS_MAX_IN_BOUND(out, set, bound) \
	for (int i = bound; i >= j; i--) { \
		if (uis_ret_true == uis_contains(&set, i)) { \
			out = i; \
			break; \
		} \
	}

#define UIS_MIN_IN_BOUND(out, set, bound) \
	for (int i = j; i <= bound; i++) { \
		if (uis_ret_true == uis_contains(&set, i)) { \
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
	switch (mt_get_value(&the_table, idx, &out)) { \
	case mt_ret_alloc_fail: \
		puts("Failed to allocate space for copy of memo!"); \
		pt_failed = 1; \
		return NULL; \
	case mt_ret_not_found: \
		puts("Memotable the_table did not contain a needed memo at idx"); \
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
struct node_stmt            *build_stmt(int, int, int*);
struct node_proc_call       *build_proc_call(int, int, int*);
struct node_arg_list        *build_arg_list(int, int, int*);
struct node_logical_exp     *build_logical_exp(int, int, int*);
struct node_relational_exp  *build_relational_exp(int, int, int*);
struct node_arithmetic_exp  *build_arithmetic_exp(int, int, int*);
struct node_term            *build_term(int, int, int*);
struct node_factor          *build_factor(int, int, int*);
struct node_literal         *build_literal(int);

struct node_program *pt_create() {
	return build_program(0, token_count);
}

struct node_program *build_program(int j, int bound) {
#ifdef PARSETREE_DEBUG
	printf("build_program called, j = %d, bound = %d", j, bound);
#endif

	if (j == token_count) {
		return NULL;
	}

	struct uint_set memo;
	GET_NEEDED_MT(memo, PROGRAM_MT, j)

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
	struct node_ident end_name = build_ident(bound - 1);
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
	if (token_type_keyword_begin == maybe_begin) {
		result->var_decls = build_var_def_list(resume_at, bound - 3, &resume_at);
		has_decls = 1;
	}

	result->instructions = build_stmt_list(resume_at + 1, bound - 2, NULL);

	if (has_params) {
		if (has_decls) {
			result->variation = nv_withparams_withdecls;
		}
		else {
			result->variation = nv_withparams_nodecls;
		}
	}
	else {
		if (has_decls) {
			result->variation = nv_noparams_withdecls;
		}
		else {
			result->variation = nv_noparams_nodecls;
		}
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
	if (NULL == result->more) { 
		result->variation = nv_last;
	}
	else {
		result->variation = nv_partial;
	}

	return result;
}

struct node_ident_list build_ident_list(int j, int bound) {
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
	strcpy(result->name, ident_name.val_str);

	return result;
}

struct node_stmt_list *build_stmt_list(int j, int bound, int *resume_out) {
#ifdef PARSETREE_DEBUG
	printf("build_stmt_list called, j = %d, bound = %d\n", j, bound);
#endif

	
}











