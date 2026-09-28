#pragma once

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>
#include "safeAlloc.h"

/*---------------------------------------------- 
 * Lexer error handling
 *---------------------------------------------*/ 

typedef enum
{
    MINITCL_LEXER_OK,
    MINITCL_LEXER_ERROR,
    MINITCL_LEXER_CANT_ESCAPE,
    MINITCL_LEXER_BRACE_NOT_CLOSED,
    MINITCL_LEXER_COMMAND_NOT_CLOSED,
    MINITCL_LEXER_QUOTE_NOT_CLOSED,
    MINITCL_LEXER_ARRAY_INDEX_NOT_CLOSED,
    MINITCL_LEXER_NO_COMMAND_CONTEXT,
    MINITCL_LEXER_NO_BRACE_CONTEXT,
    MINITCL_LEXER_NO_ARRAY_CONTEXT,
    MINITCL_LEXER_NO_QUOTE_CONTEXT,
    MINITCL_LEXER_NO_SCRIPT_CONTEXT
} MiniTclLexerStatus;

typedef struct 
{
    MiniTclLexerStatus error;
    size_t line;
    size_t column;
    const char *msg;
} MiniTclLexerError;

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

/* Regex operations for simple tokens */
typedef struct 
{
    pcre2_code *name;       
    pcre2_code *variable;  
} MiniTclLexerRegex;

/* Lexer information */
typedef struct 
{
    const char *p;               // Current source char         
    size_t len;                  // Source file length
    size_t capacity;             // Tokens capacity
    size_t count;                // Tokens count
    size_t line;                 // Token line
    size_t column;               // Token columns
    size_t index;                // Token index
    size_t alloc_size;           // Allocated chunks
    size_t alloc_count;          // Total of allocated chunks
    char **alloc;                // Allocated cucks for words
    MiniTclToken *tokens;        // List of tokens
    MiniTclLexerRegex *regex;    // Regex rules
    pcre2_match_data *match;     // Regex match
    MiniTclLexerStatus status;   // Lexer operation status
} MiniTclLexer;

/*----------------------------------------------
    LEXER PUBLIC API
-----------------------------------------------*/

// Create and delete
MiniTclLexer *MiniTclLexer_create(const char *source);
void MiniTclLexer_destroy(MiniTclLexer *lex);

void printLexerTokens(MiniTclLexer *lex);

// Tokenize 
MiniTclLexer *MiniTclLexer_tokenize(const char *source);