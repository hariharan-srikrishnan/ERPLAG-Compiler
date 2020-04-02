#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "symboltableDef.h"
#include "symboltable.h"

// for testing
// #include "lexer.h"
// #include "parser.h"
// #include "parserutils.h"


idSymbolTable currentIdTable, globalIdTable;
funcSymbolTable funcTable;


// create a new entry in identifier symbol table
symbolTableIdEntry createIdEntry(token id, astnode* type) {
    symbolTableIdEntry entry;
    entry.id = id;
    strcpy(entry.name, id.lexeme);

    if (type->TorNT == 0 && type->data.T.tid == ARRAY) {
        entry.AorP = 1;
        entry.type.array.arr = type->data.T;
        entry.type.array.lowerBound = type->sibling->children->data.T; // can also be a variable
        entry.type.array.upperBound = type->sibling->children->sibling->data.T; // can also be a variable
        entry.type.array.datatype.datatype = type->sibling->sibling->children->data.T;

        if (entry.type.array.datatype.datatype.tid == INTEGER)
            entry.type.array.datatype.width = 4;

        else if (entry.type.array.datatype.datatype.tid == REAL)
            entry.type.array.datatype.width = 8;
        
        else 
            entry.type.array.datatype.width = 1;
    }  

    else {
        entry.AorP = 0;
        entry.type.primitive.datatype = type->data.T;

        if (entry.type.primitive.datatype.tid == INTEGER)
            entry.type.primitive.width = 4;
        
        else if (entry.type.primitive.datatype.tid == REAL)
            entry.type.primitive.width = 8;
        
        else 
            entry.type.primitive.width = 1;
    }
    return entry;
}


// create a new entry in function symbol table
symbolTableFuncEntry createFuncEntry(token functionName, parameters* inputParams, parameters* outputParams) {
    symbolTableFuncEntry entry;
    strcpy(entry.name, functionName.lexeme);
    entry.id = functionName;
    entry.inputParameters = inputParams;
    entry.outputParameters = outputParams;
    entry.link = NULL; // NEED TO LINK THIS
    return entry;
}


