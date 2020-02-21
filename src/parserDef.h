#ifndef _PARSERDEF
#define _PARSERDEF
#include "lexerDef.h"


// list of non-terminals
typedef enum {
	program,
	moduleDeclarations,
	moduleDeclaration,
	otherModules,
	driverModule,
	module,
	ret,
	input_plist,
	N1,
	output_plist,
	N2,
	dataType,
	range_arrays,
	type,
	moduleDef,
	statements,
	statement,
	ioStmt,
	boolConstt, 
	var_id_num,
	var,
	whichId,
	simpleStmt,
	assignmentStmt,
	whichStmt,
	lvalueIDStmt,
	lvalueARRStmt,
	_index,
	moduleReuseStmt,
	optional,
	idList,
	N3,
	expression,
	U,
	new_NT,
	unary_op,
	arithmeticOrBooleanExpr,
	N7,
	anyTerm,
	N8,
	arithmeticExpr,
	N4,
	N5,
	term,
	factor, 
	op1,
	op2,
	logicalOp,
	relationalOp,
	declareStmt,
	conditionalStmt,
	caseStmts,
	N9,
	value,
	_default,
	iterativeStmt,
	range
} nonTermid; 


typedef struct {
	nonTermid ntid;
	char name[25];
} nonterminal;


typedef struct {
	tokenid tid;
	char name[25];
} terminal;


typedef union {
	terminal T;
	nonterminal NT;
} symbol;


typedef struct _rhsnode {
	symbol S;
	int TorNT; // 0: T, 1: NT
	struct _rhsnode* next;
	struct _rhsnode* prev;
} rhsnode;


typedef struct {
	nonterminal NT;
	rhsnode* head;
} grammar[105];


#endif