#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "parser.h"
#include "parsetree.h"

#define BUILD_INDENTATION \
	char *indentation = malloc(sizeof(char) * (indent + 1)); \
	memset(indentation, ' ', indent); \
	indentation[indent] = '\0';

#define IPRINT(str) \
	printf("%s%s", indentation, str);

void print_program(struct node_program *, int);
void print_proc_def(struct node_proc_def *, int);
void print_var_def_list(struct node_var_def_list *, int);
void print_ident_list(struct node_ident_list *, int);
void print_ident(struct node_ident *);
void print_stmt_list(struct node_stmt_list *, int);
void print_stmt(struct node_stmt *, int);
void print_proc_call(struct node_proc_call *, int);
void print_assignment(struct node_assignment *, int);
void print_if_stmt(struct node_if_stmt *, int);
void print_return(struct node_return *, int);
void print_arg_list(struct node_arg_list *, int);
void print_logical_exp(struct node_logical_exp *, int);
void print_relational_exp(struct node_relational_exp *, int);
void print_arithmetic_exp(struct node_arithmetic_exp *, int);
void print_term(struct node_term *, int);
void print_factor(struct node_factor *, int);
void print_value(struct node_value *);

int main(int argc, char **argv) {
	if (argc != 2) {
		printf("usage: %s <inputfile>\n", argv[0]);
		return 1;
	}

	parse(argv[1]);
	struct node_program *tree = pt_create();

	if (NULL == tree) {
		puts("Parsetree construction failed");
		return 2;
	}

	puts("----- BEGIN PARSETREE PRINTOUT -----");
	print_program(tree, 0);
	puts("-----  END PARSETREE PRINTOUT  -----");

	return 0;
}

void print_program(struct node_program *node, int indent) {
	BUILD_INDENTATION

	if (nv_partial == node->variation) {
		printf("Program (partial):\n");
		IPRINT("- proc => ")
		print_proc_def(node->proc, indent + 2);
		IPRINT("- more => ")
		print_program(node->more, indent + 2);
	}
	else {
		printf("Program (last):\n");
		IPRINT("- proc => ")
		print_proc_def(node->proc, indent + 2);
	}
}

void print_proc_def(struct node_proc_def *node, int indent) {
	BUILD_INDENTATION

	char *params = node->variation & nv_withparams ? "with" : "no",
	     *decls  = node->variation & nv_withdecls  ? "with" : "no";
	printf("Procedure definition (%s parameters, %s declarations):\n", params, decls);
	IPRINT("- name => ");
	print_ident(node->name);
	if (node->variation & nv_withparams) {
		IPRINT("- param_list => ")
		print_var_def_list(node->param_list, indent + 2);
	}
	if (node->variation & nv_withdecls) {
		IPRINT("- var_decls => ")
		print_var_def_list(node->var_decls, indent + 2);
	}
	
	IPRINT("- instructions => ");
	print_stmt_list(node->instructions, indent + 2);
}

void print_var_def_list(struct node_var_def_list *node, int indent) {
	BUILD_INDENTATION

	char *partial_last = nv_partial == node->variation ? "partial" : "last";
	printf("Variable definition list (%s):\n", partial_last);

	IPRINT("- type_name => ")
	print_ident(node->type_name);
	
	IPRINT("- var_names => ")
	print_ident_list(node->var_names, indent + 2);

	if (nv_partial == node->variation) {
		IPRINT("- more => ")
		print_var_def_list(node->more, indent + 2);
	}
}

void print_ident_list(struct node_ident_list *node, int indent) {
	BUILD_INDENTATION

	if (nv_partial == node->variation) {
		printf("Identifier list (partial):\n");
		IPRINT("- ident => ")
		print_ident(node->ident);
		IPRINT("- more => ")
		print_ident_list(node->more, indent + 2);
	}
	else {
		printf("Identifier list (last):\n");
		IPRINT("- ident => ")
		print_ident(node->ident);
	}
}

void print_ident(struct node_ident *node) {
	printf("Identifier: name => %s\n", node->name);
}

void print_stmt_list(struct node_stmt_list *node, int indent) {
	BUILD_INDENTATION

	char *partial_last = nv_partial == node->variation ? "partial" : "last";
	printf("Statement list (%s):\n", partial_last);
	
	IPRINT("- stmt => ")
	print_stmt(node->stmt, indent + 2);

	if (nv_partial == node->variation) {
		IPRINT("- more => ")
		print_stmt_list(node->more, indent + 2);
	}
}

