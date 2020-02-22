#ifndef _PARSER
#define _PARSER

// extern const int _NUM_RULES;
// extern const int _NUM_TERMINALS; 
// extern const int _NUM_NONTERMINALS; 

extern FILE* fp;
extern grammar g;

extern int parseTable[][];

extern unsigned long long int firstSet[];
extern unsigned long long int followSet[];
extern unsigned long long int nullSet;

#endif