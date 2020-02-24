#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexerDef.h"
#include "lexer.h"
#include "parserDef.h"
#include "parserutils.h" // includes ADT definitions

#define _ERROR -1
#define _SYN -2
#define _NUM_RULES 105
#define _NUM_TERMINALS 62 // verify
#define _NUM_NONTERMINALS 57 // verify

FILE* parserfp;
grammar g;
t_node* parseTreeRoot;


int parseTable[_NUM_NONTERMINALS][_NUM_TERMINALS];

unsigned long long int firstSet[_NUM_NONTERMINALS];
unsigned long long int followSet[_NUM_NONTERMINALS];
unsigned long long int nullSet = ((unsigned long long int)1 << EPSILON);


static inline unsigned long long int setUnion(unsigned long long int a, unsigned long long int b) {
	return a|b;
}


static inline unsigned long long int setIntersection(unsigned long long int a, unsigned long long int b) {
	return a&b;
}


// read grammar from file and populate data structure
void readGrammar(FILE* fp) {

	for(int i = 0; i < _NUM_RULES; i++) {
		char rule[200];
		char* tp = fgets(rule, sizeof(rule), fp);
		char* token = strtok(rule, " \n");
		g[i].NT = getNonTerminal(token);

		symbol S;
		rhsnode* tmp = NULL;
		while ((token = strtok(NULL, " \n")) != NULL) {
			int TorNT = 1;
			S.NT = getNonTerminal(token);
			strcpy(S.NT.name, token);

			if (S.NT.ntid == -1) {
				S.T = getTerminal(token);
				strcpy(S.T.name, token);
				TorNT = 0;
			}

			rhsnode* newNode = (rhsnode*) malloc(sizeof(rhsnode));
			newNode->S = S;
			newNode->TorNT = TorNT;
			newNode->next = NULL;
			newNode->prev = NULL;

			if (g[i].head == NULL) {
				g[i].head = newNode;
				tmp = newNode;
				continue;
			}

			tmp->next = newNode;
			newNode->prev = tmp;
			tmp = tmp->next;
		}

		free(token);
	}
}


void computeFirstSets() {
	/* initializing every first set to empty
	 Assumption: Terminals are not included in the first set array
					as their first sets are implicitly known
	*/
	for(int i=0; i<_NUM_NONTERMINALS; i++) {
		firstSet[i] = (unsigned long long int)0;
	} 


	/* Applying Rule 2 of finding first sets:
		For all non-terminals A, if there is a production rule of the form
		A -> EPSILON
		then add EPSILON to the first set of A 
	*/
	for(int i=0; i<_NUM_RULES; i++) {
		// if length of rhs of rule is 1, and value of rhs value is EPSILON
		// add EPSILON to first set of A
		nonterminal lhs = g[i].NT;
		if(g[i].head->next == NULL) {
			if(g[i].head->TorNT == 0 && (g[i].head->S).T.tid == EPSILON) {
				firstSet[lhs.ntid] = setUnion(firstSet[lhs.ntid], nullSet);
			}
		}
	}

	/* Applying Rule 3 and Rule 4:
		Rule 3: if P is an NT and P -> Q1 Q2 Q3 ... Qk is a production,
				then if for some i, First(Qi) = {x} and EPSILON is in all of
				First(Qj) for j<i,
				then add x to first(P)
		Rule 4: ifEPSILON is in all of first(Q1)...first(Qk),
				then add EPSILON to first(P)
	*/
	
	int changed = 1; // parameter for iterative solution
	while(changed) {
		
		// creating a copy of firstSet to check for change; loop termination condition
		unsigned long long int* firstSetCopy;
		firstSetCopy = calloc(_NUM_NONTERMINALS, sizeof(unsigned long long int));
		for(int i=0; i<_NUM_NONTERMINALS; i++) {
			firstSetCopy[i] = firstSet[i];
		}

		// applying rules 3 and 4 to all rules
		for(int i=0; i<_NUM_RULES; i++) {
			nonterminal lhs = g[i].NT;
			int nullable = 1; // 0 = False, 1 = True; used to check if whole rule is nullable
			// fprintf(stderr, "At rule %d\n", i);
			// iterating across RHS:
			rhsnode* temp = g[i].head;
			while(temp != NULL) {
				// check if node has a terminal or a nonterminal
				// fprintf(stderr, "Reach\n");
				if(temp->TorNT == 0) { 
					// node is a terminal
					firstSet[lhs.ntid] = setUnion(firstSet[lhs.ntid], ((unsigned long long int)1 << temp->S.T.tid));
					nullable = 0;
					break;
				}
				else { 
					// node is a nonterminal
					// check if EPSILON is in first of this node
					if(setIntersection(firstSet[temp->S.NT.ntid], nullSet)) {
						// contains EPSILON
						firstSet[lhs.ntid] = setUnion(firstSet[lhs.ntid], firstSet[temp->S.NT.ntid] & ~nullSet);
					}
					else {
						// does not contain EPSILON
						firstSet[lhs.ntid] = setUnion(firstSet[lhs.ntid], firstSet[temp->S.NT.ntid]);
						nullable = 0;
						break;
					}
				}


				temp = temp -> next;
			}

			if(nullable) {
				firstSet[i] = setUnion(firstSet[i], nullSet);
			}
		}

		changed = 0;
		for(int i=0; i<_NUM_NONTERMINALS; i++) {
			if(firstSetCopy[i] != firstSet[i]) {
				changed = 1;
				break;
			}
		}
		free(firstSetCopy);
	}
}


