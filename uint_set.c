#include <stdint.h>
#include <stdlib.h>

#include "uint_set.h"

struct uint_set {
	uint16_t max;
	uint16_t num_buckets;
	uint8_t *buckets;
};

enum uis_ret uint_set_create(struct uint_set *uis_out, uint16_t max) {
	/* We add one to max since we need enough space for zero up to and including it */
	uint16_t num_buckets = (max + 1) / 8;
	if ((max + 1) % 8) { /* not evenly divisible by 8 */
		num_buckets++; /* so we need to use part of another bucket */
	}

	uint8_t *buckets = calloc(sizeof(uint8_t), num_buckets);
	if (NULL == buckets) {
		return uis_ret_alloc_fail;
	}

	uis_out->max = max;
	uis_out->num_buckets = num_buckets;
	uis_out->buckets = buckets;

	return uis_ret_success;
}

enum uis_ret uint_set_copy(struct uint_set *dest, struct uint_set *orig) {
	dest->max = orig->max;
	uint16_t num_buckets = orig->num_buckets;
	dest->num_buckets = num_buckets;
	
	uint8_t *buckets_dest = calloc(sizeof(uint8_t), orig->num_buckets),
	        *buckets_orig = orig->buckets;
	for (uint16_t i = 0; i < num_buckets; i++) {
		buckets_dest[i] = buckets_orig[i];
	}

	dest->buckets = buckets_dest;
}

enum uis_ret uint_set_add(struct uint_set *uis, uint16_t value) {
	if (value > uis->max) {
		return uis_ret_oob;
	}

	/* Get the byte in which the associated bit lies */
	uint8_t bucket_idx = value / 8;

	/* Lowest-order bit is associated with the first value in the bucket */
	uint8_t mask = 1 << (value % 8);

	/* OR to flip the correct bit */
	uis->buckets[bucket_idx] |= mask;

	return uis_ret_success;
}

enum uis_ret uint_set_contains(struct uint_set *uis, uint16_t value) {
	if (value > uis->max) {
		return uis_ret_oob;
	}

	/* Mask off all but the relevant bit */
	uint8_t mask = 1 << (value % 8);
	/* It will be nonzero iff the relevant bit is 1 */
	if (uis->buckets[value / 8] & mask) {
		return uis_ret_true;
	}
	else {
		return uis_ret_false;
	}
}

enum uis_ret uint_set_intersect_with(struct uint_set *uis_this, struct uint_set *uis_other) {
	if (uis_other->max > uis_this->max) {
		return uis_ret_oob;
	}

	uint16_t num_buckets = uis_this->num_buckets;
	uint8_t *this_buckets = uis_this->buckets,
	        *other_buckets = uis_other->buckets;
	for (uint16_t i = 0; i < num_buckets; i++) {
		this_buckets[i] &= other_buckets[i];
	}

	return uis_ret_success;
}

enum uis_ret uint_set_union_with(struct uint_set *uis_this, struct uint_set *uis_other) {
	if (uis_other->max > uis_this->max) {
		return uis_ret_oob;
	}

	uint16_t num_buckets = uis_this->num_buckets;
	uint8_t *this_buckets = uis_this->buckets,
	        *other_buckets = uis_other->buckets;
	for (uint16_t i = 0; i < num_buckets; i++) {
		this_buckets[i] |= other_buckets[i];
	}

	return uis_ret_success;
}

void uint_set_destroy(struct uint_set *uis) {
	free(uis->buckets);
	uis->num_buckets = 0; /* signal value */
}
