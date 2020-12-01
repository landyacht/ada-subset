#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "parser.h"
#include "memotable.h"
#include "uint_set.h"
#include "scanner.h"
#include "token_store.h"

struct uint_set rule_program(int);
struct uint_set rule_proc_def(int);
struct uint_set rule_var_def_list(int);
struct uint_set rule_ident_list(int);
struct uint_set rule_stmt_list(int);
struct uint_set rule_stmt(int);
struct uint_set rule_proc_call(int);
struct uint_set rule_arg_list(int);
struct uint_set rule_assign(int);
struct uint_set rule_if_stmt(int);
struct uint_set rule_return_stmt(int);
struct uint_set rule_logical_exp(int);
struct uint_set rule_relational_exp(int);
struct uint_set rule_arithmetic_exp(int);
struct uint_set rule_term(int);
struct uint_set rule_factor(int);
struct uint_set rule_value(int);

#define RULE_FAIL(table) \
	struct uint_set empty; \
	uint_set_create(&empty, token_count); \
	memotable_store(&table, j, &empty); \
	return empty;

#define CHECK_MEMOTABLE(table) \
	struct uint_set check_result; \
	enum mt_ret get_val_rv = memotable_get_val(&table, j, &check_result); \
	if (mt_ret_success == get_val_rv) { \
		return check_result; \
	} \
	else if (mt_ret_alloc_fail == get_val_rv) { \
		puts("Failed to allocate space for copy of uint_set stored in memotable!"); \
		RULE_FAIL(table) \
	} \
	if (memotable_get_ct(&table, j) > token_count - j) { \
		RULE_FAIL(table) \
	} \
	memotable_inc_ct(&table, j, NULL);

#define UPDATE_MEMOTABLE(table) \
	memotable_store(&table, j, &result);

#define TERMINAL_RULE(wanted_token_type, set) \
	for (int i = token_count - 1; i >= j; i--) { \
		if (uis_ret_true == uint_set_contains(&set, i)) { \
			uint_set_remove(&set, i); \
			enum token_type this_token_type; \
			enum ts_ret ts_rv = token_store_get(i, &this_token_type, NULL, NULL); \
			if (ts_ret_success == ts_rv && wanted_token_type == this_token_type) { \
				uint_set_add(&set, i + 1); \
			} \
		} \
	}

#define NONTERMINAL_RULE(rule, set) { \
	struct uint_set copy; \
	uint_set_copy(&copy, &set); \
	uint_set_clear(&set); \
	for (int i = j; i < token_count; i++) { \
		if (uis_ret_true == uint_set_contains(&copy, i)) { \
			struct uint_set next_res = rule(i); \
			uint_set_union_with(&set, &next_res); \
		} \
	} \
	uint_set_destroy(&copy); \
}

int *parse(char *filename) {
	if (lexer_ret_fopen_fail == lexer_init(filename)) {
		printf("fopen() failed while initializing lexer: %s\n", lexer_current_err);
		return parser_ret_open_fail;
	}

	if (ts_ret_alloc_fail == token_store_init()) {
		printf("Token store failed to initialize due to memory allocation issues!\n");
		return parser_ret_alloc_fail;
	}

	printf("Reading and storing tokens... \n");
	token_count = 0;
	bool keep_going = true;
	while (keep_going) {
		enum lexer_ret next_ret = lexer_next();
		switch (next_ret) {
		case lexer_ret_success:
			token_count++;
			if (token_type_identifier == current_token_type) {
				current_value.val_str = malloc((strlen(current_lexeme) + 1) * sizeof(char));
				strcpy(current_value.val_str, current_lexeme);
			}
			if (ts_ret_alloc_fail == token_store_add(current_token_type, current_token_subtype, current_value)) {
				puts("Token store failed to allocate memory!");
				return parser_ret_alloc_fail;
			}
			break;
		case lexer_ret_fread_fail:
			printf("Lexer failed to read the file: %s\n", lexer_current_err);
			return parser_ret_read_fail;
		case lexer_ret_invalid:
			printf("Lexer encountered malformed input: %s\n", lexer_current_err);
			return parser_ret_lex_fail;
		case lexer_ret_eof:
			keep_going = false;
			break;
		}
	}

	/* Set up memotables */
	size_t mt_size = token_count / 6;
	for (int i = 0; i < NUM_RULES; i++) {
		enum mt_ret create_rv = memotable_create(&memotables[i], mt_size);
		if (mt_ret_alloc_fail == create_rv) {
			printf("Failed to allocate memory for memotable %d\n", i);
			return parser_ret_alloc_fail;
		}
	}

	struct uint_set result = rule_program(0);
	if (uis_ret_true == uint_set_contains(&result, token_count)) {
		return parser_ret_success;
	}

	return parser_ret_partial;
}

