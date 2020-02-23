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

FILE* fp;
grammar g;

int parseTable[_NUM_NONTERMINALS][_NUM_TERMINALS];

unsigned long long int firstSet[_NUM_NONTERMINALS];
unsigned long long int followSet[_NUM_NONTERMINALS];
unsigned long long int nullSet = ((unsigned long long int)1 << EPSILON);


static inline unsigned long long int setUnion(unsigned long long int a, unsigned long long int b) {
	return a|b;
}


static unsigned long long int setIntersection(unsigned long long int a, unsigned long long int b) {
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
	rhsnode* newNode = createNode(S, 0);
	push(s, newNode);
	nonterminal NT;
	NT.ntid = program;
	strcpy(NT.name, getNonTerminalName(NT.ntid));
	S.NT = NT;
	newNode = createNode(S, 1);
	push(s, newNode);
	/*
	while(1) {
		token t = getNextToken();
		if ( (s->top->TorNT == 0) && t.tid == s->top->S.T.tid) {
			if(t.tid == ENDMARKER) 
				return;

			pop(s, 1);
		}

		else {
			int ruleNo = parseTable[s->top->S.NT.ntid][t.tid];
			if (ruleNo == _ERROR) {
				redColor();
				printf("Error: ");
				resetColor();
				printf("Syntax error at line %d\n", t.lineNo);
			}

			pop(s, 1);
			rhsnode* tmp = g[ruleNo].head;

			while(tmp->next != NULL)
				tmp = tmp->next;

			while(tmp != NULL) {
				newNode = deepCopy(tmp);
				push(s, newNode);
				tmp = tmp->prev;
			}
		}
	}
	*/

	token t = getNextToken();
	int syntaxError = 0;
	while(1) {
		// token t = getNextToken();
		// if(t.tid == ENDMARKER)
		// 	return;

		fprintf(stdout, "Next token: %s\n", getTerminalName(t.tid));
		
		if(s->top->TorNT == 0) {
			if(t.tid == s->top->S.T.tid) {
				if(t.tid == ENDMARKER) {
					printf("Input source code is syntactically %scorrect.\n", (syntaxError)?("in"):(""));
					return;
				}

				else {
					fprintf(stdout, "Popped from top of stack: %s\n", getTerminalName(t.tid));
					pop(s, 1);
					t = getNextToken();

				}
			}

			// panic mode recovery
			else {
				redColor();
				printf("Error: ");
				resetColor();
				printf("Syntax error at line %d\n", t.lineNo);
				
				// t = getNextToken();
				pop(s, 1);
				syntaxError = 1;
			}
		}

		else {
			int ruleNo = parseTable[s->top->S.NT.ntid][t.tid];

			// panic mode - error detection
			if (ruleNo == _ERROR) {
				redColor();
			 	printf("Error: ");
			 	resetColor();
			 	printf("Syntax error at line %d\n", t.lineNo);

			 	t = getNextToken();
			 	syntaxError = 1;
			}

			// error recovery
			else if (ruleNo == _SYN) 
				pop(s, 1);
			
			// if(ruleNo == _ERROR) {
			// 	redColor();
			// 	printf("Error: ");
			// 	resetColor();
			// 	printf("Syntax error at line %d\n", t.lineNo);
				
			// 	t = getNextToken();
			// 	if (t.tid == ENDMARKER) 
			// 			return;
			// 	ruleNo = parseTable[s->top->S.NT.ntid][t.tid];

			// 	while(ruleNo != _SYN) {
			// 		t = getNextToken();
			// 		if (t.tid == ENDMARKER) 
			// 			return;
			// 		ruleNo = parseTable[s->top->S.NT.ntid][t.tid];
			// 	}

			// 	pop(s, 1);
			// }

			else {
				fprintf(stdout, "Popped from top of stack: %s\n", s->top->S.NT.name);
				pop(s, 1);
				rhsnode* temp = g[ruleNo].head;
				while(temp->next)
					temp = temp->next;

				while(temp) {
					if(temp->TorNT == 0 && temp->S.T.tid == EPSILON) {
						temp = temp->prev;
						continue;
					}

					push(s, deepCopy(temp));
					fprintf(stdout, "Pushed on to stack: %s\n", s->top->S.T.name);
					temp = temp->prev;
				}

			}
		}
	}
}


int main() {
	fp = fopen("../grammar.txt", "r");
	readGrammar(fp);
	fclose(fp);

	/*
	for(int i = 0; i < _NUM_RULES; i++) {
		printf("%s ", g[i].NT.name);
		rhsnode* tmp = g[i].head;
		while(tmp != NULL) {
			if(tmp->TorNT)
				printf("%s ", tmp->S.NT.name);
			else
				printf("%s ", tmp->S.T.name);

			tmp = tmp->next;
		}
		putchar('\n');
	
		while (tmp != g[i].head) {
			if(tmp->TorNT)
				printf("%s ", tmp->S.NT.name);
			else
				printf("%s ", tmp->S.T.name);

			tmp = tmp->prev;
		}
		printf("%s \n", tmp->S.NT.name);
	*/	
	// }

	computeFirstAndFollowSets();
	printFirstSet(stdout);
	printFollowSet(stdout);
	createParseTable();
	FILE* f = fopen("parsetable.txt", "w");
	printParseTable(f);
	fclose(f);

	char* testfile = "test.txt";
	fp = fopen(testfile, "r");
	getStream(fp);

	parseInputSourceCode(testfile);
}
