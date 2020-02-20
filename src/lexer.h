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

extern FILE* fp;

// create hash table	
extern int tableSize;
extern hashtable* t;


char* reverseMap(int tid);

void removeComments(char *testcaseFile, char *cleanFile);

void getStream(FILE* fp);

void getLexeme();

int isKeyword(char *lexeme);

char nextchar();

void retract(int n);

token getNextToken();

#endif