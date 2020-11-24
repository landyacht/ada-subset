#include <stdio.h>

#include "parser.c"
#include "memotable.c"
#include "util.c"

void print_memotable(struct memotable *mt);

char *memotable_names[NUM_RULES] = {
	"program",
	"proc_def",
	"var_def_list",
	"ident_list",
	"stmt_list",
	"stmt",
	"proc_call",
	"arg_list",
	"assign",
	"if_stmt",
	"return_stmt",
	"logical_exp",
	"relational_exp",
	"arithmetic_exp",
	"term",
	"factor",
	"value"
};

int main(int argc, char **argv) {
	if (argc != 2) {
		printf("usage: %s <inputfile>\n", argv[0]);
		return 1;
	}

	parse(argv[1]);

	enum token_type tt;
	enum token_subtype tst;

#ifdef PARSER_DEBUG
	for (int i = 0; i < token_count; i++) {
		token_store_get(i, &tt, &tst, NULL);
		printf("token @ %d is of type %d, subtype %d\n", i, tt, tst);
	}

	for (int i = 0; i < NUM_RULES; i++) {
		printf("\n--- Memotable for rule %s ---\n", memotable_names[i]);
		print_memotable(&memotables[i]);
	}
#endif

	struct uint_set whole_program_result;
	if (mt_ret_success == memotable_get_val(&PROGRAM_MT, 0, &whole_program_result)) {
		if (uis_ret_true == uint_set_contains(&whole_program_result, token_count)) {
			puts("Parse successfully traversed entire token stream");
			return 0;
		}
		else {
			if (uis_ret_true == uint_set_isempty(&whole_program_result)) {
				puts("Parse failed completely");
				return -1;
			}
			for (int i = token_count - 1; i >= 0; i++) {
				printf("Parse succeeded until token %d, then failed\n", i);
				return i;
			}
		}
	}

	return 0;
}

void print_memotable(struct memotable *mt) {
	struct uint_set out;
	for (int i = 0; i < token_count; i++) {
		if (mt_ret_success == memotable_get_val(mt, i, &out)) {
			printf("%d => ", i);
			print_uis(&out);
		}
	}
}