struct uint_set rule_program(int j) {
#ifdef PARSER_DEBUG
	printf("rule_program visited, j = %d\n", j);
#endif

	CHECK_MEMOTABLE(PROGRAM_MT)

	/* <proc_def> ";" */
	struct uint_set result = rule_proc_def(j);
	TERMINAL_RULE(token_type_statement_sep, result);

	/* ... <program> */
	struct uint_set result_cont;
	uint_set_copy(&result_cont, &result);
	NONTERMINAL_RULE(rule_program, result_cont)
	uint_set_union_with(&result, &result_cont);
	uint_set_destroy(&result_cont);

	UPDATE_MEMOTABLE(PROGRAM_MT)
	return result;
}

struct uint_set rule_proc_def(int j) {
#ifdef PARSER_DEBUG
	printf("rule_proc_def visited, j = %d\n", j);
#endif

	CHECK_MEMOTABLE(PROC_DEF_MT)

	enum token_type type_under_cursor;
	enum ts_ret ts_rv = token_store_get(j, &type_under_cursor, NULL, NULL);
	/* There can be no match if the token at j is not the procedure keyword */
	if (ts_ret_oob == ts_rv || token_type_keyword_proc != type_under_cursor) {
		printf("Expected keyword_proc (%d) @ %d but got %d\n", token_type_keyword_proc,
				j, type_under_cursor);
		RULE_FAIL(PROC_DEF_MT)
	}
	
	/* There can be no match is the token at j + 1 is not an identifier */
	ts_rv = token_store_get(j + 1, &type_under_cursor, NULL, NULL);
	if (ts_ret_oob == ts_rv || token_type_identifier != type_under_cursor) {
		printf("Expected identifier (%d) @ %d but got %d\n", token_type_identifier,
				j + 1, type_under_cursor);
		RULE_FAIL(PROC_DEF_MT)
	}

	struct uint_set result;
	ts_rv = token_store_get(j + 2, &type_under_cursor, NULL, NULL);
	if (ts_ret_success == ts_rv) {
		/* --- RHS for procedures WITHOUT parameters --- */
		if (token_type_keyword_is == type_under_cursor) {
			uint_set_create(&result, token_count);
			uint_set_add(&result, j + 3);
		}
		/* --- RHS for procedures WITH parameters --- */
		else if (token_type_lparen == type_under_cursor) {
			result = rule_var_def_list(j + 3);
			TERMINAL_RULE(token_type_rparen, result)
			TERMINAL_RULE(token_type_keyword_is, result)
		}
		else {
			RULE_FAIL(PROC_DEF_MT)
		}
		
		struct uint_set with_decls;
		uint_set_copy(&with_decls, &result);
		NONTERMINAL_RULE(rule_var_def_list, with_decls)
		uint_set_union_with(&result, &with_decls);
		uint_set_destroy(&with_decls);

		TERMINAL_RULE(token_type_keyword_begin, result)
		NONTERMINAL_RULE(rule_stmt_list, result)
		TERMINAL_RULE(token_type_keyword_end, result)
		TERMINAL_RULE(token_type_identifier, result)
	}
	else {
		RULE_FAIL(PROC_DEF_MT)
	}

	UPDATE_MEMOTABLE(PROC_DEF_MT)
	return result;
}

struct uint_set rule_var_def_list(int j) {
#ifdef PARSER_DEBUG
	printf("rule_var_def visited, j = %d\n", j);
#endif

	CHECK_MEMOTABLE(VAR_DEF_LIST_MT)

	/* <ident_list> ":" <ident> ";" */
	struct uint_set result = rule_ident_list(j);
	TERMINAL_RULE(token_type_colon, result)
	TERMINAL_RULE(token_type_identifier, result)
	TERMINAL_RULE(token_type_statement_sep, result)

	/* ... <var_def_list> */
	struct uint_set result_cont;
	uint_set_copy(&result_cont, &result);
	NONTERMINAL_RULE(rule_var_def_list, result_cont)

	/* combine possible results */
	uint_set_union_with(&result, &result_cont);
	uint_set_destroy(&result_cont);

	UPDATE_MEMOTABLE(VAR_DEF_LIST_MT)
	return result;
}

struct uint_set rule_ident_list(int j) {
#ifdef PARSER_DEBUG
	printf("rule_ident_list visited, j = %d\n", j);
#endif

