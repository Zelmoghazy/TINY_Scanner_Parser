#include "Parser.h"
#include "Scanner.h"
#include <assert.h>
#include <corecrt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Parser *newParser(char *filename, bool choice){
    Parser *parser = (Parser *)malloc(sizeof(Parser));
    assert(parser);
    if(choice){
        char *source   = loadfile(filename);
        Lexer *L       = newLexer(source);
        parser->src.L  = L;
        parser->token  = NextToken(L); // Initialize token to first token in input.
        parser->choice = choice;
    }else{
        parser->src.TL = filetoTokenList(filename);
        parser->token  = NextTokenList(parser->src.TL);
        parser->choice = choice;
    }
    return parser;
}

Node * newStatementNode(StatementType kind)
{
    Node *node = malloc(sizeof(Node));
    assert(node);
    for(size_t i = 0; i < 3; i++){
        node->children[i] = NULL;
    }
    node->sibling = NULL;
    node->nodetype = StatementT;
    node->kind.stmt = kind;
    return node;
}

Node * newExpressionNode(ExpressionType kind)
{
    Node *node = malloc(sizeof(Node));
    assert(node);
    for(size_t i = 0; i < 3; i++){
        node->children[i] = NULL;
    }
    node->sibling = NULL;
    node->nodetype = ExpressionT;
    node->kind.exp = kind;
    return node;
}

/** look for a specific token, 
 * call NextToken if it finds it
 * and declare error otherwise 
 **/

