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

// return populated non-terminal structure
nonterminal getNonTerminal(char* str) {
	nonterminal nt;
	strcpy(nt.name, str);

	if (strcmp(str, "program") == 0) 
		nt.ntid = program;
	
	else if (strcmp(str, "moduleDeclarations") == 0)
		nt.ntid = moduleDeclarations;

	else if (strcmp(str, "moduleDeclaration") == 0)
		nt.ntid = moduleDeclaration;

	else if (strcmp(str, "otherModules") == 0)
		nt.ntid = otherModules;

	else if (strcmp(str, "driverModule") == 0)
		nt.ntid = driverModule;

	else if (strcmp(str, "module") == 0)
		nt.ntid = module;

	else if (strcmp(str, "ret") ==0)
		nt.ntid = ret;

	else if (strcmp(str, "input_plist") == 0)
		nt.ntid = input_plist;

	else if (strcmp(str, "N1") == 0)
		nt.ntid = N1;

	else if (strcmp(str, "output_plist") == 0)
		nt.ntid = output_plist;

	else if (strcmp(str, "N2") == 0)
		nt.ntid = N2;

	else if (strcmp(str, "dataType") == 0)
		nt.ntid = dataType;

	else if (strcmp(str, "range_arrays") == 0)
		nt.ntid = range_arrays;

	else if (strcmp(str, "type") == 0)
		nt.ntid = type;

	else if (strcmp(str, "moduleDef") == 0)
		nt.ntid = moduleDef;

	else if (strcmp(str, "statements") == 0)
		nt.ntid = statements;

	else if (strcmp(str, "statement") == 0)
		nt.ntid = statement;

	else if (strcmp(str, "ioStmt") == 0)
		nt.ntid = ioStmt;

	else if (strcmp(str, "boolConstt") == 0)
		nt.ntid = boolConstt;

	else if (strcmp(str, "var_id_num") == 0)
		nt.ntid = var_id_num;

	else if (strcmp(str, "var") == 0)
		nt.ntid = var;

	else if (strcmp(str, "whichId") == 0)
		nt.ntid = whichId;

	else if (strcmp(str, "simpleStmt") == 0)
		nt.ntid = simpleStmt;

	else if (strcmp(str, "assignmentStmt") == 0)
		nt.ntid = assignmentStmt;

	else if (strcmp(str, "whichStmt") == 0)
		nt.ntid = whichStmt;

	else if (strcmp(str, "lvalueIDStmt") == 0)
		nt.ntid = lvalueIDStmt;

	else if (strcmp(str, "lvalueARRStmt") == 0)
		nt.ntid = lvalueARRStmt;

	else if (strcmp(str, "index") == 0)
		nt.ntid = _index;

	else if (strcmp(str, "moduleReuseStmt") == 0)
		nt.ntid = moduleReuseStmt;

	else if (strcmp(str, "optional") == 0)
		nt.ntid = optional;

	else if (strcmp(str, "idList") == 0)
		nt.ntid = idList;

	else if (strcmp(str, "N3") == 0)
		nt.ntid = N3;

	else if (strcmp(str, "expression") == 0)
		nt.ntid = expression;

	else if (strcmp(str, "U") == 0)
		nt.ntid = U;

	else if (strcmp(str, "new_NT") == 0)
		nt.ntid = new_NT;

	else if (strcmp(str, "unary_op") == 0)
		nt.ntid = unary_op;

	else if (strcmp(str, "arithmeticOrBooleanExpr") == 0)
		nt.ntid = arithmeticOrBooleanExpr;

	else if (strcmp(str, "N7") == 0)
		nt.ntid = N7;

	else if (strcmp(str, "anyTerm") == 0)
		nt.ntid = anyTerm;

	else if (strcmp(str, "N8") == 0)
		nt.ntid = N8;

	else if (strcmp(str, "arithmeticExpr") == 0)
		nt.ntid = arithmeticExpr;

	else if (strcmp(str, "N4") == 0)
		nt.ntid = N4;

	else if (strcmp(str, "term") == 0)
		nt.ntid = term;

	else if (strcmp(str, "N5") == 0)
		nt.ntid = N5;

	else if (strcmp(str, "factor") == 0)
		nt.ntid = factor;

	else if (strcmp(str, "op1") == 0)
		nt.ntid = op1;

	else if (strcmp(str, "op2") == 0)
		nt.ntid = op2;

	else if (strcmp(str, "logicalOp") == 0)
		nt.ntid = logicalOp;

	else if (strcmp(str, "relationalOp") == 0)
		nt.ntid = relationalOp;

	else if (strcmp(str, "declareStmt") == 0)
		nt.ntid = declareStmt;

	else if (strcmp(str, "conditionalStmt") == 0)
		nt.ntid = conditionalStmt;

	else if (strcmp(str, "caseStmts") == 0)
		nt.ntid = caseStmts;

	else if (strcmp(str, "N9") == 0)
		nt.ntid = N9;

	else if (strcmp(str, "value") == 0)
		nt.ntid = value;

	else if (strcmp(str, "default") == 0)
		nt.ntid = _default;

	else if (strcmp(str, "iterativeStmt") == 0)
		nt.ntid = iterativeStmt;

	else if (strcmp(str, "range") == 0)
		nt.ntid = range;

	// error
	else
		nt.ntid = -1;

	return nt;
}


static inline unsigned long long int setUnion(unsigned long long int a, unsigned long long int b) {
	return a|b;
}


static unsigned long long int setIntersection(unsigned long long int a, unsigned long long int b) {
	return a&b;
}


