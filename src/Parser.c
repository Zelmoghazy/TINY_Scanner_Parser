#include "Parser.h"
#include <assert.h>
#include <corecrt.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Parser *newParser(char *source){
    Parser *parser = (Parser *)malloc(sizeof(Parser));
    assert(parser);
    Lexer *L = newLexer(source);
    parser->L = L;
    parser->token = NextToken(L); // Initialize token to first token in input.
    return parser;
}

Node * newStatementNode(StatementType kind)
{
    Node *t = malloc(sizeof(Node));
    assert(t);
    for(size_t i = 0; i < 3; i++){
        t->children[i] = NULL;
    }
    t->sibling = NULL;
    t->nodetype = StatementT;
    t->kind.stmt = kind;
    return t;
}

Node * newExpressionNode(ExpressionType kind)
{
    Node *t = malloc(sizeof(Node));
    assert(t);
    for(size_t i = 0; i < 3; i++){
        t->children[i] = NULL;
    }
    t->sibling = NULL;
    t->nodetype = ExpressionT;
    t->kind.exp = kind;
    return t;
}

/** look for a specific token, 
 * call NextToken if it finds it
 * and declare error otherwise 
 **/

void match(Parser *parser,TokenType expected)
{
    if(parser->token.type == expected){
        parser->token = NextToken(parser->L);
    }else {
        DEBUG_PRT("Incorrect Token");
        printToken(parser->token);
    }
}

/** Four tokens in this negative test comprise the Follow set for stmt-sequence. 
 * since if a First symbol is missing, the parse would stop so a check
 * for token "not" in follow set is effective in recovering from this error
 *
 * - Naive Implementation :
 *
 *      statement();
 *       while (token == SEMIC){
 *           match(SEMIC);
 *           statement();
 *       }
 */

Node* stmt_sequence(Parser *parser)
{
    Node *t = statement(parser);
    Node *p = t;
    /* Dont depend on presence of semicolon */
    while((parser->token.type != ENDOFILE) &&
          (parser->token.type != END)      &&
          (parser->token.type != ELSE)     &&
          (parser->token.type != UNTIL))
    {
        Node *q;
        match(parser,SEMIC);
        q = statement(parser);
        if(q != NULL)
        {
            if(t == NULL){
                t = p = q;
            }else{
                p->sibling = q;
                p = q;
            }
        }
    }
    return t;
}


Node *statement(Parser *parser)
{
    Node *t = NULL;
    switch(parser->token.type)
    {
        case IF:
            t = if_stmt(parser);
            break;
        case REPEAT:
            t = repeat_stmt(parser);
            break;
        case ID:
            t = assign_stmt(parser);
            break;
        case READ:
            t = read_stmt(parser);
            break;
        case WRITE:
            t = write_stmt(parser);
            break;
        default:
            DEBUG_PRT("Incorrect token");
            printToken(parser->token);
            parser->token = NextToken(parser->L);
            break;
    }
    return t;
}

/* *************** Statements *************** */

Node *if_stmt(Parser *parser)
{
    Node *t = newStatementNode(IfStatement);
    match(parser,IF);
    t->children[0] = expression(parser);
    match(parser,THEN);
    t->children[1] = stmt_sequence(parser);
    if (parser->token.type == ELSE){
        match(parser,ELSE);
        t->children[2] = stmt_sequence(parser);
    }
    match(parser,END);
    return t;
}

Node *repeat_stmt(Parser *parser)
{
    Node *t = newStatementNode(RepeatStatement);
    match(parser,REPEAT);
    t->children[0] = stmt_sequence(parser);
    match(parser,UNTIL);
    t->children[1] = expression(parser);
    return t;
}

Node *assign_stmt(Parser *parser)
{
    Node *t = newStatementNode(AssignStatement);
    if(parser->token.type == ID){
        t->attr.name = strdup(parser->token.Literal);
    }
    match(parser,ID);
    match(parser,ASSIGN);
    t->children[0] = expression(parser);
    return t;
}

