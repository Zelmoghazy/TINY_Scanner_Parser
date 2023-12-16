#ifndef PARSER_H_
#define PARSER_H_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "Scanner.h"

#define DEBUG 1
#define DEBUG_PRT(fmt, ...)                                                                                  \
    do{                                                                                                      \
        if(DEBUG)                                                                                            \
            fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, __LINE__, __func__ __VA_OPT__(,) __VA_ARGS__);     \
    }while(0)


typedef enum {
    StatementT,
    ExpressionT
}NodeType;

typedef enum {
    IfStatement,
    RepeatStatement,
    AssignStatement,
    ReadStatement,
    WriteStatement
}StatementType;

typedef enum {
    OpT,
    ConstT,
    IdT
}ExpressionType;

typedef enum {
    Void,
    Integer,
    Boolean
}ExpressionDataType;

typedef struct Node
{
    struct Node *children[3];
    struct Node *sibling;
    NodeType nodetype;
    /* Cant be both */
    union{
        StatementType stmt;
        ExpressionType exp;
    }kind;
    /* Cant be both */
    union{
        TokenType op;
        int val;
        char *name;
    }attr;
    ExpressionDataType type;
}Node;

typedef struct Parser{
    Lexer *L;
    Token token;
}Parser;

Node * newStatementNode(StatementType kind);
Node * newExpressionNode(ExpressionType kind);
void match(Parser *parser, TokenType expected);

Node* stmt_sequence(Parser *parser);
Node* statement(Parser *parser);
Node* if_stmt(Parser *parser);
Node* repeat_stmt(Parser *parser);
Node* assign_stmt(Parser *parser);
Node* read_stmt(Parser *parser);
Node* write_stmt(Parser *parser);
Node* expression(Parser *parser);
Node* simple_exp(Parser *parser);
Node* term(Parser *parser);
Node* factor(Parser *parser);

Node *parse(Parser *parser);

Parser *newParser(char *source);

static void printSpaces(void);
void printTree(Node *tree);

void printDotTree(char *path, Node* tree);
void printDotNode(FILE* dotFile, Node* tree);
void printDotEdge(FILE* dotFile, Node* fromNode, Node* toNode);

#endif 
