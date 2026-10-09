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

typedef struct 
{
    const char *start;
    size_t length;
    size_t index;
    ExprTokenType type;
} MiniTclExprToken;

typedef struct 
{
    const char *p;
    size_t len;
    size_t index;
    size_t count;
    size_t capacity;
    MiniTclExprToken *tokens;
} MiniTclExprLexer;


void exprLexer_destroy(MiniTclExprLexer *lexExpr);
void printLexerExprTokens(MiniTclExprToken *exprTokens, size_t count);
MiniTclExprLexer *exprLexer_tokenize(const char *expr, size_t len);
