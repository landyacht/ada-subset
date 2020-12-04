/*
Class:       CS 4308 Section 2
Term:        Fall 2020
Name:        A. Eubanks, A. Watson, and A. J. Odeyemi
Instructor:   Deepa Muralidhar
Project:  Deliverable 1 Scanner - C
*/
#include "scanner.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define LEXER_SET_ERROR(...) snprintf(lexer_current_err, MAX_ERROR_LEN + 1, __VA_ARGS__)

enum lexer_state {
	state_start,         /* no characters yet */
	state_maybe_relop,   /* we have encountered < or > or / which may be alone or with an = */
	state_ident,         /* we have part or all of an identifier */
	state_kw_op,         /* we have part or all of a keyword (is, begin, etc.) or operator (mod, rem, etc.) */
	state_maybe_assign,  /* we either have the assignment op or a colon */
	state_in_str,        /* we're inside a string */
	state_num_leftside,  /* either an integer or the left side of a decimal literal */
	state_num_dec,       /* we're at the decimal point in a decimal literal */
	state_num_rightside, /* the right side of the decimal in a decimal literal */
};

FILE *fp;

enum lexer_ret lexer_init(char *filename) {
	fp = fopen(filename, "r");
	if (NULL == fp) {
		LEXER_SET_ERROR("fopen failed with errno = %d", errno);
		return 1;
	}

	current_line = 1;

	return 0;
}

enum lexer_ret lexer_next() {
	/* Where in current_lexeme we are */
	int needle = 0;
	/* For writing a string literal's contents */
	int str_needle = 0;
	/* Have we reached the end of the file? */
	static bool is_eof = 0;
	/* Our current state, so we know how to transition */
	enum lexer_state cur_state = state_start;

	/* Reset values */
	current_token_subtype = token_subtype_none;
	current_value.val_str = NULL;

	if (is_eof) {
		return 1;
	}

