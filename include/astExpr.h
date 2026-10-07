#pragma once

#include <stdio.h>
#include "lexerExpr.h"

/*
 * Abstract Syntaxt Tree for Tcl expr command
 */

typedef struct ExprNode ExprNode;

/*
 *
 * AST Nodes types:
 *  - Binary
 *  - Unary 
 *  - Number
 *  - Variable
 *  - Boolean
 *  - Call 
 *
 * */

typedef enum 
{
    AST_BINARY,
    AST_UNARY,
    AST_NUMBER,
    AST_VARIABLE,
    AST_BOOLEAN,
    AST_CALL,
} ExprNodeType;


/*
 * Tcl doesn't have type numbers. But this enum 
 * is necessary to access the correct number in 
 * NumberNode 
 */
typedef enum 
{
    INT,
    REAL
} NumberType;


/*
 * List of tree nodes 
 */

typedef struct 
{
    ExprNode **nodes;
    size_t count;
    size_t capacity;
} NodeList;


/*
 * Definition of each tree node 
 */

typedef struct
{
    ExprTokenType op;
    ExprNode *left;
    ExprNode *right;
} BinaryNode;

typedef struct 
{
    ExprTokenType op;
    ExprNode *operand;
} UnaryNode;

typedef struct 
{
    NumberType type;
    union 
    {
        long long int integer;
        double real;
    };
} NumberNode;

typedef struct
{
    const char *name;
    size_t len;
} VariableNode;

typedef struct 
{
    int value; /* 0 || 1 */ 
} BooleanNode;

typedef struct 
{
    const char *name;
    size_t len;
    NodeList args;
} CallNode;

/*
 * Tree node structure:
 *  - Type
 *  - Content
 */

typedef struct ExprNode 
{
    ExprNodeType type;
    union 
    {
        BinaryNode binary;
        UnaryNode unary;
        NumberNode number;
        VariableNode variable;
        BooleanNode boolean;
        CallNode call;
    };
} ExprNode;

/*
 * Ast methods to create tree nodes
 */

ExprNode *expr_node_binary(ExprTokenType op, ExprNode *left, ExprNode *right);
ExprNode *expr_node_unary(ExprTokenType op, ExprNode *operand);
ExprNode *expr_node_number(NumberType type, double number);
ExprNode *expr_node_variable(const char *name, size_t len);
ExprNode *expr_node_boolean(int value);
ExprNode *expr_node_call(const char *name, size_t len, NodeList args);