// traverse AST fpr type extraction
void extractTypeAST(astnode* root) {

    if (root == NULL)
        return;

    // program -> moduleDeclarations otherModules driverModule otherModules 
    if (root->TorNT == 1 && root->data.NT.ntid == program) {
        currentIdTable = createIdSymbolTable();
        globalIdTable = currentIdTable;
        funcTable = createFuncSymbolTable();

        astnode* tmp = root->children;
        while (tmp) {
            extractTypeAST(tmp);
            tmp = tmp->sibling;
        }

        // extractTypeAST(root->children);
        // extractTypeAST(root->children->sibling);
        // extractTypeAST(root->children->sibling->sibling);
        // extractTypeAST(root->children->sibling->sibling->sibling);
    }

    // moduleDeclarations -> moduleDeclaration moduleDeclarations
    else if (root->TorNT == 1 && root->data.NT.ntid == moduleDeclarations) {
        astnode* tmp = root->children;
        while (tmp) {
            extractTypeAST(tmp);
            tmp = tmp->sibling;
        }
    }

    // moduleDeclaration -> DECLARE MODULE ID SEMICOL  
    else if (root->TorNT == 1 && root->data.NT.ntid == moduleDeclaration) {
        astnode* id = root->children;
        symbolTableFuncEntry entry = createFuncEntry(id->data.T, NULL, NULL);
        funcTable = insertFunc(funcTable, entry); 
    }

    // otherModules -> module otherModules
    else if (root->TorNT == 1 && root->data.NT.ntid == otherModules) {
        astnode* tmp = root->children;
        while (tmp) {
            extractTypeAST(tmp);
            tmp = tmp->sibling;
        }
    }

    // driverModule -> DRIVERDEF DRIVER PROGRAM DRIVERENDDEF moduleDef
    else if (root->TorNT == 1 && root->data.NT.ntid == driverModule) {
        astnode* drivernode = root->children;
        astnode* moduleDefNode = drivernode->sibling;
        symbolTableFuncEntry entry = createFuncEntry(drivernode->data.T, NULL, NULL);
        funcTable = insertFunc(funcTable, entry);
        extractTypeAST(moduleDefNode);
    }

    // moduleDef -> START statements END
    else if (root->TorNT == 1 && root->data.NT.ntid == moduleDef) {
        extractTypeAST(root->children);
    }

    // module -> DEF MODULE ID ENDDEF TAKES INPUT SQBO input_plist SQBC SEMICOL ret moduleDef
    else if (root->TorNT == 1 && root->data.NT.ntid == module) {
        
        // create a new symbol table
        idSymbolTable newIdTable = createIdSymbolTable();
        newIdTable.parent = &currentIdTable;
        if (currentIdTable.child == NULL) 
            currentIdTable.child = &newIdTable;
        
        else {
            idSymbolTable* tmp = currentIdTable.child;
            while (tmp->sibling) 
                tmp = tmp->sibling;
            tmp->sibling = &newIdTable;
        }
        
        currentIdTable = newIdTable;

        // add input parameters to identifier table and function parameters to function table
        astnode* identifier = root->children;
        astnode* inputList = identifier->sibling;
        astnode* tmp = inputList->children;
        astnode* datatypenode;
        parameters* inputParams = (parameters*) malloc(sizeof(parameters));

        inputParams->id = tmp->data.T;
        tmp = tmp->sibling; // to datatype node
        datatypenode = tmp->children;
        inputParams->datatype = datatypenode->data.T;
        tmp = tmp->sibling;
        parameters* prev = inputParams;

        symbolTableIdEntry idEntry = createIdEntry(inputParams->id, datatypenode);
        currentIdTable = insertId(currentIdTable, idEntry);

        while (tmp) {
            parameters* curr = (parameters*) malloc(sizeof(parameters));
            curr->id = tmp->data.T;
            tmp = tmp->sibling; // to datatype node
            datatypenode = tmp->children;
            curr->datatype = datatypenode->data.T;
            prev->next = curr;
            prev = curr;
            tmp = tmp->sibling;

            idEntry = createIdEntry(inputParams->id, datatypenode);
            currentIdTable = insertId(currentIdTable, idEntry);
        }

        // output paramaters aren't NULL -- i.e > 1
        astnode* outputList = inputList->sibling; 
        astnode* moduleDefNode = outputList;
        parameters* outputParams;
        prev = NULL;

        if (outputList->TorNT == 1 && outputList->data.NT.ntid == output_plist) {
            moduleDefNode = outputList->sibling;
            tmp = outputList->children;
            outputParams = (parameters*) malloc(sizeof(parameters));
            outputParams->id = tmp->data.T;
            tmp = tmp->sibling;
            astnode* typenode = tmp->children;
            outputParams->datatype = typenode->data.T;
            tmp = tmp->sibling;
            prev = outputParams;

            while (tmp) {
                parameters* curr = (parameters*) malloc(sizeof(parameters));
                curr->id = tmp->data.T;
                tmp = tmp->sibling; // to datatype node
                curr->datatype = tmp->data.T;
                prev->next = curr;
                prev = curr;
                tmp = tmp->sibling;
            }
        }

        // if module has already been declared before
        symbolTableFuncEntry* existingEntry = searchFunc(funcTable, identifier->data.T.lexeme);
        if (existingEntry != NULL && existingEntry->inputParameters == NULL && existingEntry->outputParameters == NULL) {
            existingEntry->inputParameters = inputParams;
            existingEntry->outputParameters = outputParams;
            existingEntry->id = identifier->data.T;
            existingEntry->link = &currentIdTable;
        }

        // module hasn't been declared before
        else if (existingEntry == NULL) {
            symbolTableFuncEntry funcEntry = createFuncEntry(identifier->data.T, inputParams, outputParams);
            funcEntry.link = &currentIdTable;
            funcTable = insertFunc(funcTable, funcEntry);
        }

        extractTypeAST(moduleDefNode);
        currentIdTable = *(currentIdTable.parent);
    }

    // statements -> statement statements
    else if (root->TorNT == 1 && root->data.NT.ntid == statements) {
        astnode* tmp  = root->children;
        while (tmp) {
            extractTypeAST(tmp);
            tmp = tmp->sibling;
        }
    }

    // declareStmt -> DECLARE idList COLON dataType SEMICOL
    else if (root->TorNT == 1 && root->data.NT.ntid == declareStmt) {
        astnode* ids = root->children;
        astnode* datatypenode = ids->sibling->children;
        astnode* tmp = ids->children;

        while (tmp) {
            symbolTableIdEntry newEntry = createIdEntry(tmp->data.T, datatypenode);
            currentIdTable = insertId(currentIdTable, newEntry);
            tmp = tmp->sibling;
        }
    }

    // conditionalStmt -> SWITCH BO ID BC START caseStmts default END  
    else if (root->TorNT == 1 && root->data.NT.ntid == conditionalStmt) {

        // create a new symbol table
        idSymbolTable newIdTable = createIdSymbolTable();
        newIdTable.parent = &currentIdTable;
        if (currentIdTable.child == NULL) 
            currentIdTable.child = &newIdTable;
        
        else {
            idSymbolTable* tmp = currentIdTable.child;
            while (tmp->sibling) 
                tmp = tmp->sibling;
            tmp->sibling = &newIdTable;
        }
        
        currentIdTable = newIdTable;

        extractTypeAST(root->children->sibling);
        extractTypeAST(root->children->sibling->sibling);
        currentIdTable = *(currentIdTable.parent);
    }

    // caseStmts -> CASE value COLON statements BREAK SEMICOL N9
    else if (root->TorNT == 1 && root->data.NT.ntid == caseStmts) {
        astnode* tmp = root->children;
        while(tmp) {
            tmp = tmp->sibling;
            extractTypeAST(tmp);
            tmp = tmp->sibling;
        }
    }

    // iterativeStmt -> FOR BO ID IN range BC START statements END
    else if (root->TorNT == 1 && root->data.NT.ntid == iterativeStmt && root->children->TorNT == 1 && root->children->data.NT.ntid == FOR) {

        // create a new symbol table
        idSymbolTable newIdTable = createIdSymbolTable();
        newIdTable.parent = &currentIdTable;
        if (currentIdTable.child == NULL) 
            currentIdTable.child = &newIdTable;
        
        else {
            idSymbolTable* tmp = currentIdTable.child;
            while (tmp->sibling) 
                tmp = tmp->sibling;
            tmp->sibling = &newIdTable;
        }
        
        currentIdTable = newIdTable;
        extractTypeAST(root->children->sibling->sibling->sibling);
        currentIdTable = *(currentIdTable.parent);
    }

    // iterativeStmt -> WHILE BO arithmeticOrBooleanExpr BC START statements END
    else if (root->TorNT == 1 && root->data.NT.ntid == iterativeStmt && root->children->TorNT == 1 && root->children->data.NT.ntid == WHILE) {
       
        // create a new symbol table
        idSymbolTable newIdTable = createIdSymbolTable();
        newIdTable.parent = &currentIdTable;
        if (currentIdTable.child == NULL) 
            currentIdTable.child = &newIdTable;
        
        else {
            idSymbolTable* tmp = currentIdTable.child;
            while (tmp->sibling) 
                tmp = tmp->sibling;
            tmp->sibling = &newIdTable;
        }
        
        currentIdTable = newIdTable;
        extractTypeAST(root->children->sibling->sibling);
        currentIdTable = *(currentIdTable.parent);
    }
}


// int main (int argc, char* argv[]) {
//     parserfp = fopen("grammar.txt", "r");
//     readGrammar(parserfp);
//     fclose(parserfp);

//     computeFirstAndFollowSets();
//     initializeParseTree();
//     createParseTable();

//     fp = fopen(argv[1], "r");
//     if (fp == NULL)
//         printf("NULL");
//     getStream(fp);
//     parseInputSourceCode(argv[1]);
//     printParseTree(argv[2]);
//     fclose(fp);

//     createAST(parseTreeRoot);
//     printAST(parseTreeRoot->syn);
//     extractTypeAST(parseTreeRoot->syn);
// }