#ifndef _CODEGEN
#define _CODEGEN

extern FILE* asmFile;
extern int num_temps;

// generate temporary variables on the fly and store in symbol table
char* generateTemporary(astnode* root);

// traverse AST to generate assembly code
void generateASM(astnode* root, FILE* fp);

#endif