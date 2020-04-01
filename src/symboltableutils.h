#ifndef _STUTILS
#define _STUTILS

#include "symboltableDef.h"


// for identifiers
idLinkedList createIdLinkedList();

idLinkedList deleteIdLinkedList(idLinkedList list);

idLinkedList insertIdList(idLinkedList list, symbolTableIdEntry entry);

symbolTableIdEntry* searchIdList(idLinkedList list, char* key);


// for functions
funcLinkedList createFuncLinkedList();

funcLinkedList deleteFuncLinkedList(funcLinkedList list);

funcLinkedList insertFuncList(funcLinkedList list, symbolTableFuncEntry entry);

symbolTableFuncEntry* searchFuncList(funcLinkedList list, char* key);


#endif