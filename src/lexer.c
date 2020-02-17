#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 2048

const int _NUM_KEYWORDS = 3;
const int _MAX_INPUT_FILE_SIZE = 10000;

char buffer[BUFFER_SIZE];
FILE* fp;
int start, current;


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
}


// get lexeme from start to current state
char* getLexeme() {

	char* lexeme = (char*) malloc((current - start + 1) * sizeof(char));
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


// check whether a lexeme is a keyword
char isKeyword(char *lexeme) {
	
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


TOKEN getNextToken() {
	int state = 0;
	int lineno = 1;
	char c;
	int error = 0;
	start = 0;
	current = 0;

	while(1) {

		switch(state) {

			// error handling code HERE

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
						error = 1;

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
					return token;

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
					return token;

			case 6: c = nextchar();
					if (c == '+' || c == '-')
						state = 7;

					else if (c >= '0' && c <= '9')
						state = 10;

					else
						error = 1;

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
					return token;

			case 10: c = nextchar();
					 if (c >= '0' && c <= '9')
					 	state = 10;

					 else 
					 	state = 11;

					 break;

			case 11: retract(1);
					 return token;

			case 12: c = nextchar();
					 if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')
					 	state = 12;

					 else 
					 	state = 13;

					 break;

			case 13: retract(1);
					 return token;

			// MINUS
			case 14: return token;

			// PLUS
			case 15: return token;

			// DIV
			case 16: return token;

			// COMMA
			case 17: return token;

			// ASTERISK
			case 18: c = nextchar();
					 if (c == '*') 
					 	state = 20;
					 
					 else 
					 	state = 19;

					 break;

			// MUL
			case 19: retract(1);
					 return token;

			// COMMENT
			case 20: // handle comments appropriately

			// Closing Bracket
			case 21: return token;

			// opening bracket
			case 22: return token;

			// closing SQBC
			case 23: return token;

			// opening SQBC
			case 24: return token;

			// semi-colon
			case 25: return token;

			// dot
			case 26: c = nextchar();
					 if (c == '.')
					 	state = 27;

					 else
					 	error = 1;

					 break;

			// dot dot
			case 27: return token;

			// colon
			case 28: c = nextchar();
					 if (c == '=')
					 	state = 30;

					 else
					 	state = 29;

					 break;

			case 29: retract(1);
					 return token;

			// ASSIGNOP
			case 30: return token;

			// equal
			case 31: c = nextchar();
					 if (c == '=')
					 	state = 32;

					 else 
					 	error = 1;

					 break;

			// == 
			case 32: return token;

			// exclamation
			case 33: c = nextchar();
					 if (c == '=')
					 	state = 34;

					 else 
					 	error = 1;

					 break;

			// inequality
			case 34: return token;

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
					 return token;

			// >>
			case 37: return token;

			// GE
			case 38: return token;

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
					 return token();

			// <<
			case 41: return token;

			// LE
			case 42: return token;

		}
	}
}

int main() {
	char c;
	FILE* fp = fopen("test.txt", "r");
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