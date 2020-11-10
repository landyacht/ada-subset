#include <stdio.h>

#include "memotable.h"
#include "uint_set.h"
#include "util.c"

int main() {
	struct memotable table;

	memotable_create(&table, 40);
	printf("Should be zeroes: %d, %d\n",
		memotable_get_ct(&table, 0),
		memotable_get_ct(&table, 45)
	);

	memotable_inc_ct(&table, 5, NULL);
	memotable_inc_ct(&table, 45, NULL);
	int two;
	memotable_inc_ct(&table, 5, &two);

	printf("Should be 2, 2, 1: %d, %d, %d\n",
		two,
		memotable_get_ct(&table, 5),
		memotable_get_ct(&table, 45)
	);

	struct uint_set set;
	uint_set_create(&set, 7);
	uint_set_add(&set, 4);
	memotable_store(&table, 5, &set);
	uint_set_add(&set, 5);
	memotable_store(&table, 45, &set);
	memotable_inc_ct(&table, 85, NULL);
	memotable_inc_ct(&table, 125, NULL);
	memotable_inc_ct(&table, 165, NULL);
	memotable_inc_ct(&table, 205, NULL);
	uint_set_add(&set, 6);
	memotable_store(&table, 245, &set);

	printf("Should be { 4 }: ");
	struct uint_set out_set;
	memotable_get_val(&table, 5, &out_set);
	print_uis(&out_set);
	uint_set_destroy(&out_set);

	printf("Should be { 4, 5 }: ");
	memotable_get_val(&table, 45, &out_set);
	print_uis(&out_set);
	uint_set_destroy(&out_set);

	printf("Should be 2: %d\n", memotable_get_val(&table, 85, &out_set));

	memotable_destroy(&table);
	puts("After table destroyed");

	return 0;
}
