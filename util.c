#include <stdio.h>
#include <stdint.h>

#include "uint_set.h"

void print_uis(struct uint_set *set) {
	printf("{ ");
	for (uint16_t i = 0; i <= set->max; i++) {
		if (uis_ret_true == uint_set_contains(set, i)) {
			printf("%d, ", i);
		}
	}
	printf(" }\n");
}
