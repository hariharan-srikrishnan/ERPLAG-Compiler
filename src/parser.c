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
unsigned long long int nullSet = (1LL << EPSILON);


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
		firstSet[i] = 0LL;
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
				firstSet[lhs.ntid] = setUnion(firstSet[i], nullSet);
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
				if(temp->TorNT == 0) { // node is a terminal
					firstSet[lhs.ntid] = setUnion(firstSet[lhs.ntid], (1LL << temp->S.T.tid));
					nullable = 0;
					break;
				}
				else { // node is a nonterminal
					// check if EPSILON is in first of this node
					if(setIntersection(firstSet[temp->S.NT.ntid], nullSet)) {
						// contains EPSILON
						firstSet[lhs.ntid] = setUnion(firstSet[lhs.ntid], firstSet[temp->S.NT.ntid]);
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


void computeFollowSets() {

}


void computeFirstAndFollowSets() {
	computeFirstSets();
	computeFollowSets();
}


int main() {
	fp = fopen("../grammar.txt", "r");
	readGrammar(fp);

	/*
	for(int i = 0; i < _NUM_RULES; i++) {
		printf("%s ", g[i].NT.name);
		rhsnode* tmp = g[i].head;
		while(tmp->next != NULL) {
			// if(tmp->TorNT)
			// 	printf("%s ", tmp->S.NT.name);
			// else
			// 	printf("%s ", tmp->S.T.name);

			tmp = tmp->next;
		}

		
		while (tmp != g[i].head) {
			if(tmp->TorNT)
				printf("%s ", tmp->S.NT.name);
			else
				printf("%s ", tmp->S.T.name);

			tmp = tmp->prev;
		}
		printf("%s \n", tmp->S.NT.name);
		
	}
	*/
	computeFirstSets();
	
	for(int i=0; i<_NUM_NONTERMINALS; i++) {
		if(setIntersection(firstSet[i], nullSet)) {
			printf("%d Nullable\n", i);
		}
		else {
			printf("%d Not nullable\n", i);
		}
	}

	
	printf("First Sets:\n");
	for(int i=0; i<_NUM_NONTERMINALS; i++) {
		for(int j=63; j>=0; j--) {
			if(setIntersection(firstSet[i], 1LL<<j)) {
				putchar('1');
			}
			else {
				putchar('0');
			}
		}
		putchar('\n');	
	}
	
}
