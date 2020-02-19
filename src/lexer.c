#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexerDef.h"
#include "hash.h"

#define BUFFER_SIZE 2048

const int _NUM_KEYWORDS = sizeof(keywords) / sizeof(keywords[0]) ;
const int _MAX_INPUT_FILE_SIZE = 10000;

char buffer1[BUFFER_SIZE];
char buffer2[BUFFER_SIZE];
int readBuffer1, readBuffer2;
char *start, *current; // current points to the location we will read the next character from
char lexeme[25];
int lineno = 1;

FILE* fp;

// create hash table	
int tableSize = 13;
hashtable* t;


// map integers (token id) to token for aesthetics
char* reverseMap(int tid) {
	int num_tokens = sizeof(keywordTokens) / sizeof(keywordTokens[0]);
	for(int i = 0 ; i < num_tokens; i++) {
		if (keywordTokens[i] == tid) {
			char* str = (char*) malloc(sizeof(char) * 25);
			for(int j = 0; j < sizeof(keywords[i]); j++) 
				str[j] = toupper(keywords[i][j]);
			
			return str;
		}
	}
}


void removeComments(char *testcaseFile, char *cleanFile) {
	// ASSUMPTION : input source code file does not exceed _MAX_INPUT_FILE_SIZE characters
	FILE *input, *output;
	input = fopen(testcaseFile, "r");
	if(input == NULL) {
		fprintf(stderr, "Unable to open input file.\n");
		exit(0);
	}
	char buffer[_MAX_INPUT_FILE_SIZE];
	output = fopen(cleanFile, "w");
	fread(buffer, _MAX_INPUT_FILE_SIZE, 1, input);

	// removing comments from buffer
	int open = 0;
	int n = strlen(buffer);
	char cleanedFile[_MAX_INPUT_FILE_SIZE];
	int j = 0;
	for(int i=0; i<n-1; i++) {
		if(buffer[i] == '*' && buffer[i+1] == '*') {
			open = 1-open;
			i = i+1;
		}
		else {
			if(open && buffer[i] != '\n')
				continue;
			else {
				cleanedFile[j++] = buffer[i];
			}
		}
	}
	fprintf(output, "%s", cleanedFile);
}


// updates file pointer after seek
void getStream(FILE* fp) {

	if (fp == NULL) {
		printf("Can't open file!\n");
		exit(0);
	}

	// read from file into buffer, and terminate it by EOF
	int size = fread(buffer1, sizeof(char), BUFFER_SIZE, fp);
	t = createTable(tableSize);
	t = insert(t, tableSize, keywords, keywordTokens, _NUM_KEYWORDS);
}


// get lexeme from start to current state
void getLexeme() {

	int loc = 0;
	char* temp;
	if(start == current) {
		lexeme[loc++] = '\0';
		return;
	}
	temp = start;
	// Case 1: start in buffer 1 and current in buffer 1
	if((buffer1 <= start) && (start < buffer1 + BUFFER_SIZE) && (buffer1 <= current) && (current < buffer1 + BUFFER_SIZE)) {
		while(temp != current) {
			lexeme[loc++] = *temp;
			temp++;
		}
	}

	// Case 2: start in buffer 1 and current in buffer 2
	else if((buffer1 <= start) && (start < buffer1 + BUFFER_SIZE) && (buffer2 <= current) && (current < buffer2 + BUFFER_SIZE)) {
		while(temp != buffer1 + BUFFER_SIZE) {
			lexeme[loc++] = *temp;
			temp++;
		}
		temp = buffer2;
		while(temp != current) {
			lexeme[loc++] = *temp;
			temp++;
		}
	}

	// Case 3: start in buffer 2 and current in buffer 1
	else if((buffer2 <= start) && (start < buffer2 + BUFFER_SIZE) && (buffer1 <= current) && (current < buffer1 + BUFFER_SIZE)) {
		while(temp != buffer2 + BUFFER_SIZE) {
			lexeme[loc++] = *temp;
			temp++;
		}
		temp = buffer1;
		while(temp != current) {
			lexeme[loc++] = *temp;
			temp++;
		}
	}

	// Case 4: start in buffer 2 and current in buffer 2
	else {
		while(temp != current) {
			lexeme[loc++] = *temp;
			temp++;
		}

	}

	// Finally case
	start = current;
	lexeme[loc++] = '\0';
}


