#ifndef _PU
#define _PU
#include "parserDef.h"


// stack - just don't use prev pointer
typedef struct {
	rhsnode* top;
} stack;


// parse tree data structures
typedef union node tree_node;

typedef struct {
	nonterminal NT;
	tree_node* child;
} nlnode;


typedef struct {
	token T;
	tree_node* child;
} leafnode;


union node {
	nlnode n;
	leafnode l;
};


typedef struct {
	tree_node node; // store data
	int TorNT; // tag (T:0, NT:1)
} t_node;


extern t_node* parseTree;

rhsnode* createNode(symbol S, int TorNT);

void push(stack* s, rhsnode* newNode);

void pop(stack* s, int n);

nonterminal getNonTerminal(char* str);

char* getNonTerminalName(nonTermid ntid);

terminal getTerminal(char* str);

char* getTerminalName(tokenid tid);

#endif