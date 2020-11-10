#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "parser.h"
#include "memotable.h"
#include "uint_set.h"
#include "scanner.h"
#include "token_store.h"

int token_count;
const struct uint_set empty;

struct memotable
	program_mt,
	proc_def_mt,
	var_def_list_mt,
	ident_list_mt,
	ident_mt,
	stmt_list_mt,
	stmt_mt,
	proc_call_mt,
	arg_list_mt,
	assign_mt,
	if_stmt_mt, 
	logical_exp_mt,
	relational_exp_mt,
	arithmetic_exp_mt,
	term_mt,
	factor_mt,
	literal_mt;       
	

struct uint_set rule_program(int, struct node_program *);
struct uint_set rule_proc_def(int, struct node_proc_def *);
struct uint_set rule_var_def_list(int, struct node_var_def_list *);
struct uint_set rule_ident_list(int, struct node_ident_list *);
struct uint_set rule_ident(int, struct node_ident *);
struct uint_set rule_stmt_list(int, struct node_stmt_list *);
struct uint_set rule_stmt(int, struct node_stmt *);
struct uint_set rule_proc_call(int, struct node_proc_call *);
struct uint_set rule_arg_list(int, struct node_arg_list *);
struct uint_set rule_assign(int, struct node_assign *);
struct uint_set rule_if_stmt(int, struct node_if_stmt *);
struct uint_set rule_logical_exp(int, struct node_logical_exp *);
struct uint_set rule_relational_exp(int, struct node_relational_exp *);
struct uint_set rule_arithmetic_exp(int, struct node_arithmetic_exp *);
struct uint_set rule_term(int, struct node_term *);
struct uint_set rule_factor(int, struct node_factor *);
struct uint_set rule_literal(int, struct node_literal *);

#define CHECK_MEMOTABLE(table) \
	struct uint_set check_result; \
	if (mt_ret_success == memotable_get_val(&table, j, &check_result)) { \
		return check_result; \
	} \
	if (memotable_get_ct(&table, j) > token_count - j) { \
		 return empty; \
	} \
	memotable_inc_ct(&table, j, NULL);

#define UPDATE_MEMOTABLE(table) \
	struct uint_set existing_result; \
	if (mt_ret_not_found == memotable_get_val(&table, j, &existing_result)) { \
		memotable_store(&table, j, &result); \
	} \
	else { \
		uint_set_union_with(&result, &existing_result); \
	}

struct node_program *parse(char *filename) {
	if (lexer_ret_fopen_fail == lexer_init(filename)) {
		printf("fopen() failed while initializing lexer: %s\n", lexer_current_err);
		return NULL;
	}

	if (ts_ret_alloc_fail == token_store_init()) {
		printf("Token store failed to initialize due to memory allocation issues!\n");
		return NULL;
	}

	uint_set_create(&empty, token_count);


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
				return NULL;
			}
			break;
		case lexer_ret_fread_fail:
			printf("Lexer failed to read the file: %s\n", lexer_current_err);
			return NULL;
		case lexer_ret_invalid:
			printf("Lexer encountered malformed input: %s\n", lexer_current_err);
			return NULL;
		case lexer_ret_eof:
			keep_going = false;
			break;
		}
	}

	/* Set up memotables */
	size_t mt_size = token_count / 6;
	memotable_create(&program_mt, size);
	memotable_create(&proc_def_mt, size);
	memotable_create(&var_def_list_mt, size);
	memotable_create(&ident_list_mt, size);
	memotable_create(&ident_mt, size);
	memotable_create(&stmt_list_mt, size);
	memotable_create(&stmt_mt, size);
	memotable_create(&proc_call_mt, size);
	memotable_create(&arg_list_mt, size);
	memotable_create(&assign_mt, size);
	memotable_create(&if_stmt_mt, size);
	memotable_create(&logical_exp_mt, size);
	memotable_create(&relational_exp_mt, size);
	memotable_create(&arithmetic_exp_mt, size);
	memotable_create(&term_mt. size);
	memotable_create(&factor_mt, size);
	memotable_create(&literal_mt, size);

	struct node_program top;
	rule_program(0, &top);

	return top;
}

struct uint_set rule_program(int j, struct node_program *node_out) {
	printf("rule_program visited, j = %d", j);

	CHECK_MEMOTABLE(program_mt)

	struct uint_set result;

	node_out->proc = malloc(sizeof(node_proc_def));
	struct uint_set proc_set = rule_proc_def(j, node_out->proc);
	if (uint_set_isempty(&proc_set)) {
		uint_set_copy(&result, *proc_set);
	}
	else if (j < num_tokens - 1) {
		uint_set_create(&result, token_count);
		for (int i = j; i < token_count; i++) {
			if (uint_set_contains(&proc_set, i)) {
				enum token_type type;
				token_store_get(i, &type, NULL, NULL);
				if (token_type_statement_sep == type) {
					uint_set_add(&result, i + 1);
				}
			}
		}
	}
	else {
		uint_set_create(&result, token_count);
	}

	UPDATE_MEMOTABLE(program_mt)
}

struct uint_set rule_proc_def(int, struct node_proc_def *);
struct uint_set rule_var_def_list(int, struct node_var_def_list *);
struct uint_set rule_ident_list(int, struct node_ident_list *);
struct uint_set rule_ident(int, struct node_ident *);
struct uint_set rule_stmt_list(int, struct node_stmt_list *);
struct uint_set rule_stmt(int, struct node_stmt *);
struct uint_set rule_proc_call(int, struct node_proc_call *);
struct uint_set rule_arg_list(int, struct node_arg_list *);
struct uint_set rule_assign(int, struct node_assign *);
struct uint_set rule_if_stmt(int, struct node_if_stmt *);
struct uint_set rule_logical_exp(int, struct node_logical_exp *);
struct uint_set rule_relational_exp(int, struct node_relational_exp *);
struct uint_set rule_arithmetic_exp(int, struct node_arithmetic_exp *);
struct uint_set rule_term(int, struct node_term *);
struct uint_set rule_factor(int, struct node_factor *);
struct uint_set rule_literal(int, struct node_literal *);
