#include "../../include/lexer.h"


/*----------------------------------------------
    LEXER CONSTRUCTOR
-----------------------------------------------*/

static MiniTclLexer *lexer_create(const char *source)
{
    MiniTclLexer *lex = (MiniTclLexer *)xmalloc(sizeof(MiniTclLexer));
    memset(lex, 0, sizeof(*lex));
    lex->source = source;
    lex->p = source;
    lex->len = strlen(source);
    lex->capacity = 128; 
    lex->count = 0;
    lex->line = 1;
    lex->column = 1;
    lex->index = 0;
    lex->tokens = (MiniTclToken *)xmalloc(lex->capacity * sizeof(MiniTclToken));
    return lex;
}

/*----------------------------------------------
    Allocate more memory for tokens
-----------------------------------------------*/

static void lexer_grow_tokens(MiniTclLexer *lex)
{
    lex->capacity *= 2;
    size_t size = lex->capacity * sizeof(MiniTclLexer);
    lex->tokens = xrealloc(lex->tokens, size);
}

/*----------------------------------------------
    LEXER DESTRUCTOR
-----------------------------------------------*/

void lexer_destroy(MiniTclLexer *lex)
{
    if (!lex) 
        return;
    // Nota: No se libera memory de los tokens aún ya que estos serán usados en fáses posteriores
    xfree(lex);
}

/*----------------------------------------------
    DEBUGGING
-----------------------------------------------*/

static const char *token_type_to_string(TokenType type)
{
    switch (type)
    {
        case MINITCL_TOK_NAME:          return "NAME";
        case MINITCL_TOK_TEXT:          return "TEXT";
        case MINITCL_TOK_BRACED:        return "BRACED";
        case MINITCL_TOK_STRING:        return "STRING";
        case MINITCL_TOK_COMMAND_SUBST: return "COMMAND_SUBST";
        case MINITCL_TOK_VAR:           return "VAR";
        case MINITCL_TOK_ARRAY_VAR:     return "ARRAY";
        case MINITCL_TOK_SEMICOLON:     return "SEMICOLON";
        case MINITCL_TOK_NEWLINE:       return "NEWLINE";
        default:                        return "UNKNOWN";
    }
}

void printLexerTokens(MiniTclLexer *lex)
{
    if (lex == NULL)
        return;

    printf("%-4s %-16s %s\n", "N", "TIPO", "LEXEMA");
    printf("------------------------------------------\n");

    for (size_t i = 0; i < lex->count; i++)
    {
        const MiniTclToken *tok = &lex->tokens[i];

        printf("%-4zu %-16s ", i, token_type_to_string(tok->tokType));

        if (tok->tokType == MINITCL_TOK_NEWLINE)
        {
            printf("\\n\n");
        }
        else if (tok->tokType == MINITCL_TOK_ARRAY_VAR)
        {
            printf(
                "(%.*s, %.*s)\n",
                (int)tok->array->name_length,
                tok->array->name,
                (int)tok->array->index_length,
                tok->array->index
            );
        }
        else
            printf("%.*s\n", (int)tok->length, tok->start);
    }
}

/*----------------------------------------------
    LEXER  METHODS
-----------------------------------------------*/

static bool ateos(MiniTclLexer *lex) // at end of string
{
    return lex->len == 0;
}

static char peek(MiniTclLexer *lex)
{
    return ateos(lex) ? '\0' : *lex->p;
}

/* Helper function to consume current char and advance */
static void consume(MiniTclLexer *lex)
{
    if (!ateos(lex))
    {
        lex->len--;
        lex->index++;
        if (peek(lex) == '\n')
        {
            lex->line++; 
            lex->column = 1;
        }
        else
            lex->column++;
        lex->p++;
    }
}