void print_stmt(struct node_stmt *node, int indent) {
	BUILD_INDENTATION

	switch (node->variation) {
	case nv_call_stmt:
		printf("Statement (procedure call):\n");
		IPRINT("- stmt => ")
		print_proc_call((struct node_proc_call *) node->stmt, indent + 2);
		break;
	case nv_assign_stmt:
		printf("Statement (assignment):\n");
		IPRINT("- stmt => ")
		print_assignment((struct node_assignment *) node->stmt, indent + 2);
		break;
	case nv_if_stmt:
		printf("Statement (if or if-else):\n");
		IPRINT("- stmt => ")
		print_if_stmt((struct node_if_stmt *) node->stmt, indent + 2);
		break;
	case nv_ret_stmt:
		printf("Statement (return):\n");
		IPRINT("- stmt => ")
		print_return((struct node_return *) node->stmt, indent + 2);
		break;
	default:
		puts("Statement did not have any valid type!!!");
		break;
	}
}

void print_proc_call(struct node_proc_call *node, int indent) {
	BUILD_INDENTATION

	if (nv_call_args == node->variation) {
		printf("Procedure call (with arguments):\n");
		IPRINT("- proc_name => ")
		print_ident(node->proc_name);
		IPRINT("- arguments => ")
		print_arg_list(node->arguments, indent + 2);
	}
	else {
		printf("Procedure call (no arguments):\n");
		IPRINT("- proc_name => ")
		print_ident(node->proc_name);
	}
}

void print_assignment(struct node_assignment *node, int indent) {
	BUILD_INDENTATION

	char *log_arith = nv_logical == node->variation ? "logical" : "arithmetic";
	printf("Assignment (%s value):\n", log_arith);
	IPRINT("- lhs => ")
	print_ident(node->lhs);

	if (nv_logical == node->variation) {
		IPRINT("- rhs_log => ")
		print_logical_exp(node->rhs_log, indent + 2);
	}
	else {
		IPRINT("- rhs_arith => ")
		print_arithmetic_exp(node->rhs_arith, indent + 2);
	}
}

void print_if_stmt(struct node_if_stmt *node, int indent) {
	BUILD_INDENTATION

	char *has_else = nv_if_haselse == node->variation ? "with" : "without";
	printf("If statement (%s else):\n", has_else);

	IPRINT("- stmts_pos => ")
	print_stmt_list(node->stmts_pos, indent + 2);
	
	if (nv_if_noelse == node->variation) {
		IPRINT("- stmts_neg => ")
		print_stmt_list(node->stmts_neg, indent + 2);
	}
}

void print_return(struct node_return *node, int indent) {
	BUILD_INDENTATION


	if (nv_logical == node->variation) {
		printf("Return statement (logical value):\n");
		IPRINT("- retval_log => ")
		print_logical_exp(node->retval_log, indent + 2);
	}
	else if (nv_arithmetic == node->variation) {
		printf("Return statement (arithmetic value):\n");
		IPRINT("- retval_arith => ")
		print_arithmetic_exp(node->retval_arith, indent + 2);
	}
	else if (nv_ret_noval == node->variation) {
		printf("Return statement (no value)");
	}
	else {
		puts("Return statement was not of any valid variation!!!");
	}
}

void print_arg_list(struct node_arg_list *node, int indent) {
	BUILD_INDENTATION

	char *log_arith = nv_logical & node->variation ? "logical" : "arithmetic",
	     *partial_last = nv_partial & node->variation ? "partial" : "last";
	printf("Argument list (%s, %s value):\n", partial_last, log_arith);

	if (nv_logical & node->variation) {
		IPRINT("- arg_log => ")
		print_logical_exp(node->arg_log, indent + 2);
	}
	else if (nv_arithmetic & node->variation) {
		IPRINT("- arg_arith => ")
		print_arithmetic_exp(node->arg_arith, indent + 2);
	}
	else {
		puts("Argument was neither logical nor arithmetic?!");
	}

	if (nv_partial & node->variation) {
		IPRINT("- more => ")
		print_arg_list(node->more, indent + 2);
	}
}

void print_logical_exp(struct node_logical_exp *node, int indent) {
	BUILD_INDENTATION

	if (nv_binary == node->variation) {
		printf("Logical expression (binary):\n");
		IPRINT("- op => ")
		switch (node->op) {
		case token_subtype_and:
			puts("'and'");
			break;
		case token_subtype_or:
			puts("'or'");
			break;
		case token_subtype_xor:
			puts("'xor'");
			break;
		}
		IPRINT("- rel_exp (rhs) => ")
		print_relational_exp(node->rel_exp, indent + 2);
		IPRINT("- inner_exp (lhs) => ")
		print_logical_exp(node->inner_exp, indent + 2);
	}
	else if (nv_simple == node->variation) {
		printf("Logical expression (simple):\n");
		IPRINT("- rel_exp => ")
		print_relational_exp(node->rel_exp, indent + 2);
	}
	else if (nv_parenthesized == node->variation) {
		printf("Logical expression (parenthesized):\n");
		IPRINT("- inner_exp => ")
		print_logical_exp(node->inner_exp, indent + 2);
	}
	else {
		puts("Logical expression had no valid variation!!!");
	}
}

