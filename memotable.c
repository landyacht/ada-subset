#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>

#include "memotable.h"
#include "uint_set.h"

struct memotable_entry {
	size_t key;
	int call_count;
	struct uint_set *value;
	struct memotable_entry *next;
};

/* _memotable_find_or_next - Internal routine for finding a key or getting the next free slot
 * Parameters:
 *   table       - The memotable
 *   key         - Key to search for
 *   create      - If true, and the key isn't found, we are allowed to claim a slot and allocate if necessary
 *                 If false, and the key isn't found, mte_ptr_out will be set to NULL
 *   mte_ptr_out - Pointer to the memotable_entry pointer to be filled with the address
 * Returns:
 *   mt_ret_success    - Success
 *   mt_ret_not_found  - We were asked not to claim a slot, and we couldn't find the entry
 *   mt_ret_alloc_fail - We needed to allocate a new link but it failed
 */
enum mt_ret _memotable_find_or_next(
		struct memotable *table,
		size_t key,
		bool create,
		struct memotable_entry **mte_ptr_out
) {
	struct memotable_entry *array = table->array;

	size_t tbl_size = table->size;
	size_t modulus = key % tbl_size;

	/* First, attempt quadratic probing, number of probes proportional to size */
	for (int i = 0; i < tbl_size; i *= 2) {
		struct memotable_entry *current = array + (modulus + i) % table->size;

		if (current->call_count == -1) {
			if (create) {
				current->call_count = 0;
				current->key = key;
				*mte_ptr_out = current;
				return mt_ret_success;
			}
			else {
				*mte_ptr_out = NULL;
				return mt_ret_not_found;
			}
		}
		
		if (current->key == key) {
			*mte_ptr_out = current;
			return mt_ret_success;
		}

		/* If we're going to need to probe, set up the index for it */
		if (0 == i) {
			i = 1;
		}
	}

	/* Fall back on chaining */
	struct memotable_entry *current = array + modulus;
	struct memotable_entry *prev = current;
	while ((current = current->next) != NULL) {
		if (current->key == key) {
			*mte_ptr_out = current;
			return mt_ret_success;
		}

		prev = current;
	}

	if (create) {
		struct memotable_entry *new_node = calloc(sizeof(struct memotable_entry), 1);

		if (NULL == new_node) {
			return mt_ret_alloc_fail;
		}

		new_node->call_count = 0;
		new_node->key = key;

		prev->next = new_node;

		*mte_ptr_out = new_node;
	}
	else {
		*mte_ptr_out = NULL;
		return mt_ret_not_found;
	}
}

enum mt_ret memotable_create(struct memotable *table, size_t size) {
	table->size = size;
	struct memotable_entry *array = calloc(sizeof(struct memotable_entry), size);

	if (NULL == table->array) {
		return mt_ret_alloc_fail;
	}

	for (size_t i = 0; i < size; i++) {
		/* Signal value so we know which buckets are uninitialized */
		array[i].call_count = -1;
	}

	table->array = array;

	return mt_ret_success;
}

enum mt_ret memotable_inc_ct(struct memotable *table, size_t index, int *ct_out) {
	struct memotable_entry *entry;
	if (mt_ret_alloc_fail == _memotable_find_or_next(table, index, true, &entry)) {
		return mt_ret_alloc_fail;
	}

	*ct_out = ++entry->call_count;
	return mt_ret_success;
}

int memotable_get_ct(struct memotable *table, size_t index) {
	struct memotable_entry *entry;
	_memotable_find_or_next(table, index, false, &entry);
	if (NULL == entry) {
		return 0;
	}
	else {
		return entry->call_count;
	}
}

enum mt_ret memotable_store(struct memotable *table, size_t index, struct uint_set *value) {
	struct memotable_entry *entry;
	if (mt_ret_alloc_fail == _memotable_find_or_next(table, index, true, &entry)) {
		return mt_ret_alloc_fail;
	}

	if (uis_ret_alloc_fail == uint_set_copy(entry->value, value)) {
		return mt_ret_alloc_fail;
	}

	return mt_ret_success;
}

enum mt_ret memotable_get_val(struct memotable *table, size_t index, struct uint_set *value_out) {
	struct memotable_entry *entry;
	if (mt_ret_not_found == _memotable_find_or_next(table, index, false, &entry)) {
		return mt_ret_not_found;
	}

	if (uis_ret_alloc_fail == uint_set_copy(value_out, entry->value)) {
		return mt_ret_alloc_fail;
	}

	return mt_ret_success;
}

void memotable_destroy(struct memotable *table) {
	/* First deallocate any chained entries */
	size_t tbl_size = table->size;
	struct memotable_entry *tbl_array = table->array;
	for (size_t i = 0; i < tbl_size; i++) {
		struct memotable_entry *cur = tbl_array[i].next;
		while (NULL != cur) {
			struct memotable_entry *next = cur->next;
			free(cur);
			cur = next;
		}
	}

	/* Now we can deallocate the main array */
	free(tbl_array);

	table->size = 0;
}
