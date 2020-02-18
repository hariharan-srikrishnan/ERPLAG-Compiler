#ifndef _HASH
#define _HASH


typedef struct _NODE{
	char key[25];
	struct _NODE* next;
} node;


typedef struct {
	node** head;
	int elementCount;
} hashtable;


hashtable* createTable(int tableSize);

hashtable* insert(hashtable* t, int tableSize, char** keywords, int num_keywords);

int hashFunction(char* string, int tableSize);


#endif