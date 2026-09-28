#include "../../include/lexer.h"

/*----------------------------------------------
    LEXER CONSTRUCTOR
-----------------------------------------------*/

MiniTclLexer *MiniTclLexer_create(const char *source)
{
    MiniTclLexer *lex = (MiniTclLexer *)xmalloc(sizeof(MiniTclLexer));
    memset(lex, 0, sizeof(*lex));
    lex->p = source;
    lex->len = strlen(source);
    lex->capacity = 1024; 
    lex->count = 0;
    lex->line = 1;
    lex->column = 1;
    lex->index = 0;
    lex->alloc_size = 256;
    lex->alloc_count = 0;
    lex->alloc = (char **)xmalloc(lex->alloc_size * sizeof(char *));
    lex->tokens = (MiniTclToken *)xmalloc(lex->capacity * sizeof(MiniTclToken));
    lex->regex = NULL;
    lex->match = NULL;
    lex->status = MINITCL_LEXER_OK;
    return lex;
}

/*----------------------------------------------
    Allocate more memory for tokens
-----------------------------------------------*/

static void MiniTclLexer_grow_tokens(MiniTclLexer *lex)
{
    lex->capacity *= 2;
    size_t size = lex->capacity * sizeof(MiniTclLexer);
    lex->tokens = xrealloc(lex->tokens, size);
}

/*----------------------------------------------
    LEXER DESTRUCTOR
-----------------------------------------------*/

void MiniTclLexer_destroy(MiniTclLexer *lex)
{
    if (!lex) 
        return;
    pcre2_code_free(lex->regex->name);
    pcre2_code_free(lex->regex->variable);
    lex->regex = NULL;
    for (size_t i = 0; i < lex->alloc_count; i++)
        xfree(lex->alloc[i]);
    xfree(lex->alloc);
    xfree(lex->regex);
    xfree(lex->match);
    xfree(lex->tokens);
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
    for (size_t i = 0; i < lex->count; i++)
    {
        MiniTclToken tok = lex->tokens[i];
        if (lex->tokens[i].tokType == MINITCL_TOK_NEWLINE)
            printf("Token: NEWLINE\n");
        else if (lex->tokens[i].tokType == MINITCL_TOK_ARRAY_VAR)
            printf("Token: (%.*s, %.*s)\n", (int)tok.array->name_length, tok.array->name, (int)tok.array->index_length, tok.array->index);
        else 
            printf("Token: %-10.*s type = %s\n", (int)tok.length, tok.start, token_type_to_string(tok.tokType) );
    }
}

/*----------------------------------------------
    LEXER ERROR HANDLING 
-----------------------------------------------*/

static const char *MiniTclLexer_get_error(MiniTclLexerStatus error)
{
    switch(error)
    {
        case MINITCL_LEXER_CANT_ESCAPE:
            return "Invalid escape sequence.";
        case MINITCL_LEXER_BRACE_NOT_CLOSED:
            return "Unclosed brace word.";
        case MINITCL_LEXER_COMMAND_NOT_CLOSED:
            return "Unclosed command substitution.";
        case MINITCL_LEXER_QUOTE_NOT_CLOSED:
            return "\" String not closed.";
        case MINITCL_LEXER_ARRAY_INDEX_NOT_CLOSED:
            return "Unclosed array index";
        case MINITCL_LEXER_NO_COMMAND_CONTEXT:
            return "Unexpected command substitution terminator.";
        case MINITCL_LEXER_NO_BRACE_CONTEXT:
            return "Unexpected brace word termination.";
        case MINITCL_LEXER_NO_ARRAY_CONTEXT:
            return "Unexpected closed array index.";
        case MINITCL_LEXER_NO_QUOTE_CONTEXT:
            return "Unexpected closed quote word.";
        case MINITCL_LEXER_NO_SCRIPT_CONTEXT:
            return "Unexpected string termination.";;
        case MINITCL_LEXER_ERROR:
            return "Unknown error detected.";
        default:
            return "";
    }
}