// print first set into file
void printFirstSet(FILE* f) {
	printf("First Sets:\n");
	
	for(int i=0; i<_NUM_NONTERMINALS; i++) {
		fprintf(f, "%s ", getNonTerminalName(i));
		for(int j=_NUM_TERMINALS-1; j>=0; j--) {
			if(setIntersection(firstSet[i], (unsigned long long int)1<< j)) {
				// if(strcmp(getTerminalName(j), "No Terminal") == 0)
					// printf("NO TERMINAL: %d\n", j);
				fprintf(f, "%s ", getTerminalName(j));
			}
		}
		fputc('\n', f);
	}
	fputc('\n', f);
}


void computeFollowSets() {

	/* initializing every follpw set to empty
	 Assumption: Terminals are not included in the follow set array
					as their follow sets are undefined
	*/
	for(int i=0; i<_NUM_NONTERMINALS; i++) {
		followSet[i] = (unsigned long long int)0;
	} 

	// RULE 1: If S is the start symbol of the grammar, then Follow(S) = '$'
	followSet[program] = setUnion(followSet[program], (unsigned long long int)1 << ENDMARKER);

	/* Implementation of rules 2, 3, and 4
		RULE 2: If there is a production A -> B C D,
				then everything in (First(D) - EPSILON) is in follow(C)
		RULE 3: If there is a production A -> B C D,
				and First(D) contains EPSILON,
				then everything in Follow(A) is in Follow(B)
		RULE 4: If there is a production A -> B C,
				then everything in Follow(A) is in Follow(B)
	*/

	int changed = 1; // parameter for iterative solution
	while(changed) {
		
		// creating a copy of firstSet to check for change; loop termination condition
		unsigned long long int* followSetCopy;
		followSetCopy = calloc(_NUM_NONTERMINALS, sizeof(unsigned long long int));
		for(int i=0; i<_NUM_NONTERMINALS; i++) {
			followSetCopy[i] = followSet[i];
		}

		for(int i=0; i<_NUM_RULES; i++) {
			nonterminal lhs = g[i].NT;
			rhsnode* temp = g[i].head;
			int nullable = 0;
			unsigned long long int tempFirstSet;

			while(temp->next != NULL)
				temp = temp->next;
			if(temp->TorNT == 1) {
				followSet[temp->S.NT.ntid] = setUnion(followSet[temp->S.NT.ntid], followSet[lhs.ntid]);
				if(setIntersection(firstSet[temp->S.NT.ntid], nullSet)) 
					nullable = 1;
				tempFirstSet = firstSet[temp->S.NT.ntid] & ~nullSet;

			}
			else {
				tempFirstSet = (unsigned long long int)1 << temp->S.T.tid;
			}
			temp = temp->prev;
			while(temp != NULL) {
				if(temp->TorNT == 1) {
					followSet[temp->S.NT.ntid] = setUnion(followSet[temp->S.NT.ntid], tempFirstSet);
					if(nullable) {
						followSet[temp->S.NT.ntid] = setUnion(followSet[temp->S.NT.ntid], followSet[lhs.ntid]);

					}
					if(setIntersection(firstSet[temp->S.NT.ntid], nullSet)) {
						tempFirstSet = setUnion(tempFirstSet, firstSet[temp->S.NT.ntid] & ~nullSet);
					}
					else {
						tempFirstSet = firstSet[temp->S.NT.ntid] & ~nullSet;
						nullable = 0;
					}
				}

				else {
					tempFirstSet = (unsigned long long int)1 << temp->S.T.tid;
					nullable = 0;
				}

				temp = temp -> prev;

			}
		}

		changed = 0;
		for(int i=0; i<_NUM_NONTERMINALS; i++) {
			if(followSetCopy[i] != followSet[i]) {
				changed = 1;
				break;
			}
		}
		free(followSetCopy);
	}
}


