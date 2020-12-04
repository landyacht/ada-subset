if [ "$1" = "debug" ]
then
	gcc -g -o parsetree_test -std=c99 parsetree_test.c parsetree.c parser.c uint_set.c scanner.c token_store.c \
		memotable.c -DPARSETREE_DEBUG
else
	gcc -o parsetree_test -std=c99 parsetree_test.c parsetree.c parser.c uint_set.c scanner.c token_store.c \
		memotable.c
fi
