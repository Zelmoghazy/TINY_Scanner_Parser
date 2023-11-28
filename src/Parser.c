#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

typedef enum {
    Statement,
    Expression
}NodeType;

typedef enum {
    IfStatement,
    RepeatStatement,
    AssignStatement,
    ReadStatement,
    WriteStatement
}StatementKind;

typedef enum {
    OpExpression,
    ConstExpression,
    IdExpression
}ExpressionKind;


typedef enum {
    Void,
    Integer,
    Boolean
}ExpressionType;

typedef struct Node
{
    struct Node *children[3];
    struct Node *sibling;
    NodeType nodetype;
    /* Cant be both */
    union{
        StatementKind stmt;
        ExpressionKind exp;
    }kind;
    /* Cant be both */
    union{
        TokenType op;
        int val;
        char *name;
    }attr;
    ExpressionType type;
}Node;

Node * newStatementNode(StatementKind kind)
{
    Node *t = malloc(sizeof(Node));
    assert(t);
    for(size_t i = 0; i < 3; i++){
        t->children[i] = NULL;
    }
    t->sibling = NULL;
    t->nodetype = Statement;
    t->kind.stmt = kind;
    return t;
}

Node * newExpressionNode(ExpressionKind kind)
{
    Node *t = malloc(sizeof(Node));
    assert(t);
    for(size_t i = 0; i < 3; i++){
        t->children[i] = NULL;
    }
    t->sibling = NULL;
    t->nodetype = Expression;
    t->kind.exp = kind;
    t->type = Void;
    return t;
}

Node* stmt_sequence(void);
Node* statement(void);
Node* if_stmt(void);
Node* repeat_stmt(void);
Node* assign_stmt(void);
Node* read_stmt(void);
Node* write_stmt(void);
Node* exression(void);
Node* simple_exp(void);
Node* term(void);
Node* factor(void);


void match(TokenType expected)
{
    if(token == expected){
        token = getToken();
    }else {
    // TODO: Error
    }
}