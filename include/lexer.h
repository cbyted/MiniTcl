#pragma once 
#include <ctype.h>
#include "safeAlloc.h"
#include "lexerErrors.h"

/*---------------------------------------------- 
 * Lexical analysis
 *---------------------------------------------*/ 

/* Token types for Tcl */ 
typedef enum
{
    MINITCL_TOK_NAME,           // set, local_1
    MINITCL_TOK_TEXT,           // 10, -nonewline
    MINITCL_TOK_BRACED,         // {...}
    MINITCL_TOK_COMMAND_SUBST,  // [...]
    MINITCL_TOK_STRING,         // "..."
    MINITCL_TOK_VAR,            // $var
    MINITCL_TOK_ARRAY_VAR,      // $arr(..)
    MINITCL_TOK_SEMICOLON,      // ;
    MINITCL_TOK_NEWLINE,        // \n
} TokenType;

/*
 * Represents an array variable, including its name and index expression.
 *
 * Fields:
 *   name         - Name of the array variable.
 *   index        - Index expression inside the parentheses.
 *   name_length  - Length of the array variable name.
 *   index_length - Length of the index expression.
 */

typedef struct
{
    const char *name;
    const char *index;
    size_t name_length;
    size_t index_length;
} MiniTclArrayVar;

/*
 * Represents a token produced by the lexer, including its value and source
 * location.
 *
 * Fields:
 *   start  - Pointer to the start of the token's lexeme.
 *   array  - Array-variable data for tokens representing `$array(index)`.
 *   length - Length of the token's lexeme.
 *   line   - Source line where the token was encountered.
 *   column - Source column where the token starts.
 *   index  - Absolute position of the token in the source file.
 *   tokType - Type of token.
 */

typedef struct MintclToken_t
{
    union
    {
        const char *start;
        MiniTclArrayVar *array;
    };
    size_t length;
    size_t line;
    size_t column;
    size_t index;
    TokenType tokType;
} MiniTclToken;

/*
 * Holds the lexer state and the tokens produced from the source text.
 *
 * Fields:
 *   source   - Source text being tokenized.
 *   p        - Current position in the source text.
 *   len      - Length of the source text.
 *   capacity - Allocated capacity of the token array.
 *   count    - Number of tokens currently stored.
 *   line     - Current source line.
 *   column   - Current source column.
 *   index    - Absolute index.
 *   tokens   - Array of tokens produced by the lexer.
 */

typedef struct
{
    const char *source;
    const char *p;
    size_t len;
    size_t capacity;
    size_t count;
    size_t line;
    size_t column;
    size_t index;
    MiniTclToken *tokens;
} MiniTclLexer;

// Print tokens
void printLexerTokens(MiniTclLexer *lex);

/*
 * Releases the resources owned by a lexer.
 * Parameters:
 *   lex - Pointer to the lexer to destroy.
 */
void lexer_destroy(MiniTclLexer *lex);

/*
 * Tokenizes the source text and returns a pointer to the resulting lexer.
 * Parameters:
 *   source - Source text to tokenize.
 * Returns:
 *   Pointer to the lexer containing the generated tokens.
 */
MiniTclLexer *lexer_tokenize(const char *source);
