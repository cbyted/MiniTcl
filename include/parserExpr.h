#pragma once

#include "lexerExpr.h"
#include "astExpr.h"


/*
 * Recursive descending analyzer and LL(1)
 *
 * Expr LL(1) Grammar :
    E → O
    O → D O1                        O1 → "||" D O1 | ε
    D → I D1                        D1 → "&&" I D1 | ε
    I → R I1                        I1 → ("=="|"!=") R I1 | ε
    R → S R1                        R1 → ("<"|"<="|">"|">=") S R1 | ε
    S → M S1                        S1 → ("+"|"-") M S1 | ε
    M → U M1                        M1 → ("*"|"/"|"%") U M1 | ε
    U → "+" U | "-" U | "!" U | P
    P → A Q                         Q → "**" U | ε
    A → NUMBER | VARIABLE | TRUE | FALSE | "(" E ")" 

*/


/*
 * This struct saves the following fields:
 *  - Root: Node start of AST 
 *  - Nodes: List of all tree nodes (It's just to free them later)
 *  - Count: Number of tree nodes in the list 
 */

typedef struct 
{
    ExprNode *root;
    ExprNode **nodes;
    size_t count;
    size_t capacity;
} ExprTree;

/*
 * Parser struct:
 *  - List of expr tokens
 *  - Tree structure
 *  - index of current token being parse
 *  - Total tokens
 */

typedef struct 
{
    MiniTclExprToken *tokens; 
    ExprTree *tree;
    size_t idx_token;
    size_t total_tok;
} MiniTclExprParser;