	char read;
	while (1) {
		int num_read = fread(&read, 1, 1, fp);
		/* Check for error or EOF marker if we didn't read a byte */
		if (0 == num_read) {
			if (ferror(fp)) {
				LEXER_SET_ERROR("fread failed");
				clearerr(fp);
				return lexer_ret_fread_fail;
			}

			if (feof(fp)) {
				is_eof = 1;
			}
		}
		else {
			if ('\n' == read) {
				current_line++;
			}
		}

		/* For convenience in the switch statement */
		bool is_ws = is_eof
			|| ' '  == read /* space */
			|| '\t' == read /* tabulation */
			|| '\n' == read /* newline */
			|| '\r' == read /* carriage return */
			;
		bool is_alpha_lc = read >= 'a' && read <= 'z';
		bool is_alpha_uc = read >= 'A' && read <= 'Z';
		bool is_alpha = is_alpha_lc || is_alpha_uc;
		bool is_numeric = read >= '0' && read <= '9';
		bool is_alphanumeric = is_alpha || is_numeric;

		/* TODO implement the state machine here */
		switch (cur_state) {
		case state_start:
			if (is_eof) {
				return lexer_ret_eof;
			}
			else if (is_ws) {
				/* do nothing */
			}
			else if (':' == read) {
				cur_state = state_maybe_assign;
				current_lexeme[needle++] = read;
			}
			else if (';' == read) {
				current_lexeme[needle++] = read;
				current_lexeme[needle] = '\0';
				current_token_type = token_type_statement_sep;
				return lexer_ret_success;
			}
			else if (is_alpha_uc) {
				cur_state = state_ident;
				current_lexeme[needle++] = read;
			}
			else if (is_alpha_lc) {
				cur_state = state_kw_op;
				current_lexeme[needle++] = read;
			}
			else if (is_numeric) {
				cur_state = state_num_leftside;
				current_lexeme[needle++] = read;
			}
			else if ('.' == read) {
				cur_state = state_num_dec;
				current_lexeme[needle++] = read;
			}
			else if ('"' == read) {
				cur_state = state_in_str;
				current_lexeme[needle++] = read;
				current_value.val_str = malloc(sizeof(char) * MAX_LEXEME_LEN - 1);
			}
			else if ('+' == read || '-' == read || '&' == read) {
				current_lexeme[needle++] = read;
				current_lexeme[needle] = '\0';
				current_token_type = token_type_binop_additive;
				switch (read) {
					case '+': current_token_subtype = token_subtype_plus; break;
					case '-': current_token_subtype = token_subtype_minus; break;
					case '&': current_token_subtype = token_subtype_concat; break;
				}
				return lexer_ret_success;
			}
			else if ('*' == read) { /* / is not included here since it might be part of /= */
				current_lexeme[needle++] = read;
				current_lexeme[needle] = '\0';
				current_token_type = token_type_binop_multiplicative;
				current_token_subtype = token_subtype_times;
				return lexer_ret_success;
			}
			else if ('=' == read) {
				current_lexeme[needle++] = read;
				current_lexeme[needle] = '\0';
				current_token_type = token_type_binop_relational;
				current_token_subtype = token_subtype_eq;
				return lexer_ret_success;
			}
			else if ('(' == read || ')' == read) {
				current_lexeme[needle++] = read;
				current_lexeme[needle] = '\0';
				current_token_type = '(' == read ? token_type_lparen : token_type_rparen;
				return lexer_ret_success;
			}
			else if (',' == read) {
				current_lexeme[needle++] = read;
				current_lexeme[needle] = '\0';
				current_token_type = token_type_comma;
				return lexer_ret_success;
			}
			else if ('/' == read || '>' == read || '<' == read) {
				cur_state = state_maybe_relop;
				current_lexeme[needle++] = read;
			}
			else {
				LEXER_SET_ERROR("Unexpected character '%c' (ord %d) while in state_start", read, read);
				return lexer_ret_invalid;
			}
			break;
		case state_maybe_relop:
			if ('=' == read) {
				current_lexeme[needle++] = read;
				current_lexeme[needle] = '\0';
				current_token_type = token_type_binop_relational;
				switch (current_lexeme[0]) {
					case '/': current_token_subtype = token_subtype_neq; break;
					case '>': current_token_subtype = token_subtype_gte; break;
					case '<': current_token_subtype = token_subtype_lte; break;
				}
				return lexer_ret_success;
			}
			else {
				current_lexeme[needle] = '\0';
				if ('/' == current_lexeme[0]) {
					current_token_type = token_type_binop_multiplicative;
					current_token_subtype = token_subtype_div;
				}
				else {
					current_token_type = token_type_binop_relational;
					if ('>' == current_lexeme[0]) {
						current_token_subtype = token_subtype_gt;
					}
					else {
						current_token_subtype = token_subtype_lt;
					}
				}
				fseek(fp, -1L, SEEK_CUR); /* move back to prep for next call */
				return lexer_ret_success;
			}
			break;
		case state_ident:
			if (is_alphanumeric || '_' == read) {
				current_lexeme[needle++] = read;
			}
			else {
				current_lexeme[needle] = '\0';
				current_token_type = token_type_identifier;
				fseek(fp, -1L, SEEK_CUR); /* move back to prep for next call */
				return lexer_ret_success;
			}
			break;
		case state_kw_op:
			if (is_alpha_lc) {
				current_lexeme[needle++] = read;
			}
			/* whitespace, a unary operator, or a semicolon may follow a keyword */
			else if (is_ws || '-' == read || '+' == read || ';' == read) {
				if ('-' == read || '+' == read || ';' == read) {
					fseek(fp, -1L, SEEK_CUR); /* move back to prep for next call */
				}

				current_lexeme[needle] = '\0';
				if (0 == strcmp(current_lexeme, "mod")) {
					current_token_type = token_type_binop_multiplicative;
					current_token_subtype = token_subtype_mod;
					return lexer_ret_success;
				}
				else if (0 == strcmp(current_lexeme, "rem")) {
					current_token_type = token_type_binop_multiplicative;
					current_token_subtype = token_subtype_rem;
					return lexer_ret_success;
				}
				else if (0 == strcmp(current_lexeme, "and")) {
					current_token_type = token_type_binop_logical;
					current_token_subtype = token_subtype_and;
					return lexer_ret_success;
				}
				else if (0 == strcmp(current_lexeme, "or")) {
					current_token_type = token_type_binop_logical;
					current_token_subtype = token_subtype_or;
					return lexer_ret_success;
				}
				else if (0 == strcmp(current_lexeme, "xor")) {
					current_token_type = token_type_binop_logical;
					current_token_subtype = token_subtype_xor;
					return lexer_ret_success;
				}
				else if (0 == strcmp(current_lexeme, "procedure")) {
					current_token_type = token_type_keyword_proc;
					return lexer_ret_success;
				}
				else if (0 == strcmp(current_lexeme, "is")) {
					current_token_type = token_type_keyword_is;
					return lexer_ret_success;
				}
				else if (0 == strcmp(current_lexeme, "if")) {
					current_token_type = token_type_keyword_if;
					return lexer_ret_success;
				}
				else if (0 == strcmp(current_lexeme, "then")) {
					current_token_type = token_type_keyword_then;
					return lexer_ret_success;
				}
				else if (0 == strcmp(current_lexeme, "else")) {
					current_token_type = token_type_keyword_else;
					return lexer_ret_success;
				}
				else if (0 == strcmp(current_lexeme, "begin")) {
					current_token_type = token_type_keyword_begin;
					return lexer_ret_success;
				}
				else if (0 == strcmp(current_lexeme, "end")) {
					current_token_type = token_type_keyword_end;
					return lexer_ret_success;
				}
				else if (0 == strcmp(current_lexeme, "return")) {
					current_token_type = token_type_keyword_return;
					return lexer_ret_success;
				}
				else {
					LEXER_SET_ERROR("Unknown keyword '%s'", current_lexeme);
					return lexer_ret_invalid;
				}
			}
			else {
				current_lexeme[needle] = '\0';
				LEXER_SET_ERROR("Unexpected character during or after keyword '%s': '%c' (ord %d)", current_lexeme, read, read);
				return lexer_ret_invalid;
			}
			break;
		case state_maybe_assign:
			if ('=' == read) {
				current_lexeme[needle++] = read;
				current_lexeme[needle] = '\0';
				current_token_type = token_type_binop_assign;
				return lexer_ret_success;
			}
			else if (is_ws || is_alpha_uc) {
				if (is_alpha_uc) {
					fseek(fp, -1L, SEEK_CUR); /* move back to prep for next call */
				}
				current_lexeme[needle] = '\0';
				current_token_type = token_type_colon;
				return lexer_ret_success;
			}
			else {
				LEXER_SET_ERROR("Character %c after colon will never be valid", read);
				return lexer_ret_invalid;
			}
			break;
		case state_in_str:
			if ('"' == read) {
				current_lexeme[needle++] = read;
				current_lexeme[needle] = '\0';
				current_value.val_str[str_needle] = '\0';
				current_token_type = token_type_lit_str;
				return lexer_ret_success;
			}
			else if (is_eof) {
				LEXER_SET_ERROR("Encountered end of file in string literal");
				return lexer_ret_invalid;
			}
			else {
				current_lexeme[needle++] = read;
				current_value.val_str[str_needle++] = read;
			}
			break;
		case state_num_leftside:
			if (is_numeric) {
				current_lexeme[needle++] = read;
			}
			else if ('.' == read) {
				current_lexeme[needle++] = read;
				cur_state = state_num_dec;
			}
			else if (is_alpha) {
				current_lexeme[needle] = '\0';
				LEXER_SET_ERROR("Unexpected alpha character '%c' (ord %d) following integer literal '%s'", read, read, current_lexeme);
				return lexer_ret_invalid;
			}
			else {
				current_lexeme[needle] = '\0';
				current_token_type = token_type_lit_int;
				current_value.val_int = atol(current_lexeme);
				if (!is_ws) {
					fseek(fp, -1L, SEEK_CUR); /* move back to prep for next call */
				}
				return lexer_ret_success;
			}
			break;
		case state_num_dec:
			if (is_numeric) {
				cur_state = state_num_rightside;
				current_lexeme[needle++] = read;
			}
			else if (is_eof) {
				current_lexeme[needle] = '\0';
				LEXER_SET_ERROR("Expecting fractional part for decimal '%s' but encountered end of file", current_lexeme);
				return lexer_ret_invalid;
			}
			else {
				current_lexeme[needle] = '\0';
				LEXER_SET_ERROR("Expecting fractional part for decimal '%s', but found '%c'", current_lexeme, read);
				return lexer_ret_invalid;
			}
			break;
		case state_num_rightside:
			if (is_numeric) {
				current_lexeme[needle++] = read;
			}
			else if (is_alpha) {
				current_lexeme[needle] = '\0';
				LEXER_SET_ERROR("Unexpected alpha character '%c' (ord %d) following decimal literal '%s'", read, read, current_lexeme);
				return lexer_ret_invalid;
			}
			else {
				current_lexeme[needle] = '\0';
				current_token_type = token_type_lit_dec;
				current_value.val_dec = strtod(current_lexeme, NULL);
				if (!is_ws) {
					fseek(fp, -1L, SEEK_CUR); /* move back to prep for next call */
				}
				return lexer_ret_success;
			}
			break;
		}

		if (needle == MAX_LEXEME_LEN - 1) {
			LEXER_SET_ERROR("Lexeme about to exceed max lexeme length (%d)", MAX_LEXEME_LEN);
			return lexer_ret_invalid;
		}
	}

	/* Theoretically unreachable code */
	puts("This should never print!!!");
}

void lexer_close() {
	fclose(fp);
	current_line = 1;
}
