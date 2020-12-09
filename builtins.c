#include <stdio.h>

void builtin_Put(struct arg_list args) {
	for (int i = 0; i < args.arity; i++) {
		struct exp_val val = args.values[i];
		switch (val.type) {
		case symbol_type_boolean:
			printf(val.value.val_bool ? "True" : "False");
			break;
		case symbol_type_integer:
			printf("%li", val.value.val_int);
			break;
		case symbol_type_float:
			printf("%lF", val.value.val_dec);
			break;
		case symbol_type_string:
			printf("%s", val.value.val_str);
			break;
		case symbol_type_void:
			printf("(void)");
			break;
		}
	}
}

void builtin_PutLn(struct arg_list args) {
	builtin_Put(args);
	puts("");
}

long builtin_GetInt() {
	long scanned;
	scanf("%ld", &scanned);
	return scanned;
}

double builtin_GetFloat() {
	double scanned;
	scanf("%lF", &scanned);
	return scanned;
}

bool builtin_GetBoolean() {
	int scanned;
	while (1) {
		scanf("%d", &scanned);
		if (scanned == 0) {
			return false;
		}
		else if (scanned == 1) {
			return true;
		}
	}
}

char *builtin_GetLn() {
	char *buffer = malloc(sizeof(char) * (MAX_LEXEME_LEN + 1));
	char *result;
	do {
		result = fgets(buffer, MAX_LEXEME_LEN + 1, stdin);
	}
	while ('\n' == buffer[0]);
	if (NULL != result) {
		for (int i = 1; i <= MAX_LEXEME_LEN; i++) {
			if ('\n' == buffer[i]) {
				buffer[i] = '\0';
				break;
			}
		}
	}
	return buffer;
}