terminal getTerminal(char* str) {
	terminal t;
	strcpy(t.name, str);

	if (strcmp(str, "PLUS") == 0) 
		t.tid = PLUS;
	
	else if (strcmp(str, "MINUS") == 0)
		t.tid = MINUS;
	
	else if (strcmp(str, "MUL") == 0)
		t.tid = MUL;

	else if (strcmp(str, "DIV") == 0)
		t.tid = DIV;

	else if (strcmp(str, "LT") == 0)
		t.tid = LT;

	else if (strcmp(str, "LE") == 0)
		t.tid = LE;

	else if (strcmp(str, "GT") == 0)
		t.tid = GT;

	else if (strcmp(str, "GE") == 0)
		t.tid = GE;

	else if (strcmp(str, "EQ") == 0)
		t.tid = EQ;

	else if (strcmp(str, "NE") == 0)
		t.tid = NE;

	else if (strcmp(str, "DEF") == 0)
		t.tid = DEF;

	else if (strcmp(str, "ENDDEF") == 0)
		t.tid = ENDDEF;

	else if (strcmp(str, "DRIVERDEF") == 0)
		t.tid = DRIVERDEF;

	else if (strcmp(str, "DRIVERENDDEF") == 0)
		t.tid = DRIVERENDDEF;

	else if (strcmp(str, "COLON") == 0)
		t.tid = COLON;

	else if (strcmp(str, "RANGEOP") == 0)
		t.tid = RANGEOP;

	else if (strcmp(str, "SEMICOL") == 0)
		t.tid = REAL;

	else if (strcmp(str, "COMMA") == 0)
		t.tid = COMMA;

	else if (strcmp(str, "ASSIGNOP") == 0)
		t.tid = ASSIGNOP;

	else if (strcmp(str, "SQBO") == 0)
		t.tid = SQBO;

	else if (strcmp(str, "SQBC") == 0)
		t.tid = SQBC;

	else if (strcmp(str, "BO") == 0)
		t.tid = BO;

	else if (strcmp(str, "BC") == 0)
		t.tid = BC;

	else if (strcmp(str, "COMMENTMARK") == 0)
		t.tid = COMMENTMARK;

	else if (strcmp(str, "INTEGER") == 0)
		t.tid = INTEGER;

	else if (strcmp(str, "REAL") == 0)
		t.tid = REAL;

	else if (strcmp(str, "BOOLEAN") == 0)
		t.tid = BOOLEAN;

	else if (strcmp(str, "OF") == 0)
		t.tid = OF;

	else if (strcmp(str, "ARRAY") == 0)
		t.tid = ARRAY;

	else if (strcmp(str, "START") == 0)
		t.tid = START;

	else if (strcmp(str, "END") == 0)
		t.tid = END;

	else if (strcmp(str, "DECLARE") == 0)
		t.tid = DECLARE;

	else if (strcmp(str, "MODULE") == 0)
		t.tid = MODULE;

	else if (strcmp(str, "DRIVER") == 0)
		t.tid = DRIVER;

	else if (strcmp(str, "PROGRAM") == 0)
		t.tid = PROGRAM;

	else if (strcmp(str, "RECORD") == 0)
		t.tid = RECORD;

	else if (strcmp(str, "TAGGED") == 0)
		t.tid = TAGGED;

	else if (strcmp(str, "UNION") == 0)
		t.tid = UNION;

	else if (strcmp(str, "GET_VALUE") == 0)
		t.tid = GET_VALUE;

	else if (strcmp(str, "PRINT") == 0)
		t.tid = PRINT;

	else if (strcmp(str, "USE") == 0)
		t.tid = USE;

	else if (strcmp(str, "WITH") == 0)
		t.tid = WITH;

	else if (strcmp(str, "PARAMETERS") == 0)
		t.tid = PARAMETERS;

	else if (strcmp(str, "TRUE") == 0)
		t.tid = TRUE;

	else if (strcmp(str, "FALSE") == 0)
		t.tid = FALSE;

	else if (strcmp(str, "TAKES") == 0)
		t.tid = TAKES;

	else if (strcmp(str, "INPUT") == 0)
		t.tid = INPUT;

	else if (strcmp(str, "RETURNS") == 0)
		t.tid = RETURNS;

	else if (strcmp(str, "AND") == 0)
		t.tid = AND;

	else if (strcmp(str, "OR") == 0)
		t.tid = OR;

	else if (strcmp(str, "FOR") == 0)
		t.tid = FOR;

	else if (strcmp(str, "IN") == 0)
		t.tid = IN;

	else if (strcmp(str, "SWITCH") == 0)
		t.tid = SWITCH;

	else if (strcmp(str, "CASE") == 0)
		t.tid = CASE;

	else if (strcmp(str, "BREAK") == 0)
		t.tid = BREAK;

	else if (strcmp(str, "DEFAULT") == 0)
		t.tid = DEFAULT;

	else if (strcmp(str, "WHILE") == 0)
		t.tid = WHILE;

	else if (strcmp(str, "ID") == 0)
		t.tid = ID;

	else if (strcmp(str, "NUM") == 0)
		t.tid = NUM;

	else if (strcmp(str, "RNUM") == 0)
		t.tid = RNUM;

	else if (strcmp(str, "ENDMARKER") == 0)
		t.tid = ENDMARKER;

	else if (strcmp(str, "EPSILON") == 0)
		t.tid = EPSILON;

	// error
	else
		t.tid = -1;

	return t;
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
