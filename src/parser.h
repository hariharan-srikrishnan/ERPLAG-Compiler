#ifndef _PARSER
#define _PARSER

#include "parserutils.h"

// extern const int _NUM_RULES;
// extern const int _NUM_TERMINALS; 
// extern const int _NUM_NONTERMINALS; 

extern FILE* parserfp;
extern grammar g;

extern int** parseTable;
extern t_node* parseTreeRoot;


extern unsigned long long int firstSet[];
extern unsigned long long int followSet[];
extern unsigned long long int nullSet;


static inline unsigned long long int setUnion(unsigned long long int a, unsigned long long int b);

static inline unsigned long long int setIntersection(unsigned long long int a, unsigned long long int b);

void readGrammar(FILE* fp);

void computeFirstSets();

void printFirstSet(FILE* f);

void computeFollowSets();

void printFollowSet(FILE* f);

void computeFirstAndFollowSets();

void createParseTable();

void printParseTable(FILE* outfile);

void initializeParseTree();

void parseInputSourceCode(char* filename);

void printTreeNode(t_node* node, FILE* output);

void printParseTreeUtil(t_node* node, FILE* output);

void printParseTree(char* outfile);


#endif