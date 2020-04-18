/*
Anirudh S Chakravarthy - 2017A7PS1195P
Hariharan Srikrishnan  - 2017A7PS0134P
Honnesh Rohmetra	   - 2016B2A70770P
Praveen Ravirathinam   - 2017A7PS1174P
*/

#ifndef _ST
#define _ST

#include "symboltableDef.h"


// for identifiers
idSymbolTable* createIdSymbolTable();

idSymbolTable deleteIdSymbolTable(idSymbolTable table);

idSymbolTable insertId(idSymbolTable table, symbolTableIdEntry entry);

symbolTableIdEntry* searchId(idSymbolTable table, char* name);

idSymbolTable removeId(idSymbolTable table, char* name);


// for functions
funcSymbolTable createFuncSymbolTable();

funcSymbolTable deleteFuncSymbolTable(funcSymbolTable table);

funcSymbolTable insertFunc(funcSymbolTable table, symbolTableFuncEntry entry);

symbolTableFuncEntry* searchFunc(funcSymbolTable table, char* name);


#endif