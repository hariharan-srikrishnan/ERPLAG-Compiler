/*
Anirudh S Chakravarthy - 2017A7PS1195P
Hariharan Srikrishnan  - 2017A7PS0134P
Honnesh Rohmetra	   - 2016B2A70770P
Praveen Ravirathinam   - 2017A7PS1174P
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "hash.h"
#include "lexer.h"
#include "lexerDef.h"
#include "parserDef.h"
#include "parser.h"
#include "parserutils.h"
#include "ast.h"
#include "symboltableutils.h"
#include "symboltableDef.h"
#include "symboltable.h"
#include "typeExtractor.h"
#include "semantics.h"
#include "codegen.h"



void printArrayInfo(idSymbolTable* table) {

	if (table == NULL)
		return;

	// iterate over current table
	for (int i = 0; i < table->hashSize; i++) {
		idNode* itr = table->list[i].head;
		while (itr) {
			
			// if an array
			if (itr->entry.AorP == 1) {
				char* tableName = table->name;
				int startLine = table->startLineNo;
				int endLine = table->endLineNo;
				char* arrName = itr->entry.name;
				char* isStatic = (char*) malloc (sizeof(char) * 10);
				char* lb = itr->entry.type.array.lowerBound.lexeme;
				char* ub = itr->entry.type.array.upperBound.lexeme;
				char* arrtype = (char*) malloc(sizeof(char) * 10);

				if (itr->entry.type.array.dynamicArray == 0) 
					strcpy(isStatic, "static");

				else 
					strcpy(isStatic, "dynamic");

				if (itr->entry.type.array.datatype.datatype.tid == INTEGER)
					strcpy(arrtype, "integer");

				else if (itr->entry.type.array.datatype.datatype.tid == BOOLEAN)
					strcpy(arrtype, "boolean");

				else 
					strcpy(arrtype, "real");
				
				printf("%20s\t%3d-%3d\t%20s\t%10s\t[%s, %s]\t%10s\n", tableName, startLine, endLine, arrName, isStatic, lb, ub, arrtype);
				free(isStatic);
				free(arrtype);
			}
			itr = itr->next;
		}
	}

	idSymbolTable* tmp = table->child;
	while (tmp) {
		printArrayInfo(tmp);
		tmp = tmp->sibling;
	}
}

void printSymTableUtil(idSymbolTable *table, int nestingLevel) {
	if(table == NULL)
		return;
	
	// operations at node for every variable present in this table
	for(int i = 0; i < table->hashSize; i++) {
		// idLinkedList* ll = table->list;
		idNode* node = table->list[i].head;
		while (node != NULL) {
			// variable name
			printf("%15s\t", node->entry.name);
			// scope - module name ???
			printf("%15s\t", table->name); //scope - module name
			// scope - start and end line numbers of scope
			printf("%3d - %3d\t", node->entry.id.lineNo, table->endLineNo);
			// width
			if(node->entry.AorP == 0) {
				printf("%1d\t", node->entry.type.primitive.width);
			}
			else {
				if(node->entry.type.array.dynamicArray == 1) {
					printf("8\t");
				}
				else {
					int lowerbound = atoi(node->entry.type.array.lowerBound.lexeme);
					int upperbound = atoi(node->entry.type.array.upperBound.lexeme);
					int sz = upperbound-lowerbound;
					sz = (sz + 1) * node->entry.type.array.datatype.width;
					printf("%1d\t", sz);
				}
			}
			// is array?
			printf("%4s\t", (node->entry.AorP == 1)?("yes"):("no"));
			// if array, static or dynamic?
			if(node->entry.AorP == 0) {
				char* dash = (char*) malloc(sizeof(char) * 5);
				strcpy(dash, "-----");
				printf("%9s\t", dash);
				free(dash);
			}
			else {
				if(node->entry.type.array.dynamicArray == 0) {
					printf("%8s\t", "static");
				}
				else {
					printf("%8s\t", "dynamic");
				}
			}
			// if array, range variables
			if(node->entry.AorP == 0) {
				char* dash = (char*) malloc(sizeof(char) * 5);
				strcpy(dash, "----------");
				printf("%25s\t", dash);
				free(dash);
			}
			else {
				char* range = (char*) malloc(sizeof(char) * 30);
				sprintf(range, "[%s, %s]", node->entry.type.array.lowerBound.lexeme, node->entry.type.array.upperBound.lexeme);
				printf("%25s\t", range);
				free(range);
			}
			// type of element
			if(node->entry.AorP == 0) {
				char* datatype = (char*) malloc(sizeof(char) * 10);
				if (node->entry.type.primitive.datatype.tid == INTEGER)
					strcpy(datatype, "integer");

				else if (node->entry.type.primitive.datatype.tid == BOOLEAN)
					strcpy(datatype, "boolean");

				else
					strcpy(datatype, "real");
					
				printf("%8s\t", datatype);
				free(datatype);
			}
			else {
				char* datatype = (char*) malloc(sizeof(char) * 10);
				if (node->entry.type.array.datatype.datatype.tid == INTEGER)
					strcpy(datatype, "integer");

				else if (node->entry.type.array.datatype.datatype.tid == BOOLEAN)
					strcpy(datatype, "boolean");

				else
					strcpy(datatype, "real");
					
				printf("%8s\t", datatype);
				free(datatype);
			}
			// offset
			printf("%3d\t", node->entry.offset);
			// nesting level
			printf("%2d\t", nestingLevel);
			// END of printing variable
			printf("\n");
			node = node->next;
		}
	}

	// repeat for all siblings
	printSymTableUtil(table->sibling, nestingLevel);
	// repeat for all children
	printSymTableUtil(table->child, nestingLevel + 1);
}


void printSymTable() {
	idSymbolTable *temp = globalIdTable;
	printSymTableUtil(temp, -1);
}

int main (int argc, char* argv[]) {
	int choice;
	magentaColor();
	printf("Project status: LEVEL 4\n");
	printf("ASSUMPTION: sizeof(int) = sizeof(real) = sizeof(bool) = 8 - WE HAD REQUESTED FOR YOUR CONSIDERATION OVER EMAIL!\n");
	resetColor();
	printf("\t1. AST and Symbol table created working correctly.\n");
	printf("\t2. All semantic rules implemented (except input parameter shadowing) and passing all test cases.\n");
	printf("\t3. All type check and static bound checks duly performed wherever applicable.\n");
	printf("\t4. Code generation attempted - all test cases except c7.txt and c11.txt.\n");
	printf("\t5. Dynamic type check and caller-callee interaction also handled.\n");
	printf("\t6. In c7.txt, facing issues with scanf and malloc due to stack alignment - not allowing to read beyond first array.\n");
	printf("\t7. In c7.txt, attempted allocating dynamic arrays through stack and heap. Stack causing memory corruption leading to incorrect output, while malloc and scanf creating several issues for heap.\n");
	printf("\t8. In c9.txt, unable to enforce the left associativity due to the form of the grammar (however moving the subtraction to the end works correctly).\n");
	printf("\t9. Code generation on dynamic arrays / dynamic function parameters gives segmentation fault as dynamic arrays are not allowed as input parameters.\n\n\n");


	if (argc != 3) {
		printf("Number of arguments don't match\n");
		exit(0);
	}
 
	// need to read grammar only once
	int grammarRead = 0;

	do {
		magentaColor();
		printf("Please enter your choice:\n");
		resetColor();
		scanf("%d", &choice);
		switch(choice) {
			// exit
			case 0: exit(0);

			// print token list on console
			case 1: fp = fopen(argv[1], "r");
					getStream(fp);
					token tk;
					while(1) {
						tk = getNextToken();
						if (tk.tid == ENDMARKER)
							break;

						printf("Line Number: %d  Lexeme: %s  Token Name: %s\n", tk.lineNo, tk.lexeme, getTerminalName(tk.tid));
					}
					fclose(fp);
					lineno = 1;
					break;
			
			// parse source code
			case 2: if(!grammarRead) {
						parserfp = fopen("grammar.txt", "r");
						readGrammar(parserfp);
						fclose(parserfp);
						grammarRead = 1;
					}
					
					computeFirstAndFollowSets();
					initializeParseTree();
					createParseTable();

					fp = fopen(argv[1], "r");
					getStream(fp);
					parseInputSourceCode(argv[1]);
					fclose(fp);
					lineno = 1;
					break;
			
			// AST traversal
			case 3: if(!grammarRead) {
						parserfp = fopen("grammar.txt", "r");
						readGrammar(parserfp);
						fclose(parserfp);
						grammarRead = 1;
					}
					
					computeFirstAndFollowSets();
					initializeParseTree();
					createParseTable();

					fp = fopen(argv[1], "r");
					getStream(fp);
					parseInputSourceCode(argv[1]);
					fclose(fp);

					printf("Preorder Traversal order: \n");
					createAST(parseTreeRoot);
					printAST(parseTreeRoot->syn);
					lineno = 1;
					break;
			
			// PT and AST memory 
			case 4: if(!grammarRead) {
						parserfp = fopen("grammar.txt", "r");
						readGrammar(parserfp);
						fclose(parserfp);
						grammarRead = 1;
					}
					
					computeFirstAndFollowSets();
					initializeParseTree();
					createParseTable();

					fp = fopen(argv[1], "r");
					getStream(fp);
					parseInputSourceCode(argv[1]);
					fclose(fp);

					createAST(parseTreeRoot);
					computeParseTreeSize(parseTreeRoot);
					computeASTSize(parseTreeRoot->syn);

					printf("Number of Parse Tree Nodes: %d \t Memory: %llu\n", parseTreeNodes, parseTreeSize);
					printf("Number of AST Nodes: %d \t Memory: %llu\n", astNodes, astSize);

					double compressionRatio = (parseTreeSize - astSize) * 100.0 / parseTreeSize;
					printf("Compression Ratio: 100 * (%lld - %lld) / %lld = %lf%%\n", parseTreeSize, astSize, parseTreeSize, compressionRatio);
					lineno = 1;
					break;
			
			
			// Print symbol table
			case 5: printSymTable();
					break;

			// Activation record size
			case 6: magentaColor();
					printf("ASSUMPTION: sizeof(int) = sizeof(real) = sizeof(bool) = 8 - WE HAD REQUESTED FOR YOUR CONSIDERATION OVER EMAIL!\n");
					resetColor();

					for (int i = 0; i < funcTable.hashSize; i++) {
						funcNode* tmp = funcTable.list[i].head;
						while (tmp) {
							int tableSize = findNextOffset(&(tmp->entry.link));
							printf("%20s\t%5d\n", tmp->entry.name, tableSize);
							tmp = tmp->next;
						}
					}
					break;
					
			// Static and Dynamic Arrays
			case 7: printArrayInfo(globalIdTable);
					break;

			// Error Reporting and total Compiling time
			case 8: if(!grammarRead) {
						parserfp = fopen("grammar.txt", "r");
						readGrammar(parserfp);
						fclose(parserfp);
						grammarRead = 1;
					}
				
					fp = fopen(argv[1], "r");
					clock_t start_time, end_time;
					double total_CPU_time, total_CPU_time_in_seconds;
					start_time = clock();

					getStream(fp);
					computeFirstAndFollowSets();
					initializeParseTree();
					createParseTable();
					parseInputSourceCode(argv[1]);

					if (!syntaxError) {
						createAST(parseTreeRoot);
						extractTypeAST(parseTreeRoot->syn);
						semanticChecker(parseTreeRoot->syn);
						
						if (semanticError == 0)
							printf("Code compiles successfully!\n");	
					}

					end_time = clock();
					fclose(fp);

	                total_CPU_time  =  (double) (end_time - start_time);
	                total_CPU_time_in_seconds =   total_CPU_time / CLOCKS_PER_SEC;
	                printf("Total CPU time: %lf, Total CPU time in sec: %lf\n", total_CPU_time, total_CPU_time_in_seconds);
					
					lineno = 1;
					break;

			// Code Generation
			case 9: if(!grammarRead) {
						parserfp = fopen("grammar.txt", "r");
						readGrammar(parserfp);
						fclose(parserfp);
						grammarRead = 1;
					}
					
					fp = fopen(argv[1], "r");
					getStream(fp);
					computeFirstAndFollowSets();
					initializeParseTree();
					createParseTable();
					parseInputSourceCode(argv[1]);
					fclose(fp);

					createAST(parseTreeRoot);
					extractTypeAST(parseTreeRoot->syn);
					semanticChecker(parseTreeRoot->syn);
					printf("Code compiles successfully!\n");
					asmFile = fopen(argv[2], "w");
					generateASM(parseTreeRoot->syn);
					fclose(asmFile);	
					lineno = 1;
					break;	
		}
	} while (choice != 0);
}


/* STAGE 1: DRIVER */
/*
int main(int argc, char* argv[]) {
	int choice;
	magentaColor();
	printf("Project status: \n");
	resetColor();

	printf("\t a. FIRST and FOLLOW set automated.\n");
	printf("\t b. Both lexical and syntax analysis modules implemented.\n");
	printf("\t c. Parsing complete based on predictive parsing.\n");
	printf("\t d. Parse tree generated.\n");
	printf("\t e. Modules work with all testcases.\n");
	printf("\t f. Error handling also performed.\n");


	if (argc != 3) {
		printf("Number of arguments don't match\n");
		exit(0);
	}

	// need to read grammar only once
	int grammarRead = 0;

	do {
		printf("Please enter your choice:\n");
		scanf("%d", &choice);
		switch(choice) {
			case 0: exit(0);

			case 1: removeComments(argv[1], stdout);
					break;

			case 2: fp = fopen(argv[1], "r");
					getStream(fp);
					token tk;
					while(1) {
						tk = getNextToken();
						if (tk.tid == ENDMARKER)
							break;

						printf("Line Number: %d  Lexeme: %s  Token Name: %s\n", tk.lineNo, tk.lexeme, getTerminalName(tk.tid));
					}
					fclose(fp);
					lineno = 1;
					break;

			case 3: if(!grammarRead) {
						parserfp = fopen("grammar.txt", "r");
						readGrammar(parserfp);
						fclose(parserfp);
						grammarRead = 1;
					}
					
					computeFirstAndFollowSets();
					initializeParseTree();
					createParseTable();

					fp = fopen(argv[1], "r");
					getStream(fp);
					parseInputSourceCode(argv[1]);
					printParseTree(argv[2]);
					fclose(fp);
					lineno = 1;
					break;

			case 4:	if(!grammarRead) {
						parserfp = fopen("../grammar.txt", "r");
						readGrammar(parserfp);
						fclose(parserfp);
						grammarRead = 1;
					}

					fp = fopen(argv[1], "r");
					clock_t start_time, end_time;
	                double total_CPU_time, total_CPU_time_in_seconds;
	                start_time = clock();

	                // fprintf(stderr, "%p\n", g[0].head);
					getStream(fp);
					computeFirstAndFollowSets();
					initializeParseTree();
					createParseTable();
					parseInputSourceCode(argv[1]);
					if (!syntaxError)
						printParseTree(argv[2]);
					end_time = clock();

	                total_CPU_time  =  (double) (end_time - start_time);
	                total_CPU_time_in_seconds =   total_CPU_time / CLOCKS_PER_SEC;
	                printf("Total CPU time: %lf, Total CPU time in sec: %lf\n", total_CPU_time, total_CPU_time_in_seconds);

					fclose(fp);
					lineno = 1;
					break;

			default: printf("Please enter a valid choice\n");
					 break;
		}

	} while (choice != 0);
}
*/