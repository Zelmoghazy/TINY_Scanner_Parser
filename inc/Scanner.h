#ifndef SCANNER_H_
#define SCANNER_H_

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef enum {
    START, INASSIGN, INID, INNUM, INCOMMENT, DONE
} StateType;

typedef enum {
    ID, NUM,
    ASSIGN, EQ, LT, PLUS, MINUS, MULT, DIV,
    LPAREN, RPAREN, SEMIC,
    IF, THEN, ELSE, END, REPEAT, UNTIL, READ, WRITE, ERROR, ENDOFILE
} TokenType;

/* Maybe use it in parser too */
extern const char *tokentostring[22];

typedef struct Token {
    TokenType type;
    const char* Literal;
} Token;

typedef struct TokenList {
    Token* tokens;
    size_t currentToken;
    size_t size;
} TokenList;

typedef struct Lexer {
    const char* input;           // Source
    char ch;
    size_t position;       // Current position in input
    size_t readPosition;   // after current char
}Lexer;

void UngetNextCharacter(Lexer* L);
Token getReservedToken(char* str);
bool iswhitespace(char ch);
void getNextCharacter(Lexer* L);
void skipWhiteSpace(Lexer* L);
char* readIdentifier(Lexer* L);
char* readNumber(Lexer* L);
void UngetNextCharacter(Lexer* L);
Token NextToken(Lexer* L);
Lexer* newLexer(char* str);
void Terminal(FILE * in, FILE *out);
bool fileScanner(char * path_input, char *path_output);
void printToken(Token tok);

#ifdef TOKEN_LIST
Token NextToken(TokenList *L);
#endif

#endif


