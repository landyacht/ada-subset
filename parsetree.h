#ifndef PARSETREE_H
#define PARSETREE_H

/* Pre-declarations */
struct node_program;
struct node_proc_def;
struct node_var_def_list;
struct node_ident_list;
struct node_ident;
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

/* Each left-hand side may have more than one right-hand side */
enum node_variation {
	/* For list-type nodes such as node_program, node_stmt_list, etc. */
	nv_partial,
	nv_last,

	/* For procedures, which may or may not have parameters and a declaration list */
	nv_noparams_nodecls,
	nv_noparams_withdecls,
	nv_withparams_nodecls,
	nv_withparams_withdecls,

	/* For statements */
	nv_call_stmt,
	nv_assign_stmt,
	nv_if_stmt,
	nv_ret_stmt,

	/* For assignments */
	nv_logical_assign,
	nv_arithmetic_assign,

	/* For if statements */
	nv_if_noelse,
	nv_if_withelse,

	/* For expressions, terms, and factors */
	nv_exp_parenthesized,
	nv_exp_simple,
	nv_exp_binary,

	/* For factors */
	nv_fac_neg
};

/* Construct the parse tree from the parse info */
struct node_program *pt_create();

/* Structs for each kind of node in the parse tree */
struct node_program {
	enum node_variation variation;
	struct node_proc_def *proc;
	struct node_program  *more;
};

struct node_proc_def {
	enum node_variation variation;
	struct node_ident        *name,
	                         *end_name;
	struct node_var_def_list *param_list;
	struct node_var_def_list *var_decls;
	struct node_stmt_list    *instructions;
};

struct node_var_def_list {
	enum node_variation variation;
	struct node_ident_list   *var_names;
	struct node_ident        *type_name;
	struct node_var_def_list *more;
};

struct node_ident_list {
	enum node_variation variation;
	struct node_ident      *ident;
	struct node_ident_list *more;
};

struct node_ident {
	char *name;
	int symtbl_pos;
};

struct node_stmt_list {
	enum node_variation variation;
	struct node_stmt      *stmt;
	struct node_stmt_list *more;
};

struct node_stmt {
	enum node_variation variation;
	struct node_proc_call *proc_call;
	struct node_assign    *assignment;
	struct node_if_stmt   *if_stmt;
};

struct node_proc_call {
	struct node_ident    *proc_name;
	struct node_arg_list *arguments;
};

struct node_arg_list {
	enum node_variation variation;
	struct node_logical_exp *arg;
	struct node_arg_list    *more;
};

struct node_logical_exp {
	enum node_variation variation;
	struct node_relational_exp *rel_exp;
	struct node_logical_exp    *inner_exp;
	enum token_subtype          op;
};

struct node_relational_exp {
	enum node_variation variation;
	struct node_arithmetic_exp *lhs,
	                           *rhs;
	enum token_subtype          op;
};

struct node_arithmetic_exp {
	enum node_variation variation;
	struct node_term           *term;
	struct node_arithmetic_exp *lhs;
	enum token_subtype          op;
};

struct node_term {
	enum node_variation variation;
	struct node_factor *factor;
	struct node_term   *lhs;
	enum token_subtype  op;
};

struct node_factor {
	enum node_variation variation;
	struct node_literal        *literal;
	struct node_arithmetic_exp *inner_exp;
	struct node_factor         *neg_factor;
};

struct node_literal {
	enum token_type val_type;
	union lexeme_value val;
};

#endif