// print follow set into file
void printFollowSet(FILE* f) {
	printf("Follow Sets:\n");

	for(int i=0; i<_NUM_NONTERMINALS; i++) {
		fprintf(f, "%s ", getNonTerminalName(i));
		for(int j=_NUM_TERMINALS-1; j>=0; j--) {
			if(setIntersection(followSet[i], (unsigned long long int)1<< j) != 0LL) {
				fprintf(f, "%s ", getTerminalName(j));
			}
		}
		fputc('\n', f);
	}
	fputc('\n', f);
}


void computeFirstAndFollowSets() {
	computeFirstSets();
	computeFollowSets();
}


// populate predictive parse table
void createParseTable() {
	for(int i = 0; i < _NUM_NONTERMINALS; i++) {
		for(int j = 0; j < _NUM_TERMINALS; j++) {
			parseTable[i][j] = _ERROR; // initializing all states to be error by default
			if(setIntersection(followSet[i], (unsigned long long int)1 << j))
				parseTable[i][j] = _SYN;
		}
	}

	// for each production A->W
	for(int i=0; i<_NUM_RULES; i++) {
		nonterminal lhs = g[i].NT;
		rhsnode* temp = g[i].head;
		int nullable = 1;

		while(temp != NULL) {
		
			if(temp->TorNT == 0) {
				// symbol is a terminal
				parseTable[lhs.ntid][temp->S.T.tid] = i;
				if (temp->S.T.tid != EPSILON) 
					nullable = 0;
				break;
			}

			/* 
				for each terminal a in first(W), add the rule A->W to parseTable[A, a]
			*/
			for(int j = _NUM_TERMINALS-1; j >= 0; j--) {
				if( (j != EPSILON) && setIntersection(firstSet[temp->S.NT.ntid], (unsigned long long int)1 << j)) {
					parseTable[lhs.ntid][j] = i;
				}
			}

			/* 
				Let W = W_1 W_2 ... W_n
				if EPSILON in first(W_i), proceed till W_i+1
			*/
			if(setIntersection(firstSet[temp->S.NT.ntid], nullSet)) {
				temp = temp -> next;
			}
		
			// if non-nullable W_i found, move to next rule
			else {
				nullable = 0;
				break;
			}
		}

		if(!nullable)
			continue;
		
		/* 
			if EPSILON if in first(W):
				for each terminal b in Follow(A), add A->W to parseTable[A, b]
				if $ 
		*/
		for(int j = _NUM_TERMINALS-1; j >= 0; j--) {
			if(setIntersection(followSet[lhs.ntid], (unsigned long long int)1 << j)) {
				parseTable[lhs.ntid][j] = i;
			}
		}
	} 

}


// pretty print predictive parse table into a file
void printParseTable(FILE* outfile) {

	fprintf(outfile, "%23s", "");
	for(int i=0; i<_NUM_TERMINALS; i++) {
		fprintf(outfile, "%13s", getTerminalName(i));
	}
	fputc('\n', outfile);
	for (int i = 0; i < _NUM_NONTERMINALS; i++) {
		fprintf(outfile, "%23s", getNonTerminalName(i));
		for(int j = 0; j < _NUM_TERMINALS; j++)
			fprintf(outfile, "%13d", parseTable[i][j]);
		fprintf(outfile, "\n");
	}
}


// initializing parse tree
void initializeParseTree() {
	parseTreeRoot = (t_node*) malloc(sizeof(t_node)); 
	parseTreeRoot->TorNT = 1;
	parseTreeRoot->data.NT.ntid = program;
	strcpy(parseTreeRoot->data.NT.name, getNonTerminalName(program));
	parseTreeRoot->parent = NULL;
}


