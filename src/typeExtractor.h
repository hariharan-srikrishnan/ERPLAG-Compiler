#ifndef _TE
#define _TE

#include "symboltableDef.h"
#include "ast.h"


extern idSymbolTable currentIdTable, globalIdTable;
extern funcSymbolTable funcTable;

// create an entry for ID symbol table
symbolTableIdEntry createIdEntry(token id, astnode* type);

// create an entry for function symbol table
symbolTableFuncEntry createFuncEntry(token functionName, parameters* inputParams, int numInput, parameters* outputParams, int numOutput);

// traverse AST for type extraction
void extractTypeAST(astnode* root);

#endif