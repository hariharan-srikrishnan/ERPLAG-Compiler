#ifndef _PU
#define _PU
#include "parserDef.h"


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


// stack
typedef struct _stacknode {
	symbol S;
	int TorNT; // 0: T, 1: NT
	struct _stacknode* next;
	t_node* treeptr;
} stacknode;


typedef struct {
	stacknode* top;
} stack;


stacknode* createNode(symbol S, int TorNT);

stacknode* deepCopy(rhsnode* rhs);

void push(stack* s, stacknode* newNode);

void pop(stack* s, int n);

nonterminal getNonTerminal(char* str);

char* getNonTerminalName(nonTermid ntid);

terminal getTerminal(char* str);

char* getTerminalName(tokenid tid);

#endif