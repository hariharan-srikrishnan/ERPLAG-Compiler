#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 2048

char buffer[BUFFER_SIZE + 1];

const int _NUM_KEYWORDS = 3;
const int _MAX_INPUT_FILE_SIZE = 10000;

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


// returns updated file pointer after seek
FILE* getStream(FILE* fp) {

	if (fp == NULL) {
		printf("Can't open file!");
		exit(0);
	}

	// read from file into buffer, and terminate it by EOF
	int size = fread(buffer, sizeof(char), BUFFER_SIZE, fp);
	buffer[BUFFER_SIZE] = EOF;
	return fp;
}
