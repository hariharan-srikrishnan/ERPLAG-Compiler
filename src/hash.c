#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash.h"


hashtable* createTable(int tableSize) {
	hashtable* t = (hashtable*) malloc(sizeof(hashtable));
	t->head = (node**) malloc(sizeof(node*) * tableSize);
	for(int i = 0; i < tableSize; i++) 
		t->head[i] = NULL;

	t->elementCount = 0;
	return t;
}


hashtable* insert(hashtable* t, int tableSize, char** keywords, int* keywordTokens, int num_keywords) {
	for(int index = 0; index < num_keywords; index++) {

		int hashValue = hashFunction(keywords[index], tableSize);
		node* tmp = t->head[hashValue];
		int flag = 0;

		while(tmp != NULL) {

			// to store last node
			if(tmp->next == NULL)
				break;

			tmp = tmp->next;
		}

		node* newNode = (node*) malloc(sizeof(node));
		strcpy(newNode->lexeme, keywords[index]);
		newNode->token = keywordTokens[index];
		newNode->next = NULL;
		t->elementCount++;
		printf("Inserted: %s %d\n", keywords[index], keywordTokens[index]);

		if(t->head[hashValue] == NULL)
			t->head[hashValue] = newNode;

		else
			tmp->next = newNode;
	}
	
	return t;
} 


int hashFunction(char* string, int tableSize) {
	int result = 0;

	for(int i = 0; i < strlen(string); i++) 
		result += 1 * string[i] * string[i];
	
	result %= tableSize;

	if(result < 0)
		result += tableSize;

	return result;
}
