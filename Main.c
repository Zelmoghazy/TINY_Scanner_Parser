#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Scanner.h"
#include "Parser.h"


int main(int argc, char* argv[])
{
    if(argc != 4){
        DEBUG_PRT("Please enter filename and -scanner/-parser -toklist/-lexer.\n");
        exit(EXIT_FAILURE);
    }

    char filename[FILENAME_MAX]; // Maximum file name on windows probably too much

    assert(strlen(argv[1]) < FILENAME_MAX);

    strcpy(filename,argv[1]);

    if(!strcmp(argv[2], "-scanner")){
        fileScanner(filename, "./data/out.txt");
    }else if(!strcmp(argv[2], "-parser")){
        if(!strcmp(argv[3], "-lexer")){                     // Use Lexer as source of tokens
            fileParser(filename,"./data/tree.dot",true);
        }else if(!strcmp(argv[3], "-toklist")){
            fileParser(filename,"./data/tree.dot",false);   // Use an external token list as source of tokens
        }else{
            DEBUG_PRT("Please specify -toklist/-lexer..\n");
        }
    }else{
        DEBUG_PRT("Please specify -scanner/-parser.\n");
        exit(EXIT_FAILURE);
    }
    return 0;
}
