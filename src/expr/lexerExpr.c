#include "../../include/lexerExpr.h"

static MiniTclExprLexer *MiniTclExprLexer_create(const char *expr, size_t len)
{
    MiniTclExprLexer *lexExpr = (MiniTclExprLexer *)xmalloc(sizeof(MiniTclExprLexer));
    lexExpr->p = expr;
    lexExpr->len = len;
    lexExpr->index = 0;
    lexExpr->count = 0;
    lexExpr->capacity = 128;
    lexExpr->tokens = (MiniTclExprToken *)xmalloc(lexExpr->capacity * sizeof(MiniTclExprToken));
    return lexExpr;
}

void MiniTclExprLexer_destroy(MiniTclExprLexer *lexExpr)
{
    if (!lexExpr)
        return;
    // Nota: No se libera memory de los tokens aún ya que estos serán usados en fáses posteriores
    xfree(lexExpr);
}

/*----------------------------------------------
    LEXER PRETTY PRINTER 
-----------------------------------------------*/

static const char *expr_token_type_to_string(ExprTokenType type)
{
    switch (type)
    {
        case MINITCL_TOK_NUMBER:   return "NUMBER";
        case MINITCL_TOK_EXPR_VAR: return "VAR";
        case MINITCL_TOK_POWER:    return "POWER";
        case MINITCL_TOK_ASSIGN:   return "ASSIGN";
        case MINITCL_TOK_GE:       return "GE";
        case MINITCL_TOK_LE:       return "LE";
        case MINITCL_TOK_EQ:       return "EQ";
        case MINITCL_TOK_NE:       return "NE";
        case MINITCL_TOK_GT:       return "GT";
        case MINITCL_TOK_LT:       return "LT";
        case MINITCL_TOK_NOT:      return "NOT";
        case MINITCL_TOK_AND:      return "AND";
        case MINITCL_TOK_OR:       return "OR";
        case MINITCL_TOK_PLUS:     return "PLUS";
        case MINITCL_TOK_MINUS:    return "MINUS";
        case MINITCL_TOK_STAR:     return "STAR";
        case MINITCL_TOK_SLASH:    return "SLASH";
        case MINITCL_TOK_PERCENT:  return "PERCENT";
        case MINITCL_TOK_LPAREN:   return "LPAREN";
        case MINITCL_TOK_RPAREN:   return "RPAREN";
        case MINITCL_TOK_COMMA:    return "COMMA";
        case MINITCL_TOK_BOOLEAN:  return "BOOLEAN";
        case MINITCL_TOK_CALL:     return "CALL";
        default:                   return "UNKNOWN";
    }
}

void printLexerExprTokens(MiniTclExprToken *exprTokens, size_t count)
{
    if (exprTokens == NULL)
        return;

    printf("%-4s %-12s %s\n", "N.º", "TIPO", "VALOR");
    printf("--------------------------------\n");

    for (size_t i = 0; i < count; i++)
    {
        const MiniTclExprToken *tok = &exprTokens[i];
        printf(
            "%-4zu %-12s %.*s\n",
            i,
            expr_token_type_to_string(tok->type),
            (int)tok->length,
            tok->start
        );
    }
}

/*------------------------------------------------* 
 *  Expr lexer: helper methods   
 *------------------------------------------------*/  

static bool ateoe(MiniTclExprLexer *lexExpr) // At end of expr
{
    return lexExpr->len == 0;
}

static bool have(MiniTclExprLexer *lexExpr, size_t n)
{
    return lexExpr->len >= n;
}

static char peek(MiniTclExprLexer *lexExpr)
{
    return *lexExpr->p;
}

static char peek_next(MiniTclExprLexer *lexExpr)
{
    if (have(lexExpr, 1))
        return lexExpr->p[1];
    else
        return '\0';
}

static void consume(MiniTclExprLexer *lexExpr)
{
    if (!ateoe(lexExpr))
    {
        lexExpr->len--;
        lexExpr->index++;
        lexExpr->p++;
    }
}

