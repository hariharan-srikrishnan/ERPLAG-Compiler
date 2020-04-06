#ifndef _AST
#define _AST

#include "parserutils.h"
#include "symboltableDef.h"

union _treeData;
typedef union _treeData treeData;

typedef struct _astnode {
    treeData data; // current symbol
    int TorNT; // terminal or non-terminal
    token datatype; // for type-check
    struct _astnode* children;
    struct _astnode* sibling;
    idSymbolTable scopeTable; // corresponding symbol table for relevant node
    int isRedeclared; // whether identifier has already been declared before
} astnode;


// create a new AST node from a parse tree node
astnode* createASTNode(t_node* node);

// traverse parse tree root to generate AST
void createAST(t_node* root);

// print AST structure
void printAST(astnode* root);

#endif