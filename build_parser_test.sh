if [ "$1" = 'debug' ]
then
	gcc -g -o parser_test -std=c99 parser_test.c uint_set.c scanner.c token_store.c -DPARSER_DEBUG
else
	gcc -o parser_test -std=c99 parser_test.c uint_set.c scanner.c token_store.c
fi
