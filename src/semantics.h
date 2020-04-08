#ifndef _SEMANTICS
#define _SEMANTICS

#include "ast.h"
#include "symboltableDef.h"


extern int semanticError;
extern int idFound;


// match type and number of function parameter
void matchParameters(symbolTableFuncEntry* entry, parameters* param, astnode* idlist);

// semantic rules and type checking by traversing AST
void semanticChecker(astnode* root);


# endif