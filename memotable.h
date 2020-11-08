#ifndef MEMOTABLE_H
#define MEMOTABLE_H

#include <stddef.h>

enum mt_ret {
	mt_ret_success,
	mt_ret_alloc_fail,
	mt_ret_not_found,
};

struct memotable_entry;
struct memotable {
	size_t size;
	struct memotable_entry *table;
};

/* memotable_create - Creates a new memotable in the struct given
 * Parameters:
 *   table - Pointer to a struct to be filled
 *   size  - Number of buckets
 * Returns:
 *   mt_ret_success    - Success
 *   mt_ret_alloc_fail - Couldn't allocate the memory
 */
enum mt_ret memotable_create(struct memotable *table, size_t size);

/* memotable_inc_ctr - Increment the call count for a particular token index
 * Parameters:
 *   table - The memotable
 *   index - Index of the token
 * Returns: The new count
 */
int memotable_inc_ctr(struct memotable *table, size_t index);

/* memotable_get_ctr - Get the call count for a particular token index
 * Parameters:
 *   table - The memotable
 *   index - Index of the token
 * Returns: The current count
 */
int memotable_get_ctr(struct memotable *table, size_t index);

/* memotable_store - Memoize a return value (of type struct int_set) for the given token index
 * Paramters:
 *   table - The memotable
 *   index - Index of the token
 *   value - The set to be memoized
 * Returns:
 *   mt_ret_success    - Success
 *   mt_ret_alloc_fail - We needed to chain, but allocating a new link failed
 */
enum mt_ret memotable_store(struct memotable *table, size_t index, struct int_set *value);

/* memotable_get_val - Try to get a memoized return value for the given token index
 * Parameters:
 *   table     - The memotable
 *   index     - Index of the token
 *   value_out - Pointer to a struct int_set which will be filled with a copy of the stored value
 * Returns:
 *   mt_ret_success   - Success
 *   mt_ret_not_found - There was not a memoized value
 */
enum mt_ret memotable_get_val(struct memotable *table, size_t index, struct int_set *value_out);

/* memotable_destroy - Deallocate memory associated with a table */
void memotable_destroy(struct memotable *table);

#endif
