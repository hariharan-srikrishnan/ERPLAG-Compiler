/*
Anirudh S Chakravarthy - 2017A7PS1195P
Hariharan Srikrishnan  - 2017A7PS0134P
Honnesh Rohmetra	   - 2016B2A70770P
Praveen Ravirathinam   - 2017A7PS1174P
*/

#ifndef _STDEF
#define _STDEF

#include "lexerDef.h"


// for INT, REAL
typedef struct {
    token datatype;
    int width; // size of variable
} primitiveType;


// for arrays
typedef struct {
    token arr; // to store array keyword
    primitiveType datatype; // array datatype
    token lowerBound, upperBound; // bounds of array
    int dynamicArray; // whether bounds are known at compile-time
} arrayType;


// type information
typedef union { 
    primitiveType primitive;
    arrayType array;
} typeinfo;


// format for identifier symbol table entry
typedef struct _symbolTableIdEntry {
    char name[25]; // identifier name (for easier comparisons)
    token id; // identifier information
    typeinfo type; // type
    int offset; 
    int AorP; // 1: array, 0: primitive
    int isUpdated; // whether variable occurs in LHS of an assignment
    int loopVariable; // whether it is a loop variable
    int isInputParam; // is it an input parameter
    int isShadowed; // is input param shadowed
    // int shadowLineNo; // line number at which input parameter
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
    struct _idSymbolTable *parent, *child, *sibling; // for nested scopes
    char name[25];
    int startLineNo, endLineNo; // scope information
} idSymbolTable;


// information for module parameter list
typedef struct _parameters {
    token datatype;
    token id;
    struct _parameters* next;
} parameters;


// format for function symbol table entry
typedef struct _symbolTableFuncEntry {
    char name[25]; // function name (for easier comparisons)
    token id; // function identifier information   
    parameters *inputParameters; // list of input parameters
    int numInputParams; // number of input parameters
    parameters *outputParameters;  // list of output parameters
    int numOutputParams; // number of output parameters
    struct _idSymbolTable link; // link to symbol table for that scope
    int declarationLineNo, definitionLineNo; // line numbers of the module declaration and definitions respetively
    int declarationUsed;
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


#endif