static MiniTclLexerError *MiniTclLexer_create_error(
  MiniTclLexerStatus error, size_t line, size_t column)
{
    MiniTclLexerError *err = (MiniTclLexerError *)xmalloc(sizeof(MiniTclLexerError));
    err->error  = error;
    err->line   = line;
    err->column = column;
    err->msg    = MiniTclLexer_get_error(error);
    return err;
}

static void MiniTclLexer_show_error(MiniTclLexerStatus error, size_t line, size_t column)
{
    MiniTclLexerError *err = MiniTclLexer_create_error(error, line, column);
    die("[!] Lexical error Minitcl[line=%zu, col=%zu]: %s", err->line, err->column, err->msg);
}

/*----------------------------------------------
    LEXER  METHODS
-----------------------------------------------*/

static bool MiniTclLexer_ateos(MiniTclLexer *lex) // at end of string
{
    return lex->len == 0 ? true : false;
}

static char MiniTclLexer_peek(MiniTclLexer *lex)
{
    return *lex->p;
}

/* Helper function to consume current char and advance */
static void MiniTclLexer_consume(MiniTclLexer *lex)
{
    if (!MiniTclLexer_ateos(lex))
    {
        lex->len--;
        lex->index++;
        if (*lex->p == '\n')
        {
            lex->line++; 
            lex->column = 1;
        }
        else
            lex->column++;
        ++lex->p;
    }
}

/* Saved allocated chunk for words and variables in array so we can free them later*/
static void MiniTclLexer_pushAlloc(MiniTclLexer *lex, char *alloc)
{
    if (lex->alloc_count == lex->alloc_size)
    {
        lex->alloc_size *= 2;
        lex->alloc = (char **)xrealloc(lex->alloc, lex->alloc_size);
    }
    lex->alloc[lex->alloc_count++] = alloc;
}

/* Save token in lexer */
static void MiniTclLexer_push_token(
  MiniTclLexer *lex, void *value, 
  size_t length, TokenType type)
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

static  bool MiniTclLexer_isspace(char ch)
{
    if (ch == ' ' || ch == '\t' || ch == '\r')
        return true;
    return false;
}

static bool MiniTclLexer_isWordStop(char ch)
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

static bool MiniTclLexer_isVarStop(char ch)
{
    switch (ch) 
    {
        case 'a' ... 'z':
        case 'A' ... 'Z':
        case '0' ... '9':
        case '_': case '\\':
            return false;
        default:
            return true;
    }
}

/*----------------------------------------------
    Regular expressions rules
-----------------------------------------------*/

static pcre2_code *MiniTclLexer_compile_regex(const char *pattern)
{
    int error_number;
    PCRE2_SIZE error_offset;
    pcre2_code *re = pcre2_compile(
        (PCRE2_SPTR8)pattern,           /* the pattern */
        PCRE2_ZERO_TERMINATED,          /* indicates pattern is zero-terminated */
        0,                              /* default options */
        &error_number,                  /* for error number */
        &error_offset,                  /* for error offset */
        NULL                            /* use default compile context */
    );                 

    // PCRE2 error
    if (re == NULL) {
        if (re == NULL) {
            PCRE2_UCHAR message[256];
            int length = pcre2_get_error_message(
                error_number,
                message,
                sizeof(message)
            );
            if (length >= 0) {
                fprintf(stderr,
                        "Pattern error at offset %zu: %.*s\n",
                        (size_t)error_offset,
                        length,
                        (char *)message);
            } 
            else 
            {
                fprintf(stderr,
                        "Pattern error at offset %zu: code %d\n",
                        (size_t)error_offset,
                        error_number);
            }
            exit(EXIT_FAILURE);
        }
    }
    return re;
}

