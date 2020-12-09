#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "scanner.h"
#include "parser.h"
#include "parsetree.h"
#include "symbol_table.h"

struct exp_val {
	enum symbol_type type;
	union lexeme_value value;
};

struct arg_list {
	int arity;
	struct exp_val *values;
};

#include "builtins.c"

int exec_procedure(struct node_proc_def *proc);
bool exec_stmt_list(struct node_stmt_list *root, char *namespace, int *retval);
struct exp_val eval_proc_call(struct node_proc_call *call, char *namespace);
struct arg_list eval_arg_list(struct node_arg_list *root, char *namespace);
void exec_assignment(struct node_assignment *node, char *namespace);
bool exec_if_stmt(struct node_if_stmt *node, char *namespace, int *retval);
bool eval_logical_exp(struct node_logical_exp *node, char *namespace);
bool eval_relational_exp(struct node_relational_exp *node, char *namespace);
struct exp_val eval_arithmetic_exp(struct node_arithmetic_exp *node, char *namespace);
struct exp_val eval_term(struct node_term *node, char *namespace);
struct exp_val eval_factor(struct node_factor *node, char *namespace);
struct exp_val eval_value(struct node_value *node, char *namespace);

int main(int argc, char **argv) {
	if (argc != 2) {
		printf("Usage: %s <inputfile>\n", argv[0]);
		return -1;
	}

	parse(argv[1]);

	/* Pre-load the symbol table to make True and False available */
	symtbl_init();

	int const_pos;
	union lexeme_value const_val;

	symtbl_add(NULL, "True", symbol_type_boolean, &const_pos);
	const_val.val_bool = true;
	symtbl_update(const_pos, symbol_type_boolean, const_val);

	symtbl_add(NULL, "False", symbol_type_boolean, &const_pos);
	const_val.val_bool = false;
	symtbl_update(const_pos, symbol_type_boolean, const_val);

	struct node_program *tree = pt_create();

	if (NULL == tree) {
		puts("Parstree construction failed");
		return -2;
	}

	int proc_def_count = 0;
	struct node_program *cur = tree;
	do {
		proc_def_count++;
	}
	while (NULL != (cur = cur->more));

	struct node_proc_def *procedures[proc_def_count];
	int main = -1;
	cur = tree;
	for (int i = 0; i < proc_def_count; i++) {
		procedures[i] = cur->proc;
		if (0 == strcmp(cur->proc->name->name, "Main")) {
			main = i;
		}
		cur = cur->more;
	}

	if (-1 == main) {
		puts("Fatal: No Main procedure found!");
		return -3;
	}

	return exec_procedure(procedures[main]);
}

int exec_procedure(struct node_proc_def *proc) {
	if (proc->variation & nv_withparams) {
		puts("Fatal: parameters for user-defined procedures not implemented yet!");
		exit(-13);
	}

	char *namespace = proc->name->name;

	int retval = 0;
	exec_stmt_list(proc->instructions, namespace, &retval);
	return retval;
}

bool exec_stmt_list(struct node_stmt_list *root, char *namespace, int *retval) {
	struct node_stmt_list *cur_stmt_list = root;
	while (1) {
		struct node_stmt *cur_stmt = cur_stmt_list->stmt;

		switch (cur_stmt->variation) {
		case nv_call_stmt:
			eval_proc_call((struct node_proc_call *) cur_stmt->stmt, namespace);
			break;
		case nv_assign_stmt:
			exec_assignment((struct node_assignment *) cur_stmt->stmt, namespace);
			break;
		case nv_if_stmt:
			if (exec_if_stmt((struct node_if_stmt *) cur_stmt->stmt, namespace, retval)) {
				return true;
			}
			break;
		case nv_ret_stmt: {
			struct node_return *ret_stmt = (struct node_return *) cur_stmt->stmt;
			if (nv_logical == ret_stmt->variation) {
				*retval = eval_logical_exp(ret_stmt->retval_log, namespace);
			}
			else {
				*retval = eval_arithmetic_exp(ret_stmt->retval_arith, namespace).value.val_int;
			}
			return true;
		}
		}

		if (nv_last == cur_stmt_list->variation) {
			break;
		}
		cur_stmt_list = cur_stmt_list->more;
	}
	
	return false;
}

struct exp_val eval_proc_call(struct node_proc_call *call, char *namespace) {
	struct arg_list arguments;
	if (call->variation == nv_call_noargs) {
		arguments.arity = 0;
	}
	else {
		arguments = eval_arg_list(call->arguments, namespace);
	}