void print_relational_exp(struct node_relational_exp *node, int indent) {
	BUILD_INDENTATION

	if (nv_binary == node->variation) {
		printf("Relational expression (binary):\n");
		IPRINT("- op => ")
		switch (node->op) {
		case token_subtype_eq:
			puts("'='");
			break;
		case token_subtype_gt:
			puts("'>'");
			break;
		case token_subtype_lt:
			puts("'<'");
			break;
		case token_subtype_neq:
			puts("'/='");
			break;
		case token_subtype_gte:
			puts("'>='");
			break;
		case token_subtype_lte:
			puts("'<='");
			break;
		}

		IPRINT("- lhs => ")
		print_arithmetic_exp(node->lhs, indent + 2);

		IPRINT("- rhs => ")
		print_arithmetic_exp(node->rhs, indent + 2);
	}
	else if (nv_simple == node->variation) {
		printf("Relational expression (simple):\n");
		IPRINT("- val => ")
		print_value(node->val);
	}
	else {
		puts("Relational expression had no valid variation!!!");
	}
}

void print_arithmetic_exp(struct node_arithmetic_exp *node, int indent) {
	BUILD_INDENTATION

	if (nv_binary == node->variation) {
		printf("Arithmetic expression (binary):\n");
		IPRINT("- op => ")
		switch (node->op) {
		case token_subtype_plus:
			puts("'+'");
			break;
		case token_subtype_minus:
			puts("'-'");
			break;
		case token_subtype_concat:
			puts("'&'");
			break;
		}

		IPRINT("- term (rhs) => ");
		print_term(node->term, indent + 2);

		IPRINT("- inner_exp (lhs) => ");
		print_arithmetic_exp(node->inner_exp, indent + 2);
	}
	else if (nv_simple == node->variation) {
		printf("Arithmetic expression (simple):\n");
		IPRINT("- term => ")
		print_term(node->term, indent + 2);
	}
	else {
		puts("Arithmetic expression had no valid variation!!!");
	}
}

void print_term(struct node_term *node, int indent) {
	BUILD_INDENTATION

	if (nv_binary == node->variation) {
		printf("Term (binary):\n");
		IPRINT("- op => ")
		switch (node->op) {
		case token_subtype_times:
			puts("'*'");
			break;
		case token_subtype_div:
			puts("'/'");
			break;
		case token_subtype_rem:
			puts("'rem'");
			break;
		}

		IPRINT("- factor (rhs) => ")
		print_factor(node->factor, indent + 2);
		
		IPRINT("- inner_term (lhs) => ")
		print_term(node->inner_term, indent + 2);
	}
	else if (nv_simple == node->variation) {
		printf("Term (simple):\n");
		IPRINT("- factor => ")
		print_factor(node->factor, indent + 2);
	}
	else {
		puts("Term has no valid variation!!!");
	}
}

void print_factor(struct node_factor *node, int indent) {
	BUILD_INDENTATION

	if (nv_fac_negative == node->variation) {
		printf("Factor (negation of inner factor):\n");
		IPRINT("- neg_factor => ")
		print_factor(node->neg_factor, indent + 2);
	}
	else if (nv_simple == node->variation) {
		printf("Factor (simple):\n");
		IPRINT("- val => ")
		print_value(node->val);
	}
	else if (nv_parenthesized == node->variation) {
		printf("Factor (parenthesized exp):\n");
		IPRINT("- inner_exp => ")
		print_arithmetic_exp(node->inner_exp, indent + 2);
	}
	else if (nv_fac_proc_call == node->variation) {
		printf("Factor (procedure call):\n");
		IPRINT("- proc_call => ")
		print_proc_call(node->proc_call, indent + 2);
	}
	else {
		puts("Factor had no valid variation!!!");
	}
}

void print_value(struct node_value *node) {
	if (nv_val_literal == node->variation) {
		printf("Value (");
		switch (node->lit_type) {
		case token_type_lit_int:
			printf("integer): %ld\n", node->val.val_int);
			break;
		case token_type_lit_dec:
			printf("decimal): %F\n", node->val.val_dec);
			break;
		case token_type_lit_str:
			printf("string): \"%s\"\n", node->val.val_str);
			break;
		}
	}
	else if (nv_val_ident) {
		printf("Value (identifier): ");
		print_ident(node->ident);
	}
	else {
		puts("Value had no valid variation!!!");
	}
}
