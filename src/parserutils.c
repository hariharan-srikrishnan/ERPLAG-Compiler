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


// deep copy
rhsnode* deepCopy(rhsnode* rhs) {
	rhsnode* newNode = createNode(rhs->S, rhs->TorNT);
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
char* getNonTerminalName(nonTermid ntid) {

	if (ntid == program)
		return "program"; 
	
	else if (ntid == moduleDeclarations)
		return "moduleDeclarations";

	else if (ntid == moduleDeclaration)
		return "moduleDeclaration";

	else if (ntid == otherModules)
		return "otherModules";

	else if (ntid == driverModule)
		return "driverModule";

	else if (ntid == module)
		return "module";

	else if (ntid == ret)
		return "ret";

	else if (ntid == input_plist)
		return "input_plist";

	else if (ntid == N1)
		return "N1";

	else if (ntid == output_plist)
		return "output_plist";

	else if (ntid == N2)
		return "N2";

	else if (ntid == dataType)
		return "dataType";

	else if (ntid == range_arrays)
		return "range_arrays";

	else if (ntid == type)
		return "type";

	else if (ntid == moduleDef)
		return "moduleDef";

	else if (ntid == statements)
		return "statements";

	else if (ntid == statement)
		return "statement";

	else if (ntid == ioStmt)
		return "ioStmt";

	else if (ntid == boolConstt)
		return "boolConstt";

	else if (ntid == var_id_num)
		return "var_id_num";

	else if (ntid == var)
		return "var";

	else if (ntid == whichId)
		return "whichId";

	else if (ntid == simpleStmt)
		return "simpleStmt";

	else if (ntid == assignmentStmt)
		return "assignmentStmt";

	else if (ntid == whichStmt)
		return "whichStmt";

	else if (ntid == lvalueIDStmt)
		return "lvalueIDStmt";

	else if (ntid == lvalueARRStmt)
		return "lvalueARRStmt";

	else if (ntid == _index)
		return "index";

	else if (ntid == moduleReuseStmt)
		return "moduleReuseStmt";

	else if (ntid == optional)
		return "optional";

	else if (ntid == idList)
		return "idList";

	else if (ntid == N3)
		return "N3";

	else if (ntid == expression)
		return "expression";

	else if (ntid == U)
		return "U";

	else if (ntid == new_NT)
		return "new_NT";

	else if (ntid == unary_op)
		return "unary_op";

	else if (ntid == arithmeticOrBooleanExpr)
		return "arithmeticOrBooleanExpr";

	else if (ntid == N7)
		return "N7";

	else if (ntid == anyTerm)
		return "anyTerm";

	else if (ntid == N8)
		return "N8";

	else if (ntid == arithmeticExpr)
		return "arithmeticExpr";

	else if (ntid == N4)
		return "N4";

	else if (ntid == term)
		return "term";

	else if (ntid == N5)
		return "N5";

	else if (ntid == factor)
		return "factor";

	else if (ntid == op1)
		return "op1";

	else if (ntid == op2)
		return "op2";

	else if (ntid == logicalOp)
		return "logicalOp";

	else if (ntid == relationalOp)
		return "relationalOp";

	else if (ntid == declareStmt)
		return "declareStmt";

	else if (ntid == conditionalStmt)
		return "conditionalStmt";

	else if (ntid == caseStmts)
		return "caseStmts";

	else if (ntid == N9)
		return "N9";

	else if (ntid == value)
		return "value";

	else if (ntid == _default)
		return "default";

	else if (ntid == iterativeStmt)
		return "iterativeStmt";

	else if (ntid == range)
		return "range";

	// error
	else 
		return "No Nonterminal";
}


// return populated terminal structure
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
		t.tid = SEMICOL;

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


//return terminal name given id
char* getTerminalName(tokenid tid) {

	if (tid == PLUS)
		return "PLUS"; 
	
	else if (tid == MINUS)
		return "MINUS";
	
	else if (tid == MUL)
		return "MUL";

	else if (tid == DIV)
		return "DIV";

	else if (tid == LT)
		return "LT";

	else if (tid == LE)
		return "LE";

	else if (tid == GT)
		return "GT";

	else if (tid == GE)
		return "GE";

	else if (tid == EQ)
		return "EQ";

	else if (tid == NE)
		return "NE";

	else if (tid == DEF)
		return "DEF";

	else if (tid == ENDDEF)
		return "ENDDEF";

	else if (tid == DRIVERDEF)
		return "DRIVERDEF";

	else if (tid == DRIVERENDDEF)
		return "DRIVERENDDEF";

	else if (tid == COLON)
		return "COLON";

	else if (tid == RANGEOP)
		return "RANGEOP";

	else if (tid == SEMICOL)
		return "SEMICOL";

	else if (tid == COMMA)
		return "COMMA";

	else if (tid == ASSIGNOP)
		return "ASSIGNOP";

	else if (tid == SQBO)
		return "SQBO";

	else if (tid == SQBC)
		return "SQBC";

	else if (tid == BO)
		return "BO";

	else if (tid == BC)
		return "BC";

	else if (tid == COMMENTMARK)
		return "COMMENTMARK";

	else if (tid == INTEGER)
		return "INTEGER";

	else if (tid == REAL)
		return "REAL";

	else if (tid == BOOLEAN)
		return "BOOLEAN";

	else if (tid == OF)
		return "OF";

	else if (tid == ARRAY)
		return "ARRAY";

	else if (tid == START)
		return "START";

	else if (tid == END)
		return "END";

	else if (tid == DECLARE)
		return "DECLARE";

	else if (tid == MODULE)
		return "MODULE";

	else if (tid == DRIVER)
		return "DRIVER";

	else if (tid == PROGRAM)
		return "PROGRAM";

	else if (tid == RECORD)
		return "RECORD";

	else if (tid == TAGGED)
		return "TAGGED";

	else if (tid == UNION)
		return "UNION";

	else if (tid == GET_VALUE)
		return "GET_VALUE";

	else if (tid == PRINT)
		return "PRINT";

	else if (tid == USE)
		return "USE";

	else if (tid == WITH)
		return "WITH";

	else if (tid == PARAMETERS)
		return "PARAMETERS";

	else if (tid == TRUE)
		return "TRUE";

	else if (tid == FALSE)
		return "FALSE";

	else if (tid == TAKES)
		return "TAKES";

	else if (tid == INPUT)
		return "INPUT";

	else if (tid == RETURNS)
		return "RETURNS";

	else if (tid == AND)
		return "AND";

	else if (tid == OR)
		return "OR";

	else if (tid == FOR)
		return "FOR";

	else if (tid == IN)
		return "IN";

	else if (tid == SWITCH)
		return "SWITCH";

	else if (tid == CASE)
		return "CASE";

	else if (tid == BREAK)
		return "BREAK";

	else if (tid == DEFAULT)
		return "DEFAULT";

	else if (tid == WHILE)
		return "WHILE";

	else if (tid == ID)
		return "ID";

	else if (tid == NUM)
		return "NUM";

	else if (tid == RNUM)
		return "RNUM";

	else if (tid == ENDMARKER)
		return "ENDMARKER";

	else if (tid == EPSILON)
		return "EPSILON";

	// error
	else
		return "No Terminal";
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