	struct exp_val proc_ret;
	char *proc_name = call->proc_name->name;
	if (0 == strcmp("Put", proc_name)) {
		if (arguments.arity < 1) {
			puts("Fatal: Procedure 'Put' requires at least one argument!");
			exit(-4);
		}
		builtin_Put(arguments);
		proc_ret.type = symbol_type_void;
	}
	else if (0 == strcmp("PutLn", proc_name)) {
		builtin_PutLn(arguments);
		proc_ret.type = symbol_type_void;
	}
	else if (0 == strcmp("GetInt", proc_name)) {
		if (0 != arguments.arity) {
			printf("Fatal: Function 'GetInt' expected 0 arguments but got %d!\n", arguments.arity);
			exit(-4);
		}
		proc_ret.type = symbol_type_integer;
		proc_ret.value.val_int = builtin_GetInt();
	}
	else if (0 == strcmp("GetFloat", proc_name)) {
		if (0 != arguments.arity) {
			printf("Fatal: Function 'GetFloat' expected 0 arguments but got %d!\n", arguments.arity);
			exit(-4);
		}
		proc_ret.type = symbol_type_float;
		proc_ret.value.val_dec = builtin_GetFloat();
	}
	else if (0 == strcmp("GetBoolean", proc_name)) {
		if (0 != arguments.arity) {
			printf("Fatal: Function 'GetBoolean' expected 0 arguments but got %d\n", arguments.arity);
			exit(-4);
		}
		proc_ret.type = symbol_type_boolean;
		proc_ret.value.val_bool = builtin_GetBoolean();
	}
	else if (0 == strcmp("GetLn", proc_name)) {
		if (0 != arguments.arity) {
			printf("Fatal: Function 'GetLn' expected 0 arguments but got %d!\n", arguments.arity);
			exit(-4);
		}
		proc_ret.type = symbol_type_string;
		proc_ret.value.val_str = builtin_GetLn();
	}
	else {
		printf("Fatal: Unrecognized procedure/function '%s'!\n", proc_name);
		exit(-5);
	}

	return proc_ret;
}

struct arg_list eval_arg_list(struct node_arg_list *root, char *namespace) {
	struct arg_list result;
	result.arity = 0;

	struct node_arg_list *cur = root;
	do {
		result.arity++;
	}
	while (NULL != (cur = cur->more));

	result.values = calloc(result.arity, sizeof(struct exp_val));
	cur = root;
	for (int i = 0; i < result.arity; i++) {
		if (cur->variation & nv_logical) {
			result.values[i].type = symbol_type_boolean;
			result.values[i].value.val_bool = eval_logical_exp(cur->arg_log, namespace);
		}
		else {
			result.values[i] = eval_arithmetic_exp(cur->arg_arith, namespace);
		}

		cur = cur->more;
	}

	return result;
}

void exec_assignment(struct node_assignment *node, char *namespace) {
	struct node_ident *lhs = node->lhs;

	if (-1 == lhs->symtbl_pos) {
		printf("Fatal: Unrecognized identifier on left-hand side of assignment: %s\n", lhs->name);
		exit(-6);
	}
	
	if (nv_logical == node->variation) {
		union lexeme_value logical_result;
		logical_result.val_bool = eval_logical_exp(node->rhs_log, namespace);
		if (symtbl_ret_type_mismatch == symtbl_update(lhs->symtbl_pos, symbol_type_boolean, logical_result)) {
			printf("Fatal: New value's type (Boolean) does not match variable's type in assignment into %s!\n",
					lhs->name);
			exit(-7);
		}
	}
	else {
		struct exp_val rhs_val = eval_arithmetic_exp(node->rhs_arith, namespace);
		if (symtbl_ret_type_mismatch == symtbl_update(lhs->symtbl_pos, rhs_val.type, rhs_val.value)) {
			char *typename =
				rhs_val.type == symbol_type_integer ? "Integer":
				rhs_val.type == symbol_type_float   ? "Float"  :
				rhs_val.type == symbol_type_string  ? "String" :
				rhs_val.type == symbol_type_boolean ? "Boolean":
				rhs_val.type == symbol_type_void    ? "void"   :
				                                      "UNKNOWN";
			printf("Fatal: New value's type (%s) does not match variable's type in assignment into %s!\n",
					typename, lhs->name);
			exit(-7);
		}
	}
}

bool exec_if_stmt(struct node_if_stmt *node, char *namespace, int *retval) {
	if (eval_logical_exp(node->cond, namespace)) {
		return exec_stmt_list(node->stmts_pos, namespace, retval);
	}
	else {
		return exec_stmt_list(node->stmts_neg, namespace, retval);
	}
}