static void push_token(MiniTclExprLexer *lexExpr, const char *value, size_t length, ExprTokenType type)
{
    size_t pos = lexExpr->count; 
    lexExpr->tokens[pos].start = value;
    lexExpr->tokens[pos].length = length;
    lexExpr->tokens[pos].index = lexExpr->index;
    lexExpr->tokens[pos].type = type;
    lexExpr->count++;
}

static bool isSpace(char ch)
{
    switch (ch) 
    {
        case ' ': 
        case '\t':
        case '\r':
            return true;
        default:
            return false;
    }
}


/*
 * Check if character is not longer a valid numberic symbol or 
 * check if character is not longer a valid boolean 
 */
static bool isWordStop(const char ch)
{
    switch(ch)
    {   
        case 'a' ... 'z':
        case 'A' ... 'Z':
        case '0' ... '9':
        case '+': case '-':
        case '.': case '_':
            return false;
        default:
            return true;
    }
}

/*
 * Check if character is no longer a valid variable symbol
 */
static bool isVarStop(char ch)
{
    switch (ch) 
    {
        case 'a' ... 'z':
        case 'A' ... 'Z':
        case '0' ... '9':
        case '_':  case '{': 
        case '}':
            return false;
        default:
            return true;
    }
}

/*
 * Check if the number read has a valid syntax
 */
static bool isValidNumber(const char *start, size_t len)
{
    if (!start || len == 0)
        return false;

    char ch = start[0];

    // Invalid single: e, E, ., +, -
    if (len == 1 && !isdigit(ch))
        return false;

    // Invalid: Enum, enum, +num, -num 
    if (len > 1 && (ch == 'e' || ch == 'E' || ch == '+' || ch == '-'))
        return false;

    // Invalid: .+, .-, .e, .E, ..
    if (len > 1 && (ch == '.' && !isdigit(start[1])))
        return false;

    //Inalid: nume, numE, num+, num-, num.
    ch = start[len - 1];
    if (len > 1 && (ch == 'e' || ch == 'E' || ch == '+' || ch == '-' || ch == '.'))
        return false;

    /* 
     * Other invalid cases 
     *  More than one '.'
     *  More than one 'e' or 'E'
     *  More than one '+' or '-'
     */
    bool isFloat = false, isExp = false, isExpSigned = false;
    for (size_t i = 0; i < len; i++)
    {
        // Invalid: ee, EE, .., etc 
        if (!isdigit(ch) && (ch == start[i+1]))
            return false; 

        ch = start[i];
        if (ch == '.')
        {
            // More than one decimal . is not a valid number 
            if (!isFloat)
                isFloat = true;
            else 
                return false;
        }
        else if (ch == 'e' || ch == 'E')
        {
            if (!isExp)
                isExp = true;
            else
                return false;
        }
        else if (ch == '+' || ch == '-')
        {
            if (!isExpSigned)
                isExpSigned = true;
            else
                return false;
        }
        else if (!isdigit(ch) && (ch != 'e' || ch != 'E' || ch != '+' || ch != '-' || ch != '.'))
            return false;
        else 
            continue;
    }
    return true;
}


/*
 * Check if the variable read has a valid syntax 
 */

static bool isValidVar(const char *start, size_t len)
{
    if (start == NULL || len < 2 || start[0] != '$')
        return false;

    // First letter 
    char ch = start[1];
    if (!isalpha(ch) && ch != '_')
        return false;

    // Rest of variable 
    for (size_t i = 2; i < len; ++i) 
    {
        ch = start[i];
        if (!isalnum(ch) && ch != '_')
            return false;
    }
    return true;
}


/*
 * Check if read boolean has a valid syntax 
 */

static bool isValidBoolean(const char *start, size_t len)
{
    if (!start || len == 0)
        return false;
    
    // start is not \0 terminated, so we need to check the length of the word
    if (
        !strncmp("true", start, 4) || !strncmp("false", start, 5) ||
        !strncmp("on", start, 2)   || !strncmp("off", start, 3)   ||
        !strncmp("yes", start, 3)  || !strncmp("no", start, 2)
    )
        return true;
    return false;
}