void match(Parser *parser,TokenType expected)
{
    if(parser->token.type == expected){
        if(parser->choice){
            parser->token = NextToken(parser->src.L);
        }else{
            parser->token = NextTokenList(parser->src.TL);
        }
    }else {
        DEBUG_PRT("Incorrect Token");
        printToken(parser->token);
        printf("-----------------------\n");
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
    /* May return null in case of error token */
    Node *node = statement(parser);
    Node *temp = node;
    /* Dont depend on presence of semicolon */
    while((parser->token.type != ENDOFILE) &&
          (parser->token.type != END)      &&
          (parser->token.type != ELSE)     &&
          (parser->token.type != UNTIL))
    {
        Node *next;
        match(parser,SEMIC);
        next = statement(parser);
        if(next != NULL)
        {
            if(node == NULL){
                node = temp = next;
            }else{
                temp->sibling = next;
                temp = next;
            }
        }
    }
    return node;
}


Node *statement(Parser *parser)
{
    Node *node = NULL;
    switch(parser->token.type)
    {
        case IF:
            node = if_stmt(parser);
            break;
        case REPEAT:
            node = repeat_stmt(parser);
            break;
        case ID:
            node = assign_stmt(parser);
            break;
        case READ:
            node = read_stmt(parser);
            break;
        case WRITE:
            node = write_stmt(parser);
            break;
        default:
            DEBUG_PRT("Incorrect token");
            printToken(parser->token);
            printf("-----------------------\n");
            if(parser->choice){
                parser->token = NextToken(parser->src.L);
            }else{
                if(parser->src.TL->currentToken < parser->src.TL->size){
                    parser->token = NextTokenList(parser->src.TL);
                }else{
                    parser->token.type    = ENDOFILE;
                    parser->token.Literal = "\0";
                }
            }
            break;
    }
    return node;
}

/* *************** Statements *************** */

Node *if_stmt(Parser *parser)
{
    Node *node = newStatementNode(IfStatement);
    match(parser,IF);
    node->children[0] = expression(parser);
    match(parser,THEN);
    node->children[1] = stmt_sequence(parser);
    if (parser->token.type == ELSE){
        match(parser,ELSE);
        node->children[2] = stmt_sequence(parser);
    }
    match(parser,END);
    return node;
}

Node *repeat_stmt(Parser *parser)
{
    Node *node = newStatementNode(RepeatStatement);
    match(parser,REPEAT);
    node->children[0] = stmt_sequence(parser);
    match(parser,UNTIL);
    node->children[1] = expression(parser);
    return node;
}

Node *assign_stmt(Parser *parser)
{
    Node *node = newStatementNode(AssignStatement);
    if(parser->token.type == ID){
        node->attr.name = strdup(parser->token.Literal);
    }
    match(parser,ID);
    match(parser,ASSIGN);
    node->children[0] = expression(parser);
    return node;
}

Node *read_stmt(Parser *parser)
{
    Node *node = newStatementNode(ReadStatement);
    match(parser,READ);
    if(parser->token.type == ID){
        node->attr.name = strdup(parser->token.Literal);
    }
    match(parser,ID);
    return node;
}


Node *write_stmt(Parser *parser)
{
    Node *node = newStatementNode(WriteStatement);
    match(parser,WRITE);
    node->children[0] = expression(parser);
    return node;
}

/* ********************************************************************* */


/*
                                    (Operator)
                                  /           \
    (simple-exp)  or  (simple-exp)             (simple-exp)

 */

Node *expression(Parser *parser)
{
    Node *node = simple_exp(parser);
    if(parser->token.type == LT ||
       parser->token.type == EQ)
       {
        Node *temp = newExpressionNode(OpT);
        temp->children[0] = node;
        temp->attr.op = parser->token.type;
        node = temp;
        match(parser,parser->token.type);
        node->children[1] = simple_exp(parser);
    }
    return node;
}

/*
                                    (Operator)
                                  /           \
    (term)        or        (term)             (term)

 */

Node *simple_exp(Parser *parser)
{
    Node *node = term(parser);
    while((parser->token.type == PLUS) ||
          (parser->token.type == MINUS))
    {
        Node *temp = newExpressionNode(OpT);
        temp->children[0] = node;
        temp->attr.op = parser->token.type;
        node = temp;
        match(parser,parser->token.type);
        node->children[1] = term(parser);
    }          
    return node;
}

/*
                                    (Operator)
                                  /           \
    (factor)        or    (factor)             (factor)

 */
/* Precedence Levels of expressions */
Node *term(Parser *parser)
{
    Node *node = factor(parser);
    while((parser->token.type == MULT) ||
          (parser->token.type == DIV))
    {
        Node *temp = newExpressionNode(OpT);
        temp->children[0] = node;
        temp->attr.op = parser->token.type;
        node = temp;
        match(parser,parser->token.type);
        node->children[1] = factor(parser);
    }          
    return node;
}

Node *factor(Parser *parser)
{
    Node *node = NULL;
    switch(parser->token.type)
    {
        case NUM:
            node = newExpressionNode(ConstT);
            if(parser->token.type == NUM){
                node->attr.val = atoi(parser->token.Literal);
            }
            match(parser,NUM);
            break;
        case ID:
            node = newExpressionNode(IdT);
            if(parser->token.type == ID){
                node->attr.name = strdup(parser->token.Literal);
            }
            match(parser,ID);
            break;
        case LPAREN:
            match(parser,LPAREN);
            node = expression(parser);
            match(parser,RPAREN);
            break;
        default:
            DEBUG_PRT("Incorrect Token");
            printToken(parser->token);
            printf("-----------------------\n");
            if(parser->choice){
                parser->token = NextToken(parser->src.L);
            }else{
                if(parser->src.TL->currentToken < parser->src.TL->size){
                    parser->token = NextTokenList(parser->src.TL);
                }else{
                    parser->token.type    = ENDOFILE;
                    parser->token.Literal = "\0";
                }
            }
            break;
    }
    return node;
}

Node *parse(Parser *parser)
{
    Node *root;
    root = stmt_sequence(parser);
    // Check for the end of the source file.
    if(parser->token.type != ENDOFILE){
        DEBUG_PRT("End of source file not reached.\n");
    }
    // Return Constructed tree
    return root;
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
                    fprintf(stdout, "Assign\n(%s)\n",tree->attr.name);
                    break;
                case ReadStatement:
                    fprintf(stdout, "Read\n(%s)\n",tree->attr.name);
                    break;
                case WriteStatement:
                    fprintf(stdout, "Write\n");
                    break;
                default:
                    fprintf(stdout, "Error Statement node\n");
                    break;
            }
        }else if (tree->nodetype == ExpressionT){
            switch (tree->kind.exp) {
                case OpT:
                    fprintf(stdout, "Op\n");
                    fprintf(stdout,"(%s)\n",tokentostring[tree->attr.op]);
                    break;
                case ConstT:
                    fprintf(stdout, "const\n(%d)\n",tree->attr.val);
                    break;
                case IdT:
                    fprintf(stdout, "Id\n(%s)\n",tree->attr.name);
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

/* AUTHOR : Abdo */
void printDotTree(char* path, Node* tree)
{
    FILE *dotFile = fopen(path, "w");
    fprintf(dotFile, "digraph AST {\n");
    fprintf(dotFile, "margin=0;\n");  // Set margin to 0 to minimize space
    fprintf(dotFile, "color=transparent;\n");  // Set color to transparent to make the border invisible
    fprintf(dotFile, "node [shape=box, style=filled, color=black, fillcolor=lightgray];\n");
    // fprintf(dotFile, "subgraph cluster_children {\n");
    // fprintf(dotFile, "rankdir=TB;\n");
    printDotNode(dotFile, tree);
    fprintf(dotFile, "}\n");
    // fprintf(dotFile, "}\n");
}


void printDotNode(FILE* dotFile, Node* tree) 
{
    fprintf(dotFile, "subgraph cluster_children%p {\n", (void*)tree);
    fprintf(dotFile, "rankdir=TB;\n");

    fprintf(dotFile, "node%p [label=\"", (void*)tree);

    if(tree->nodetype == StatementT){
        switch (tree->kind.stmt) {
            case IfStatement:
                fprintf(dotFile, "If \n");
                break;
            case RepeatStatement:
                fprintf(dotFile, "Repeat \n");
                break;
            case AssignStatement:
                fprintf(dotFile, "Assign\n(%s)\n",tree->attr.name);
                break;
            case ReadStatement:
                fprintf(dotFile, "Read\n(%s)\n",tree->attr.name);
                break;
            case WriteStatement:
                fprintf(dotFile, "Write\n");
                break;
            default:
                fprintf(dotFile, "Error Statement node\n");
                break;
        }
    }else if (tree->nodetype == ExpressionT){
        switch (tree->kind.exp) {
            case OpT:
                fprintf(dotFile, "Op\n");
                fprintf(dotFile,"(%s)\n",tokentostring[tree->attr.op]);
                break;
            case ConstT:
                fprintf(dotFile, "const\n(%d)\n",tree->attr.val);
                break;
            case IdT:
                fprintf(dotFile, "Id\n(%s)\n",tree->attr.name);
                break;
            default:
                fprintf(dotFile, "Error Expression node\n");
                break;
        }
    }else{
        fprintf(dotFile, "Unknown node type\n");
    }
    fprintf(dotFile, "\", shape=");
    // Set shape based on the node type
    if (tree->nodetype == StatementT) {
        fprintf(dotFile, "rectangle");
    } else if (tree->nodetype == ExpressionT) {
        fprintf(dotFile, "oval");
    }
    fprintf(dotFile, "];\n");
    if (tree->sibling != NULL) {
        printDotNode(dotFile, tree->sibling);
        printDotEdge(dotFile, tree, tree->sibling);
    }
    fprintf(dotFile, "}\n");

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
}

void printDotEdge(FILE* dotFile, Node* fromNode, Node* toNode) {
    if (fromNode->sibling == toNode) {
        fprintf(dotFile, "node%p -> node%p [dir=none] [constraint=false];\n", (void*)fromNode, (void*)toNode);
    } else {
        fprintf(dotFile, "node%p -> node%p [dir=down];\n", (void*)fromNode, (void*)toNode);
    }
}

/* DOT Notation Stuff */
// void printDotTree(char *path, Node* tree) {
//     FILE *dotFile = fopen(path, "w");
//     fprintf(dotFile, "digraph AST {\n");
//     fprintf(dotFile, "layout=neato;\n");
//     fprintf(dotFile, "node [shape=box, style=filled, color=black, fillcolor=lightgray];\n");

//     fprintf(dotFile, "rankdir=TB;\n");

//     printDotNode(dotFile, tree);

//     fprintf(dotFile, "}\n");

// }

// static int indent = 0;
// static int counter = 0;

// void printDotNode(FILE* dotFile, Node* tree) {
//     indent+=2;
//     while(tree != NULL){
//         fprintf(dotFile, "node%p [color = \"#6c8ebf\",fillcolor=\"#dae8fc\" ,penwidth=3,fontname=\"times bold\",pos=\"%d,-%d!\",label=\"", (void*)tree,counter,indent);
//         counter+=3;
//     // edge [penwidth=3,color="#6c8ebf"]
// 	// node [fontname="CMU Typewriter Text",fontcolor = black,fontsize = 24, penwidth=3,]
//     // node [color = "#6c8ebf",fillcolor="#dae8fc", style=filled, shape = circle, fixedsize=true,width=0.5];
//         if (tree->nodetype == StatementT) {
//             switch (tree->kind.stmt) {
//                 case IfStatement:
//                     fprintf(dotFile, "If");
//                     break;
//                 case RepeatStatement:
//                     fprintf(dotFile, "Repeat");
//                     break;
//                 case AssignStatement:
//                     fprintf(dotFile, "Assign to: %s", tree->attr.name);
//                     break;
//                 case ReadStatement:
//                     fprintf(dotFile, "Read: %s", tree->attr.name);
//                     break;
//                 case WriteStatement:
//                     fprintf(dotFile, "Write");
//                     break;
//                 default:
//                     fprintf(dotFile, "Error Statement node");
//                     break;
//             }
//         } else if (tree->nodetype == ExpressionT) {
//             switch (tree->kind.exp) {
//                 case OpT:
//                     fprintf(dotFile, "Op: %s", tokentostring[tree->attr.op]);
//                     break;
//                 case ConstT:
//                     fprintf(dotFile, "Const: %d", tree->attr.val);
//                     break;
//                 case IdT:
//                     fprintf(dotFile, "Id: %s", tree->attr.name);
//                     break;
//                 default:
//                     fprintf(dotFile, "Error Expression node");
//                     break;
//             }
//         }

//         fprintf(dotFile, "\", shape=");
        
//         // Set shape based on the node type
//         if (tree->nodetype == StatementT) {
//             fprintf(dotFile, "rectangle");
//         } else if (tree->nodetype == ExpressionT) {
//             fprintf(dotFile, "oval");
//         }

//         fprintf(dotFile, "];\n");
//         fprintf(dotFile, "edge [penwidth=3,color=\"#6c8ebf\"]");

        
//         for (size_t i = 0; i < 3; i++) {
//             if (tree->children[i] != NULL) {
//                 if(i==0 && tree->children[1] == NULL){
//                     counter-=3;
//                 }else if(i==0 && tree->children[1] != NULL){
//                     counter -=4;
//                 }else if(i==0 && tree->children[2] != NULL){
//                     counter -=5;
//                 }
//                 printDotNode(dotFile, tree->children[i]);
//                 fprintf(dotFile, "node%p -> node%p ;\n", (void*)tree, (void*)tree->children[i]);
//             }
//         }

//         if (tree->sibling != NULL) {
//             fprintf(dotFile, "node%p -> node%p ;\n", (void*)tree, (void*)tree->sibling);
//         }
//         tree = tree->sibling;
//     }
//     indent-=2;
// }

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

void fileParser(char *filename, char *dotfile, bool choice)
{
    Parser *parser   = newParser(filename,choice);
    Node *syntaxTree = parse(parser);
    printDotTree(dotfile,syntaxTree);
    // if(!choice){
    //     printTokenList(parser->src.TL);
    // }
    // printTree(syntaxTree);
}