bool eval_logical_exp(struct node_logical_exp *node, char *namespace) {
	switch (node->variation) {
	case nv_simple:
		return eval_relational_exp(node->rel_exp, namespace);
	case nv_parenthesized:
		return eval_logical_exp(node->inner_exp, namespace);
	case nv_binary:
		switch (node->op) {
		case token_subtype_and:
			return eval_logical_exp(node->inner_exp, namespace)
				&& eval_relational_exp(node->rel_exp, namespace);
		case token_subtype_or:
			return eval_logical_exp(node->inner_exp, namespace)
				|| eval_relational_exp(node->rel_exp, namespace);
		case token_subtype_xor:
			return eval_logical_exp(node->inner_exp, namespace)
				 ^ eval_relational_exp(node->rel_exp, namespace);
		}
	}
}

#define COMPARE(valtype_l, valtype_r) \
	switch (node->op) { \
	case token_subtype_eq: \
		return lhs.value.valtype_l == rhs.value.valtype_r; \
	case token_subtype_gt: \
		return lhs.value.valtype_l  > rhs.value.valtype_r; \
	case token_subtype_lt: \
		return lhs.value.valtype_l  < rhs.value.valtype_r; \
	case token_subtype_neq: \
		return lhs.value.valtype_l != rhs.value.valtype_r; \
	case token_subtype_gte: \
		return lhs.value.valtype_l >= rhs.value.valtype_r; \
	case token_subtype_lte: \
		return lhs.value.valtype_l <= rhs.value.valtype_r; \
	}

bool eval_relational_exp(struct node_relational_exp *node, char *namespace) {
	switch (node->variation) {
	case nv_simple: {
		struct exp_val value_result = eval_value(node->val, namespace);
		if (symbol_type_string == value_result.type) {
			puts("Fatal: Strings are not valid in logical expressions!");
			exit(-8);
		}
		return value_result.value.val_bool;
	}
	case nv_binary: {
		struct exp_val lhs = eval_arithmetic_exp(node->lhs, namespace),
		               rhs = eval_arithmetic_exp(node->rhs, namespace);
		if (lhs.type == symbol_type_string || rhs.type == symbol_type_string) {
			puts("Fatal: String comparisons are not yet implemented!");
			exit(-9);
		}
		if (lhs.type == symbol_type_float) {
			if (rhs.type == symbol_type_float) {
				COMPARE(val_dec, val_dec)
			}
			else {
				COMPARE(val_dec, val_int)
			}
		}
		else {
			if (rhs.type == symbol_type_float) {
				COMPARE(val_int, val_dec)
			}
			else {
				COMPARE(val_int, val_int)
			}
		}
	}
	}
}

#define ARITHMETIC_EVAL(valtype_result, valtype_l, valtype_r) \
	switch (node->op) { \
	case token_subtype_plus: \
		result.value.valtype_result = lhs.value.valtype_l + rhs.value.valtype_r; \
		break; \
	case token_subtype_minus: \
		result.value.valtype_result = lhs.value.valtype_l - rhs.value.valtype_r; \
	}

#define CONCAT(lhs_fstr, valtype_l) \
	switch (rhs.type) { \
	case symbol_type_integer: \
		snprintf(concat_result, concat_result_len, lhs_fstr "%li", \
				lhs.value.valtype_l, rhs.value.val_int); \
		break; \
	case symbol_type_float: \
		snprintf(concat_result, concat_result_len, lhs_fstr "%lF", \
				lhs.value.valtype_l, rhs.value.val_dec); \
		break; \
	case symbol_type_boolean: \
		snprintf(concat_result, concat_result_len, lhs_fstr "%s", \
				lhs.value.valtype_l, rhs.value.val_bool ? "True" : "False"); \
		break; \
	case symbol_type_string: \
		snprintf(concat_result, concat_result_len, lhs_fstr "%s", \
				lhs.value.valtype_l, rhs.value.val_str); \
	}

