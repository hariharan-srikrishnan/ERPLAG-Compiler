/*
Anirudh S Chakravarthy - 2017A7PS1195P
Hariharan Srikrishnan  - 2017A7PS0134P
Honnesh Rohmetra	   - 2016B2A70770P
Praveen Ravirathinam   - 2017A7PS1174P
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "lexerDef.h"
#include "lexer.h"

int main(){

	fp = fopen("t1.txt","r");
	token tk;

	clock_t start_time, end_time;
    double total_CPU_time, total_CPU_time_in_seconds;
    start_time = clock();

	getStream(fp);
	
	while(1){

		tk = getNextToken();
		if(tk.tid == ENDMARKER){
			//while breaking condition
			break;
		}

		printf("Token Lexeme: %s  Token ID: %d  Token Line Number: %d\n", tk.lexeme, tk.tid, tk.lineNo);
		// printf("Token Lexeme: %s  Token ID: %d  Token Line Number: %d\n", tk.lexeme, tk.tid, tk.lineNo);

	}

	end_time = clock();

	total_CPU_time = (double) (end_time - start_time);
    total_CPU_time_in_seconds = total_CPU_time / CLOCKS_PER_SEC;
    printf("Total CPU time: %lf, Total CPU time in sec: %lf\n", total_CPU_time, total_CPU_time_in_seconds);
	fclose(fp);
}