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