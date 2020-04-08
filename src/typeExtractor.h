#ifndef _TE
#define _TE

#include "symboltableDef.h"
#include "ast.h"


extern idSymbolTable *currentIdTable, *globalIdTable;
extern funcSymbolTable funcTable;
extern int currentOffset;

// create an entry for ID symbol table
symbolTableIdEntry createIdEntry(token id, astnode* type);

// create an entry for function symbol table
symbolTableFuncEntry createFuncEntry(token functionName, parameters* inputParams, int numInput, parameters* outputParams, int numOutput);

// traverse AST for type extraction
void extractTypeAST(astnode* root);

// link two tables
void linkTables (idSymbolTable* currentTable, idSymbolTable* newTable);

// traverse all identifier symbol tables until global symbol table
void traverseSymbolTable(idSymbolTable* curr);

// print the function symbol table
void printFunctionTable(funcSymbolTable table);


#endif