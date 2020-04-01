#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "symboltableDef.h"
#include "symboltable.h"
#include "ast.h"


idSymbolTable currentIdTable, globalIdTable;
funcSymbolTable funcTable;


// create a new entry in identifier symbol table
symbolTableIdEntry createIdEntry(token id, astnode* type) {
    symbolTableIdEntry entry;
    entry.id = id;
    entry.name = (char*) malloc(sizeof(id.lexeme));
    strcpy(entry.name, id.lexeme);

    if (type->TorNT == 0 && type->children->data.T.tid == ARRAY) {
        entry.AorP = 1;
        entry.type.array.arr = type->data.T;
        entry.type.array.lowerBound = type->children->sibling->children->data.T; // can also be a variable
        entry.type.array.upperBound = type->children->sibling->children->sibling->data.T; // can also be a variable
        entry.type.array.datatype.datatype = type->children->sibling->sibling->children->data.T;

        if (entry.type.array.datatype.datatype.tid == INTEGER)
            entry.type.array.datatype.width = 4;
        else 
            entry.type.array.datatype.width = 8;
    }  

    else {
        entry.AorP = 0;
        entry.type.primitive.datatype = type->children->data.T;

        if (entry.type.primitive.datatype.tid == INTEGER)
            entry.type.primitive.width = 4;
        else 
            entry.type.primitive.width = 8;
    }
    return entry;
}


// create a new entry in function symbol table
symbolTableFuncEntry createFuncEntry(token functionName, parameters* inputParams, parameters* outputParams) {
    symbolTableFuncEntry entry;
    entry.name = (char*) malloc(sizeof(functionName.lexeme));
    strcpy(entry.name, functionName.lexeme);
    entry.id = functionName;
    entry.inputParameters = inputParams;
    entry.outputParameters = outputParams;
    entry.link = NULL; // NEED TO LINK THIS
    return entry;
}


// traverse AST fpr type extraction
void traverseAST(astnode* root) {

    // program -> moduleDeclarations otherModules driverModule otherModules 
    if (root->TorNT == 1 && root->data.NT.ntid == program) {
        currentIdTable = createIdSymbolTable();
        globalIdTable = currentIdTable;
        funcTable = createFuncSymbolTable();

        traverseAST(root->children);
        traverseAST(root->children->sibling);
        traverseAST(root->children->sibling->sibling);
        traverseAST(root->children->sibling->sibling->sibling);
    }

    // moduleDeclarations -> moduleDeclaration moduleDeclarations
    else if (root->TorNT == 1 && root->data.NT.ntid == moduleDeclarations) {
        astnode* tmp = root->children;
        while (tmp) {
            traverseAST(tmp);
            tmp = tmp->sibling;
        }
    }

    // moduleDeclaration -> DECLARE MODULE ID SEMICOL  
    else if (root->TorNT == 1 && root->data.NT.ntid == moduleDeclaration) {
        astnode* id = root->children;
        char* moduleName = id->data.T.lexeme;
        symbolTableFuncEntry* entry = (symbolTableFuncEntry*) malloc(sizeof(symbolTableFuncEntry));
        strcpy(entry->name, moduleName);
        funcTable = insertFunc(funcTable, *entry); 
    }

    // otherModules -> module otherModules
    else if (root->TorNT == 1 && root->data.NT.ntid == otherModules) {
        astnode* tmp = root->children;
        while (tmp) {
            traverseAST(tmp);
            tmp = tmp->sibling;
        }
    }

    // driverModule -> DRIVERDEF DRIVER PROGRAM DRIVERENDDEF moduleDef
    else if (root->TorNT == 1 && root->data.NT.ntid == driverModule) {
        astnode* drivernode = root->children;
        astnode* moduleDefNode = drivernode->sibling;
        symbolTableFuncEntry entry = createFuncEntry(drivernode->data.T, NULL, NULL);
        funcTable = insertFunc(funcTable, entry);
        traverseAST(moduleDefNode);
    }

    // moduleDef -> START statements END
    else if (root->TorNT == 1 && root->data.NT.ntid == moduleDef) {
        traverseAST(root->children);
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
            while (tmp) {
                if (tmp->sibling != NULL)
                    tmp = tmp->sibling;
            }
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

        symbolTableFuncEntry funcEntry = createFuncEntry(identifier->data.T, inputParams, outputParams);
        funcTable = insertFunc(funcTable, funcEntry);
        traverseAST(moduleDefNode);
        currentIdTable = *(currentIdTable.parent);
    }

    // statements -> statement statements
    else if (root->TorNT == 1 && root->data.NT.ntid == statements) {
        astnode* tmp  = root->children;
        while (tmp) {
            traverseAST(tmp);
            tmp = tmp->sibling;
        }
    }

    // declareStmt -> DECLARE idList COLON dataType SEMICOL
    else if (root->TorNT == 1 && root->data.NT.ntid == declareStmt) {
        astnode* ids = root->children;
        astnode* datatypenode = ids->sibling;
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
            while (tmp) {
                if (tmp->sibling != NULL)
                    tmp = tmp->sibling;
            }
            tmp->sibling = &newIdTable;
        }
        
        currentIdTable = newIdTable;

        traverseAST(root->children->sibling);
        traverseAST(root->children->sibling->sibling);
        currentIdTable = *(currentIdTable.parent);
    }

    // caseStmts -> CASE value COLON statements BREAK SEMICOL N9
    else if (root->TorNT == 1 && root->data.NT.ntid == caseStmts) {
        astnode* tmp = root->children;
        while(tmp) {
            tmp = tmp->sibling;
            traverseAST(tmp);
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
            while (tmp) {
                if (tmp->sibling != NULL)
                    tmp = tmp->sibling;
            }
            tmp->sibling = &newIdTable;
        }
        
        currentIdTable = newIdTable;
        traverseAST(root->children->sibling->sibling->sibling);
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
            while (tmp) {
                if (tmp->sibling != NULL)
                    tmp = tmp->sibling;
            }
            tmp->sibling = &newIdTable;
        }
        
        currentIdTable = newIdTable;
        traverseAST(root->children->sibling->sibling);
        currentIdTable = *(currentIdTable.parent);
    }
}