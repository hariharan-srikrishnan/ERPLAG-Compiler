#ifndef _ST
#define _ST

#include "symboltableDef.h"


// for identifiers
idSymbolTable createIdSymbolTable();

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