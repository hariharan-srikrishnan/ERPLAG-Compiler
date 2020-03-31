#ifndef _ST
#define _ST

#include "lexerDef.h"


// format for identifier symbol table entry
typedef struct _symbolTableIdEntry {
    char* name; // identifier name (for easier comparisons)
    token id; // identifier information
    token type; // type
    int offset; 
    int width; // size of variable
    int lowerBound, upperBound; // bounds for arrays - IS IT NEEDED?
} symbolTableIdEntry;


// node in hash table chain
typedef struct _idNode {
    symbolTableIdEntry entry;
    struct _idNode* next;
} idNode;


// list of nodes for a given hash value
typedef struct _idLinkedList {
    idNode* head;
    int length; // length of linked list
} idLinkedList;


// identifier symbol table
typedef struct _idSymbolTable{
    idLinkedList* list; // buckets of lists each corresponding to a particular hash
    int hashSize; // number of hash values
    struct _idSymbolTable *sibling, *child; // for nested scopes
} idSymbolTable;


// information for module parameter list
typedef struct _parameters {
    token datatype;
    token id;
    struct _parameters* next;
} parameters;


// format for function symbol table entry
typedef struct _symbolTableFuncEntry {
    char* name; // function name (for easier comparisons)
    token id; // function identifier information   
    parameters *inputParameters; // list of input parameters
    parameters *outputParameters;  // list of output parameters
    struct _idSymbolTable* link; // link to symbol table for that scope
} symbolTableFuncEntry;


// node in hash table chain
typedef struct _funcNode {
    symbolTableFuncEntry entry;
    struct _funcNode* next;
} funcNode;


// list of nodes for a given hash value
typedef struct _funcLinkedList {
    funcNode* head;
    int length; // length of linked list
} funcLinkedList;


// function symbol table
typedef struct _funcSymbolTable {
    funcLinkedList* list; // buckets of lists each corresponding to a particular hash
    int hashSize; // number of hash values
} funcSymbolTable;


// for identifiers
idSymbolTable createIdSymbolTable();

idSymbolTable deleteIdSymbolTable(idSymbolTable table);

idSymbolTable insertId(idSymbolTable table, symbolTableIdEntry entry);

symbolTableIdEntry* searchId(idSymbolTable table, char* name);


// for functions
funcSymbolTable createFuncSymbolTable();

funcSymbolTable deleteFuncSymbolTable(funcSymbolTable table);

funcSymbolTable insertFunc(funcSymbolTable table, symbolTableFuncEntry entry);

symbolTableFuncEntry* searchFunc(funcSymbolTable table, char* name);


#endif