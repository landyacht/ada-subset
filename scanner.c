#include "scanner.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

enum lexer_state {
	state_start,         /* no characters yet */
	state_maybe_relop,   /* we have encountered < or > or / which may be alone or with an = */
	state_def_relop,     /* we definitely have a relop */
	state_ident_kw_op,   /* we have part or all of an identifier, keyword, or operator (and, or, xor, mod, rem) */
	state_def_binop,     /* we definitely have a binary operator (+ - * &) */
	state_maybe_assign,  /* we either have the assignment op or a colon */
	state_in_str,        /* we're inside a string */
	state_num_leftside,  /* either an integer or the left side of a decimal literal */
	state_num_dec,       /* we're at the decimal point in a decimal literal */
	state_num_rightside, /* the right side of the decimal in a decimal literal */
};

FILE *fp;

int lexer_init(char *filename) {
	fp = fopen(filename, "r");
	if (NULL == fp) {
		sprintf(current_error, "fopen failed with errno = %d", errno);
		return 1;
	}

	return 0;
}

int lexer_next() {
	/* Where in current_lexeme we are */
	static int needle = 0;
	/* Have we reached the end of the file? */
	static bool is_eof = 0;
	/* Our current state, so we know how to transition */
	enum lexer_state cur_state = state_start;

	if (is_eof) {
		return 1;
	}

	char read;
	while (1) {
		int num_read = fread(&read, 1, 1, fp);
		/* Check for error or EOF marker if we didn't read a byte */
		if (0 == num_read) {
			if (ferror(fp)) {
				sprintf(current_error, "fread failed");
				return 2;
			}

			if (feof(fp)) {
				is_eof = 1;
			}

			clearerr(fp);
		}

		/* For convenience in the switch statement */
		bool is_ws = is_eof
			| read == ' '  /* space */
			| read == '\t' /* tabulation */
			| read == '\n' /* newline */
			| read == '\r' /* carriage return */
			;

		/* TODO implement the state machine here */
		switch (cur_state) {
		case state_start:
			break;
		case state_maybe_relop:
			break;
		case state_def_relop:
			break;
		case state_ident_kw_op:
			break;
		case state_def_binop:
			break;
		case state_maybe_assign:
			break;
		case state_in_str:
			break;
		case state_num_leftside:
			break;
		case state_num_dec:
			break;
		case state_num_rightside:
			break;
		}
	}

	/* Theoretically unreachable code */
	puts("This should never print!!!");
}
