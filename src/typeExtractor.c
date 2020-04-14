
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "symboltableDef.h"
#include "symboltable.h"

void traverseSymbolTable(idSymbolTable* table);


idSymbolTable *currentIdTable, *globalIdTable;
funcSymbolTable funcTable;
int currentOffset = 0;


// create a new entry in identifier symbol table
symbolTableIdEntry createIdEntry(token id, astnode* type) {
    symbolTableIdEntry entry;
    entry.id = id;
    strcpy(entry.name, id.lexeme);

    if (type->TorNT == 0 && type->data.T.tid == ARRAY) {
        entry.AorP = 1;
        entry.type.array.arr = type->data.T;
        entry.type.array.dynamicArray = 0;
        entry.type.array.lowerBound = type->sibling->children->data.T;
        entry.type.array.upperBound = type->sibling->children->sibling->data.T;

        // dynamic array
        if (type->sibling->children->data.T.tid == ID || type->sibling->children->sibling->data.T.tid == ID) 
            entry.type.array.dynamicArray = 8;
        
        entry.type.array.datatype.datatype = type->sibling->sibling->children->data.T;
        entry.type.array.datatype.width = 8;

        // offset only assigned for static arrays
        if (entry.type.array.dynamicArray == 0) {
            entry.offset = currentOffset;

            // new offset calculation
            int lb = atoi(entry.type.array.lowerBound.lexeme);
            int ub = atoi(entry.type.array.upperBound.lexeme);
            currentOffset += (ub - lb - 1) * entry.type.array.datatype.width;
        }
    }  

    else {
        entry.AorP = 0;
        entry.type.primitive.datatype = type->data.T;
        entry.type.primitive.width = 8;
        
        // offset calculation
        entry.offset = currentOffset;
        currentOffset += entry.type.primitive.width;
    }
    return entry;
}


// create a new entry in function symbol table
symbolTableFuncEntry createFuncEntry(token functionName, parameters* inputParams, int numInput, parameters* outputParams, int numOutput) {
    symbolTableFuncEntry entry;
    strcpy(entry.name, functionName.lexeme);
    entry.id = functionName;
    entry.inputParameters = inputParams;
    entry.numInputParams = numInput;
    entry.outputParameters = outputParams;
    entry.numOutputParams = numOutput;
    entry.declarationLineNo = -1;
    entry.definitionLineNo = -1;
    // entry.link = NULL; // NEED TO LINK THIS
    return entry;
}


// link two symbol tables
void linkTables (idSymbolTable* currentTable, idSymbolTable* newTable) {
    if (currentTable == newTable)
        return;
    
    newTable->parent = currentTable;
    if (currentTable->child == NULL) 
        currentTable->child = newTable;

    else {
        idSymbolTable* tmp = currentTable->child;
        while (tmp->sibling)
            tmp = tmp->sibling;
        tmp->sibling = newTable;
    }
}


