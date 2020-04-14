#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "symboltableDef.h"
#include "symboltableutils.h"
#include "hash.h"


int hashTableSize = 13;

/* For identifier symbol table */

// create an empty identifier symbol table
idSymbolTable* createIdSymbolTable() {
    idSymbolTable* table = (idSymbolTable*) malloc(sizeof(idSymbolTable));
    table->hashSize = hashTableSize;
    table->list = (idLinkedList*) malloc(sizeof(idLinkedList) * hashTableSize);
    for (int i = 0; i < table->hashSize; i++) 
        table->list[i] = createIdLinkedList();
    table->child = NULL;
    table->parent = NULL;
    table->sibling = NULL;
    return table;
}


// delete symbol table
idSymbolTable deleteIdSymbolTable(idSymbolTable table){
    for(int i = 0; i < table.hashSize; i++) 
        deleteIdLinkedList(table.list[i]); 
    table.list = NULL;
    table.hashSize = 0;
    table.child = NULL;
    table.parent = NULL;
    table.sibling = NULL;
    return table;
}


// insert into symbol table
idSymbolTable insertId(idSymbolTable table, symbolTableIdEntry entry){
    int hashValue = hashFunction(entry.name, table.hashSize);
    table.list[hashValue] = insertIdList(table.list[hashValue], entry);
    return table;
}


// symbol table lookup
symbolTableIdEntry* searchId(idSymbolTable table, char* name){
    int hashValue = hashFunction(name, table.hashSize);
    symbolTableIdEntry* entry = searchIdList(table.list[hashValue], name);
    return entry;
}


// remove an identifier from symbol table
idSymbolTable removeId(idSymbolTable table, char* name) {
    int hashValue = hashFunction(name, table.hashSize);
    table.list[hashValue] = removeFromIdList(table.list[hashValue], name);
    return table;
}


/* For function symbol table */

// create an empty function symbol table
funcSymbolTable createFuncSymbolTable() {
    funcSymbolTable table;
    table.hashSize = hashTableSize;
    table.list = (funcLinkedList*) malloc(sizeof(funcLinkedList) * hashTableSize);
    for (int i = 0; i < table.hashSize; i++) 
        table.list[i] = createFuncLinkedList();
    return table;
}


// delete symbol table
funcSymbolTable deleteFuncSymbolTable(funcSymbolTable table){
    for(int i = 0; i < table.hashSize; i++) 
        deleteFuncLinkedList(table.list[i]); 
    table.list = NULL;
    table.hashSize = 0;
    return table;
}


// insert into symbol table
funcSymbolTable insertFunc(funcSymbolTable table, symbolTableFuncEntry entry){
    int hashValue = hashFunction(entry.name, table.hashSize);
    table.list[hashValue] = insertFuncList(table.list[hashValue], entry);
    return table;
}


// symbol table lookup
symbolTableFuncEntry* searchFunc(funcSymbolTable table, char* name){
    int hashValue = hashFunction(name, table.hashSize);
    symbolTableFuncEntry* entry = searchFuncList(table.list[hashValue], name);
    return entry;
}


// int main() {
//     idSymbolTable table = createIdSymbolTable();
//     symbolTableIdEntry entrya;
//     strcpy(entrya.name, "A");
//     table = insertId(table, entrya);

//     symbolTableIdEntry entryb;
//     strcpy(entryb.name, "B");
//     table = insertId(table, entryb);

//     symbolTableIdEntry entryc;
//     strcpy(entryc.name, "C");
//     table = insertId(table, entryc);

//     symbolTableIdEntry* a = searchId(table, "A");
//     symbolTableIdEntry* b = searchId(table, "B");
//     symbolTableIdEntry* c = searchId(table, "C");

//     printf("%s\n",a->name);
//     printf("%s\n",b->name);
//     if (c)
//         printf("%s\n",c->name);
//     else 
//         printf("NULL\n");

//     table = removeId(table, "A");
//     a = searchId(table, "A");
//     if (a == NULL)
//         printf("Deleted successfully!\n");

//     table = removeId(table, "C");
//     c = searchId(table, "C");
//     if (c == NULL)
//         printf("Deleted successfully!\n");

//     c = searchId(table,"C");
//     if (c)
//         printf("%s\n",c->name);
//     else 
//         printf("NULL\n");
// }