// check whether a lexeme is a keyword or an identifier
int isKeyword(char *lexeme) {
	int hashValue = hashFunction(lexeme, tableSize);
	node* tmp = t->head[hashValue];

	while(tmp != NULL) {
		if (strcmp(tmp->lexeme, lexeme) == 0) 
			return tmp->token;
		tmp = tmp->next;
	}

	return ID;
}


// read next character from input buffer
char nextchar() {

	// If reading from buffer 1
	if(readBuffer1) {
		// Check for overflow
		if(current >= buffer1 + BUFFER_SIZE) {
			// overflow condition, read block into buffer 2 and set current = buffer2
			
			int sz = fread(buffer2, sizeof(char), BUFFER_SIZE, fp);
			
			current = buffer2;
			readBuffer1 = 0;
			readBuffer2 = 1;

		}
		
	}

	// If reading from buffer 2

	else if(readBuffer2) {
		// Check for overflow
		if(current >= buffer2 + BUFFER_SIZE) {
			// overflow condition
			
			int sz = fread(buffer1, sizeof(char), BUFFER_SIZE, fp);
			
			current = buffer1;
			readBuffer1 = 1;
			readBuffer2 = 0;
		}
	}


	// Default case:
	
	char ret = *current;
	current++;
	return ret;
}


// retracts the lookahead (current) pointer by n characters
void retract(int n) {

	// Case 1: current is in buffer 1, and current-n is also in buffer 1
	if((buffer1 <= current) && (current < buffer1+BUFFER_SIZE) && (buffer1 <= current-n) && (current-n < buffer1+BUFFER_SIZE)) {
		current = current - n;
	}

	// Case 2: current is in buffer 1, but current-n is not in buffer 1
	else if( (buffer1 <= current) && (current < buffer1+BUFFER_SIZE) && !( (buffer1 <= current-n) && (current-n < buffer1+BUFFER_SIZE) ) ) {
		int left = n - (int)(current - buffer1);
		current = (buffer2 + BUFFER_SIZE) - left;
	}

	// Case 3: current is in buffer 2, and current-n is also in buffer 2
	else if( (buffer2 <= current) && (current < buffer2+BUFFER_SIZE) && (buffer2 <= current-n) && (current-n < buffer2+BUFFER_SIZE) ) {
		current = current - n;
	}

	// Case 4: current is in buffer 2, and current-n is not in buffer 2
	else {
		int left = n - (int)(current - buffer2);
		current = (buffer1 + BUFFER_SIZE) - left;
	}
}

