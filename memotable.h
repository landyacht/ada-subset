#ifndef MEMOTABLE_H
#define MEMOTABLE_H

#include <stddef.h>

#include "uint_set.h"

enum mt_ret {
	mt_ret_success,
	mt_ret_alloc_fail,
	mt_ret_not_found,
};

struct memotable_entry;
struct memotable {
	size_t size;
	struct memotable_entry *array;
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

/* memotable_inc_ct - Increment the call count for a particular token index
 * Parameters:
 *   table  - The memotable
 *   index  - Index of the token
 *   ct_out - Pointer to an integer to be set to the count, or NULL if not interested
 * Returns:
 *   mt_ret_success    - Success
 *   mt_ret_alloc_fail - We needed to chain, but allocating a new link failed
 */
enum mt_ret memotable_inc_ct(struct memotable *table, size_t index, int *ct_out);

/* memotable_get_ct - Get the call count for a particular token index
 * Parameters:
 *   table  - The memotable
 *   index  - Index of the token
 * Returns: The call count for this index (doesn't necessarily mean there is an entry)
 */
int memotable_get_ct(struct memotable *table, size_t index);

/* memotable_store - Memoize a return value (of type struct int_set) for the given token index
 * Paramters:
 *   table - The memotable
 *   index - Index of the token
 *   value - The set to be memoized (gets copied)
 * Returns:
 *   mt_ret_success    - Success
 *   mt_ret_alloc_fail - We needed to chain, but allocating a new link failed, or copying the
 *                       uint_set failed due to memory allocation issues
 */
enum mt_ret memotable_store(struct memotable *table, size_t index, struct uint_set *value);

/* memotable_get_val - Try to get a memoized return value for the given token index
 * Parameters:
 *   table     - The memotable
 *   index     - Index of the token
 *   value_out - Pointer to a struct uint_set which will be filled with a copy of the stored value
 * Returns:
 *   mt_ret_success    - Success
 *   mt_ret_not_found  - There was not a memoized value
 *   mt_ret_alloc_fail - Copying the uint_set failed due to memory allocation issues
 */
enum mt_ret memotable_get_val(struct memotable *table, size_t index, struct uint_set *value_out);

/* memotable_destroy - Deallocate memory associated with a table */
void memotable_destroy(struct memotable *table);

#endif
