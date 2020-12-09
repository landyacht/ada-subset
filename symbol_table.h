#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "scanner.h"

#define SYMTBL_MAX_NAME_LEN 16

enum symtbl_ret {
	symtbl_ret_success,
	symtbl_ret_not_found,
	symtbl_ret_type_mismatch,
	symtbl_ret_name_too_long,
	symtbl_ret_already_defined
};

enum symbol_type {
	symbol_type_integer,
	symbol_type_float,
	symbol_type_string,
	symbol_type_boolean,
	symbol_type_void,
};

/* symtbl_init - Initialize the symbol table */
void symtbl_init();

/* symtbl_add - Add a new symbol to the table
 * Parameters:
 *   namespace - The namespace in which the symbol lives
 *   name      - The symbol's name
 *   type      - Type of the symbol
 *   pos_out   - Will be filled with the index of the symbol
 * Returns:
 *   symtbl_ret_success         - The symbol was added successfully
 *   symtbl_ret_name_too_long   - The symbol name had more than SYMTBL_MAX_NAME_LEN chars
 *   symtbl_ret_already_defined - The symbol has already been defined
 */
enum symtbl_ret symtbl_add(char *namespace, char *name, enum symbol_type type, int *pos_out);

/* symtbl_find - Search the table for a particular name
 * Parameters:
 *   namespace - The namespace in which the symbol lives
 *   name      - The symbol name
 * Returns: The index of the symbol's entry, or -1 if not found
 */
int symtbl_find(char *namespace, char *name);

/* symtbl_update - Update the value of a symbol
 * Parameters:
 *   pos   - The index of the symbol
 *   type  - Type of the new value for verification
 *   value - New value for the symbol
 * Returns:
 *   symtbl_ret_success       - The symbol was updated successfully
 *   symtbl_ret_type_mismatch - The type of the new value does not match
 *   symtbl_ret_not_found     - Could not find symbol at pos
 */
enum symtbl_ret symtbl_update(int pos, enum symbol_type type, union lexeme_value value);

/* symtbl_get - Get the value of a symbol
 * Parameters:
 *   pos       - The index of the symbol
 *   type_out  - Pointer to a type to be filled
 *   value_out - Pointer to value to be filled
 * Returns:
 *   symtbl_ret_success   - The info was successfully copied
 *   symtbl_ret_not_found - Could not find symbol at pos
 */
enum symtbl_ret symtbl_get(int pos, enum symbol_type *type_out, union lexeme_value *value_out);

#endif