static void MiniTclLexer_regex_rules(MiniTclLexer *lex)
{
    MiniTclLexerRegex *regex = (MiniTclLexerRegex *)xmalloc(sizeof(MiniTclLexerRegex));
    regex->name = MiniTclLexer_compile_regex("[A-Za-z_][A-Za-z0-9_]*\\Z");
    regex->variable = MiniTclLexer_compile_regex("[A-Za-z_][A-Za-z0-9_]*|{[A-Za-z_][A-Za-z0-9_]*}");
    lex->regex = regex;
}

/*-------------------------------------------*
    Scan Command Substitution: [....]
---------------------------------------------*/

// Save content without start and end brackets
static void MiniTclLexer_scan_command_subst(MiniTclLexer *lex)
{
    MiniTclLexer_consume(lex); // Consume first [
    const char *start = lex->p;
    size_t len = 0, depth = 1;
    while (!MiniTclLexer_ateos(lex))
    {
        const char ch = MiniTclLexer_peek(lex);
        if (ch == '\\' && lex->len - 1 > 0)
        {
            MiniTclLexer_consume(lex);
            MiniTclLexer_consume(lex);
            len += 2;
            continue;
        }
        else if (MiniTclLexer_peek(lex) == '[')
            depth++;
        else if (MiniTclLexer_peek(lex) == ']')
        {
            depth--;
            if (depth == 0)
            {
                MiniTclLexer_consume(lex); // consume ]
                MiniTclLexer_push_token(lex, (void *) start, len, MINITCL_TOK_COMMAND_SUBST);
                return;
            }
        } 
        MiniTclLexer_consume(lex);
        len++;
    }
    if (MiniTclLexer_ateos(lex))
    {
        lex->status = MINITCL_LEXER_COMMAND_NOT_CLOSED;
        MiniTclLexer_show_error(lex->status, lex->line, lex->column);   
    }
}

/*-------------------------------------------*
    Scan Braced word {....}
---------------------------------------------*/

// Save content without start and end brackets
static void MiniTclLexer_scan_braced(MiniTclLexer *lex)
{
    MiniTclLexer_consume(lex); // Consume first {
    const char *start = lex->p;
    size_t len = 0, depth = 1;
    while (!MiniTclLexer_ateos(lex))
    {
        const char ch = MiniTclLexer_peek(lex);
        if (ch == '\\' && lex->len - 1 > 0)
        {
            MiniTclLexer_consume(lex);
            MiniTclLexer_consume(lex);
            len += 2;
            continue;
        }
        else if (MiniTclLexer_peek(lex) == '{')
            depth++;
        else if (MiniTclLexer_peek(lex) == '}')
        {
            depth--;
            if (depth == 0)
            {
                MiniTclLexer_consume(lex); // consume }
                MiniTclLexer_push_token(lex, (void *) start, len, MINITCL_TOK_BRACED);
                return;
            }
        } 
        MiniTclLexer_consume(lex);
        len++;
    }
    if (MiniTclLexer_ateos(lex))
    {
        lex->status = MINITCL_LEXER_BRACE_NOT_CLOSED;
        MiniTclLexer_show_error(lex->status, lex->line, lex->column);   
    }
}

/*-------------------------------------------*
    Scan string "...."
---------------------------------------------*/

// Save content without start and end brackets
static void MiniTclLexer_scan_string(MiniTclLexer *lex)
{
    MiniTclLexer_consume(lex); // Consume first "
    const char *start = lex->p;
    size_t len = 0;
    while (!MiniTclLexer_ateos(lex))
    {
        const char ch = MiniTclLexer_peek(lex);    
        if (ch == '\\' && lex->len - 1 > 0)
        {
            MiniTclLexer_consume(lex);
            MiniTclLexer_consume(lex);
            len += 2;
            continue;
        }
        else if (ch == '"')
        {
            MiniTclLexer_consume(lex); // consume "
            MiniTclLexer_push_token(lex, (void *) start, len, MINITCL_TOK_STRING);
            return;
        }
        MiniTclLexer_consume(lex);
        len++;
    }
    if (MiniTclLexer_ateos(lex))
    {
        lex->status = MINITCL_LEXER_QUOTE_NOT_CLOSED;
        MiniTclLexer_show_error(lex->status, lex->line, lex->column);   
    }
}

