#include "../../include/astExpr.h"

/*----------------------------------------------
    PUSHING NODE TO ARRAY OF TREE NODES
-----------------------------------------------*/

void push_tree_node(TreeNodes *tree_nodes, ExprNode *node)
{
    if (tree_nodes->count == tree_nodes->capacity)
    {
        tree_nodes->capacity *= 2;
        size_t newsize= tree_nodes->capacity * sizeof(*tree_nodes);
        tree_nodes->nodes = (ExprNode **)xrealloc(tree_nodes->nodes, newsize);
    }
    tree_nodes->nodes[tree_nodes->count++] = node;
}

/*----------------------------------------------
    CREATE AND RETURN TREE NODES 
-----------------------------------------------*/


static ExprNode *create_node(ExprNodeType type)
{
    ExprNode *node = (ExprNode *)xmalloc(sizeof(ExprNode));
    node->type = type;
    return node;
}

ExprNode *expr_node_binary(ExprTokenType op, ExprNode *left, ExprNode *right)
{
    ExprNode *node = create_node(AST_BINARY);
    node->binary.op = op;
    node->binary.left = left;
    node->binary.right = right;
    return node;
}

ExprNode *expr_node_unary(ExprTokenType op, ExprNode *operand)
{
    ExprNode *node = create_node(AST_UNARY);
    node->unary.op = op;
    node->unary.operand = operand;
    return node;
}

ExprNode *expr_node_number_int(long long int number)
{
    ExprNode *node = create_node(AST_NUMBER);
    node->number.integer = number;
    return node;
}

ExprNode *expr_node_number_real(double number)
{
    ExprNode *node = create_node(AST_NUMBER);
    node->number.real = number;
    return node;
}

ExprNode *expr_node_variable(const char *name, size_t len)
{
    ExprNode *node = create_node(AST_VARIABLE);
    node->variable.name = name;
    node->variable.len = len;
    return node;
}

ExprNode *expr_node_boolean(int value)
{
    ExprNode *node = create_node(AST_BOOLEAN);
    node->boolean.value = value;
    return node;
}

ExprNode *expr_node_call(const char *name, size_t len, NodeList args)
{
    ExprNode *node = create_node(AST_CALL);
    node->call.name = name;
    node->call.len = len;
    node->call.args = args;
    return node;
}
