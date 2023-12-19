#include "Scanner.h"
#include <assert.h>
#include <corecrt.h>
#include <stdio.h>
#include <stdlib.h>


Token reservedWords[] = {
    {IF, "if"},     {THEN, "then"},     {ELSE, "else"},
    {END, "end"},   {REPEAT, "repeat"}, {UNTIL, "until"},
    {READ, "read"}, {WRITE, "write"}
};

const char *tokentostring[] = {
    "IDENTIFIER", "NUMBER",     "ASSIGN",       "EQUAL",
    "LESSTHAN",   "PLUS",       "MINUS",        "MULT",
    "DIV",        "OPENBRACKET","CLOSEDBRACKET","SEMICOLON",
    "IF",         "THEN",       "ELSE",         "END",
    "REPEAT",     "UNTIL",      "READ",         "WRITE",
    "ERROR",      "ENDOFILE"
};

Token getReservedToken(char* str)
{
    for (size_t i = 0; i < sizeof(reservedWords) / sizeof(reservedWords[0]); i++) {
        if (!strcmp(str, reservedWords[i].Literal)) {
            return reservedWords[i];
        }
    }
    Token tok = {ID,str};
    return tok;
}

bool iswhitespace(char ch) 
{
    if (ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t') {
        return true;
    }else {
        return false;
    }
}
char *chartostr(char ch){
    char *res = malloc(sizeof(char)*2);
    res[0] = ch;
    res[1] = '\0';
    return res;
}

void getNextCharacter(Lexer* L) 
{
    size_t size = strlen(L->input);
    if (L->readPosition >= size) {
        L->ch = EOF;
    }else {
        L->ch = L->input[L->readPosition];
        L->position = L->readPosition;
        L->readPosition++;
    }
}

void skipWhiteSpace(Lexer* L)
{
    /* Skip White spaces */
    while (iswhitespace(L->ch)) {
        getNextCharacter(L);
    }
}

char* readIdentifier(Lexer* L)
{
    size_t start = L->position;
    while (isalpha(L->ch)){
        getNextCharacter(L);
    }
    char* str = (char *)malloc((L->readPosition - start + 2) * sizeof(char));
    strncpy(str, L->input + start, (L->readPosition - start - 1));
    str[L->readPosition - start -1] = '\0';
    return str;
}

char* readNumber(Lexer* L)
{
    size_t start = L->position;
    while (isdigit(L->ch)){
        getNextCharacter(L);
    }
    char* str = (char *)malloc((L->readPosition - start + 2) * sizeof(char));
    strncpy(str, L->input + start, (L->readPosition - start - 1));
    str[L->readPosition - start -1] = '\0';
    return str;
}

void UngetNextCharacter(Lexer* L)
{
    L->readPosition--;
}

Token NextToken(Lexer* L)
{
    Token currentToken;
    StateType currentState = START;

    while (currentState != DONE)
    {
        skipWhiteSpace(L);
        switch (currentState)
        {
        case START:
            if (isdigit(L->ch)) {
                currentState = INNUM;
                UngetNextCharacter(L);
            }
            else if (isalpha(L->ch)) {
                currentState = INID;
                UngetNextCharacter(L);
            }
            else if (L->ch == '{') {
                currentState = INCOMMENT;
            }
            else if (L->ch == ':') {
                currentState = INASSIGN;
            }
            else {
                currentState = DONE;
                switch (L->ch) {
                case EOF:
                    currentToken.type = ENDOFILE;
                    break;
                case '=':
                    currentToken.type = EQ;
                    currentToken.Literal = "=";
                    break;
                case '<':
                    currentToken.type = LT;
                    currentToken.Literal = "<";
                    break;
                case '+':
                    currentToken.type = PLUS;
                    currentToken.Literal = "+";
                    break;
                case '-':
                    currentToken.type = MINUS;
                    currentToken.Literal = "-";
                    break;
                case '*':
                    currentToken.type = MULT;
                    currentToken.Literal = "*";
                    break;
                case '/':
                    currentToken.type = DIV;
                    currentToken.Literal = "/";
                    break;
                case '(':
                    currentToken.type = LPAREN;
                    currentToken.Literal = "(";
                    break;
                case ')':
                    currentToken.type = RPAREN;
                    currentToken.Literal = ")";
                    break;
                case ';':
                    currentToken.type = SEMIC;
                    currentToken.Literal = ";";
                    break;
                default:
                    currentToken.type = ERROR;
                    currentToken.Literal = chartostr(L->ch);
                    break;
                }
            }
            break;
        case INCOMMENT:
            while(L->ch != '}'){
                getNextCharacter(L);
            }
            currentState = START;
            getNextCharacter(L);
            break;
        case INASSIGN:
            currentState = DONE;
            getNextCharacter(L);
            if (L->ch == '=') {
                currentToken.type = ASSIGN;
                currentToken.Literal = ":=";
            }else {
                currentToken.type = ERROR;
                currentToken.Literal = "\0";
                UngetNextCharacter(L);
            }
            break;
        case INNUM:
            currentToken.Literal = readNumber(L);
            currentToken.type = NUM;
            currentState = DONE;
            UngetNextCharacter(L);
            break;
        case INID:
            currentToken = getReservedToken(readIdentifier(L));
            currentState = DONE;
            UngetNextCharacter(L);
            break;
        case DONE:
            break;
        default:
            currentState = DONE;
            currentToken.type = ERROR;
            currentToken.Literal = "\0";
            printf("Error in lexer, Last character %c", L->ch);
            break;
        }
    }
    getNextCharacter(L);
    return currentToken;
}