	CHECK_MEMOTABLE(IDENT_LIST_MT)

	struct uint_set result;
	uint_set_create(&result, token_count);

	/* <ident> */
	enum token_type type_under_cursor;
	enum ts_ret ts_rv = token_store_get(j, &type_under_cursor, NULL, NULL);
	if (ts_ret_success == ts_rv && token_type_identifier == type_under_cursor) {
		uint_set_add(&result, j + 1);
	}
	else {
		RULE_FAIL(IDENT_LIST_MT)
	}

	/* ... "," <ident_list> */
	ts_rv = token_store_get(j + 1, &type_under_cursor, NULL, NULL);
	if (ts_ret_success == ts_rv && token_type_comma == type_under_cursor) {
		struct uint_set result_cont = rule_ident_list(j + 2);
		uint_set_union_with(&result, &result_cont);
		uint_set_destroy(&result_cont);
	}

	UPDATE_MEMOTABLE(IDENT_LIST_MT)
	return result;
}

struct uint_set rule_stmt_list(int j) {
#ifdef PARSER_DEBUG
	printf("rule_stmt_list visited, j = %d\n", j);
#endif

	CHECK_MEMOTABLE(STMT_LIST_MT)

	/* <stmt> ";" */
	struct uint_set result = rule_stmt(j);
	TERMINAL_RULE(token_type_statement_sep, result)
	
	/* ... <stmt_list> */
	struct uint_set result_cont;
	uint_set_copy(&result_cont, &result);
	NONTERMINAL_RULE(rule_stmt_list, result_cont);
	uint_set_union_with(&result, &result_cont);
	uint_set_destroy(&result_cont);

	UPDATE_MEMOTABLE(STMT_LIST_MT)
	return result;
}

struct uint_set rule_stmt(int j) {
#ifdef PARSER_DEBUG
	printf("rule_stmt visited, j = %d\n", j);
#endif

	CHECK_MEMOTABLE(STMT_MT)

	/* <proc_call> */
	struct uint_set result = rule_proc_call(j);
	/* <assign> */
	struct uint_set result_alt = rule_assign(j);
	uint_set_union_with(&result, &result_alt);
	uint_set_destroy(&result_alt);
	/* <if_stmt> */
	result_alt = rule_if_stmt(j);
	uint_set_union_with(&result, &result_alt);
	uint_set_destroy(&result_alt);
	/* <return_stmt> */
	result_alt = rule_return_stmt(j);
	uint_set_union_with(&result, &result_alt);
	uint_set_destroy(&result_alt);

	UPDATE_MEMOTABLE(STMT_MT)
	return result;
}

struct uint_set rule_proc_call(int j) {
#ifdef PARSER_DEBUG
	printf("rule_proc_call visited, j = %d\n", j);
#endif

	CHECK_MEMOTABLE(PROC_CALL_MT)

	enum token_type type_under_cursor;
	enum ts_ret ts_rv = token_store_get(j, &type_under_cursor, NULL, NULL);
	if (ts_ret_oob == ts_rv || token_type_identifier != type_under_cursor) {
		RULE_FAIL(PROC_CALL_MT)
	}

	ts_rv = token_store_get(j + 1, &type_under_cursor, NULL, NULL);
	if (ts_ret_oob == ts_rv || token_type_lparen != type_under_cursor) {
		RULE_FAIL(PROC_CALL_MT)
	}

	struct uint_set result = rule_arg_list(j + 2);
	TERMINAL_RULE(token_type_rparen, result);

	UPDATE_MEMOTABLE(PROC_CALL_MT)
	return result;
}

struct uint_set rule_arg_list(int j) {
#ifdef PARSER_DEBUG
	printf("rule_arg_list visited, j = %d\n", j);
#endif

	CHECK_MEMOTABLE(ARG_LIST_MT)

	struct uint_set result_log = rule_logical_exp(j);
	struct uint_set result_log_copy;
	uint_set_copy(&result_log_copy, &result_log);
	TERMINAL_RULE(token_type_comma, result_log_copy)
	NONTERMINAL_RULE(rule_arg_list, result_log_copy)
	uint_set_union_with(&result_log, &result_log_copy);
	uint_set_destroy(&result_log_copy);

	struct uint_set result_arith = rule_arithmetic_exp(j);
	struct uint_set result_arith_copy;
	uint_set_copy(&result_arith_copy, &result_arith);
	TERMINAL_RULE(token_type_comma, result_arith_copy)
	NONTERMINAL_RULE(rule_arg_list, result_arith_copy)
	uint_set_union_with(&result_arith, &result_arith_copy);
	uint_set_destroy(&result_arith_copy);