// predictive parsing algorithm	
void parseInputSourceCode(char* filename) {
	FILE* f = fopen(filename, "r");
	stack* s = (stack*) malloc(sizeof(stack));
	s->top = NULL;
	terminal T; 

	// initialize stack
	T.tid = ENDMARKER;
	strcpy(T.name, getTerminalName(T.tid));
	symbol S;
	S.T = T;
	stacknode* newNode = createNode(S, 0);
	push(s, newNode);
	nonterminal NT;
	NT.ntid = program;
	strcpy(NT.name, getNonTerminalName(NT.ntid));
	S.NT = NT;
	newNode = createNode(S, 1);
	newNode->treeptr = parseTreeRoot;
	push(s, newNode);

	token t = getNextToken();
	int syntaxError = 0;
	t_node *parent, *catchReturnValue;
	
	while(1) {
		// fprintf(stdout, "Next token: %s\n", getTerminalName(t.tid));
		
		// if top of stack is a token/terminal
		if(s->top->TorNT == 0) {
			if(t.tid == s->top->S.T.tid) {
				if(t.tid == ENDMARKER) {
					printf("Input source code is syntactically %scorrect.\n", (syntaxError)?("in"):(""));
					return;
				}

				else {
					// fprintf(stdout, "Popped from top of stack: %s\n", getTerminalName(t.tid));
					catchReturnValue = pop(s, 1);
					catchReturnValue->data.T = t;
					t = getNextToken();

				}
			}

			// panic mode recovery
			else {
				redColor();
				printf("Syntax Error: ");
				resetColor();
			 	printf("Error at line %d. Encountered token: %s. Expected token: %s\n", t.lineNo, getTerminalName(t.tid), getTerminalName(s->top->S.T.tid));
				
				// t = getNextToken();
				catchReturnValue = pop(s, 1);
				syntaxError = 1;
			}
		}

		// else, top of stack is a nonterminal
		else {
			int ruleNo = parseTable[s->top->S.NT.ntid][t.tid];

			// panic mode - error detection
			if (ruleNo == _ERROR) {
				redColor();
			 	printf("Syntax Error: ");
			 	resetColor();
			 	printf("Error at line %d. Encountered token: %s. Expected tokens: ", t.lineNo, getTerminalName(t.tid));

			 	for(int j=_NUM_TERMINALS-1; j>=0; j--) {
			 		if(setIntersection((unsigned long long int)1 << j, firstSet[s->top->S.NT.ntid])) {
			 			if((unsigned long long int)1 << j != nullSet) {
			 				printf("%s ", getTerminalName(j));
			 			}
			 		}
			 	}
			 	putchar('\n');

			 	t = getNextToken();
			 	syntaxError = 1;
			}

			// error recovery
			else if (ruleNo == _SYN) 
				catchReturnValue = pop(s, 1);

			else {
				t_node* children = NULL;
				t_node* childitr = NULL;
				t_node* childAddresses[15];
				int pos = 0;
				// fprintf(stdout, "Popped from top of stack: %s\n", s->top->S.NT.name);
				parent = pop(s, 1);
				rhsnode* temp = g[ruleNo].head;
				while(temp) {
					t_node* newNode = (t_node*) malloc(sizeof(t_node));
					childAddresses[pos++] = newNode;
					newNode->TorNT = temp->TorNT;
					newNode->sibling = NULL;
					newNode->parent = parent;
					newNode->children = NULL;
					
					// if symbol is a terminal
					if(newNode->TorNT == 0) {
						
						if(temp->S.T.tid == EPSILON) {
							newNode->data.T.tid = EPSILON;
							newNode->data.T.lineNo = t.lineNo;
							strcpy(newNode->data.T.lexeme, getTerminalName(EPSILON));
						}

						else {
							newNode->data.T.tid = temp->S.T.tid;
							newNode->data.T.lineNo = t.lineNo;
							strcpy(newNode->data.T.lexeme, temp->S.T.name);
						}
					}

					// else, symbol is a nonterminal 
					else {
						newNode->data.NT = temp->S.NT;
					}

					// first element in rhs of rule
					if(childitr == NULL) {
						children = newNode;
						childitr = newNode;
						childitr->parent = parent;
					}

					// one element already exists
					else {
						// setting sibling value
						childitr->parent = parent;
						childitr->sibling = newNode;
						childitr = newNode;
					}

					temp = temp->next;
				}

				// setting temp to point to last symbol in rule
				temp = g[ruleNo].head;
				while(temp->next) {
					temp = temp->next;
				}

				parent->children = children;

				while(temp) {
					if(temp->TorNT == 0 && temp->S.T.tid == EPSILON) {
						temp = temp->prev;
						--pos;
						continue;
					}


					stacknode* x = deepCopy(temp);
					x->treeptr = childAddresses[--pos];
					push(s, x);
					// fprintf(stdout, "Pushed on to stack: %s\n", (s->top->TorNT)?(s->top->S.NT.name):(s->top->S.T.name));
					temp = temp->prev;
				}

			}
		}
	}
}


