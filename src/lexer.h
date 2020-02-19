#ifndef _LEXER
#define _LEXER
#include "lexerDef.h"

char* reverseMap(int tid);

void removeComments(char *testcaseFile, char *cleanFile);

void getStream(FILE* fp);

void getLexeme();

int isKeyword(char *lexeme);

char nextchar();

void retract(int n);

token getNextToken();

#endif