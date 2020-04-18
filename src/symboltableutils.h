/*
Anirudh S Chakravarthy - 2017A7PS1195P
Hariharan Srikrishnan  - 2017A7PS0134P
Honnesh Rohmetra	   - 2016B2A70770P
Praveen Ravirathinam   - 2017A7PS1174P
*/

#ifndef _STUTILS
#define _STUTILS

#include "symboltableDef.h"


// for identifiers
idLinkedList createIdLinkedList();

idLinkedList deleteIdLinkedList(idLinkedList list);

idLinkedList insertIdList(idLinkedList list, symbolTableIdEntry entry);

symbolTableIdEntry* searchIdList(idLinkedList list, char* key);

idLinkedList removeFromIdList(idLinkedList list, char* key);


// for functions
funcLinkedList createFuncLinkedList();

funcLinkedList deleteFuncLinkedList(funcLinkedList list);

funcLinkedList insertFuncList(funcLinkedList list, symbolTableFuncEntry entry);

symbolTableFuncEntry* searchFuncList(funcLinkedList list, char* key);


#endif