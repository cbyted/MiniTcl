#pragma once

#include "lexerExpr.h"
#include "astExpr.h"


/*
 * Parser for Tcl expr command: Recursive descending analyzer and LL(1)
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
    P → A Q                         
    Q → "**" U | ε
    A → NUMBER | VARIABLE | TRUE | FALSE | FUNCTION "(" L ")" | "(" E ")" 
    L → E L1 | ε                    L1 → "," E L | ε
*/

/*
 * Parser struct:
 *  - List of expr tokens
 *  - Structure tha contains all the allocated nodes in an array 
 *  - index of current token being parse
 *  - Total tokens
 */

typedef struct 
{
    MiniTclExprToken *tokens; 
    TreeNodes *tree_nodes;
    ExprNode *root;
    size_t index;
    size_t total_tokens;
} MiniTclExprParser;
