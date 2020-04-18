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

int main(int argc, char* argv[]){
	
	// open and read grammar
	parserfp = fopen("grammar.txt", "r");
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
	// char* testfile = "t2.txt";
	fp = fopen(argv[1], "r");
	if (fp == NULL)
		printf("HRE!");
	getStream(fp);

	// parse the program and print the Parse Tree to a text file
	parseInputSourceCode(argv[2]);
	if (!syntaxError)
		printParseTree("parseTree.txt");

}