/*----------------------------------------------
    Scan speacial: //, \n, \\n, #comment
-----------------------------------------------*/

static void MiniTclLexer_scan_newlines(MiniTclLexer *lex)
{
    const char *start = lex->p;
    size_t len = 0;
    while (!MiniTclLexer_ateos(lex) && MiniTclLexer_peek(lex) == '\n')
    {
        MiniTclLexer_consume(lex);
        len++;
    }
    MiniTclLexer_push_token(lex, (void *) start, len, MINITCL_TOK_NEWLINE);
}

static void MiniTclLexer_scan_comment(MiniTclLexer *lex)
{
    while (!MiniTclLexer_ateos(lex) && MiniTclLexer_peek(lex) != '\n') 
        MiniTclLexer_consume(lex);
}

static void MiniTclLexer_scan_single(MiniTclLexer *lex, TokenType type)
{
    MiniTclLexer_push_token(lex, (void *) lex->p, 1, type);
    MiniTclLexer_consume(lex);
}

/*----------------------------------------------
    Lexer scan variable       
-----------------------------------------------*/

static void MiniTclLexer_scan_variable(MiniTclLexer *lex)
{
    MiniTclLexer_consume(lex); // Consume $
    const char *start = lex->p;
    int rc;
    size_t size = 1024, len = 0;
    char *chars = (char *)xmalloc(size);
    memset(chars, 0, size);
    
    // Copy string fragment handling escaped chars 
    while (!MiniTclLexer_ateos(lex))
    {
        const char ch = MiniTclLexer_peek(lex);
        if (MiniTclLexer_isVarStop(ch))
            break;
        if (len == size)
        {
            size *= 2;
            chars = xrealloc(chars, size);
        }
        if (MiniTclLexer_peek(lex) == '\\')
        {
            MiniTclLexer_consume(lex);
            chars[len++] = MiniTclLexer_peek(lex);
            MiniTclLexer_consume(lex);            
            continue;
        }
        chars[len++] = ch;
        MiniTclLexer_consume(lex);
    }
    chars[len] = '\0'; 

    // Check if it's a valid variable type
    lex->match = pcre2_match_data_create_from_pattern(lex->regex->variable, NULL);
    if (lex->match == NULL) 
        die("PCRE2: Error creating match_data\n");

    rc = pcre2_match(
        lex->regex->variable,                
        (PCRE2_SPTR8)chars,                 
        len,                                   
        0,                                   
        PCRE2_ANCHORED,  
        lex->match,                          
        NULL                                 
    );               

    if (rc <= 0)
    {
        xfree(chars);
        fprintf(stderr, "Variable regex matching error\n");
        return;
    }
    else 
    {
        PCRE2_SIZE *ovector = pcre2_get_ovector_pointer(lex->match);
        PCRE2_SIZE matched_len = ovector[1] - ovector[0];
        if (matched_len == 0)
            fprintf(stderr, "Expression produced an empty coincidence\n");
        else 
        {
            MiniTclLexer_pushAlloc(lex, chars);
            lex->p += matched_len;
            len = matched_len;
        }
    }

    // Check if it's an index array expression
    if (!MiniTclLexer_ateos(lex) && MiniTclLexer_peek(lex) == '(')
    {
        MiniTclLexer_consume(lex); // Consume (
        const char *idx_start = lex->p;
        size_t idx_len = 0;
        while(!MiniTclLexer_ateos(lex) && MiniTclLexer_peek(lex) != ')')
        {
            MiniTclLexer_consume(lex);
            idx_len++;
        }
        if (MiniTclLexer_ateos(lex))
        {
            lex->status = MINITCL_LEXER_ARRAY_INDEX_NOT_CLOSED;
            MiniTclLexer_show_error(lex->status, lex->line, lex->column);
        }
        MiniTclLexer_consume(lex); // Consume )
        MiniTclArrayVar *array = (MiniTclArrayVar *)xmalloc(sizeof(MiniTclArrayVar));
        memset(array, 0, sizeof(*array));
        array->name = start;
        array->name_length = len;
        array->index = idx_start;
        array->index_length = idx_len;
        MiniTclLexer_push_token(lex, (void *)array, len + idx_len, MINITCL_TOK_ARRAY_VAR);
    }
    else 
        MiniTclLexer_push_token(lex, (void *)start, len, MINITCL_TOK_VAR);
    pcre2_match_data_free(lex->match);
    lex->match = NULL;
}

