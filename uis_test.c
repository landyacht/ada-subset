#include <stdio.h>
#include <stdint.h>

#include "uint_set.h"

#include "util.c"

int main() {
	struct uint_set one_to_five;
	uint_set_create(&one_to_five, 10);
	for (uint16_t i = 1; i <= 5; i++) {
		uint_set_add(&one_to_five, i);
	}

	struct uint_set evens;
	uint_set_create(&evens, 10);
	for (uint16_t i = 2; i <= 10; i += 2) {
		uint_set_add(&evens, i);
	}

	printf("Should contain 1..5: ");
	print_uis(&one_to_five);

	struct uint_set otf_copy;
	uint_set_copy(&otf_copy, &one_to_five);

	printf("Should be same as above: ");
	print_uis(&otf_copy);

	uint_set_intersect_with(&one_to_five, &evens);
	printf("Should contain 2, 4: ");
	print_uis(&one_to_five);

	uint_set_union_with(&otf_copy, &evens);
	printf("Should contain 1, 2, 3, 4, 5, 6, 8, 10: ");
	print_uis(&otf_copy);
	
	printf("Should be 10: %d\n", uint_set_max(&otf_copy));
	printf("Should be 4: %d\n", uint_set_max(&one_to_five));
	struct uint_set empty;
	uint_set_create(&empty, 50);
	printf("Should be -1: %d\n", uint_set_max(&empty));

	return 0;
}
