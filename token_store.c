#include <stdint.h>
#include <stdlib.h>

#include "token_store.h"
#include "scanner.h"

static uint8_t *tokens;
static union lexeme_value *values;
static int needle, cur_size;

enum ts_ret token_store_init() {
	tokens = malloc(TS_INITIAL_SIZE * sizeof(uint8_t));
	values = malloc(TS_INITIAL_SIZE * sizeof(union lexeme_value));

	if (NULL == tokens || NULL == values) {
		return ts_ret_alloc_fail;
	}

	needle = 0;
	cur_size = TS_INITIAL_SIZE;

	return ts_ret_success;
}

enum ts_ret token_store_add(
		enum token_type type,
		enum token_subtype subtype,
		union lexeme_value value
) {
	/* allocate more space if needed */
	if (needle == cur_size) {
		cur_size *= 2;
		tokens = realloc(tokens, cur_size * sizeof(uint8_t));
		values = realloc(values, cur_size * sizeof(union lexeme_value));
		
		if (NULL == tokens || NULL == values) {
			return ts_ret_alloc_fail;
		}
	}

	/* The type goes in the 5 high bits and the subtype goes in the 3 low bits */
	uint8_t type_packed = (uint8_t) (type << 3 | subtype);
	tokens[needle++] = type_packed;

	values[needle++] = value;
}

enum ts_ret token_store_get(
		int i,
		enum token_type *type_out,
		enum token_subtype *subtype_out,
		union lexeme_value *value_out
) {
	/* At least one of the pieces of token type information is requested */
	if (NULL != type_out || NULL != subtype_out) {
		uint8_t type_packed = tokens[i];
		if (NULL != type_out) {
			*type_out = type_packed >> 3;
		}
		if (NULL != subtype_out) {
			*subtype_out = type_packed & 7;
		}
	}

	if (NULL != value_out) {
		*value_out = values[i];
	}
}

void token_store_deinit() {
	free(tokens);
	free(values);
}
