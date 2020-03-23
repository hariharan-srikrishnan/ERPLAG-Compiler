#ifndef _AST
#define _AST

#include "parserutils.h"
#include "parserDef.h"


typedef struct _astnode {
    treeData data; // current symbol
    int TorNT; // terminal or non-terminal
    token datatype; // for type-check
    struct _astnode* parent;
    struct _astnode* children;
    struct _astnode* sibling;
} astnode;


#endif