#pragma once
#include "safeAlloc.h"
#include "lexer.h"
#include "lexerErrors.h"

/*
 * The following cases are not supported :
 *  - Command substitution
 *  - Strings
 */

typedef enum 
{
    MINITCL_TOK_NUMBER,          // 12, 1.23, -2, ...
    MINITCL_TOK_EXPR_VAR,        // $VAR
    MINITCL_TOK_POWER,           // **
    MINITCL_TOK_GE,              // >=
    MINITCL_TOK_LE,              // <=
    MINITCL_TOK_EQ,              // ==
    MINITCL_TOK_ASSIGN,          // =
    MINITCL_TOK_NE,              // !=
    MINITCL_TOK_GT,              // >
    MINITCL_TOK_LT,              // <
    MINITCL_TOK_NOT,             // !
    MINITCL_TOK_BITWISE_OR,      // &
    MINITCL_TOK_AND,             // &&
    MINITCL_TOK_BITWISE_AND,     // |
    MINITCL_TOK_OR,              // ||
    MINITCL_TOK_PLUS,            // +
    MINITCL_TOK_MINUS,           // -
    MINITCL_TOK_STAR,            // *
    MINITCL_TOK_SLASH,           // /
    MINITCL_TOK_PERCENT,         // %
    MINITCL_TOK_LPAREN,          // (
    MINITCL_TOK_RPAREN,          // )
    MINITCL_TOK_COMMA,           // ,
    MINITCL_TOK_BOOLEAN,         // true, yes, on, ...
    MINITCL_TOK_CALL,            // Function call: sqrt, abs, etc 
} ExprTokenType;


/*
 * Represents a token from an expr command, including its location in the source.
 *
 * Fields:
 *   start  - Pointer to the first character of the token in the source text.
 *   length - Number of characters in the token.
 *   index  - Token's starting offset in the source text.
 *   type   - Kind of token.
 */


typedef struct 
{
    const char *start;
    size_t length;
    size_t index;
    ExprTokenType type;
} MiniTclExprToken;


/*
 * Holds the lexer state and the tokens produced while parsing an expression.
 *
 * Fields:
 *   source   - Original expression text.
 *   p        - Current position in the source text.
 *   len      - Length of the source text.
 *   line     - Source line where the expression was encountered.
 *   index    - absolute position in the source .
 *   count    - Number of tokens produced.
 *   capacity - Allocated capacity of the token array.
 *   tokens   - Array of tokens produced by the lexer.
 */

typedef struct 
{
    const char *source;
    const char *p;
    size_t len;
    size_t line;
    size_t index;
    size_t count;
    size_t capacity;
    MiniTclExprToken *tokens;
} MiniTclExprLexer;

// Print tokens
void printLexerExprTokens(MiniTclExprToken *exprTokens, size_t count);

/*
 * Releases the resources owned by an expression lexer.
 * Parameters:
 *   lexExpr - Pointer to the lexer to destroy.
 */
void exprLexer_destroy(MiniTclExprLexer *lexExpr);

/*
 * Tokenizes the arguments of an `expr` command.
 * Parameters:
 *   expr - The contents of the `expr` command.
 *   len  - The length of `expr`.
 *   line - The source line where the command was encountered.
 * Returns:
 *   Pointer to the lexer containing the generated tokens of expr command.
 */
MiniTclExprLexer *exprLexer_tokenize(const char *expr, size_t len, size_t line);
