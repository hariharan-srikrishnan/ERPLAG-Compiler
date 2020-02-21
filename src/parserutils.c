#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parserutils.h"


t_node* parseTree;


// utility function to create new node for stack
rhsnode* createNode(symbol S, int TorNT) {
	rhsnode* newNode = (rhsnode*) malloc(sizeof(rhsnode));
	newNode->S = S;
	newNode->TorNT = TorNT;
	newNode->next = NULL;
	return newNode;
}

// push new node onto the stack
void push(stack* s, rhsnode* newNode) {
	if (s->top == NULL) 
		s->top = newNode;

	else {
		newNode->next = s->top;
		s->top = newNode;	
	}

	return;
}


// pop n items from stack
void pop(stack* s, int n) {
	for(int i = 0; i < n; i++) {
		
		// stack already empty
		if (s->top == NULL) 
			return;

		rhsnode* tmp = s->top;
		s->top = tmp->next;
		free(tmp);
	}
}

int main() {
	stack* s = (stack*) malloc(sizeof(stack));
	s->top = NULL;
	terminal T; 
	T.tid = EPSILON;
	strcpy(T.name, "EPSILON");
	symbol S;
	S.T = T;
	rhsnode* newNode = createNode(S, 0);
	push(s, newNode);
	printf("%s\n", s->top->S.T.name);
	newNode = createNode(S, 0);
	push(s, newNode);
	printf("%s\n", s->top->S.T.name);
	pop(s, 3);
	return 0;
}
