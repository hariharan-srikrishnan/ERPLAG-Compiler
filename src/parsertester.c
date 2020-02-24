/*
Anirudh S Chakravarthy - 2017A7PS1195P
Hariharan Srikrishnan  - 2017A7PS0134P
Honnesh Rohmetra	   - 2016B2A70770P
Praveen Ravirathinam   - 2017A7PS1174P
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexerDef.h"
#include "lexer.h"
#include "parserDef.h"
#include "parserutils.h"
#include "parser.h"

int main(){
	
	// open and read grammar
	parserfp = fopen("../grammar.txt", "r");
	readGrammar(parserfp);
	fclose(parserfp);

	// compute and print First and Follow sets of the grammar
	computeFirstAndFollowSets();
	printFirstSet(stdout);
	printFollowSet(stdout);
	
	// create and print the Parse Tree to a text file
	initializeParseTree();
	createParseTable();
	FILE* f = fopen("parsetable_test.txt", "w");
	printParseTable(f);
	fclose(f);

	// open and read program to parse
	char* testfile = "t2.txt";
	fp = fopen(testfile, "r");
	getStream(fp);

	// parse the program and print the Parse Tree to a text file
	parseInputSourceCode(testfile);
	printParseTree("parseTree_test.txt");

}