struct exp_val eval_arithmetic_exp(struct node_arithmetic_exp *node, char *namespace) {
	switch (node->variation) {
	case nv_simple:
		return eval_term(node->term, namespace);
	case nv_binary: {
		struct exp_val result,
			           lhs = eval_arithmetic_exp(node->inner_exp, namespace),
		               rhs = eval_term(node->term, namespace);
		if (token_subtype_concat == node->op) {
			result.type = symbol_type_string;
			int concat_result_len = 2 * MAX_LEXEME_LEN;
			char *concat_result = malloc(sizeof(char) * (concat_result_len + 1));
			concat_result[concat_result_len] = '\0';
			switch (lhs.type) {
			case symbol_type_integer:
				CONCAT("%li", val_int)
				break;
			case symbol_type_float:
				CONCAT("%lf", val_dec)
				break;
			case symbol_type_boolean:
				lhs.value.val_str = lhs.value.val_bool ? "True" : "False";
				CONCAT("%s", val_str)
				break;
			case symbol_type_string:
				CONCAT("%s", val_str)
			}
			result.value.val_str = concat_result;
		}
		else {
			if (symbol_type_float == lhs.type) {
				result.type = symbol_type_float;
				if (symbol_type_float == rhs.type) {
					ARITHMETIC_EVAL(val_dec, val_dec, val_dec)
				}
				else {
					ARITHMETIC_EVAL(val_dec, val_dec, val_int)
				}
			}
			else {
				if (symbol_type_float == rhs.type) {
					result.type = symbol_type_float;
					ARITHMETIC_EVAL(val_dec, val_int, val_dec)
				}
				else {
					result.type = symbol_type_integer;
					ARITHMETIC_EVAL(val_int, val_int, val_int)
				}
			}
		}

		return result;
	}
	}
}

#define TERM_EVAL(valtype_result, valtype_l, valtype_r) \
	switch (node->op) { \
	case token_subtype_times: \
		result.value.valtype_result = lhs.value.valtype_l * rhs.value.valtype_r; \
		break; \
	case token_subtype_div: \
		result.value.valtype_result = lhs.value.valtype_l / rhs.value.valtype_r; \
	}

struct exp_val eval_term(struct node_term *node, char *namespace) {
	switch (node->variation) {
	case nv_simple:
		return eval_factor(node->factor, namespace);
	case nv_binary: {
		struct exp_val result,
		               lhs = eval_term(node->inner_term, namespace),
		               rhs = eval_factor(node->factor, namespace);
		if (symbol_type_string == lhs.type || symbol_type_string == rhs.type) {
			puts("Fatal: String is invalid as operand in term!");
			exit(-10);
		}
		if (token_subtype_rem == node->op) {
			if (symbol_type_float == lhs.type || symbol_type_float == rhs.type) {
				puts("Fatal: 'rem' operator cannot be used with Floats!");
			}
			result.type = symbol_type_integer;
			result.value.val_int = lhs.value.val_int % rhs.value.val_int;
		}
		else if (symbol_type_float == lhs.type) {
			result.type = symbol_type_float;
			if (symbol_type_float == rhs.type) {
				TERM_EVAL(val_dec, val_dec, val_dec)
			}
			else {
				TERM_EVAL(val_dec, val_dec, val_int)
			}
		}
		else {
			if (symbol_type_float == rhs.type) {
				result.type = symbol_type_float;
				TERM_EVAL(val_dec, val_int, val_dec)
			}
			else {
				result.type = symbol_type_integer;
				TERM_EVAL(val_int, val_int, val_int)
			}
		}

		return result;
	}
	}
}

struct exp_val eval_factor(struct node_factor *node, char *namespace) {
	switch (node->variation) {
	case nv_simple:
		return eval_value(node->val, namespace);
	case nv_parenthesized:
		return eval_arithmetic_exp(node->inner_exp, namespace);
	case nv_fac_negative: {
		struct exp_val inner_result = eval_factor(node->neg_factor, namespace),
		               result;
		switch (inner_result.type) {
		case symbol_type_string:
			puts("Strings cannot be negated with minus operator!");
			exit(-11);
		case symbol_type_float:
			result.value.val_dec = -(inner_result.value.val_dec);
			result.type = symbol_type_float;
			return result;
		case symbol_type_integer:
		case symbol_type_boolean:
			result.value.val_int = -(inner_result.value.val_int);
			result.type = symbol_type_integer;
			return result;
		}
	}
	case nv_fac_proc_call:
		return eval_proc_call(node->proc_call, namespace);
	}
}

struct exp_val eval_value(struct node_value *node, char *namespace) {
	struct exp_val result;
	switch (node->variation) {
	case nv_val_literal:
		result.value = node->val;
		switch (node->lit_type) {
		case token_type_lit_int:
			result.type = symbol_type_integer;
			break;
		case token_type_lit_dec:
			result.type = symbol_type_float;
			break;
		case token_type_lit_str:
			result.type = symbol_type_string;
		}
		return result;
	case nv_val_ident:
		if (-1 == node->ident->symtbl_pos) {
			printf("Fatal: Undeclared symbol %s used in expression!\n", node->ident->name);
			exit(-12);
		}
		symtbl_get(node->ident->symtbl_pos, &result.type, &result.value);
		return result;
	}
}

