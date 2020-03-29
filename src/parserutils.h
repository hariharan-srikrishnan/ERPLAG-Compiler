/*
Anirudh S Chakravarthy - 2017A7PS1195P
Hariharan Srikrishnan  - 2017A7PS0134P
Honnesh Rohmetra	   - 2016B2A70770P
Praveen Ravirathinam   - 2017A7PS1174P
*/


#ifndef _PU
#define _PU
#include "parserDef.h"


// parse tree data structures
typedef union _treeData treeData;
struct _astnode;

union _treeData {
	token T;
	nonterminal NT;
};

typedef struct _tnode{
	int TorNT; // tag (T:0, NT:1)
	struct _tnode* sibling;
	struct _tnode* children;
	struct _tnode* parent;
	treeData data;
	struct _astnode *syn, *inh; 
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

t_node* pop(stack* s, int n);

nonterminal getNonTerminal(char* str);

char* getNonTerminalName(nonTermid ntid);

terminal getTerminal(char* str);

char* getTerminalName(tokenid tid);

#endif