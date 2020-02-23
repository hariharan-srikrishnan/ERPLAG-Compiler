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


	if (argc != 3) {
		printf("Number of arguments don't match\n");
		exit(0);
	}

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
					break;

			case 3: parserfp = fopen("../grammar.txt", "r");
					readGrammar(parserfp);
					fclose(parserfp);
					computeFirstAndFollowSets();
					initializeParseTree();
					createParseTable();

					fp = fopen(argv[1], "r");
					getStream(fp);
					parseInputSourceCode(argv[1]);
					printParseTree(argv[2]);
					fclose(fp);
					break;

			case 4: parserfp = fopen("../grammar.txt", "r");
					fp = fopen(argv[1], "r");
					clock_t start_time, end_time;
	                double total_CPU_time, total_CPU_time_in_seconds;
	                start_time = clock();

					readGrammar(parserfp);
					computeFirstAndFollowSets();
					initializeParseTree();
					createParseTable();
					getStream(fp);
					parseInputSourceCode(argv[1]);
					printParseTree(argv[2]);
					end_time = clock();

	                total_CPU_time  =  (double) (end_time - start_time);
	                total_CPU_time_in_seconds =   total_CPU_time / CLOCKS_PER_SEC;
	                printf("Total CPU time: %lf, Total CPU time in sec: %lf\n", total_CPU_time, total_CPU_time_in_seconds);

					fclose(parserfp);
					fclose(fp);
					break;

		}

	} while (choice != 0);
}
