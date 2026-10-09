#pragma once 

#include <stdio.h>
#include <stdlib.h>
#define ERROR_MAX_SIZE 256

/*
 * Lexical error codes for MiniTcl lexer and expr lexer
 */

typedef enum 
{
    UNTERMINATED_QUOTE,
    UNTERMINATED_BRACE,
    UNTERMINATED_COMMAND,
    ARRAY_NOT_CLOSED,
    INVALID_CHARACTER,
    INVALID_VARIABLE,
    INVALID_WORD
} ErrorCode;

/*
 * Stores the source location of a token.
 * Fields:
 *  - off_start: Pointer to the first character of the token
 *  - off_end:   Pointer just past the last character of the token
 *  - start_line: Line where the token begins
 *  - start_col:  Column where the token begins
 *  - end_line:   Line where the token ends
 *  - end_col:    Column where the token ends
 * 
 * IMPORTANT: The lexer of expr command doesn't store information
 * like column or line. 
 */ 
 
typedef struct 
{
    const char *off_start, *off_end;
    size_t start_line, start_col;
} errorSpan;

/*
 * Store lexical error information 
 * Fields:
 *  - msg: The error message 
 *  - code: The error code 
 *  - span: Error source location 
 */

typedef struct 
{
    const char *msg;
    ErrorCode code;
    errorSpan span;
} LexerError;

/*
 * Reports a lexer error, including the relevant source location.
 *
 * Parameters:
 *   source - Source file.
 *   msg    - Error message to display.
 *   code   - Error code to identify the type of error.
 *   span   - Source span associated with the error.
 */
void lexer_error(const char *source, const char *msg, ErrorCode code, errorSpan span);
