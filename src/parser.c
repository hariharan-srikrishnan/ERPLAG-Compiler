#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexerDef.h"
#include "lexer.h"
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
	
	else if (strcmp(str, "DRIVERDEF") == 0)
		t.tid = DRIVERDEF;

	else if (strcmp(str, "DRIVERENDDEF") == 0)
		t.tid = EPSILON;

	// error
	else
		t.tid = -1;

	return t;
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