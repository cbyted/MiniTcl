#include "../../include/lexerErrors.h"

/* Convert the error code into the corresponding message */
static const char *code_to_string(ErrorCode code)
{
    switch (code)
    {
        case UNTERMINATED_QUOTE:
            return "Expected '\"'";
        case UNTERMINATED_BRACE:
            return "Expected '}'";
        case UNTERMINATED_COMMAND:
            return "Expected ']'";
        case ARRAY_NOT_CLOSED:
            return "Expected ')'";
        case INVALID_CHARACTER:
            return "Invalid char";
        case INVALID_VARIABLE:
            return "Invalid var";
        case INVALID_WORD:
            return "Invalid word";
        default:
            return NULL;
    }
}

/* Create and display the error message */
static void lexer_fatal(LexerError *error, const char *source)
{
    fprintf(
        stderr,
        "MiniTcl:%zu:%zu: error[%s]: %s\n",
        error->span.start_line,
        error->span.start_col,
        code_to_string(error->code),
        error->msg
    );

    // Find start of the line
    const char *start = error->span.off_start;
    while (start > source && *(start - 1) != '\n')
        start--;
    
    const char *end = error->span.off_end;
    int len = end - start;
    fputs("  ", stderr);
    fprintf(stderr, "%.*s\n", len, start);
 
    for (int i = 0; i <= len; ++i) 
        fputc(' ', stderr);

    fputc('^', stderr);
    fputc('\n', stderr);
}

/* Show the error message and terminate the program */
void lexer_error(const char *source, const char *msg, ErrorCode code, errorSpan span)
{
    LexerError error = {
        .msg = msg,
        .span = span,
        .code = code
    };
    lexer_fatal(&error, source);
    exit(1);
}
