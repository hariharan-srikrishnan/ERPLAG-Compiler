#ifndef _PARSER
#define _PARSER

extern const int _NUM_RULES;
extern const int _NUM_TERMINALS; 
extern const int _NUM_NONTERMINALS; 

extern FILE* fp;
extern grammar g;

extern int parseTable[_NUM_NONTERMINALS][_NUM_TERMINALS];

#endif