#ifndef PARSER_H
#define PARSER_H

int token_count;

#define NUM_RULES 17
struct memotable memotables[NUM_RULES];
#define PROGRAM_MT          memotables[0]
#define PROC_DEF_MT         memotables[1]
#define VAR_DEF_LIST_MT     memotables[2]
#define IDENT_LIST_MT       memotables[3]
#define STMT_LIST_MT        memotables[4]
#define STMT_MT             memotables[5]
#define PROC_CALL_MT        memotables[6]
#define ARG_LIST_MT         memotables[7]
#define ASSIGN_MT           memotables[8]
#define IF_STMT_MT          memotables[9]
#define RETURN_STMT_MT      memotables[10]
#define LOGICAL_EXP_MT      memotables[11]
#define RELATIONAL_EXP_MT   memotables[12]
#define ARITHMETIC_EXP_MT   memotables[13]
#define TERM_MT             memotables[14]
#define FACTOR_MT           memotables[15]
#define VALUE_MT            memotables[16]

enum parser_ret {
	parser_ret_open_fail,
	parser_ret_alloc_fail,
	parser_ret_read_fail,
	parser_ret_lex_fail,
	parser_ret_success,
	parser_ret_partial
};

/* parse - attempt to parse the specified file, gathering all information necessary
 *         for parse tree construction in the memotables
 * Parameters:
 *   filename - The file to be parsed
 * Returns:
 *   parser_ret_open_fail  - The lexer failed to open the file
 *   parser_ret_alloc_fail - Somewhere we failed to allocate memory
 *   parser_ret_read_fail  - The lexer failed to read the file
 *   parser_ret_lex_fail   - The lexer encountered malformed input
 *   parser_ret_success    - The whole file was traversed in the parse
 *   parser_ret_partial    - Only part of the file was traversed in the parse
 */
enum parser_ret parse(char *filename);

#endif