/*----------------------------------------------
    Lexer scan word
-----------------------------------------------*/

static void MiniTclLexer_scan_word(MiniTclLexer *lex)
{
    const char *start = lex->p;
    int rc;
    size_t len = 0, size = 1024;
    char *chars = (char *)xmalloc(size);
    memset(chars, 0, size);
    while (!MiniTclLexer_ateos(lex))
    {
        const char ch = MiniTclLexer_peek(lex);
        if (MiniTclLexer_isspace(ch) ||  MiniTclLexer_isWordStop(ch))
            break;
        if (len == size)
        {
            size *= 2;
            chars = xrealloc(chars, size);
        }
        if (MiniTclLexer_peek(lex) == '\\')
        {
            MiniTclLexer_consume(lex);
            chars[len++] = MiniTclLexer_peek(lex);
            MiniTclLexer_consume(lex);            
            continue;
        }
        chars[len++] = ch;
        MiniTclLexer_consume(lex);
    }
    chars[len] = '\0'; 

    // Chck if token 
    lex->match = pcre2_match_data_create_from_pattern(
        lex->regex->name, 
        NULL
    );
    rc = pcre2_match(
        lex->regex->name,                
        (PCRE2_SPTR8)chars,                 
        len,                                   
        0,                                   
        PCRE2_ANCHORED,  
        lex->match,                          
        NULL                                 
    );
    if (rc <= 0)
    {
        xfree(chars);
        fprintf(stderr, "Name regex matching error\n");
        exit(EXIT_FAILURE);
    }     
    else 
    {
        MiniTclLexer_pushAlloc(lex, chars);
        if (rc == PCRE2_ERROR_NOMATCH)
            MiniTclLexer_push_token(lex, (void *)chars, len, MINITCL_TOK_TEXT);
        else
            MiniTclLexer_push_token(lex, (void *)chars, len, MINITCL_TOK_NAME);
    }    
}

/*----------------------------------------------
    Lexer main function
-----------------------------------------------*/

MiniTclLexer *MiniTclLexer_tokenize(const char *source)
{
    MiniTclLexer *lex = MiniTclLexer_create(source);
    MiniTclLexer_regex_rules(lex);
    while (!MiniTclLexer_ateos(lex))
    {
        if (lex->count == lex->capacity)
            MiniTclLexer_grow_tokens(lex);
        
        char ch = MiniTclLexer_peek(lex);
        if (ch == ' ' || ch == '\t' || ch == '\r')
            MiniTclLexer_consume(lex);
        else if (ch == '\n')
            MiniTclLexer_scan_newlines(lex);
        else if (ch == ';')
            MiniTclLexer_scan_single(lex, MINITCL_TOK_SEMICOLON);
        else if (ch == '#')
            MiniTclLexer_scan_comment(lex);
        else if (ch == '[')
            MiniTclLexer_scan_command_subst(lex);
        else if (ch == '{')
            MiniTclLexer_scan_braced(lex);
        else if (ch == '"')
            MiniTclLexer_scan_string(lex);
        else if (ch == '$')
            MiniTclLexer_scan_variable(lex);
        else 
            MiniTclLexer_scan_word(lex);
    }
    return lex;
}