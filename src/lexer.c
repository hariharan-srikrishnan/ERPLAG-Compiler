#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexerDef.h"
#include "hash.h"

#define BUFFER_SIZE 2048

const int _NUM_KEYWORDS = sizeof(keywords) / sizeof(keywords[0]) ;
const int _MAX_INPUT_FILE_SIZE = 10000;

char buffer[BUFFER_SIZE];
FILE* fp;
int start, current;

// create hash table	
int tableSize = 13;
hashtable* t = createTable(tableSize);


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
	int size = fread(buffer, sizeof(char), BUFFER_SIZE, fp);
	t = insert(t, tableSize, keywords, keywordTokens, _NUM_KEYWORDS);
}


// get lexeme from start to current state
char* getLexeme() {

	char lexeme[25];
	int c = 0;
	int track = start; // avoiding changing start number
	while (track <= current){
		lexeme[c] = buffer[track];
		c++;
		track++;
	}

	lexeme[c] = '\0';
	start = track + 1;
	current = track + 1;
	return lexeme;
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

	if(current == BUFFER_SIZE) {
		int j = 0;
		for(int i = start; i < current; i++) {
			buffer[j] = buffer[i];
			j++;
		}

		int size = fread(buffer + j, sizeof(char), BUFFER_SIZE - j - 1, fp);
		current = j;//current - start - 1;
		start = 0;
	}

	return buffer[current++];
}

char nextchar1() {
	if(current == BUFFER_SIZE) {
		int copy_idx = 0;
		for(int i=start; i<current; i++) {
			buffer[copy_idx++] = buffer[i];
		}
		int copied = fread(buffer + copy_idx, sizeof(char), BUFFER_SIZE - copy_idx, fp);
		current = copy_idx;
		start = 0;
	}
	return buffer[current++];
}


token getNextToken() {
	int state = 0;
	token t;
	int lineno = 1;
	char c;
	int error = -1; //default
	start = 0;
	current = 0;

	while(1) {

		// error handling - print lexeme tokenized so far as well
		switch(error) {

			state = 0;
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
					t.lexeme = getLexeme();
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
					t.lexeme = getLexeme();
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
					t.lexeme = getLexeme();
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
					 t.lexeme = getLexeme();
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

			// keyword or identifier????
			case 13: retract(1);
					 t.tid = RNUM;
					 t.lexeme = getLexeme();
					 t.lineNo = lineno;
					 start = current + 1;
					 current = start;
					 return t;

			// MINUS
			case 14: t.tid = MINUS;
					 t.lexeme = getLexeme();
					 t.lineNo = lineno;
					 start = current + 1;
					 current = start;
					 return t;

			// PLUS
			case 15: t.tid = PLUS;
					 t.lexeme = getLexeme();
					 t.lineNo = lineno;
					 start = current + 1;
					 current = start;
					 return t;

			// DIV
			case 16: t.tid = DIV;
					 t.lexeme = getLexeme();
					 t.lineNo = lineno;
					 start = current + 1;
					 current = start;
					 return t;

			// COMMA
			case 17: t.tid = COMMA;
					 t.lexeme = getLexeme();
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
					 t.lexeme = getLexeme();
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
					 t.lexeme = getLexeme();
					 t.lineNo = lineno;
					 start = current + 1;
					 current = start;
					 return t;

			// opening bracket
			case 22: t.tid = BO;
					 t.lexeme = getLexeme();
					 t.lineNo = lineno;
					 start = current + 1;
					 current = start;
					 return t;

			// closing SQBC
			case 23: t.tid = SQBC;
					 t.lexeme = getLexeme();
					 t.lineNo = lineno;
					 start = current + 1;
					 current = start;
					 return t;

			// opening SQBO
			case 24: t.tid = SQBO;
					 t.lexeme = getLexeme();
					 t.lineNo = lineno;
					 start = current + 1;
					 current = start;
					 return t;

			// semi-colon
			case 25: t.tid = SEMICOL;
					 t.lexeme = getLexeme();
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
			case 27: t.tid = RANEGOP;
					 t.lexeme = getLexeme();
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
					 t.lexeme = getLexeme();
					 t.lineNo = lineno;
					 start = current + 1;
					 current = start;
					 return t;

			// ASSIGNOP
			case 30: t.tid = ASSIGNOP;
					 t.lexeme = getLexeme();
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
					 t.lexeme = getLexeme();
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
					 t.lexeme = getLexeme();
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
					 t.lexeme = getLexeme();
					 t.lineNo = lineno;
					 start = current + 1;
					 current = start;
					 return t;

			// >>
			case 37: t.tid = ENDDEF;
					 t.lexeme = getLexeme();
					 t.lineNo = lineno;
					 start = current + 1;
					 current = start;
					 return t;

			// GE
			case 38: t.tid = GE;
					 t.lexeme = getLexeme();
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
					 t.lexeme = getLexeme();
					 t.lineNo = lineno;
					 start = current + 1;
					 current = start;
					 return t;;

			// <<
			case 41: t.tid = DEF;
					 t.lexeme = getLexeme();
					 t.lineNo = lineno;
					 start = current + 1;
					 current = start;
					 return t;

			// LE
			case 42: t.tid = LE;
					 t.lexeme = getLexeme();
					 t.lineNo = lineno;
					 start = current + 1;
					 current = start;
					 return t;

		}
	}
}

int main() {
	char c;
	fp = fopen("test.txt", "r");
	getStream(fp);
	start = 0; current = 0;

	c = nextchar1();
	putchar(c);
	c = nextchar1();
	putchar(c);
	start = 2; 
	c = nextchar1();
	putchar(c);
	c = nextchar1();
	putchar(c);
	// 
	c = nextchar1();
	putchar(c);
	c = nextchar1();
	putchar(c);
	c = nextchar1();
	putchar(c);
	c = nextchar1();
	putchar(c);
	c = nextchar1();
	putchar(c);
	return 0;
}