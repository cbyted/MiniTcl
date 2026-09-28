#include "../include/lexer.h"

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


int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Include file\n");
        return EXIT_FAILURE;
    }
    const char *file = openSourceCodeFile(argv[1]);
    MiniTclLexer *lex = MiniTclLexer_tokenize(file);
    printLexerTokens(lex);
    return EXIT_SUCCESS;
}