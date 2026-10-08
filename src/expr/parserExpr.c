#include "../../include/parserExpr.h"

/*------------------------------------------------* 
 *  Create and delete parser struct
 *------------------------------------------------*/  

MiniTclExprParser *exprParser_create(MiniTclExprToken *tokens, size_t count) 
{
    MiniTclExprParser *exprParser = (MiniTclExprParser *)xmalloc(sizeof(MiniTclExprParser));
    exprParser->tokens = tokens;
    exprParser->root = NULL;
    exprParser->index = 0;
    exprParser->total_tokens = count;
    return exprParser;
}

void exprParser_delete(MiniTclExprParser *exprParser)
{
    if (!exprParser)
        return;
    xfree(exprParser);
}
