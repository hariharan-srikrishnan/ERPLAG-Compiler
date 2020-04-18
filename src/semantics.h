/*
Anirudh S Chakravarthy - 2017A7PS1195P
Hariharan Srikrishnan  - 2017A7PS0134P
Honnesh Rohmetra	   - 2016B2A70770P
Praveen Ravirathinam   - 2017A7PS1174P
*/

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