	struct uint_set result = result_log;
	uint_set_union_with(&result, &result_arith);
	uint_set_destroy(&result_arith);

	UPDATE_MEMOTABLE(ARG_LIST_MT)
	return result;
}

struct uint_set rule_assign(int j) {
#ifdef PARSER_DEBUG
	printf("rule_assign visited, j = %d\n", j);
#endif

	CHECK_MEMOTABLE(ASSIGN_MT)

	enum token_type type_under_cursor;
	enum ts_ret ts_rv = token_store_get(j, &type_under_cursor, NULL, NULL);
	if (ts_ret_oob == ts_rv || token_type_identifier != type_under_cursor) {
		RULE_FAIL(ASSIGN_MT)
	}

	ts_rv = token_store_get(j + 1, &type_under_cursor, NULL, NULL);
	if (ts_ret_oob == ts_rv || token_type_binop_assign != type_under_cursor) {
		RULE_FAIL(ASSIGN_MT)
	}

	struct uint_set result = rule_logical_exp(j + 2);
	struct uint_set result_arith = rule_arithmetic_exp(j + 2);
	uint_set_union_with(&result, &result_arith);
	uint_set_destroy(&result_arith);

	UPDATE_MEMOTABLE(ASSIGN_MT)
	return result;
}

struct uint_set rule_if_stmt(int j) {
#ifdef PARSER_DEBUG
	printf("rule_if_stmt visited, j = %d\n", j);
#endif

	CHECK_MEMOTABLE(IF_STMT_MT)

	enum token_type type_under_cursor;
	enum ts_ret ts_rv = token_store_get(j, &type_under_cursor, NULL, NULL);
	if (ts_ret_oob == ts_rv || token_type_keyword_if != type_under_cursor) {
		RULE_FAIL(IF_STMT_MT)
	}

	struct uint_set result = rule_logical_exp(j + 1);
	/* Fail early if matching logical_exp failed, since we'd be doing a lot of needless work otherwise */
	if (uis_ret_true == uint_set_isempty(&result)) {
		return result;
	}

	TERMINAL_RULE(token_type_keyword_then, result)
	NONTERMINAL_RULE(rule_stmt_list, result)
	struct uint_set result_withelse; /* save for later */
	uint_set_copy(&result_withelse, &result);
	TERMINAL_RULE(token_type_keyword_end, result)
	TERMINAL_RULE(token_type_keyword_if, result)

	TERMINAL_RULE(token_type_keyword_else, result_withelse)
	NONTERMINAL_RULE(rule_stmt_list, result_withelse)
	TERMINAL_RULE(token_type_keyword_end, result_withelse)
	TERMINAL_RULE(token_type_keyword_if, result_withelse)
	
	uint_set_union_with(&result, &result_withelse);
	uint_set_destroy(&result_withelse);

	UPDATE_MEMOTABLE(IF_STMT_MT)
	return result;
}

struct uint_set rule_return_stmt(int j) {
#ifdef PARSER_DEBUG
	printf("rule_return_stmt visited, j = %d\n", j);
#endif

	CHECK_MEMOTABLE(RETURN_STMT_MT)

	enum token_type type_under_cursor;
	enum ts_ret ts_rv = token_store_get(j, &type_under_cursor, NULL, NULL);
	if (ts_ret_oob == ts_rv || token_type_keyword_return != type_under_cursor) {
		RULE_FAIL(RETURN_STMT_MT)
	}
	
	struct uint_set result = rule_logical_exp(j + 1);
	struct uint_set result_arith = rule_arithmetic_exp(j + 1);
	uint_set_union_with(&result, &result_arith);
	uint_set_destroy(&result_arith);

	UPDATE_MEMOTABLE(RETURN_STMT_MT)
	return result;
}

struct uint_set rule_logical_exp(int j) {
#ifdef PARSER_DEBUG
	printf("rule_logical_exp visited, j = %d\n", j);
#endif

	CHECK_MEMOTABLE(LOGICAL_EXP_MT)

	/* <relational_exp> */
	struct uint_set result = rule_relational_exp(j);

	/* "(" <logical_exp> ")" */
	struct uint_set result_pexp;
	enum token_type type_under_cursor;
	enum ts_ret ts_rv = token_store_get(j, &type_under_cursor, NULL, NULL);
	if (ts_ret_success == ts_rv && token_type_lparen == type_under_cursor) {
		result_pexp = rule_logical_exp(j + 1);
		TERMINAL_RULE(token_type_rparen, result_pexp)
	}
	else {
		uint_set_create(&result_pexp, token_count);
	}