static void push_token(MiniTclLexer *lex, void *value, size_t length, TokenType type)
{
    size_t pos = lex->count;
    if (type == MINITCL_TOK_ARRAY_VAR)
        lex->tokens[pos].array = (MiniTclArrayVar *)value;
    else 
        lex->tokens[pos].start = (const char *)value;
    lex->tokens[pos].length  = length;
    lex->tokens[pos].line    = lex->line;
    lex->tokens[pos].column  = lex->column;
    lex->tokens[pos].index   = lex->index;
    lex->tokens[pos].tokType = type;
    lex->count++;
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

static bool isWordStop(char ch)
{
    switch (ch) 
    {
        case '[': case '{':
        case '"': case '$':
        case ';': case '\n':
            return true;
        default:
            return false;
    }
}

static bool isVarStop(char ch)
{
    switch (ch) 
    {
        case 'a' ... 'z':
        case 'A' ... 'Z':
        case '0' ... '9':
        case '_': case '{':
        case '}':
            return false;
        default:
            return true;
    }
}

static bool isValidIdentifier(const char *start, size_t len)
{
    char ch;  
    if (start == NULL || len == 0)
        return false;
    
        // The first letter can only contain letters and '_'
    ch = start[0];
    if (isalpha(ch) || ch == '_')
    {
        // Check rest of the word
        for (size_t i = 1; i < len; i++)
        {
            ch = start[i];
            if (!isalnum(ch) && ch != '_')
                return false;
        }
        return true;
    }
    return false;
}

static bool isValidVar(const char *start, size_t len)
{
    size_t i;
    char ch;
    
    if (start == NULL || len == 0)
        return false;

    ch = start[0];
    if (ch == '{')
    {
        /* Check if var has at least {x} */
        if (len < 3 || start[len - 1] != '}')
            return false;
        
        /* Internal content */    
        for (i = 1; i < len - 1; ++i)
        {
            ch = start[i];
            if (!isalnum(ch) && ch != '_')
                return false;
        }
        return true;
    }

    // First letter
    if (!isalpha(ch) && ch != '_')
        return false;
    // Rest of var    
    for (i = 1; i < len; ++i)
    {
        ch = start[i];
        if (!isalnum(ch) && ch != '_')
            return false;
    }
    return true;
}

/*-------------------------------------------*
    Scan Command Substitution: [....]
---------------------------------------------*/

// Save content without start and end brackets
static void scan_command_subst(MiniTclLexer *lex)
{
    consume(lex); // Consume first [
    const char *start = lex->p;
    size_t len = 0, depth = 1;
    size_t start_line = lex->line, start_col = lex->column;
    while (!ateos(lex))
    {
        const char ch = peek(lex);
        if (ch == '\\' && lex->len - 1 > 0)
        {
            consume(lex);
            consume(lex);
            len += 2;
            continue;
        }
        else if (peek(lex) == '[')
            depth++;
        else if (peek(lex) == ']')
        {
            depth--;
            if (depth == 0)
            {
                consume(lex); // consume ]
                push_token(lex, (void *) start, len, MINITCL_TOK_COMMAND_SUBST);
                return;
            }
        } 
        consume(lex);
        len++;
    }
    if (ateos(lex))
    {
        errorSpan span = {
            .off_start = start,
            .off_end = lex->p,
            .start_line = start_line,
            .start_col = start_col 
        };
        lexer_error(lex->source, "Command Substitution not closed", UNTERMINATED_COMMAND, span);
    }
}

/*-------------------------------------------*
    Scan Braced word {....}
---------------------------------------------*/

// Save content without start and end brackets
static void scan_braced(MiniTclLexer *lex)
{
    consume(lex); // Consume first {
    const char *start = lex->p;
    size_t len = 0, depth = 1;
    size_t start_line = lex->line, start_col = lex->column;
    while (!ateos(lex))
    {
        const char ch = peek(lex);
        if (ch == '\\' && lex->len - 1 > 0)
        {
            consume(lex);
            consume(lex);
            len += 2;
            continue;
        }
        else if (peek(lex) == '{')
            depth++;
        else if (peek(lex) == '}')
        {
            depth--;
            if (depth == 0)
            {
                consume(lex); // consume }
                push_token(lex, (void *) start, len, MINITCL_TOK_BRACED);
                return;
            }
        } 
        consume(lex);
        len++;
    }
    if (ateos(lex))
    {
        errorSpan span = {
            .off_start = start,
            .off_end = lex->p,
            .start_line = start_line,
            .start_col = start_col 
        };
        lexer_error(lex->source, "Brace word not closed", UNTERMINATED_BRACE, span);
    }
}

/*-------------------------------------------*
    Scan string "...."
---------------------------------------------*/

// Save content without start and end brackets
static void scan_string(MiniTclLexer *lex)
{
    consume(lex); // Consume first "
    const char *start = lex->p;
    size_t len = 0;
    size_t start_line = lex->line, start_col = lex->column;
    while (!ateos(lex))
    {
        const char ch = peek(lex);    
        if (ch == '\\' && lex->len - 1 > 0)
        {
            consume(lex);
            consume(lex);
            len += 2;
            continue;
        }
        else if (ch == '"')
        {
            consume(lex); // consume "
            push_token(lex, (void *) start, len, MINITCL_TOK_STRING);
            return;
        }
        consume(lex);
        len++;
    }
    if (ateos(lex))
    {
        errorSpan span = {
            .off_start = start,
            .off_end = lex->p,
            .start_line = start_line,
            .start_col = start_col 
        };
        lexer_error(lex->source, "Quote not closed", UNTERMINATED_QUOTE, span);
    }
}

/*----------------------------------------------
    Scan speacial: //, \n, \\n, #comment
-----------------------------------------------*/

static void scan_newlines(MiniTclLexer *lex)
{
    const char *start = lex->p;
    size_t len = 0;
    while (!ateos(lex) && peek(lex) == '\n')
    {
        consume(lex);
        len++;
    }
    push_token(lex, (void *) start, len, MINITCL_TOK_NEWLINE);
}

static void scan_comment(MiniTclLexer *lex)
{
    while (!ateos(lex) && peek(lex) != '\n') 
        consume(lex);
}

static void scan_single(MiniTclLexer *lex, TokenType type)
{
    push_token(lex, (void *) lex->p, 1, type);
    consume(lex);
}

/*----------------------------------------------
    Lexer scan variable       
-----------------------------------------------*/

static void scan_variable(MiniTclLexer *lex)
{
    consume(lex); // Consume $

    const char *start = lex->p;
    size_t len = 0;
    size_t start_line = lex->line, start_col = lex->column;
    char ch;
    
    while (!ateos(lex))
    {
        ch = peek(lex);
        if (isSpace(ch) || isVarStop(ch))
            break;
        len++;
        consume(lex);
    }

    if (ateos(lex) || len == 0)  // Just a single '$'
     {
        errorSpan span = {
            .off_start = start,
            .off_end = lex->p,
            .start_line = start_line,
            .start_col = start_col 
        };
        lexer_error(lex->source, "'$' is not a valid variable", INVALID_VARIABLE, span);
    }     

    if (isValidVar(start, len))
    {
        if (!ateos(lex) && peek(lex) == '(')
        {
            consume(lex); // Consume (
            const char *idx_start = lex->p;
            size_t idx_len = 0; 
            bool closed = false;
            
            while (!ateos(lex))
            {
                if (peek(lex) == ')')
                {
                    closed = true;
                    break;
                }
                idx_len++;
                consume(lex);
            }

            if (ateos(lex) && !closed)
            {
                errorSpan span = {
                    .off_start = start,
                    .off_end = lex->p,
                    .start_line = start_line,
                    .start_col = start_col 
                };
                lexer_error(lex->source, "Index array not closed", ARRAY_NOT_CLOSED, span);
             }     

            consume(lex); // Consume )
            MiniTclArrayVar *array = (MiniTclArrayVar *)xmalloc(sizeof(MiniTclArrayVar));
            array->name = start;
            array->name_length = len;
            array->index = idx_start;
            array->index_length = idx_len;
            push_token(lex, (void *)array, len + idx_len, MINITCL_TOK_ARRAY_VAR);
        }
        else 
            push_token(lex, (void *)start, len, MINITCL_TOK_VAR);
    }
    else
    {
        errorSpan span = {
            .off_start = start,
            .off_end = lex->p,
            .start_line = start_line,
            .start_col = start_col 
        };
        lexer_error(lex->source, "Malformed variable", INVALID_VARIABLE, span);
    }      
}

/*----------------------------------------------
    Lexer scan word
-----------------------------------------------*/

static void scan_word(MiniTclLexer *lex)
{
    const char *start = lex->p;
    size_t len = 0;

    while (!ateos(lex))
    {
        const char ch = peek(lex);        
        if (isSpace(ch) || isWordStop(ch))
            break; 
        if (ch == '\\')
        {
            consume(lex);      /* Consume '\' char */
            if (ateos(lex))    
                break;  // Improve lexerError
            consume(lex);     
            len+=2;
            continue;
        }
        len++;
        consume(lex);
    }

    if (isValidIdentifier(start, len))
        push_token(lex, (void *)start, len, MINITCL_TOK_NAME);    
    else 
        push_token(lex, (void *)start, len, MINITCL_TOK_TEXT);          
}

/*----------------------------------------------
    Lexer main function
-----------------------------------------------*/

MiniTclLexer *lexer_tokenize(const char *source)
{
    MiniTclLexer *lex = lexer_create(source);
    while (!ateos(lex))
    {
        if (lex->count == lex->capacity)
            lexer_grow_tokens(lex);
        
        char ch = peek(lex);
        if (ch == ' ' || ch == '\t' || ch == '\r')
            consume(lex);
        else if (ch == '\n')
            scan_newlines(lex);
        else if (ch == ';')
            scan_single(lex, MINITCL_TOK_SEMICOLON);
        else if (ch == '#')
            scan_comment(lex);
        else if (ch == '[')
            scan_command_subst(lex);
        else if (ch == '{')
            scan_braced(lex);
        else if (ch == '"')
            scan_string(lex);
        else if (ch == '$')
            scan_variable(lex);
        else 
            scan_word(lex);
    }
    return lex;
}
