#ifndef UINT_SET_H
#define UINT_SET_H

#include <stdint.h>

struct uint_set {
        uint16_t max;
        uint16_t num_buckets;
        uint8_t *buckets;
};

enum uis_ret {
	uis_ret_success,
	uis_ret_true,
	uis_ret_false,
	uis_ret_alloc_fail,
	uis_ret_oob,
};

/* uint_set_create - Create a new set in the struct pointed to
 * Parameters:
 *   uis_out - Pointer to a uint_set struct to be filled
 *   max     - Greatest possible value for a member of this set
 * Returns:
 *   uis_ret_success    - Success
 *   uis_ret_alloc_fail - malloc() failed
 */
enum uis_ret uint_set_create(struct uint_set *uis_out, uint16_t max);

int uint_set_isempty(struct uint_set *uis);

/* uint_set_copy - Create a copy of a set
 * Parameters:
 *   dest - Pointer to a struct to be filled with the copy
 *   orig - Original to be copied
 * Returns:
 *   uis_ret_success    - Successfully copied
 *   uis_ret_alloc_fail - Failed to allocate memory for the copy
 */
enum uis_ret uint_set_copy(struct uint_set *dest, struct uint_set *orig);

/* uint_set_add - Add an integer to the set
 * Parameters:
 *   uis   - Pointer to the uint_set struct
 *   value - Integer to be added
 * Returns:
 *   uis_ret_success - Success
 *   uis_ret_oob     - value was out of bounds
 */
enum uis_ret uint_set_add(struct uint_set *uis, uint16_t value);

/* uint_set_contains - Check if the set contains a value
 * Parameters:
 *   uis   - Pointer to the uint_set struct
 *   value - The value to check for
 * Returns:
 *   uis_ret_true  - The set contains the value
 *   uis_ret_false - The set does not contain the value
 *   uis_ret_oob   - The value was out of bounds
 */
enum uis_ret uint_set_contains(struct uint_set *uis, uint16_t value);

/* uint_set_intersect_with - Intersect this set with another (mutates)
 * Parameters:
 *   uis_this  - Pointer to the uint_set to be modified
 *   uis_other - Pointer to the other operand (not mutated)
 * Returns:
 *   uis_success - The sets were intersected successfully
 *   uis_oob     - uis_other had a larger max than uis_this
 */
enum uis_ret uint_set_intersect_with(struct uint_set *uis_this, struct uint_set *uis_other);

/* uint_set_union_with - Union this set with another (mutates)
 * Parameters:
 *   uis_this  - Pointer to the uint_set to be modified
 *   uis_other - Pointer to the other operand (not mutated)
 * Returns:
 *   uis_success - The sets were unioned successfully
 *   uis_oob     - uis_other had a larger max than uis_this
 */
enum uis_ret uint_set_union_with(struct uint_set *uis_this, struct uint_set *uis_other);

/* uint_set_destroy - Free memory associated with the set */
void uint_set_destroy(struct uint_set *uis);

#endif
