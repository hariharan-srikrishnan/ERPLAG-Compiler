#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexerDef.h"
#include "lexer.h"

int main(){

	fp = fopen("tokentest_1.txt","r");
	getStream(fp);

	token tk;
	
	while(1){

		tk = getNextToken();
		if(tk.tid == ENDMARKER){
			//while breaking condition
			break;
		}

		printf("Token Lexeme: %s  Token ID: %d  Token Line Number: %d\n", tk.lexeme, tk.tid, tk.lineNo);
		// printf("Token Lexeme: %s  Token ID: %d  Token Line Number: %d\n", tk.lexeme, tk.tid, tk.lineNo);

	}

	fclose(fp);
}