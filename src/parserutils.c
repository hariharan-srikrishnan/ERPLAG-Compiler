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


// return the name of the nonterminal given its id
char* getNonTerminal(nonTermid ntid) {

	if(ntid == program)
		return "program"; 
	
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

/*
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
*/