// Helper function for printParseTree()
void printTreeNode(t_node* node, FILE* output) {

	/* Need to print:
		lexeme	lineNo	tokenName	valueIfNumber	parentNodeSymbol	isLeafNode	nodeSymbol
	*/
	
	// Node is a token/terminal/leaf node
	if(node->TorNT == 0) {
		if(node->data.T.tid == NUM) 
			fprintf(output, "%25s %8d %14s %15d %20s %12s %25s\n", node->data.T.lexeme, node->data.T.lineNo, getTerminalName(node->data.T.tid), atoi(node->data.T.lexeme), getNonTerminalName(node->parent->data.NT.ntid), "Y", "--------");
		else if(node->data.T.tid == RNUM) 
			fprintf(output, "%25s %8d %14s %15f %20s %12s %25s\n", node->data.T.lexeme, node->data.T.lineNo, getTerminalName(node->data.T.tid), atof(node->data.T.lexeme), getNonTerminalName(node->parent->data.NT.ntid), "Y", "--------");
		else 
			fprintf(output, "%25s %8d %14s %15s %20s %12s %25s\n", node->data.T.lexeme, node->data.T.lineNo, getTerminalName(node->data.T.tid), "--------", getNonTerminalName(node->parent->data.NT.ntid), "Y", "--------");
	}

	// Node is a nonterminal/non-leaf node
	else {
		if(node->parent != NULL)
			fprintf(output, "%25s %8s %14s %15s %20s %12s %25s\n", "--------", "--------", "--------", "--------", getNonTerminalName(node->parent->data.NT.ntid), "N", getNonTerminalName(node->data.NT.ntid));
		else
			fprintf(output, "%25s %8s %14s %15s %20s %12s %25s\n", "--------", "--------", "--------", "--------", "ROOT", "N", getNonTerminalName(node->data.NT.ntid));
	}

}


// Utility function for supporting recursive calls in printing parse tree
void printParseTreeUtil(t_node* node, FILE* output) {

	if(node == NULL)
		return;
	
	int num_children = 0;
	t_node* childList[12];
	t_node* iter = node->children;
	while(iter) {
		childList[num_children++] = iter;
		iter = iter->sibling;
	}
	if(num_children == 0) {
		printTreeNode(node, output);
		return;
	}

	printParseTreeUtil(childList[0], output);
	printTreeNode(node, output);

	for(int i=1; i<num_children; i++)
		printParseTreeUtil(childList[i], output);

	return;
}


// Print inorder traversal of the parse tree
void printParseTree(char* outfile) {
	FILE* output;
	output = fopen(outfile, "w");
	if(output == NULL) {
		fprintf(stderr, "Unable to open output file for printing parse tree!\n");
		return;
	}

	// printing column names
	fprintf(output, "%25s %8s %14s %15s %20s %12s %25s\n\n", "lexeme", "lineNo", "tokenName", "valueIfNumber", "parentNodeSymbol", "isLeafNode", "nodeSymbol");

	printParseTreeUtil(parseTreeRoot, output);
	fclose(output);
}


// int main() {
// 	parserfp = fopen("../grammar.txt", "r");
// 	readGrammar(parserfp);
// 	fclose(parserfp);

// 	/*
// 	for(int i = 0; i < _NUM_RULES; i++) {
// 		printf("%s ", g[i].NT.name);
// 		rhsnode* tmp = g[i].head;
// 		while(tmp != nullable) {
// 			if(tmp->TorNT)
// 				printf("%s ", tmp->S.NT.name);
// 			else
// 				printf("%s ", tmp->S.T.name);

// 			tmp = tmp->next;
// 		}
// 		putchar('\n');
	
// 		while (tmp != g[i].head) {
// 			if(tmp->TorNT)
// 				printf("%s ", tmp->S.NT.name);
// 			else
// 				printf("%s ", tmp->S.T.name);

// 			tmp = tmp->prev;
// 		}
// 		printf("%s \n", tmp->S.NT.name);
// 	*/	
// 	// }

// 	computeFirstAndFollowSets();
// 	printFirstSet(stdout);
// 	printFollowSet(stdout);
// 	initializeParseTree();
// 	createParseTable();
// 	FILE* f = fopen("parsetable.txt", "w");
// 	printParseTable(f);
// 	fclose(f);


// 	char* testfile = "t3.txt";
// 	parserfp = fopen(testfile, "r");
// 	getStream(parserfp);

// 	parseInputSourceCode(testfile);
// 	// FILE* parseTreeOutput = fopen("parseTree.txt", "w");
// 	printParseTree("parseTree.txt");
// 	// printTreeNode(parseTreeOutput, parseTreeRoot);
// 	t_node* tmp =  parseTreeRoot->children->sibling->sibling->children->sibling->sibling->sibling->sibling->children->sibling->sibling;
// 	printf("---------------%s--------------\n",getTerminalName(tmp->data.T.tid));
// 	// fclose(parseTreeOutput);
// }