Lexer* newLexer(char* str)
{
    Lexer* L = (Lexer *)malloc(sizeof(Lexer));
    assert(L);
    L->input = str;
    L->position = 0;
    L->readPosition = 0;
    L->ch = 0;
    getNextCharacter(L);
    return L;
}

void Terminal(FILE * in, FILE *out)
{
    char input [1024];
    while(1)
    {
        printf(">> ");
        if(fgets(input,1024,in) == NULL){
            return;
        }
        Lexer *L = newLexer(input);
        while(1)
        {
            Token tok = NextToken(L);
            if(tok.type == ENDOFILE){
                printf("\n");
                break;
            }
            fprintf(out,"\n [%s] - %s\n",tokentostring[tok.type],tok.Literal);
        }
    }
}

bool fileScanner(char *path_input, char *path_output)
{
    /* Reading */
    FILE *reading_file = fopen(path_input, "r");
    if (!reading_file){
        fprintf(stderr, "Couldnt load reading_file");
        return false;
    }
    /* Doesnt return accurate size */
    fseek(reading_file, 0, SEEK_END);
    size_t size = (size_t)ftell(reading_file);
    fseek(reading_file, 0, SEEK_SET);

    char *source = (char *)malloc(sizeof(char) * size + 2);
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

    /* Writing */
    FILE *writing_file = fopen(path_output,"w");
    if (!writing_file){
        fprintf(stderr, "Couldnt load writing_file");
        return false;
    }
    Lexer *L = newLexer(source);
    while(1)
    {
        Token tok = NextToken(L);
        /* Break loop when entire source is consumed */
        if(tok.type == ENDOFILE){
            printf("\n");
            break;
        }
        fprintf(writing_file,"%s,%s\n",tok.Literal,tokentostring[tok.type]);
    }
    fclose(writing_file);
    free(source);
    return true;
}

void printToken(Token tok)
{
    fprintf(stdout,"\n [%s] - %s\n",tokentostring[tok.type],tok.Literal);
}

TokenList *newTokenList(size_t size)
{
    TokenList * toklist = (TokenList *) malloc(sizeof(TokenList));
    assert(toklist);
    toklist->tokens = (Token*)malloc(size * sizeof(Token));
    assert(toklist->tokens);
    toklist->size = size;
    toklist->currentToken = 0;
    return  toklist;
}

TokenType getTokenType(const char* tokenTypeStr) {
    if (strcmp(tokenTypeStr, "IDENTIFIER") == 0) return ID;
    if (strcmp(tokenTypeStr, "NUMBER") == 0) return NUM;
    if (strcmp(tokenTypeStr, "ASSIGN") == 0) return ASSIGN;
    if (strcmp(tokenTypeStr, "EQUAL") == 0) return EQ;
    if (strcmp(tokenTypeStr, "LESSTHAN") == 0) return LT;
    if (strcmp(tokenTypeStr, "PLUS") == 0) return PLUS;
    if (strcmp(tokenTypeStr, "MINUS") == 0) return MINUS;
    if (strcmp(tokenTypeStr, "MULT") == 0) return MULT;
    if (strcmp(tokenTypeStr, "DIV") == 0) return DIV;
    if (strcmp(tokenTypeStr, "OPENBRACKET") == 0) return LPAREN;
    if (strcmp(tokenTypeStr, "CLOSEDBRACKET") == 0) return RPAREN;
    if (strcmp(tokenTypeStr, "SEMICOLON") == 0) return SEMIC;
    if (strcmp(tokenTypeStr, "IF") == 0) return IF;
    if (strcmp(tokenTypeStr, "THEN") == 0) return THEN;
    if (strcmp(tokenTypeStr, "ELSE") == 0) return ELSE;
    if (strcmp(tokenTypeStr, "END") == 0) return END;
    if (strcmp(tokenTypeStr, "REPEAT") == 0) return REPEAT;
    if (strcmp(tokenTypeStr, "UNTIL") == 0) return UNTIL;
    if (strcmp(tokenTypeStr, "READ") == 0) return READ;
    if (strcmp(tokenTypeStr, "WRITE") == 0) return WRITE;
    if (strcmp(tokenTypeStr, "ERROR") == 0) return ERROR;
    if (strcmp(tokenTypeStr, "ENDOFILE") == 0) return ENDOFILE;
    return ERROR;  
}


TokenList *filetoTokenList(char *path)
{
    FILE* file = fopen(path, "r");
    assert(file);

    size_t nooftokens = 0;
    int ch;
    static size_t i = 0; 

    while ((ch = fgetc(file)) != EOF) {
        if (ch == '\n') {
            nooftokens++;
        }
    }
    /* Revert file pointer */
    fseek(file, 0, SEEK_SET);

    TokenList *tokenList = newTokenList(nooftokens+1); 
    
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '\n' || (line[0] == '\r' && line[1] == '\n')) {
            continue;
        }
        char* tokenLiteral = strtok(line, ",");
        char* tokenTypeStr = strtok(NULL, "\n");

        tokenList->tokens[i].type = getTokenType(tokenTypeStr);
        tokenList->tokens[i].Literal = strdup(tokenLiteral);
        i++;
    }
    tokenList->tokens[i].type = ENDOFILE;
    tokenList->tokens[i].Literal = "\0";

    fclose(file);

    return tokenList;
}

Token NextTokenList(TokenList *L)
{
    assert(L->currentToken < L->size);
    return L->tokens[L->currentToken++];
}

void printTokenList(TokenList *L)
{
    for(size_t i = 0; i < L->size; i++){
        printToken(L->tokens[i]);
    }
}