/*
 * Check if word is a function:
 *  - If word is not number 
 *  - If word is not boolean 
 *  - Word might be a function name
 */

static bool isValidCall(const char *start, size_t len)
{
    if (!start || len == 0)
        return false;
    
    for (size_t i = 0; i < len; i++)
    {
        if (!isalnum(start[i]) && start[i] != '_')
            return false;
    }
    return true;
}

/*----------------------------------------------
    LEXER ERROR HANDLING 
-----------------------------------------------*/

// Diseño provisional (Debe mejorar)
static void MiniTclExprLexer_show_error(const char *error, size_t index, const char *fragment, size_t len)
{
    die("%s [index=%zu]: %.*s", error, index, len, fragment);
}

/*------------------------------------------------* 
 *  Expr lexer: scanning tokens
 *------------------------------------------------*/  

/*
 * Scanning numbers:
 *  - 123    supported
 *  - 12.3   supported
 *  - .23    supported
 *  - 10e10  supported
 *  - 10E10  supported
 *  - 10e+2  supported
 *  - 0xhex  not supported
 *  - 0b101  not supported
 *  - 0B101  not supported
 *  - 0o222  not supported
 *  - 0O222  not supported
 *
 * Scanning booleans:
 *  - true|false  supported
 *  - yes/no      supported
 *  - on/off      supported
 */

static void expr_scan_word(MiniTclExprLexer *lexExpr)
{
    const char *start = lexExpr->p;
    size_t len = 0;
    char ch;

    // Read word  
    while (!ateoe(lexExpr))
    {
        ch = peek(lexExpr);
        if (isWordStop(ch))
            break;
    
        consume(lexExpr);
        len++;
    }

    if (isValidNumber(start, len))
    {
        push_token(lexExpr, start, len, MINITCL_TOK_NUMBER);
        return;
    }
    else if (isValidBoolean(start, len))
    {
        push_token(lexExpr, start, len, MINITCL_TOK_BOOLEAN);
        return;
    }
    else if (isValidCall(start, len))
    {
        push_token(lexExpr, start, len, MINITCL_TOK_CALL);
        return;
    }
    else
        MiniTclExprLexer_show_error("La palabra escaneada no esta soportada", lexExpr->index, start-1, len+1);
}

/*
 * Scanning variables
 *  - $var         supported
 *  - $var(indx)   not supported 
 *  - $var::v      not supported
 */

static void expr_scan_var(MiniTclExprLexer *lexExpr)
{
    consume(lexExpr);
    const char *start = lexExpr->p - 1; // Include '$'
    size_t len = 1;
    char ch;

    while (!ateoe(lexExpr)) 
    {
        ch = peek(lexExpr);
        if (isSpace(ch) || isVarStop(ch))
            break;
        consume(lexExpr);
        len++;
    }

    if (len == 1) 
        MiniTclExprLexer_show_error("Un solo '$' no es una variable valida", lexExpr->index, start - 1, 1);

    if (isValidVar(start, len)) 
        push_token(lexExpr, start, len, MINITCL_TOK_EXPR_VAR);
    else 
        MiniTclExprLexer_show_error("Variable mal formada", lexExpr->index, start - 1, len + 1);
}


/*
 * Tokenize expr arguments
 *   Func Calls     supported
 *   variables      supported
 *   numbers        supported
 *   Booleans       supported
 *   <|>|<=|>=      supported
 *   =|==|!=|!      supported
 *   +|-|*|/|/mod   supported
 *   ()             supported
 */

