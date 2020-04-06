#ifndef _SEMANTICS
#define _SEMANTICS

#include "ast.h"
#include "symboltableDef.h"


extern int semanticError;
extern int idFound;


void matchParameters(symbolTableFuncEntry* entry, parameters* param, astnode* idlist);

void semanticChecker(astnode* root);


# endif