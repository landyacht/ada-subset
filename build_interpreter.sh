gcc -g -o interpreter -std=c99 interpreter.c parsetree.c parser.c uint_set.c scanner.c token_store.c \
	memotable.c symbol_table.c