Node *read_stmt(Parser *parser)
{
    Node *t = newStatementNode(ReadStatement);
    match(parser,READ);
    if(parser->token.type == ID){
        t->attr.name = strdup(parser->token.Literal);
    }
    match(parser,ID);
    return t;
}


Node *write_stmt(Parser *parser)
{
    Node *t = newStatementNode(WriteStatement);
    match(parser,WRITE);
    t->children[0] = expression(parser);
    return t;
}

/* ********************************************************************* */

Node *expression(Parser *parser)
{
    Node *t = simple_exp(parser);
    if(parser->token.type == LT ||
       parser->token.type == EQ)
       {
        Node *p = newExpressionNode(OpT);
        p->children[0] = t;
        p->attr.op = parser->token.type;
        t = p;
        match(parser,parser->token.type);
        t->children[1] = simple_exp(parser);
    }
    return t;
}

Node *simple_exp(Parser *parser)
{
    Node *t = term(parser);
    while((parser->token.type == PLUS) ||
          (parser->token.type == MINUS))
    {
        Node *p = newExpressionNode(OpT);
        p->children[0] = t;
        p->attr.op = parser->token.type;
        t = p;
        match(parser,parser->token.type);
        t->children[1] = term(parser);
    }          
    return t;
}

/* Precedece Levels of expressions */
Node *term(Parser *parser)
{
    Node *t = factor(parser);
    while((parser->token.type == MULT) ||
          (parser->token.type == DIV))
    {
        Node *p = newExpressionNode(OpT);
        p->children[0] = t;
        p->attr.op = parser->token.type;
        t = p;
        match(parser,parser->token.type);
        t->children[1] = factor(parser);
    }          
    return t;
}

Node *factor(Parser *parser)
{
    Node *t = NULL;
    switch(parser->token.type)
    {
        case NUM:
            t = newExpressionNode(ConstT);
            if(parser->token.type == NUM){
                t->attr.val = atoi(parser->token.Literal);
            }
            match(parser,NUM);
            break;
        case ID:
            t = newExpressionNode(IdT);
            if(parser->token.type == ID){
                t->attr.name = strdup(parser->token.Literal);
            }
            match(parser,ID);
            break;
        case LPAREN:
            match(parser,LPAREN);
            t = expression(parser);
            match(parser,RPAREN);
            break;
        default:
            DEBUG_PRT("Incorrect Token");
            printToken(parser->token);
            parser->token = NextToken(parser->L);
            break;
    }
    return t;
}


Node *parse(Parser *parser)
{
    Node *t;
    t = stmt_sequence(parser);
    // Check for the end of the source file.
    if(parser->token.type != ENDOFILE){
        DEBUG_PRT("End of source file not reached.\n");
    }
    // Return Constructed tree
    return t;
}

static size_t indentation = 0; 

static void printSpaces(void)
{
    for(size_t i = 0; i < indentation; i++){
      fprintf(stdout, " ");
    }
}

void printTree(Node *tree){
    indentation+=2;
    while(tree != NULL)
    {
        printSpaces();
        if(tree->nodetype == StatementT){
            switch (tree->kind.stmt) {
                case IfStatement:
                    fprintf(stdout, "If \n");
                    break;
                case RepeatStatement:
                    fprintf(stdout, "Repeat \n");
                    break;
                case AssignStatement:
                    fprintf(stdout, "Assign to: %s\n",tree->attr.name);
                    break;
                case ReadStatement:
                    fprintf(stdout, "Read: %s\n",tree->attr.name);
                    break;
                case WriteStatement:
                    fprintf(stdout, "Write:\n");
                    break;
                default:
                    fprintf(stdout, "Error Statement node\n");
                    break;
            }
        }else if (tree->nodetype == ExpressionT){
            switch (tree->kind.exp) {
                case OpT:
                    fprintf(stdout, "Op: ");
                    fprintf(stdout, tokentostring[tree->attr.op]);
                    break;
                case ConstT:
                    fprintf(stdout, "const: %d\n",tree->attr.val);
                    break;
                case IdT:
                    fprintf(stdout, "Id: %s\n",tree->attr.name);
                    break;
                default:
                    fprintf(stdout, "Error Expression node\n");
                    break;
            }
        }else{
            fprintf(stdout, "Unknown node type\n");
        }
        for(size_t i = 0; i < 3; i++){
            printTree(tree->children[i]);
        }
        tree = tree->sibling;
    }
    indentation -=2;
}

