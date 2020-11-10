#ifndef TOKEN_STORE_H
#define TOKEN_STORE_H

#include <math.h>

#include "scanner.h"

#define TS_INITIAL_SIZE 25

/* Named return codes */
enum ts_ret {
	ts_ret_success,
	ts_ret_alloc_fail,
	ts_ret_oob, /* out of bounds */
};

/* token_store_init - Allocate memory and set initial values
 * Returns:
 *   ts_ret_success    - Success
 *   ts_ret_alloc_fail - malloc failed
 */
enum ts_ret token_store_init();

/* token_store_add - Add a token to the list
 * Parameters:
 *   type    - The token's type
 *   subtype - The token's subtype (or token_subtype_none)
 *   value   - A value associated with the token (or ts_no_value)
 * Returns:
 *   ts_ret_success    - Success
 *   ts_ret_alloc_fail - The array needed to be reallocated and this failed
 *
 */
enum ts_ret token_store_add(
		enum token_type type,
		enum token_subtype subtype,
		union lexeme_value value
);

/* token_store_get - Get a specific token's information
 * Parameters:
 *   i           - Index of the token
 *   type_out    - Pointer to a token_type variable where the type will be stored, or NULL if not needed
 *   subtype_out - Pointer to a token_subtype variable ""
 *   value_out   - Pointer to a lexeme_value variable ""
 * Returns:
 *   ts_ret_success - Success
 *   ts_ret_oob     - i was out of bounds
 */
enum ts_ret token_store_get(
		int i,
		enum token_type *type_out,
		enum token_subtype *subtype_out,
		union lexeme_value *value_out
);

/* token_store_deinit - Free memory */
void token_store_deinit();

#endif
