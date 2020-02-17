#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 2048

char buffer[BUFFER_SIZE + 1];

struct Pair {
	int first;
	int second;
};


void removeComments(char *testcaseFile, char *cleanFile) {

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
