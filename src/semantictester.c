// for testing
#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"
#include "parser.h"
#include "parserutils.h"
#include "ast.h"
#include "symboltableDef.h"
#include "symboltable.h"
#include "typeExtractor.h"
#include "semantics.h"


// Args: input text file, parseTree output text file
int main (int argc, char* argv[]) {
    parserfp = fopen("grammar.txt", "r");
    readGrammar(parserfp);
    fclose(parserfp);

    computeFirstAndFollowSets();
    initializeParseTree();
    createParseTable();

    fp = fopen(argv[1], "r");
    if (fp == NULL)
        printf("NULL");
    getStream(fp);
    parseInputSourceCode(argv[1]);
    printParseTree(argv[2]);
    fclose(fp);

    if (syntaxError == 0) {
        createAST(parseTreeRoot);
        // printAST(parseTreeRoot->syn);
        extractTypeAST(parseTreeRoot->syn);
        // traverseSymbolTable(globalIdTable);
        // printFunctionTable(funcTable);
        printf("\nSemantic Checks: \n");
        semanticChecker(parseTreeRoot->syn);
    }
}
