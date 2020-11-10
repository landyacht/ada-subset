#include <stdio.h>

#include "parser.h"

int main(int argc, char **argv) {
	if (argc != 2) {
		printf("usage: %s <inputfile>\n", argv[0]);
		return 1;
	}

	struct node_program *tree = parse(argv[1]);
	printf("Done\n");
}
