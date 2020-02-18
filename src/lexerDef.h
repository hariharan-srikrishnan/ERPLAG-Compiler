#ifndef _LEXERDEF
#define _LEXERDEF


// for tokenizing integers or floats
typedef struct {
	int v1;
	float v2;
	int intOrFloat;
} VALUE;


// list of tokens
typedef enum {
	PLUS,
	MINUS,
	MUL,
	DIV,
	LT,
	LE,
	GT,
	GE,
	EQ,
	NE,
	DEF,
	ENDDEF,
	COLON,
	RANGEOP,
	SEMICOL,
	COMMA,
	ASSIGNOP,
	SQBO,
	SQBC, 
	BO,
	BC,
	COMMENTMARK,
	INTEGER,
	REAL,
	BOOLEAN,
	OF,
	ARRAY,
	START,
	END,
	DECLARE,
	MODULE,
	DRIVER,
	PROGRAM,
	RECORD,
	TAGGED,
	UNION,
	GET_VALUE,
	PRINT,
	USE,
	WITH,
	PARAMETERS,
	TRUE,
	FALSE,
	TAKES,
	INPUT, 
	RETURNS,
	AND,
	OR,
	FOR,
	IN,
	SWITCH,
	CASE,
	BREAK,
	DEFAULT,
	WHILE,
	ID,
	NUM,
	RNUM
} tokenid; 


typedef struct {
	tokenid tid;
	char lexeme[25];
	int lineNo;
} token;


char* keywords[] = {
	"integer",
	"real",
	"boolean",
	"of",
	"array",
	"start",
	"end",
	"declare",
	"module",
	"driver",
	"program",
	"record",
	"tagged",
	"union",
	"get_value",
	"print",
	"use",
	"with",
	"parameters",
	"true",
	"false",
	"takes",
	"input",
	"returns",
	"AND",
	"OR",
	"for",
	"in",
	"switch",
	"case",
	"break",
	"default",
	"while"
};


int keywordTokens[] = {
	INTEGER,
	REAL,
	BOOLEAN,
	OF,
	ARRAY,
	START,
	END,
	DECLARE,
	MODULE,
	DRIVER,
	PROGRAM,
	RECORD,
	TAGGED,
	UNION,
	GET_VALUE,
	PRINT,
	USE,
	WITH,
	PARAMETERS,
	TRUE,
	FALSE,
	TAKES,
	INPUT, 
	RETURNS,
	AND,
	OR,
	FOR,
	IN,
	SWITCH,
	CASE,
	BREAK,
	DEFAULT,
	WHILE,
};

#endif