// retrieves the next token from the input file
token getNextToken() {
	int state = 0;
	token t;
	char c;
	int error = -1; //default
	// start = 0;
	// current = 0;

	while(1) {

		// error handling - print lexeme tokenized so far as well
		switch(error) {

			// update start, current

			case 0:
				printf("Invalid character at line number: %d", lineno);
				break;

			case 1:
				printf("Expected a number at line number: %d", lineno);
				break;

			case 2:
				printf("Expected number or + or - at line number: %d", lineno);
				break;

			case 3:
				printf("Expected a . at line number: %d", lineno);
				break;

			case 4:
				printf("Expected a = at line number: %d", lineno);
				break;

		}

		if(error >= 0) {
			state = 0;
			error = -1;
		}

		switch(state) {

			case 0: c = nextchar();
					if (c == ' ' || c == '\t')
						state = 0;

					else if (c == '\n')
						lineno++;

					else if (c >= '0' && c <= '9')
						state = 1;

					else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
						state = 12;

					else if (c == '-')
						state = 14;

					else if (c == '+')
						state = 15;

					else if (c == '/')
						state = 16;

					else if (c == ',')
						state = 17;

					else if (c == '*')
						state = 18;

					else if (c == ')')
						state = 21;

					else if (c == '(')
						state = 22;

					else if (c == ']')
						state = 23;

					else if (c == '[')
						state = 24;

					else if (c == ';')
						state = 25;

					else if (c == '.')
						state = 26;

					else if (c == ':')
						state = 28;

					else if (c == '=')
						state = 31;

					else if (c == '!')
						state = 33;

					else if (c == '>')
						state = 35;

					else if (c == '<')
						state = 39;

					else
						error = 0;

					break;

			case 1: c = nextchar();
					if (c == '.')
						state = 3;

					else if (c >= '0' && c <= '9')
						state = 1;

					else 
						state = 2;

					break;

			case 2: retract(1);
					t.tid = NUM;
					strcpy(t.lexeme, lexeme);
					t.lineNo = lineno;
					start = current + 1;
					current = start;
					return t;

			case 3: c = nextchar();
					if (c >= '0' && c <= '9')
						state = 4;

					else
						error = 1;

					break;

			case 4: c = nextchar();
					if (c >= '0' && c <= '9')
						state = 4;

					else if (c == 'e' || c == 'E') 
						state = 6;

					else
						state = 5;

					break;

			case 5: retract(1);
					t.tid = RNUM;
					strcpy(t.lexeme, lexeme);
					t.lineNo = lineno;
					start = current + 1;
					current = start;
					return t;

			case 6: c = nextchar();
					if (c == '+' || c == '-')
						state = 7;

					else if (c >= '0' && c <= '9')
						state = 10;

					else
						error = 2;

					break;

			case 7: c = nextchar();
					if (c >= '0' && c <= '9')
						state = 8;

					else
						error = 1;

					break;

			case 8: c = nextchar();
					if (c >= '0' && c <= '9')
						state = 8;

					else 
						state = 9;

					break;

			case 9: retract(1);
					t.tid = RNUM;
					strcpy(t.lexeme, lexeme);
					t.lineNo = lineno;
					start = current + 1;
					current = start;
					return t;

			case 10: c = nextchar();
					 if (c >= '0' && c <= '9')
					 	state = 10;

					 else 
					 	state = 11;

					 break;

			case 11: retract(1);
					 t.tid = RNUM;
					 strcpy(t.lexeme, lexeme);
					 t.lineNo = lineno;
					 start = current + 1;
					 current = start;
					 return t;

			case 12: c = nextchar();
					 if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')
					 	state = 12;

					 else 
					 	state = 13;

					 break;

			// keyword or identifier?
			case 13: retract(1);
					 t.tid = RNUM;
					 strcpy(t.lexeme, lexeme);
					 t.lineNo = lineno;
					 start = current + 1;
					 current = start;
					 return t;

			// MINUS
			case 14: t.tid = MINUS;
					 strcpy(t.lexeme, lexeme);
					 t.lineNo = lineno;
					 start = current + 1;
					 current = start;
					 return t;

			// PLUS
			case 15: t.tid = PLUS;
					 strcpy(t.lexeme, lexeme);
					 t.lineNo = lineno;
					 start = current + 1;
					 current = start;
					 return t;

			// DIV
			case 16: t.tid = DIV;
					 strcpy(t.lexeme, lexeme);
					 t.lineNo = lineno;
					 start = current + 1;
					 current = start;
					 return t;

			// COMMA
			case 17: t.tid = COMMA;
					 strcpy(t.lexeme, lexeme);
					 t.lineNo = lineno;
					 start = current + 1;
					 current = start;
					 return t;

			// ASTERISK
			case 18: c = nextchar();
					 if (c == '*') 
					 	state = 20;
					 
					 else 
					 	state = 19;

					 break;

			// MUL
			case 19: retract(1);
					 t.tid = MUL;
					 strcpy(t.lexeme, lexeme);
					 t.lineNo = lineno;
					 start = current + 1;
					 current = start;
					 return t;

			// COMMENTS - do not tokenize
			case 20: while(1) {
					 	if (c == '*') {
					 		char d = nextchar();
					 		if (d == '*') {
					 			break;
					 		}
					 	}
					 }

					 state = 0;
					 break;

			// Closing Bracket
			case 21: t.tid = BC;
					 strcpy(t.lexeme, lexeme);
					 t.lineNo = lineno;
					 start = current + 1;
					 current = start;
					 return t;

			// opening bracket
			case 22: t.tid = BO;
					 strcpy(t.lexeme, lexeme);
					 t.lineNo = lineno;
					 start = current + 1;
					 current = start;
					 return t;

			// closing SQBC
			case 23: t.tid = SQBC;
					 strcpy(t.lexeme, lexeme);
					 t.lineNo = lineno;
					 start = current + 1;
					 current = start;
					 return t;

			// opening SQBO
			case 24: t.tid = SQBO;
					 strcpy(t.lexeme, lexeme);
					 t.lineNo = lineno;
					 start = current + 1;
					 current = start;
					 return t;

			// semi-colon
			case 25: t.tid = SEMICOL;
					 strcpy(t.lexeme, lexeme);
					 t.lineNo = lineno;
					 start = current + 1;
					 current = start;
					 return t;

			// dot
			case 26: c = nextchar();
					 if (c == '.')
					 	state = 27;

					 else
					 	error = 3;

					 break;

			// dot dot
			case 27: t.tid = RANGEOP;
					 strcpy(t.lexeme, lexeme);
					 t.lineNo = lineno;
					 start = current + 1;
					 current = start;
					 return t;

			// colon
			case 28: c = nextchar();
					 if (c == '=')
					 	state = 30;

					 else
					 	state = 29;

					 break;

			case 29: retract(1);
					 t.tid = COLON;
					 strcpy(t.lexeme, lexeme);
					 t.lineNo = lineno;
					 start = current + 1;
					 current = start;
					 return t;

			// ASSIGNOP
			case 30: t.tid = ASSIGNOP;
					 strcpy(t.lexeme, lexeme);
					 t.lineNo = lineno;
					 start = current + 1;
					 current = start;
					 return t;

			// equal
			case 31: c = nextchar();
					 if (c == '=')
					 	state = 32;

					 else 
					 	error = 4;

					 break;

			// == 
			case 32: t.tid = EQ;
					 strcpy(t.lexeme, lexeme);
					 t.lineNo = lineno;
					 start = current + 1;
					 current = start;
					 return t;

			// exclamation
			case 33: c = nextchar();
					 if (c == '=')
					 	state = 34;

					 else 
					 	error = 4;

					 break;

			// inequality
			case 34: t.tid = NE;
					 strcpy(t.lexeme, lexeme);
					 t.lineNo = lineno;
					 start = current + 1;
					 current = start;
					 return t;

			// >
			case 35: c = nextchar();
					 if (c == '>')
					 	state = 37;

					 else if (c == '=')
					 	state = 38;

					 else 
					 	state = 36;

					 break;

			// GT
			case 36: retract(1);
					 t.tid = GT;
					 strcpy(t.lexeme, lexeme);
					 t.lineNo = lineno;
					 start = current + 1;
					 current = start;
					 return t;

			// >>
			case 37: t.tid = ENDDEF;
					 strcpy(t.lexeme, lexeme);
					 t.lineNo = lineno;
					 start = current + 1;
					 current = start;
					 return t;

			// GE
			case 38: t.tid = GE;
					 strcpy(t.lexeme, lexeme);
					 t.lineNo = lineno;
					 start = current + 1;
					 current = start;
					 return t;

			// <
			case 39: c = nextchar();
					 if (c == '<')
					 	state = 41;

					 else if (c == '=')
					 	state = 42;

					 else 
					 	state = 40;

					 break;

			// LT
			case 40: retract(1);
					 t.tid = LT;
					 strcpy(t.lexeme, lexeme);
					 t.lineNo = lineno;
					 start = current + 1;
					 current = start;
					 return t;

			// <<
			case 41: t.tid = DEF;
					 strcpy(t.lexeme, lexeme);
					 t.lineNo = lineno;
					 start = current + 1;
					 current = start;
					 return t;

			// LE
			case 42: t.tid = LE;
					 strcpy(t.lexeme, lexeme);
					 t.lineNo = lineno;
					 start = current + 1;
					 current = start;
					 return t;

		}
	}
}
