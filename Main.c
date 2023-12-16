#include "Scanner.h"
#include "Parser.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

char* loadfile(char *path)
{
    /* Reading */
    FILE *reading_file = fopen(path, "r");
    if (!reading_file){
        fprintf(stderr, "Couldnt load reading_file");
        return NULL;
    }
    /* Doesnt return accurate size */
    fseek(reading_file, 0, SEEK_END);
    size_t size = (size_t)ftell(reading_file);
    fseek(reading_file, 0, SEEK_SET);

    char *source = (char *)malloc(sizeof(char) * size + 2);
    assert(source);
    size = 0;
    int ch = fgetc(reading_file);
    /* read input character by character */
    while (ch!=EOF)
    {
        source[size++]=(char)ch;
        ch = fgetc(reading_file);
    }
    source[size++] = '\n';     // Always insert a newline at the end.
    source[size]   = '\0';
    fclose(reading_file);

    return source;
}

int main(int argc, char* argv[])
{
    if(argc != 2){
        fprintf(stderr,"Please enter filename\n");
        exit(EXIT_FAILURE);
    }
    char filename[256]; // Maximum file name on windows probably too much
    strcpy(filename,argv[1]);

    fileScanner(filename, "out.txt");
    char *source = loadfile(filename);

    Parser *parser = newParser(source);

    Node *syntaxTree = parse(parser);

    printTree(syntaxTree);

    printDotTree("tree.dot",syntaxTree);

    return 0;
}