// traverse AST for type extraction
void extractTypeAST(astnode* root) {

    if (root == NULL)
        return;

    // program -> moduleDeclarations otherModules driverModule otherModules 
    if (root->TorNT == 1 && root->data.NT.ntid == program) {
        idSymbolTable* firstTable = createIdSymbolTable();
        currentIdTable = firstTable;
        globalIdTable = currentIdTable;
        funcTable = createFuncSymbolTable();

        astnode* tmp = root->children;
        while (tmp) {
            extractTypeAST(tmp);
            tmp = tmp->sibling;
        }
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
        symbolTableFuncEntry entry = createFuncEntry(id->data.T, NULL, 0, NULL, 0);
        
        // module hasn't been declared yet
        if (entry.declarationLineNo == -1)
            entry.declarationLineNo = id->data.T.lineNo;

        // if it is declared, insert won't insert anything and flag an error in semantics
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
        
        // create a new symbol table
        idSymbolTable* newIdTable = createIdSymbolTable();
        linkTables(currentIdTable, newIdTable);
        currentIdTable = newIdTable;
        root->scopeTable = *currentIdTable;
        currentOffset = 0;
        
        astnode* drivernode = root->children;
        astnode* moduleDefNode = drivernode->sibling;
        symbolTableFuncEntry entry = createFuncEntry(drivernode->data.T, NULL, 0, NULL, 0);
        entry.link = *currentIdTable;
        entry.definitionLineNo = root->children->data.T.lineNo;
        funcTable = insertFunc(funcTable, entry);
        
        extractTypeAST(moduleDefNode);
        currentIdTable = currentIdTable->parent;
    }

    // moduleDef -> START statements END
    else if (root->TorNT == 1 && root->data.NT.ntid == moduleDef) {
        extractTypeAST(root->children);
    }

    // module -> DEF MODULE ID ENDDEF TAKES INPUT SQBO input_plist SQBC SEMICOL ret moduleDef
    else if (root->TorNT == 1 && root->data.NT.ntid == module) {
        
        // create a new symbol table
        idSymbolTable* newIdTable = createIdSymbolTable();
        linkTables(currentIdTable, newIdTable);
        currentIdTable = newIdTable;
        root->scopeTable = *currentIdTable;
        currentOffset = 0;

        // add input parameters to identifier table and function parameters to function table
        astnode* identifier = root->children;
        astnode* inputList = identifier->sibling;
        astnode* tmp = inputList->children;
        astnode* datatypenode;
        int numInputParams = 1;
        parameters* inputParams = (parameters*) malloc(sizeof(parameters));

        inputParams->id = tmp->data.T;
        tmp = tmp->sibling; // to datatype node
        datatypenode = tmp->children;
        inputParams->datatype = datatypenode->data.T;
        tmp = tmp->sibling;
        parameters* prev = inputParams;

        symbolTableIdEntry idEntry = createIdEntry(inputParams->id, datatypenode);
        *currentIdTable = insertId(*currentIdTable, idEntry);

        while (tmp) {
            parameters* curr = (parameters*) malloc(sizeof(parameters));
            curr->id = tmp->data.T;
            tmp = tmp->sibling; // to datatype node
            datatypenode = tmp->children;
            curr->datatype = datatypenode->data.T;
            prev->next = curr;
            prev = curr;
            tmp = tmp->sibling;
            numInputParams++;

            idEntry = createIdEntry(curr->id, datatypenode);
            *currentIdTable = insertId(*currentIdTable, idEntry);
        }

        // output paramaters aren't NULL -- i.e > 1
        astnode* outputList = inputList->sibling; 
        astnode* moduleDefNode = outputList;
        parameters* outputParams;
        int numOutputParams = 0;
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
            numOutputParams++;

            idEntry = createIdEntry(outputParams->id, typenode);
            *currentIdTable = insertId(*currentIdTable, idEntry);

            while (tmp) {
                parameters* curr = (parameters*) malloc(sizeof(parameters));
                curr->id = tmp->data.T;
                tmp = tmp->sibling; // to datatype node
                typenode = tmp->children;
                curr->datatype = tmp->data.T;
                prev->next = curr;
                prev = curr;
                tmp = tmp->sibling;
                numOutputParams++;

                idEntry = createIdEntry(curr->id, typenode);
                *currentIdTable = insertId(*currentIdTable, idEntry);
            }
        }

        // if module has already been declared before
        symbolTableFuncEntry* existingEntry = searchFunc(funcTable, identifier->data.T.lexeme);
        if (existingEntry != NULL && existingEntry->inputParameters == NULL && existingEntry->outputParameters == NULL) {
            existingEntry->inputParameters = inputParams;
            existingEntry->outputParameters = outputParams;
            existingEntry->id = identifier->data.T;
            existingEntry->link = *currentIdTable;
            existingEntry->numInputParams = numInputParams;
            existingEntry->numOutputParams = numOutputParams;
            existingEntry->definitionLineNo = identifier->data.T.lineNo;
        }

        // module hasn't been declared before
        else if (existingEntry == NULL) {
            symbolTableFuncEntry funcEntry = createFuncEntry(identifier->data.T, inputParams, numInputParams, outputParams, numOutputParams);
            funcEntry.link = *currentIdTable;
            funcEntry.definitionLineNo = identifier->data.T.lineNo;
            funcTable = insertFunc(funcTable, funcEntry);
        }

        extractTypeAST(moduleDefNode);
        currentIdTable = currentIdTable->parent;
        // return currentIdTable;
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
            symbolTableIdEntry* entry = searchId(*currentIdTable, tmp->data.T.lexeme);

            // identifier has not been declared before in current scope
            if (entry == NULL) {
                symbolTableIdEntry newEntry = createIdEntry(tmp->data.T, datatypenode);
                tmp->datatype = datatypenode->data.T;
                *currentIdTable = insertId(*currentIdTable, newEntry);
            }

            // redeclaration
            else {
                tmp->isRedeclared = 1;
            }

            tmp = tmp->sibling;
        }
    }

    // conditionalStmt -> SWITCH BO ID BC START caseStmts default END  
    else if (root->TorNT == 1 && root->data.NT.ntid == conditionalStmt) {

        // create a new symbol table
        idSymbolTable* newIdTable = createIdSymbolTable();
        linkTables(currentIdTable, newIdTable);
        currentIdTable = newIdTable;
        root->scopeTable = *currentIdTable;

        extractTypeAST(root->children->sibling);
        extractTypeAST(root->children->sibling->sibling);
        currentIdTable = currentIdTable->parent;
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
        idSymbolTable* newIdTable = createIdSymbolTable();
        linkTables(currentIdTable, newIdTable);
        currentIdTable = newIdTable;
        root->scopeTable = *currentIdTable;

        extractTypeAST(root->children->sibling->sibling->sibling);
        currentIdTable = currentIdTable->parent;
    }

    // iterativeStmt -> WHILE BO arithmeticOrBooleanExpr BC START statements END
    else if (root->TorNT == 1 && root->data.NT.ntid == iterativeStmt && root->children->TorNT == 1 && root->children->data.NT.ntid == WHILE) {
       
        // create a new symbol table
        idSymbolTable* newIdTable = createIdSymbolTable();
        linkTables(currentIdTable, newIdTable);
        currentIdTable = newIdTable;
        root->scopeTable = *currentIdTable;

        extractTypeAST(root->children->sibling->sibling);
        currentIdTable = currentIdTable->parent;
    }
}


// traverse current and all children symbol tables
void traverseSymbolTable(idSymbolTable* curr) {
    if (curr == NULL)
        return;

    for (int i = 0; i < curr->hashSize; i++) {
        idNode* tmp = curr->list[i].head;
        while (tmp) {
            printf("\t%s %s\n", tmp->entry.name, tmp->entry.type.primitive.datatype.lexeme);
            tmp = tmp->next;
        }
    }
    idSymbolTable* tmp = curr->child;
    while (tmp) {
        printf("\n");
        traverseSymbolTable(tmp);
        tmp = tmp->sibling;
    }
}


// print function symbol table
void printFunctionTable(funcSymbolTable table) {
    for(int i = 0; i < table.hashSize; i++) {
        funcNode* tmp = table.list[i].head;
        while (tmp) {
            printf("%s Dec: %d Def: %d\n", tmp->entry.name, tmp->entry.declarationLineNo, tmp->entry.definitionLineNo);
            tmp = tmp->next;
        }
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
//     traverseSymbolTable(globalIdTable);
//     printFunctionTable(funcTable);
// }