	/* <logical_exp> <logical_op> <relational_exp> */
	struct uint_set result_cpd = rule_logical_exp(j);
	TERMINAL_RULE(token_type_binop_logical, result_cpd)
	NONTERMINAL_RULE(rule_relational_exp, result_cpd)
	
	uint_set_union_with(&result, &result_pexp);
	uint_set_destroy(&result_pexp);
	uint_set_union_with(&result, &result_cpd);
	uint_set_destroy(&result_cpd);

	UPDATE_MEMOTABLE(LOGICAL_EXP_MT)
	return result;
}

struct uint_set rule_relational_exp(int j) {
#ifdef PARSER_DEBUG
	printf("rule_relational_exp called, j = %d\n", j);
#endif

	CHECK_MEMOTABLE(RELATIONAL_EXP_MT)

	struct uint_set result = rule_arithmetic_exp(j);
	TERMINAL_RULE(token_type_binop_relational, result)
	NONTERMINAL_RULE(rule_arithmetic_exp, result)

	struct uint_set result_simple = rule_value(j);

	uint_set_union_with(&result, &result_simple);
	uint_set_destroy(&result_simple);

	UPDATE_MEMOTABLE(RELATIONAL_EXP_MT)
	return result;
}

struct uint_set rule_arithmetic_exp(int j) {
#ifdef PARSER_DEBUG
	printf("rule_arithmetic_exp visited, j = %d\n", j);
#endif

	CHECK_MEMOTABLE(ARITHMETIC_EXP_MT)

	struct uint_set result = rule_arithmetic_exp(j);
	TERMINAL_RULE(token_type_binop_additive, result)
	NONTERMINAL_RULE(rule_term, result)

	struct uint_set result_simple = rule_term(j);

	uint_set_union_with(&result, &result_simple);
	uint_set_destroy(&result_simple);

	UPDATE_MEMOTABLE(ARITHMETIC_EXP_MT)
	return result;
}

struct uint_set rule_term(int j) {
#ifdef PARSER_DEBUG
	printf("rule_term visited, j = %d\n", j);
#endif

	CHECK_MEMOTABLE(TERM_MT)

	struct uint_set result = rule_term(j);
	TERMINAL_RULE(token_type_binop_multiplicative, result)
	NONTERMINAL_RULE(rule_factor, result)

	struct uint_set result_simple = rule_factor(j);

	uint_set_union_with(&result, &result_simple);
	uint_set_destroy(&result_simple);

	UPDATE_MEMOTABLE(TERM_MT)
	return result;
}

struct uint_set rule_factor(int j) {
#ifdef PARSER_DEBUG
	printf("rule_factor visited, j = %d\n", j);
#endif

	CHECK_MEMOTABLE(FACTOR_MT)

	enum token_type type_under_cursor;
	enum token_subtype subtype_under_cursor;
	enum ts_ret ts_rv = token_store_get(j, &type_under_cursor, &subtype_under_cursor, NULL);
	if (ts_ret_oob == ts_rv) {
		RULE_FAIL(FACTOR_MT)
	}

	struct uint_set result;
	if (token_type_lparen == type_under_cursor) {
		result = rule_arithmetic_exp(j + 1);
		TERMINAL_RULE(token_type_rparen, result)
	}
	else if (token_type_binop_additive == type_under_cursor
			&& token_subtype_minus == subtype_under_cursor) {
		result = rule_factor(j + 1);
	}
	else {
		result = rule_value(j);
	}

	UPDATE_MEMOTABLE(FACTOR_MT)
	return result;
}

struct uint_set rule_value(int j) {
#ifdef PARSER_DEBUG
	printf("rule_value visited, j = %d\n", j);
#endif

	CHECK_MEMOTABLE(VALUE_MT)

	struct uint_set result;
	uint_set_create(&result, token_count);

	enum token_type type_under_cursor;
	enum ts_ret ts_rv = token_store_get(j, &type_under_cursor, NULL, NULL);
	if (ts_ret_success == ts_rv &&
			(token_type_identifier == type_under_cursor
			|| token_type_lit_int == type_under_cursor
			|| token_type_lit_dec == type_under_cursor
			|| token_type_lit_str == type_under_cursor)) {
		uint_set_add(&result, j + 1);
	}

	UPDATE_MEMOTABLE(VALUE_MT)
	return result;
}