MiniTclExprLexer *MiniTclExprLexer_tokenize(const char *expr, size_t len)
{
    MiniTclExprLexer *lexExpr = MiniTclExprLexer_create(expr, len);

    while (!ateoe(lexExpr))
    {
        const char ch = peek(lexExpr);
        switch (ch)
        {
            case ' ':
            case '\r':
            case '\t':
            {
                consume(lexExpr);
                break;
            }
            case '*':
            {
                if (peek_next(lexExpr) == '*')
                {
                    push_token(lexExpr, lexExpr->p, 2, MINITCL_TOK_POWER);
                    consume(lexExpr);    
                }
                else 
                    push_token(lexExpr, lexExpr->p, 1, MINITCL_TOK_STAR);
                consume(lexExpr);
                break;
            }
            case '>':
            {
                if (peek_next(lexExpr) == '=')
                {
                    push_token(lexExpr, lexExpr->p, 2, MINITCL_TOK_GE);
                    consume(lexExpr);    
                }
                else 
                    push_token(lexExpr, lexExpr->p, 1, MINITCL_TOK_GT);
                consume(lexExpr);
                break;
            }
            case '<':
            {
                if (peek_next(lexExpr) == '=')
                {
                    push_token(lexExpr, lexExpr->p, 2, MINITCL_TOK_LE);
                    consume(lexExpr);    
                }
                else 
                    push_token(lexExpr, lexExpr->p, 1, MINITCL_TOK_LT);
                consume(lexExpr);
                break;
            }
            case '=':
            {
                if (peek_next(lexExpr) == '=')
                {
                    push_token(lexExpr, lexExpr->p, 2, MINITCL_TOK_EQ);
                    consume(lexExpr);    
                }
                else 
                    push_token(lexExpr, lexExpr->p, 1, MINITCL_TOK_ASSIGN);
                consume(lexExpr);
                break;
            }
            case '!':
            {
                if (peek_next(lexExpr) == '=')
                {
                    push_token(lexExpr, lexExpr->p, 2, MINITCL_TOK_NE);
                    consume(lexExpr);    
                }
                else 
                    push_token(lexExpr, lexExpr->p, 1, MINITCL_TOK_NOT);
                consume(lexExpr);
                break;
            }
            case '&':
            {
                if (peek_next(lexExpr) == '&')
                {
                    push_token(lexExpr, lexExpr->p, 2, MINITCL_TOK_AND);
                    consume(lexExpr);    
                }
                else 
                    push_token(lexExpr, lexExpr->p, 1, MINITCL_TOK_BITWISE_AND);
                consume(lexExpr);
                break;
            }
            case '|':
            {
                if (peek_next(lexExpr) == '|')
                {
                    push_token(lexExpr, lexExpr->p, 2, MINITCL_TOK_OR);
                    consume(lexExpr);    
                }
                else 
                    push_token(lexExpr, lexExpr->p, 1, MINITCL_TOK_BITWISE_OR);
                consume(lexExpr);
                break;
            }
            case '+':
            {
                push_token(lexExpr, lexExpr->p, 1, MINITCL_TOK_PLUS);
                consume(lexExpr);
                break;
            }
            case '-':
            {
                push_token(lexExpr, lexExpr->p, 1, MINITCL_TOK_MINUS);
                consume(lexExpr);
                break;
            }
            case '/':
            {
                push_token(lexExpr, lexExpr->p, 1, MINITCL_TOK_SLASH);
                consume(lexExpr);
                break;
            }
            case '%':
            {
                push_token(lexExpr, lexExpr->p, 1, MINITCL_TOK_PERCENT);
                consume(lexExpr);
                break;
            }
            case '(':
            {
                push_token(lexExpr, lexExpr->p, 1, MINITCL_TOK_LPAREN);
                consume(lexExpr);
                break;
            }
            case ')':
            {
                push_token(lexExpr, lexExpr->p, 1, MINITCL_TOK_RPAREN);
                consume(lexExpr);
                break;
            }
            case ',':
            {
                push_token(lexExpr, lexExpr->p, 1, MINITCL_TOK_COMMA);
                consume(lexExpr);
                break;
            }
            case '$':
            {
                expr_scan_var(lexExpr);
                break;
            }
            default:
            {
                // Can be a number, a boolean token or a function (call) token 
                expr_scan_word(lexExpr);
                break;
            }
        }
    }
    return lexExpr;
}
