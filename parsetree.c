#include "parsetree.h"
#include "parser.h"
#include "memotable.h"
#include "uint_set.h"
#include "token_store.h"

#define 

/* Pre-declarations */
struct node_program         *build_program(int, int);
struct node_proc_def        *build_proc_def(int, int);
struct node_var_def_list    *build_var_def_list(int, int);
struct node_ident_list      *build_ident_list(int, int);
struct node_ident           *build_ident(int, int);
struct node_stmt_list       *build_stmt_list(int, int);
struct node_stmt            *build_stmt(int, int);
struct node_proc_call       *build_proc_call(int, int);
struct node_arg_list        *build_arg_list(int, int);
struct node_logical_exp     *build_logical_exp(int, int);
struct node_relational_exp  *build_relational_exp(int, int);
struct node_arithmetic_exp  *build_arithmetic_exp(int, int);
struct node_term            *build_term(int, int);
struct node_factor          *build_factor(int, int);
struct node_literal         *build_literal(int, int);

struct node_program *pt_create() {
	return build_program(0, token_count);
}

struct node_program *build_program(int i, int bound) {
	
}
