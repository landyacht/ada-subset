#include <stdlib.h>
#include <string.h>

#include "symbol_table.h"

struct symtbl_entry {
	char namespace[SYMTBL_MAX_NAME_LEN + 1];
	char name[SYMTBL_MAX_NAME_LEN + 1];
	enum symbol_type type;
	union lexeme_value value;
};

static struct symtbl_entry *array;
static int array_size, elems;

void symtbl_init() {
	array = calloc(10, sizeof(struct symtbl_entry));
	array_size = 10;
}

enum symtbl_ret symtbl_add(char *namespace, char *name, enum symbol_type type, int *pos_out) {
	if (SYMTBL_MAX_NAME_LEN < strlen(name)) {
		return symtbl_ret_name_too_long;
	}

	for (int i = 0; i < elems; i++) {
		struct symtbl_entry *entry = array + i;
		if ((NULL == namespace || 0 == strcmp(entry->namespace, namespace)) && 0 == strcmp(entry->name, name)) {
			return symtbl_ret_already_defined;
		}
	}

	if (elems == array_size) {
		array_size *= 2;
		array = realloc(array, array_size * sizeof(struct symtbl_entry));
	}

	if (NULL != namespace) {
		strcpy(array[elems].namespace, namespace);
	}
	strcpy(array[elems].name, name);
	array[elems].type = type;
	*pos_out = elems;

	elems++;
	return symtbl_ret_success;
}

int symtbl_find(char *namespace, char *name) {
	for (int i = 0; i < elems; i++) {
		struct symtbl_entry *entry = array + i;
		if ((NULL == entry->namespace || 0 == strcmp(entry->namespace, namespace)) && 0 == strcmp(entry->name, name)) {
			return i;
		}
	}

	return -1;
}

enum symtbl_ret symtbl_update(int pos, enum symbol_type type, union lexeme_value value) {
	if (pos >= elems) {
		return symtbl_ret_not_found;
	}

	if (type != array[pos].type) {
		return symtbl_ret_type_mismatch;
	}

	array[pos].value = value;

	return symtbl_ret_success;
}

enum symtbl_ret symtbl_get(int pos, enum symbol_type *type_out, union lexeme_value *value_out) {
	if (pos >= elems) {
		return symtbl_ret_not_found;
	}

	if (NULL != type_out) {
		*type_out = array[pos].type;
	}

	if (NULL != value_out) {
		*value_out = array[pos].value;
	}

	return symtbl_ret_success;
}
