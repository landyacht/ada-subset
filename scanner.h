/*
Class:       CS 4308 Section 2
Term:        Fall 2020
Name:        A. Eubanks, A. Watson, and A. J. Odeyemi
Instructor:   Deepa Muralidhar
Project:  Deliverable 1 Scanner - C
*/
#ifndef SCANNER_H
#define SCANNER_H

#include <stdbool.h>

#define MAX_LEXEME_LEN 150
#define MAX_ERROR_LEN (MAX_LEXEME_LEN + 100)

/* Associate named token types with integer values */
enum token_type {
	token_type_keyword_proc,         /* "procedure" */
	token_type_keyword_is,           /* "is" */
	token_type_keyword_if,           /* "if" */
	token_type_keyword_then,         /* "then" */
	token_type_keyword_else,         /* "else" */
	token_type_keyword_begin,        /* "begin" */
	token_type_keyword_end,          /* "end" */
	token_type_keyword_return,       /* "return" */
	token_type_comma,                /* Comma for procedure calls with multiple args */
	token_type_colon,                /* Colon for declarations, e.g. Foo: Integer */
	token_type_lparen,               /* ( */
	token_type_rparen,               /* ) */
	token_type_identifier,           /* Identifiers such as variable & procedure names, type names */
	token_type_lit_int,              /* Integer literal */
	token_type_lit_dec,              /* Decimal (floating point) literal */
	token_type_lit_bool,             /* Boolean literal */
	token_type_lit_str,              /* String literal */
	/* The following operators listed in order of decreasing precedence */
	token_type_unop_not,             /* Logical negation (not) */
	token_type_binop_multiplicative, /* Binary operators with multiplicative precedence (* / mod rem) */
	token_type_binop_additive,       /* Binary operators with additive precedence (+ - &) */
	token_type_binop_relational,     /* Relational operators like =, >, <, /=, etc. */
	token_type_binop_logical,        /* Logical operators like and, or, xor */
	token_type_binop_assign,         /* := */
	/* End operator listing */
	token_type_statement_sep         /* Semicolons separating statements */
};

/* Named return codes for lexer_init and lexer_next */
enum lexer_ret {
	lexer_ret_success,
	lexer_ret_fopen_fail,
	lexer_ret_eof,
	lexer_ret_fread_fail,
	lexer_ret_invalid,
};


/* 
 * Union type for possible lexeme types 
 * If the lexeme is a string, val_str shall point to a heap-allocated copy of the string contents
 * This will reduce the amount of work for the parser in building the symbol table
 */
union lexeme_value {
	bool    val_bool;
	long    val_int;
	double  val_dec;
	char   *val_str;
};

/* Globals to store current values after a call to next_lexeme */
char current_lexeme[MAX_LEXEME_LEN + 1]; /* 1 byte for null terminator */
enum token_type current_token_type;
union lexeme_value current_value;
char lexer_current_err[MAX_ERROR_LEN + 1]; /* 1 byte for null terminator */
int current_line; /* source file line number, starting at 1 */

/*
 * lexer_init - Initialize the lexer (must be called before any other lexer subroutines)
 * Parameters:
 *   filename - Path to file to be lexed
 * Returns:
 *   lexer_ret_success    - Success
 *   lexer_ret_fopen_fail - Failed to open file (lexer_current_err will be set)
 */
enum lexer_ret lexer_init(char *filename);

/*
 * lexer_next - Read a lexeme and put relevant info in globals
 * Returns:
 *   lexer_ret_success    - Success
 *   lexer_ret_eof        - Reached end of file (no lexeme to report)
 *   lexer_ret_fread_fail - Failed to read (lexer_current_err will be set)
 *   lexer_ret_invalid    - Invalid token (lexer_current_err will be set)
 */
enum lexer_ret lexer_next();

/* lexer_close - Release resources and return to un-initialized state */
void lexer_close();

/* this endif ends the include guard */
#endif 
