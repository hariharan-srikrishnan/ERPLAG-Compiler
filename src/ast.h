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
    struct _astnode* parent;
    struct _astnode* children;
    struct _astnode* sibling;
    idSymbolTable scopeTable;
} astnode;


// create a new AST node from a parse tree node
astnode* createASTNode(t_node* node);

// traverse parse tree root to generate AST
void createAST(t_node* root);

// print AST structure
void printAST(astnode* root);

#endif