#ifndef SCANNER_H
#define SCANNER_H

#define MAX_LEXEME_LEN 150
#define MAX_ERROR_LEN 100

/* Associate named token types with integer values */
enum token_type {
	token_type_keyword_proc,         /* "procedure" */
	token_type_keyword_is,           /* "is" */
	token_type_keyword_begin,        /* "begin" */
	token_type_keyword_end,          /* "end" */
	token_type_colon,                /* Colon for declarations, e.g. Foo: Integer */
	token_type_lparen,               /* ( */
	token_type_rparen,               /* ) */
	token_type_identifier,           /* Identifiers such as variable & procedure names, type names */
	token_type_lit_int,              /* Integer literal */
	token_type_lit_dec,              /* Decimal (floating point) literal */
	token_type_lit_str,              /* String literal */
	/* The following operators listed in order of decreasing precedence */
	token_type_unop_not,             /* Logical negation (not) */
	token_type_binop_multiplicative, /* Binary operators with multiplicative precedence (* / mod rem) */
	token_type_binop_additive,       /* Binary operators with additive precedence (+ -) */
	token_type_binop_relational,     /* Relational operators like =, >, <, /=, etc. */
	token_type_binop_logical,        /* Logical operators like and, or, xor */
	token_type_binop_assign,         /* := */
	/* End operator listing */
	token_type_statement_sep         /* Semicolons separating statements */
};

/* 
 * Union type for possible lexeme types 
 * If the lexeme is a string, val_str shall point to a heap-allocated copy of the string contents
 * This will reduce the amount of work for the parser in building the symbol table
 */
union lexeme_value {
	long    val_int;
	double  val_dec;
	char   *val_str;
};

/* Globals to store current values after a call to next_lexeme */
char current_lexeme[MAX_LEXEME_LEN + 1]; /* 1 byte for null terminator */
enum token_type current_token_type;
union lexeme_value current_value;
char current_error[MAX_ERROR_LEN + 1]; /* 1 byte for null terminator */

/*
 * lexer_init - Initialize the lexer (must be called before any other lexer subroutines)
 * Parameters:
 *   filename - Path to file to be lexed
 * Returns:
 *   0 - Success
 *   1 - Failed to open file (current_error will be set)
 */
int lexer_init(char *filename);

/*
 * lexer_next - Read a lexeme and put relevant info in globals
 * Returns:
 *   0 - Success
 *   1 - Reached end of file (no lexeme to report)
 *   2 - Failed to read (current_error will be set)
 *   3 - Invalid token (current_error will be set)
 */
int lexer_next();

/* lexer_close - Release resources and return to un-initialized state */
void lexer_close();

/* this endif ends the include guard */
#endif 
