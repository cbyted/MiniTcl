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

typedef struct 
{
    const char *name;
    const char *index;
    size_t name_length;   
    size_t index_length;
} MiniTclArrayVar;

/* Token information */
typedef struct MintclToken_t
{
    union
    {
        const char *start;      // Clasic lexema
        MiniTclArrayVar *array;  // $array(index)
    };
    size_t length;              // Length of lexema
    size_t line;                // Line of lexema
    size_t column;              // Column of lexema
    size_t index;               // Index in tokens array
    TokenType tokType; 
} MiniTclToken;


/* Lexer information */
typedef struct 
{
    const char *source;          // Save source
    const char *p;               // Current source pointer       
    size_t len;                  // Source file length
    size_t capacity;             // Tokens capacity
    size_t count;                // Tokens count
    size_t line;                 // Token line
    size_t column;               // Token columns
    size_t index;                // Token index
    MiniTclToken *tokens;        // List of tokens
} MiniTclLexer;


// Delete lexer
void lexer_destroy(MiniTclLexer *lex);
void printLexerTokens(MiniTclLexer *lex);

// Tokenize 
MiniTclLexer *lexer_tokenize(const char *source);
