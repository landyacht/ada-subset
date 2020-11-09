#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>

#include "scanner.h"


/* Pre-declarations */
struct node_program;
struct node_proc_def;
struct node_var_def_list;
struct node_ident_list;
struct node_stmt_list;
struct node_stmt;
struct node_proc_call;
struct node_arg_list;
struct node_logical_exp;
struct node_relational_exp;
struct node_arithmetic_exp;
struct node_term;
struct node_factor;
struct node_literal;

/* Structs for each kind of node in the parse tree */
struct node_program {
	uint8_t variation;
	struct node_proc_def *proc;
	struct node_program  *more;
};

struct node_proc_def {
	uint8_t variation;
	struct node_ident        *name;
	struct node_var_def_list *param_list;
	struct node_var_def_list *var_decls;
	struct node_stmt_list    *instructions;
};

struct node_var_def_list {
	uint8_t variation;
	struct node_ident_list   *var_names;
	struct node_ident        *type_name;
	struct node_var_def_list *more;
};

struct node_ident_list {
	uint8_t variation;
	struct node_ident      *ident;
	struct node_ident_list *more;
};

struct node_stmt_list {
	uint8_t variation;
	struct node_stmt      *stmt;
	struct node_stmt_list *more;
};

struct node_stmt {
	uint8_t variation;
	struct node_proc_call *proc_call;
	struct node_assign    *assignment;
	struct node_if_stmt   *if_stmt;
};

struct node_proc_call {
	struct node_ident    *proc_name;
	struct node_arg_list *arguments;
};

struct node_arg_list {
	uint8_t variation;
	struct node_logical_exp *arg;
	struct node_arg_list    *more;
};

struct node_logical_exp {
	uint8_t variation;
	struct node_relational_exp *rel_exp;
	struct node_logical_exp    *inner_exp;
	enum token_subtype          op;
};

struct node_relational_exp {
	uint8_t variation;
	struct node_arithmetic_exp *lhs,
	                           *rhs;
	enum token_subtype          op;
};

struct node_arithmetic_exp {
	uint8_t variation;
	struct node_term           *term;
	struct node_arithmetic_exp *lhs;
	enum token_subtype          op;
};

struct node_term {
	uint8_t variation;
	struct node_factor *factor;
	struct node_term   *lhs;
	enum token_subtype  op;
};

struct node_factor {
	uint8_t variation;
	struct node_literal        *literal;
	struct node_arithmetic_exp *inner_exp;
	struct node_factor         *neg_factor;
};

struct node_literal {
	enum token_type val_type;
	union lexeme_value val;
}

#endif
