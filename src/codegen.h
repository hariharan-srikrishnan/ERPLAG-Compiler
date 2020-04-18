/*
Anirudh S Chakravarthy - 2017A7PS1195P
Hariharan Srikrishnan  - 2017A7PS0134P
Honnesh Rohmetra	   - 2016B2A70770P
Praveen Ravirathinam   - 2017A7PS1174P
*/


#ifndef _CODEGEN
#define _CODEGEN

extern FILE* asmFile;
extern int num_labels;
extern int num_temps;


// find the next possible offset for temporary variable
int findNextOffset(idSymbolTable* table);

// generate temporary variables on the fly and store in symbol table
char* generateTemporary(astnode* root);

// traverse AST to generate assembly code
void generateASM(astnode* root);

#endif