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
#include "codegen.h"


int main (int argc, char* argv[]) {
    parserfp = fopen("../grammar.txt", "r");
    readGrammar(parserfp);
    fclose(parserfp);

    fp = fopen(argv[1], "r");
    getStream(fp);
    computeFirstAndFollowSets();
    initializeParseTree();
    createParseTable();
    parseInputSourceCode(argv[1]);
    fclose(fp);

    if (!syntaxError) {
        createAST(parseTreeRoot);
        // printAST(parseTreeRoot->syn);
        extractTypeAST(parseTreeRoot->syn);
        // traverseSymbolTable(globalIdTable);
        // printFunctionTable(funcTable);
        semanticChecker(parseTreeRoot->syn);

        if (!semanticError) {
            asmFile = fopen(argv[2], "w");
            generateASM(parseTreeRoot->syn);
            fclose(asmFile);
        }
    }
}
