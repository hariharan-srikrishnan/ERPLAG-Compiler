/*
Anirudh S Chakravarthy - 2017A7PS1195P
Hariharan Srikrishnan  - 2017A7PS0134P
Honnesh Rohmetra	   - 2016B2A70770P
Praveen Ravirathinam   - 2017A7PS1174P
*/


#ifndef _HASH
#define _HASH


typedef struct _NODE{
	char lexeme[25];
	int token;
	struct _NODE* next;
} node;


typedef struct {
	node** head;
	int elementCount;
} hashtable;


hashtable* createTable(int tableSize);

hashtable* insert(hashtable* t, int tableSize, char** keywords, int* keywordTokens, int num_keywords);

int hashFunction(char* string, int tableSize);


#endif