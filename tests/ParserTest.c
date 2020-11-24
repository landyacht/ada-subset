void print_program(struct node_program *node, int indentation_level){
	for (int i = 0; i < indentation_level; i++) {
		/* indent */
		printf(" ");
	}
  	printf("program_list");
		/* Print the first identifier */
	print_proc_def(node -> proc, indentation_level + 1);
  
	if (nv_partial == node->variation) { /* There are more items in the list */
   
		print_program(node -> more, identation_level + 1); /* Recurse and print the rest */
	}
}

void print_proc_def(struct node_proc_def *node, int indentation_level){
	for (int i = 0; i < indentation_level; i++) { 
		/* indent */
		printf(" ");
	}
	printf("proc_def_list");
		/* Print the first identifier */
	print_ident(node -> name, indentation_level + 1);
  
	if (nv_partial == node->variation) { /* There are more items in the list */
   
		print_var_def_list(node -> param_list, identation_level + 1); /* Recurse and print the rest */
		
	}
	if (nv_partial == node->variation) { 
   
		print_var_def_list(node -> var_decls, identation_level + 1); 
	}
	if (nv_partial == node->variation) { 
   
		print_stmt_list(node -> instructions, identation_level + 1); 
	}
}

void print_var_def_list(struct node_var_def_list *node, int indentation_level){
		for (int i = 0; i < indentation_level; i++) { /* indent */
    printf(" ");
	}
	printf("var_def_list");
	/* Print the first identifier */
	print_ident_list(node -> var_names, indentation_level + 1);
  
	if (nv_partial == node->variation) { /* There are more items in the list */
   
		print_ident (node -> type_name, identation_level + 1); /* Recurse and print the rest */
	}
	if (nv_partial == node->variation) { 
   
		print_var_def_list(node -> more, identation_level + 1); 
	}
}
void print_ident_list(struct node_ident_list *node, int indentation_level) {
  for (int i = 0; i < indentation_level; i++) { /* indent */
    printf(" ");
  }
  printf("ident_list");
  /* Print the first identifier */
  print_ident(node->ident, indentation_level + 1);

  if (nv_partial == node->variation) { /* There are more items in the list */
    print_ident_list(node->more, identation_level + 1); /* Recurse and print the rest */
  }
}

void print_ident(struct node_ident *node, int indentation_level) {
  for (int i = 0; i < indentation_level; i++) { /* indent */
    printf(" ");
  }
  printf("Identifier: %s", node->name);
}
void print_stmt_list(struct node_stmt_list *node, int indentation_level) {
  for (int i = 0; i < indentation_level; i++) { 
  /* indent */
    printf(" ");
  }
  printf("stmt_list_list");
  /* Print the first identifier */
  print_stmt(node->stmt, indentation_level + 1);
  
}
void print_stmt(struct node_stmt *node, int indentation_level){
		for (int i = 0; i < indentation_level; i++) { /* indent */
    printf(" ");
	}
	printf("stmt_list");
	/* Print the first identifier */
	print_proc_call(node -> proc_call, indentation_level + 1);
  
	if (nv_partial == node->variation) { /* There are more items in the list */
   
		print_assign (node -> assignment, identation_level + 1); /* Recurse and print the rest */
	}
	if (nv_partial == node->variation) { 
   
		print_if_stmt(node -> if_stmt, identation_level + 1); 
	}
}

void proc_call(struct node_proc_call *node, int indentation_level) {
  for (int i = 0; i < indentation_level; i++) { /* indent */
    printf(" ");
  }
  printf("proc_call_list");
  /* Print the first identifier */
  print_ident(node->proc_name, indentation_level + 1);

  if (nv_partial == node->variation) { /* There are more items in the list */
    print_arg_list(node->arguments, identation_level + 1); /* Recurse and print the rest */
  }
}

void print_arg_list(struct arg_list *node, int indentation_level){
		for (int i = 0; i < indentation_level; i++) { /* indent */
    printf(" ");
	}
	printf("arg_list");
	/* Print the first identifier */
	print_logical_exp(node -> arg, indentation_level + 1);
  
	if (nv_partial == node->variation) { /* There are more items in the list */
   
		print_arg_list (node -> more, identation_level + 1); /* Recurse and print the rest */
	}
}

void print_logical_exp(struct node_logical_exp *node, int indentation_level){
		for (int i = 0; i < indentation_level; i++) { /* indent */
    printf(" ");
	}
	printf("Logical_exp_list");
	/* Print the first identifier */
	print_relational_exp(node -> rel_exp, indentation_level + 1);
  
	if (nv_partial == node->variation) { /* There are more items in the list */
   
		print_logical_exp (node -> inner_exp, identation_level + 1); /* Recurse and print the rest */
	}
}

void print_relational_exp(struct node_relational_exp *node, int indentation_level) {
  for (int i = 0; i < indentation_level; i++) { 
  /* indent */
    printf(" ");
  }
  printf("Relational_exp_list");
  /* Print the first identifier */
  print_arithmetic_exp(node->lhs, indentation_level + 1);
  
}

void print_arithmetic_exp(struct node_arithmetic_exp *node, int indentation_level){
		for (int i = 0; i < indentation_level; i++) { /* indent */
    printf(" ");
	}
	printf("Arithmetic_exp_list");
	/* Print the first identifier */
	print_term(node -> term, indentation_level + 1);
  
	if (nv_partial == node->variation) { /* There are more items in the list */
   
		print_arithmetic_exp (node -> lhs, identation_level + 1); /* Recurse and print the rest */
	}
}

void print_term(struct node_term *node, int indentation_level){
		for (int i = 0; i < indentation_level; i++) { /* indent */
    printf(" ");
	}
	printf("Term_list");
	/* Print the first identifier */
	print_factor(node -> factor, indentation_level + 1);
  
	if (nv_partial == node->variation) { /* There are more items in the list */
   
		print_term (node -> lhs, identation_level + 1); /* Recurse and print the rest */
	}
}

void print_factor(struct node_factor *node, int indentation_level){
		for (int i = 0; i < indentation_level; i++) { /* indent */
    printf(" ");
	}
	printf("Factor_list");
	/* Print the first identifier */
	print_literal(node -> literal, indentation_level + 1);
  
	if (nv_partial == node->variation) { /* There are more items in the list */
   
		print_arithmetic_exp (node -> inner_exp, identation_level + 1); /* Recurse and print the rest */
	}
		if (nv_partial == node->variation) { 
   
		print_factor(node -> neg_factor, identation_level + 1); 
	}
}


