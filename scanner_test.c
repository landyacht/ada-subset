/*
Class:       CS 4308 Section 2
Term:        Fall 2020
Name:        A. Eubanks, A. Watson, and A. J. Odeyemi
Instructor:   Deepa Muralidhar
Project:  Deliverable 1 Scanner - C
*/
#include "scanner.h"

#include <stdio.h>
#include <stdbool.h>

int main(int argc, char **argv) {
	if (argc != 2) {
		printf("usage: %s <inputfile>\n", argv[0]);
		return 1;
	}

	printf("Initializing lexer with filename = '%s'... ", argv[1]);
	enum lexer_ret init_ret = lexer_init(argv[1]);
	if (lexer_ret_fopen_fail == init_ret) {
		printf("lexer_init failed: %s\n", lexer_current_err);
		return 2;
	}
	puts("lexer_init succeeded.");

	puts("Scanning file until end or error. Lexemes are presented in brackets to show any leading/trailing whitespace.");
	bool keep_going = true;
	while (keep_going) {
		enum lexer_ret next_ret = lexer_next();
		switch (next_ret) {
		case lexer_ret_success:
			printf("Token ID %2d / Lexeme: [%s]\n", current_token_type, current_lexeme);
			break;
		case lexer_ret_eof:
			puts("Successfully reached end of file.");
			keep_going = false;
			break;
		case lexer_ret_fread_fail:
			puts("lexer_next returned lexer_ret_fread_fail.");
			keep_going = false;
			break;
		case lexer_ret_invalid:
			puts("lexer_next returned lexer_ret_invalid. The error is:");
			puts(lexer_current_err);
			keep_going = false;
			break;
		}
	}

	printf("De-initializing lexer... ");
	lexer_close();
	puts("Done.");

	return 0;
}
