#include "../include/lexer.h"
#include "../include/lexerExpr.h"


/*----------------------------------------------
    OPEN SOURCE FILE 
-----------------------------------------------*/

static int getFileLength(FILE *fp)
{
    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (size < 0)
        die("Failed to get file size");
    return size;
}

char *openSourceCodeFile(const char *filename)
{
    FILE *fp = fopen(filename, "r");
    size_t bytes_read = 0;
    int BUFFER_SIZE = 0;
    char *buffer;
    if (!fp)
        die("fopen failed to open file %s", filename);
    BUFFER_SIZE = getFileLength(fp);
    buffer = (char *) xcalloc(BUFFER_SIZE + 1, sizeof(char));
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, fp)) > 0);
    fclose(fp);
    return buffer;
}


/*----------------------------------------------
    QUICK TESTS FOR COMMAND: "expr"
-----------------------------------------------*/

const char *exprTests[] =
{
    "$VAR + 1.23 * 2 ** 3",
    "a >= b && a != 0",    "a = min(12, 1.23) + max(2, 4)",
    "!true || false && true", 
    "sqrt(9) + abs(-2) * min(1, 2)",
    "$x + max(1, 2) * 3 >= 4 && !false || true",
    "($VAR ** 2 + 1.23) / 2 >= min(12, 4) && true",
};

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("[!] Please include a Tcl source file\n");
        return EXIT_FAILURE;
    }
    const char *file = openSourceCodeFile(argv[1]);

    // Tcl Lexer quick test
    printf("MiniTcl lexer test\n");
    MiniTclLexer *lex = lexer_tokenize(file);
    printLexerTokens(lex);

    printf("\n\n");

    // expr lexer quick test 
   
    printf("expr lexer tests\n");
    size_t test_count = sizeof(exprTests) / sizeof(exprTests[0]);
    for (size_t i = 0; i < test_count; i++)
    {
        printf("\nExpr test: %s\n", exprTests[i]);
        MiniTclExprLexer *lexExpr = exprLexer_tokenize(exprTests[i], strlen(exprTests[i]), i);

        if (lexExpr == NULL)
        {
            printf("[!] MiniTclExprLexer: Failed to tokenize expression\n");
            continue;
        }
        printLexerExprTokens(lexExpr->tokens, lexExpr->count);
        xfree(lexExpr->tokens);
        exprLexer_destroy(lexExpr);
    }

    // Free tokens 
    xfree(lex->tokens);

    // Destroy structs 
    lexer_destroy(lex);

    return EXIT_SUCCESS;
}
