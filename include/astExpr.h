#pragma once

#include <stdio.h>
#include "lexerExpr.h"

/*
 * Defines the abstract syntax tree structures used to represent Tcl `expr`
 * commands.
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
 * Identifies the numeric representation stored in a NumberNode.
 * Tcl has a single numeric type (WORD), but the parser needs to distinguish integers
 * from real numbers to access the correct union member.
 */

typedef enum
{
    INT,
    REAL
} NumberType;


/*
 * Stores a collection of tree nodes.
 *
 * Fields:
 *   nodes    - Array of pointers to the nodes.
 *   count    - Number of nodes currently stored.
 *   capacity - Allocated capacity of the node array.
 */

typedef struct
{
    ExprNode **nodes;
    size_t count;
    size_t capacity;
} NodeList;


/* Represents a binary operation. */
typedef struct
{
    ExprTokenType op;
    ExprNode *left;
    ExprNode *right;
} BinaryNode;

/* Represents a unary operation. */
typedef struct
{
    ExprTokenType op;
    ExprNode *operand;
} UnaryNode;

/* Stores an integer or real number. */
typedef struct
{
    NumberType type;
    union
    {
        long long int integer;
        double real;
    };
} NumberNode;

/* Represents a variable by its name and length. */
typedef struct
{
    const char *name;
    size_t len;
} VariableNode;

/* Represents a Boolean value: 0 for FALSE, 1 for TRUE. */
typedef struct
{
    int value;
} BooleanNode;

/* Represents a function call, including its name and arguments. */
typedef struct
{
    const char *name;
    size_t len;
    NodeList args;
} CallNode;

/*
 * Represents a node of the 'expr' tree. The node type determines which
 * member of the content union is used.
 *
 * Fields:
 *   type - Type of node.
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
 * Store 'expr' tree's nodes so they can be released together.
 *
 * Fields:
 *   nodes    - Array of pointers to the tree's nodes.
 *   count    - Number of nodes currently stored.
 *   capacity - Allocated capacity of the node array.
 */

typedef struct
{
    ExprNode **nodes;
    size_t count;
    size_t capacity;
} TreeNodes;


/*
 * Ast methods to create tree nodes
 */

ExprNode *expr_node_binary(ExprTokenType op, ExprNode *left, ExprNode *right);
ExprNode *expr_node_unary(ExprTokenType op, ExprNode *operand);
ExprNode *expr_node_number(NumberType type, double number);
ExprNode *expr_node_variable(const char *name, size_t len);
ExprNode *expr_node_boolean(int value);
ExprNode *expr_node_call(const char *name, size_t len, NodeList args);

/* Push ast node to TreeNodes->nodes */
void push_tree_node(TreeNodes *tree_nodes, ExprNode *node);