/* DOT Notation Stuff */
void printDotTree(char *path, Node* tree) {
    FILE *dotFile = fopen(path, "w");
    fprintf(dotFile, "digraph AST {\n");
    fprintf(dotFile, "node [shape=box, style=filled, color=black, fillcolor=lightgray];\n");

    fprintf(dotFile, "subgraph cluster_children {\n");
    fprintf(dotFile, "rankdir=TB;\n");

    printDotNode(dotFile, tree);

    fprintf(dotFile, "}\n");

    fprintf(dotFile, "}\n");
}

void printDotNode(FILE* dotFile, Node* tree) {
    fprintf(dotFile, "node%p [label=\"", (void*)tree);

    if (tree->nodetype == StatementT) {
        switch (tree->kind.stmt) {
            case IfStatement:
                fprintf(dotFile, "If");
                break;
            case RepeatStatement:
                fprintf(dotFile, "Repeat");
                break;
            case AssignStatement:
                fprintf(dotFile, "Assign to: %s", tree->attr.name);
                break;
            case ReadStatement:
                fprintf(dotFile, "Read: %s", tree->attr.name);
                break;
            case WriteStatement:
                fprintf(dotFile, "Write");
                break;
            default:
                fprintf(dotFile, "Error Statement node");
                break;
        }
    } else if (tree->nodetype == ExpressionT) {
        switch (tree->kind.exp) {
            case OpT:
                fprintf(dotFile, "Op: %s", tokentostring[tree->attr.op]);
                break;
            case ConstT:
                fprintf(dotFile, "Const: %d", tree->attr.val);
                break;
            case IdT:
                fprintf(dotFile, "Id: %s", tree->attr.name);
                break;
            default:
                fprintf(dotFile, "Error Expression node");
                break;
        }
    } else {
        fprintf(dotFile, "Unknown node type");
    }

    fprintf(dotFile, "\"];\n");

    // Create a subgraph for children (downward layout)
    fprintf(dotFile, "subgraph cluster_children%p {\n", (void*)tree);
    fprintf(dotFile, "rankdir=TB;\n");

    for (size_t i = 0; i < 3; i++) {
        if (tree->children[i] != NULL) {
            printDotNode(dotFile, tree->children[i]);
            printDotEdge(dotFile, tree, tree->children[i]);
        }
    }
    fprintf(dotFile, "}\n");
    fprintf(dotFile, "subgraph cluster_siblings%p {\n", (void*)tree);
    fprintf(dotFile, "rankdir=LR;\n");

    if (tree->sibling != NULL) {
        printDotNode(dotFile, tree->sibling);
        printDotEdge(dotFile, tree, tree->sibling);
    }
    fprintf(dotFile, "}\n");
}

void printDotEdge(FILE* dotFile, Node* fromNode, Node* toNode) {
    if (fromNode->sibling == toNode) {
        // Sibling link, make it to the right
        fprintf(dotFile, "node%p -> node%p [dir=right];\n", (void*)fromNode, (void*)toNode);
    } else {
        // Children link, make it downward
        fprintf(dotFile, "node%p -> node%p [dir=down];\n", (void*)fromNode, (void*)toNode);
    }
}

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

void fileParser(char *filename, char *dotfile){
    char *source = loadfile(filename);
    Parser *parser = newParser(source);
    Node *syntaxTree = parse(parser);
    printDotTree(dotfile,syntaxTree);
    printTree(syntaxTree);
}