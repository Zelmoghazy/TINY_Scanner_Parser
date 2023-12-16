#include "Scanner.h"
#include <assert.h>
#include <stdio.h>


Token reservedWords[] = {
    {IF, "if"},     {THEN, "then"},     {ELSE, "else"},
    {END, "end"},   {REPEAT, "repeat"}, {UNTIL, "until"},
    {READ, "read"}, {WRITE, "write"}
};

const char *tokentostring[] = {
    "IDENTIFIER", "NUMBER",     "ASSIGN",      "EQUAL",
    "LESS_THAN",  "PLUS",       "MINUS",       "MULTIPLY",
    "DIVISION",   "LEFT_PAREN", "RIGHT_PAREN", "SEMICOLON",
    "IF",         "THEN",       "ELSE",        "END",
    "REPEAT",     "UNTIL",      "READ",        "WRITE",
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
                    currentToken.Literal = "\0";
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
        fprintf(writing_file,"\n [%s] - %s\n",tokentostring[tok.type],tok.Literal);
    }
    fclose(writing_file);
    free(source);
    return true;
}

void printToken(Token tok)
{
    fprintf(stdout,"\n [%s] - %s\n",tokentostring[tok.type],tok.Literal);
}
