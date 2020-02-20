#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parserDef.h"


const int _NUM_RULES = 105;
const int _NUM_TERMINALS = 62; // verify
const int _NUM_NONTERMINALS = 56; // verify

FILE* fp;
grammar g;


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
		nt.ntid = index;

	else if (strcmp(str, "moduleReuseStmt") == 0)
		nt.ntid = moduleReuseStmt;

	else if (strcmp(str, "optional") == 0)
		nt.ntid = optional;

	else if (strcmp(str, "idList") == 0)
		nt.ntid = idList;

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

	else if (strcmp(str, "_default") == 0)
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


terminal getTerminal() {

}


// read grammar from file and populate data structure
void readGrammar(FILE* fp) {

	for(int i = 0; i < _NUM_RULES; i++) {
		char tmp[200];
		fgets(tmp, sizeof(tmp), fp);
		char* token = strtok(tmp, " \n");
		g[i].NT = getNonTerminal(token);

		symbol S;
		rhsnode* tmp = NULL;
		while (token != NULL) {
			int TorNT = 1;
			S = getNonTerminal(token);

			if (S.ntid == -1) {
				S = getTerminal(token);
				TorNT = 0;
			}

			strcpy(S.name, token);
			rhsnode* newNode = (rhsnode*) malloc(sizeof(rhsnode));
			newNode->S = S;
			newNode->TorNT = TorNT;
			newNode->next = NULL;

			if (g[i].head == NULL) {
				g[i].head = newNode;
				tmp = newNode;
				continue;
			}

			tmp->next = newNode;
			tmp = tmp->next;
			token = strtok(NULL, " \n");
		}
	}
}