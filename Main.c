#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Scanner.h"
#include "Parser.h"


int main(int argc, char* argv[])
{
    if(argc != 3){
        DEBUG_PRT("Please enter filename and -scanner/-parser.\n");
        exit(EXIT_FAILURE);
    }
    
    char filename[256]; // Maximum file name on windows probably too much
    strcpy(filename,argv[1]);

    if(!strcmp(argv[2], "-scanner"))
    {
        fileScanner(filename, "out.txt");
    }
    else if(!strcmp(argv[2], "-parser"))
    {
        fileParser(filename,"tree.dot");
        system("dot -Tpng -o output.png tree.dot");
    }
    else
    {
        DEBUG_PRT("Please specify -scanner/-parser.\n");
        exit(EXIT_FAILURE);
    }
    return 0;
}
