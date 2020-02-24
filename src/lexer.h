/*
Anirudh S Chakravarthy - 2017A7PS1195P
Hariharan Srikrishnan  - 2017A7PS0134P
Honnesh Rohmetra	   - 2016B2A70770P
Praveen Ravirathinam   - 2017A7PS1174P
*/


#ifndef _LEXER
#define _LEXER
#include "lexerDef.h"
#include "hash.h"

extern char* keywords[];
extern int keywordTokens[];

extern const int _NUM_KEYWORDS;
extern const int _MAX_INPUT_FILE_SIZE;

extern char buffer1[];
extern char buffer2[];
extern int readBuffer1, readBuffer2;
extern char *start, *current; // current points to the location we will read the next character from
extern char lexeme[];
extern int lineno;
extern int testIdentifierLength;
extern int idError;

extern FILE* fp;

// create hash table	
extern int tableSize;
extern hashtable* t;


void redColor();

void magentaColor();

void resetColor();

char* reverseMap(int tid);

void removeComments(char *testcaseFile, FILE *output);

void getStream(FILE* fp);

void getLexeme();

int isKeyword(char *lexeme);

char nextchar();

void retract(int n);

token getNextToken();

#endif