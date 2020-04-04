#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"
#include "ast.h"
#include "symboltableDef.h"
#include "symboltable.h"
#include "typeExtractor.h"

int semanticError = 0;
int idFound = 1;

/*
    TO-DO:
        1. Arithmetic expressions - DONE
        2. Check parent scopes for variable - DONE
            (a) Should parent and child be pointers - YES, IT WORKS!
        3. Change link from arrow to dot in function symbol table entries - DONE
        4. Print relevant semantic errors wherever semanticError = 1 -- IN PROGRESS
        5. Arrays in arithmetic expressions
            (a) Dynamic upper and lower bounds? - DONE
            (b) Array dynamic bounds should be checked for integer and not inserted in symbol table
        6. Set semanticError = 2 back to 0 everywhere an arithmetic expression occurs
        7. Ensure for loop variable not updated - DONE
        8. If module defined but not declared etc. - DONE
*/

// semantic rules and type checking by traversing AST
void semanticChecker(astnode* root) {

    if (root->TorNT == 1 && root->data.NT.ntid == program) {
        currentIdTable = globalIdTable;
        astnode* tmp = root->children;
        while (tmp) {
            semanticChecker(tmp);
            tmp = tmp->sibling;
        }
        currentIdTable = globalIdTable;
    }

    else if (root->TorNT == 1 && root->data.NT.ntid == moduleDeclarations) {
        astnode* tmp = root->children;
        while (tmp) {
            semanticChecker(root);
            tmp = tmp->sibling;
        }
    }

    /*
        1. Module redeclaration - line number would be different
    */
    else if (root->TorNT == 1 && root->data.NT.ntid == moduleDeclaration) {
        astnode* funcName = root->children;
        symbolTableFuncEntry* entry = searchFunc(funcTable, funcName->data.T.lexeme);

        // function redeclaration isn't allowed
        if (entry->declarationLineNo != funcName->data.T.lineNo) {
            redColor();
            printf("Semantic Error: ");
            resetColor();
            printf("Function at line %d already declared.\n", funcName->data.T.lineNo);
            semanticError = 1;
        }
    }

    else if (root->TorNT == 1 && root->data.NT.ntid == otherModules) {
        astnode* tmp = root->children;
        while (tmp) {
            semanticChecker(tmp);
            tmp = tmp->sibling;
        }
    }

    else if (root->TorNT == 1 && root->data.NT.ntid == driverModule) {
        currentIdTable = root->scopeTable;
        semanticChecker(root->children->sibling);
        currentIdTable = *(currentIdTable.parent);
    }

    /*
        1. Function redefinition
        2. Output parameters should be updated (if they exist) - see isUpdated AST node variable
    */
    else if (root->TorNT == 1 && root->data.NT.ntid == module) {
        currentIdTable = root->scopeTable;
        astnode* funcName = root->children;

        symbolTableFuncEntry* entry = searchFunc(funcTable, funcName->data.T.lexeme);
        
        // function has already been defined once
        if (entry->definitionLineNo != -1) {
            redColor();
            printf("Semantic Error: ");
            resetColor();
            printf("Function at line %d already defined.\n", funcName->data.T.lineNo);
            semanticError = 1;
        }

        astnode* ret_ast = NULL;
        astnode* moduledef;

        // no return values
        if (root->children->sibling->sibling->TorNT == 1 && root->children->sibling->sibling->data.NT.ntid == moduleDef) 
            moduledef = root->children->sibling->sibling;
        
        // output parameters exist
        else {
            ret_ast = root->children->sibling->sibling;
            moduledef = ret_ast->sibling;
        }

        semanticChecker(moduledef);

        if (ret_ast) {
            astnode* tmp = ret_ast->children;
            while (tmp) {

                // output parameter hasn't been updated
                if (tmp->isUpdated == 0) {
                    redColor();
                    printf("Semantic Error: ");
                    resetColor();
                    printf("Output parameter %s not assigned a value in function defined at line number: %d.\n", tmp->data.T.lexeme, funcName->data.T.lineNo);
                    semanticError = 1;
                }
                tmp = tmp->sibling;
            }
        }

        currentIdTable = *(currentIdTable.parent);
    }

    else if (root->TorNT == 1 && root->data.NT.ntid == moduleDef) {
        semanticChecker(root->children);
    }

    else if (root->TorNT == 1 && root->data.NT.ntid == statements) {
        astnode* tmp = root->children;
        while (tmp) {
            semanticChecker(tmp);
            tmp = tmp->sibling;
        }
    }
    
    /*
        1. Identifier declared
    */
    else if (root->TorNT == 1 && root->data.NT.ntid == ioStmt && root->children->TorNT == 0 && root->children->data.T.tid == GET_VALUE) {
        astnode* id = root->children->sibling;
        semanticChecker(id);
    }

    /*
        1. Identifier declared
        2. Bound check for arrays
    */
    else if (root->TorNT == 1 && root->data.NT.ntid == ioStmt && root->children->TorNT == 0 && root->children->data.T.tid == PRINT) {
        
        // check if array or not
        if (root->TorNT == 0 && root->children->sibling->children->data.T.tid == ID) {
            
            astnode* id_ast = root->children->sibling->children;
            // check if array or not
            if (id_ast->sibling == NULL) {
                
                // identifier
                semanticChecker(id_ast);
                // semanticError = 0;
            }

            else {
                // array
                char* id = id_ast->data.T.lexeme;
                idSymbolTable tmp = currentIdTable;
                symbolTableIdEntry* entry = NULL; 

                while (1) {
                    entry = searchId(tmp, id);
                    
                    // found in the symbol table
                    if (entry != NULL)
                        break;
                    
                    // not found in global symbol table as well
                    if (tmp.parent == NULL)
                        break;
                    tmp = *(tmp.parent);
                }

                if (entry == NULL) {
                    redColor();
                    printf("Semantic Error: ");
                    resetColor();
                    printf("Identifier %s at line %d has not been declared.\n", id, root->data.T.lineNo);
                    semanticError = 1;
                    return;
                }

                // can static type check be done
                if (id_ast->sibling->TorNT == 0 && id_ast->sibling->datatype.tid == INTEGER && entry->AorP == 1 && entry->type.array.dynamicArray == 0) {
                    char* indice = id_ast->sibling->data.T.lexeme; // indice we want to print
                    
                    // check if indice is within range
                    int lb = atoi(entry->type.array.lowerBound.lexeme);
                    int ub = atoi(entry->type.array.upperBound.lexeme);
                    int indexValue = atoi(indice);

                    // bound check
                    if (indexValue < lb || indexValue > ub) {
                        redColor();
                        printf("Type Error: ");
                        resetColor();
                        printf("Index %d out of bounds at line numer: %d.\n", indexValue, id_ast->data.T.lineNo);
                        semanticError = 1;               
                    }

                }

                // index is not an integer
                else if (id_ast->sibling->TorNT == 0 && id_ast->sibling->datatype.tid != INTEGER) {
                    redColor();
                    printf("Semantic Error: ");
                    resetColor();
                    printf("Index is not an integer at line numer: %d.\n", id_ast->data.T.lineNo);
                    semanticError = 1;               
                }
            }
        }
    }

    else if (root->TorNT == 1 && root->data.NT.ntid == simpleStmt) {
        semanticChecker(root->children);
    }

    /*
        1. Type check for LHS and RHS
        2. Can't update loop variable - see loopVariable AST node variable
        3. Bound check
    */
    else if (root->TorNT == 1 && root->data.NT.ntid == assignmentStmt) {
        astnode* assignop = root->children;
        astnode* id = assignop->children;
        
        // not an array
        if (id->sibling->TorNT == 1 && id->sibling->data.NT.ntid == lvalueIDStmt) {
            semanticChecker(id);

            // cannot update loop variable
            if (idFound == 1 && id->loopVariable == 1) {
                redColor();
                printf("Semantic Error: ");
                resetColor();
                printf("Cannot update loop variable at line number: %d.\n", id->data.T.lineNo);
                semanticError = 1;
            }
            
            astnode* expr = id->sibling->children;
            semanticChecker(expr);

            // type mismatch
            if (idFound == 1 && expr->datatype.tid != id->datatype.tid) {
                redColor();
                printf("Type Error: ");
                resetColor();
                printf("Type mismatch at line number: %d.\n", id->data.T.lineNo);
                semanticError = 1;
            }

            idFound = 1;
        }

        // array
        else {
            astnode* idx = id->sibling;
            idSymbolTable tmp = currentIdTable;
            symbolTableIdEntry* entry = NULL; 

            while (1) {
                entry = searchId(tmp, id->data.T.lexeme);
                
                // found in the symbol table
                if (entry != NULL)
                    break;
                
                // not found in global symbol table as well
                if (tmp.parent == NULL)
                    break;
                tmp = *(tmp.parent);
            }

            if (entry == NULL) {
                redColor();
                printf("Semantic Error: ");
                resetColor();
                printf("Identifier %s at line %d has not been declared.\n", id, root->data.T.lineNo);
                semanticError = 1;
                idFound = 1;
                return;
            }
            
            astnode* expr = idx->sibling->children;
            semanticChecker(expr);

            // whether we can perform static bound check
            if (idx->TorNT == 0 && idx->data.T.tid == NUM && entry->AorP == 1 && entry->type.array.dynamicArray == 0) {
                int lb = atoi(entry->type.array.lowerBound.lexeme);
                int ub = (entry->type.array.upperBound.lexeme);
                int indexValue = atoi(idx->data.T.lexeme);

                // bound check
                if (indexValue >= lb && indexValue <= ub) {

                    // type mismatch
                    if (expr->datatype.tid != entry->type.array.datatype.datatype.tid) {
                        redColor();
                        printf("Type Error: ");
                        resetColor();
                        printf("Type mismatch at line number: %d.\n", id->data.T.lineNo);
                        semanticError = 1;
                    }
                }

                // out of bounds
                else {
                    redColor();
                    printf("Semantic Error: ");
                    resetColor();
                    printf("Index %d out of bounds at line number: %d.\n", indexValue, id->data.T.lineNo);
                    semanticError = 1;
                }
            }

            // index is not an integer
            else if (idx->TorNT == 0 && idx->datatype.tid != INTEGER) {
                redColor();
                printf("Semantic Error: ");
                resetColor();
                printf("Index is not an integer at line numer: %d.\n", id->data.T.lineNo);
                semanticError = 1;               
            }
        }

    }

    /* 
        1. Function declared but not defined
        2. If declared, then declaration line number < current function line Number < definition line number
        3. If not declared, should be defined before 
        4. Input parameter type and number check 
        5. Output parameter type and number check
    */
    else if (root->TorNT == 1 && root->data.NT.ntid == moduleReuseStmt) {
        astnode *functionName, *opt;
        if (root->children->TorNT == 1 && root->children->data.NT.ntid == optional) {
            opt = root->children;
            functionName = opt->sibling;
        }

        else {
            opt = NULL;
            functionName = root->children;
        }

        // called function not defined
        symbolTableFuncEntry* entry = searchFunc(funcTable, functionName->data.T.lexeme);
        int currLineNo;
        if (entry == NULL || entry->definitionLineNo == -1) {
            redColor();
            printf("Semantic Error: ");
            resetColor();
            printf("Function %s has not been defined, called at line number: %d.\n", functionName->data.T.lexeme, currLineNo);
            semanticError = 1;
            return;
        }

        // recursive call -- current symbol table is same as the one being called
        if (&currentIdTable == &(entry->link)) {
            redColor();
            printf("Semantic Error: ");
            resetColor();
            printf("Recursion not allowed at line number: %d.\n", currLineNo);
            semanticError = 1;
        }

        // called function not declared - and defined after current function
        else if (entry->declarationLineNo == -1 && entry->definitionLineNo > currLineNo) {
            redColor();
            printf("Semantic Error: ");
            resetColor();
            printf("Function %s has not been defined yet, called at line number: %d.\n", functionName->data.T.lexeme, currLineNo);
            semanticError = 1;
        }

        // declaration and definition both occur before caller function
        else if (entry->declarationLineNo != -1 && entry->definitionLineNo != -1 && entry->declarationLineNo < currLineNo && entry->definitionLineNo < currLineNo) {
            redColor();
            printf("Semantic Error: ");
            resetColor();
            printf("Function %s declaration is redundant at line number: %d.\n", functionName->data.T.lexeme, entry->declarationLineNo);
            semanticError = 1;
        }        

        // match type and number of input parameters
        astnode* idlist = functionName->sibling;
        astnode* idItr = idlist->children;
        parameters* inputItr = entry->inputParameters;
        
        while (idItr) {
            idSymbolTable tmp = currentIdTable;
            symbolTableIdEntry* idEntry = NULL;

            while (1)  {
                idEntry = searchId(tmp, idItr->data.T.lexeme);

                if (idEntry != NULL)
                    break;
                
                // reached global symbol table
                if (tmp.parent == NULL)
                    break;
                tmp = *(tmp.parent);
            }

            // identifier not in any symbol table
            if (idEntry == NULL) {
                redColor();
                printf("Semantic Error: ");
                resetColor();
                printf("Identifier %s has not been declared at line number: %d.\n", idItr->data.T.lexeme, idItr->data.T.lineNo);
                semanticError = 1;
            }

            // data type comparison
            else if (idItr->datatype.tid == inputItr->datatype.tid) {
                idSymbolTable relevantTable = entry->link;
                symbolTableIdEntry* inputEntry = searchId(relevantTable, inputItr->id.lexeme);

                // arrays
                if (idEntry->AorP == 1 && inputEntry->AorP == 1) {

                    // should be of same data type 
                    if (idEntry->type.array.datatype.datatype.tid == inputEntry->type.array.datatype.datatype.tid) {

                        // bound check only for static arrays
                        if (idEntry->type.array.dynamicArray == 0 && inputEntry->type.array.dynamicArray == 0) {

                            // bound mismatch
                            if (atoi(idEntry->type.array.lowerBound.lexeme) != atoi(inputEntry->type.array.lowerBound.lexeme) || atoi(idEntry->type.array.upperBound.lexeme) != atoi(inputEntry->type.array.upperBound.lexeme)) {
                                redColor();
                                printf("Type Error: ");
                                resetColor();
                                printf("Input parameter bound mismatch at line number: %d.\n", idItr->data.T.lineNo);
                                semanticError = 1;
                            }
                        }
                    }
                }

                else if (idEntry->AorP != inputEntry->AorP) {
                    redColor();
                    printf("Type Error: ");
                    resetColor();
                    printf("Input parameter type mismatch at line number: %d.\n", idItr->data.T.lineNo);
                    semanticError = 1;
                }
            }
            
            // number of parameter mismatch
            if ((idItr->sibling == NULL && inputItr->next != NULL) || (idItr->sibling != NULL && inputItr->next == NULL)) {
                redColor();
                printf("Semantic Error: ");
                resetColor();
                printf("Input parameter number mismatch at line number: %d.\n", idItr->data.T.lineNo);
                semanticError = 1;
            }

            else {
                idItr = idItr->sibling;
                inputItr = inputItr->next;
            }
        }

        // parameters used to invoke function call should exist in symbol table (see optional rule of AST)
        if (opt != NULL) {
            astnode* tmp = opt->children;
            while (tmp) {
                semanticChecker(tmp);
                // semanticError = 0;
                tmp = tmp->sibling;
            }
        }
        
        // has return type, but not being assigned or vice-versa
        parameters* outputItr = entry->outputParameters;
        if ((opt == NULL && outputItr != NULL) || (opt != NULL && outputItr == NULL)) {
            redColor();
            printf("Semantic Error: ");
            resetColor();
            printf("Output parameter(s) number mismatch at line number: %d.\n", idItr->data.T.lineNo);
            semanticError = 1;
            return;
        }

        // check type and number of output parameters
        idlist = opt->sibling;
        idItr = idlist->children;
        
        while (idItr->data.T.tid != ASSIGNOP) {
            idSymbolTable tmp = currentIdTable;
            symbolTableIdEntry* idEntry = NULL;

            while (1)  {
                idEntry = searchId(tmp, idItr->data.T.lexeme);

                if (idEntry != NULL)
                    break;
                
                // reached global symbol table
                if (tmp.parent == NULL)
                    break;
                tmp = *(tmp.parent);
            }

            // identifier not in any symbol table 
            if (idEntry == NULL) {
                redColor();
                printf("Semantic Error: ");
                resetColor();
                printf("Identifier %s at line %d has not been declared.\n", idItr->data.T.lexeme, idItr->data.T.lineNo);
                semanticError = 1;
            }

            // data type comparison
            else if (idItr->datatype.tid == outputItr->datatype.tid) {
                idSymbolTable relevantTable = entry->link;
                symbolTableIdEntry* outputEntry = searchId(relevantTable, outputItr->id.lexeme);

                // arrays
                if (idEntry->AorP == 1 && outputEntry->AorP == 1) {

                    // should be of same data type 
                    if (idEntry->type.array.datatype.datatype.tid == outputEntry->type.array.datatype.datatype.tid) {

                        // bound check only for static arrays
                        if (idEntry->type.array.dynamicArray == 0 && outputEntry->type.array.dynamicArray == 0) {

                            // bound mismatch
                            if (atoi(idEntry->type.array.lowerBound.lexeme) != atoi(outputEntry->type.array.lowerBound.lexeme) || atoi(idEntry->type.array.upperBound.lexeme) != atoi(outputEntry->type.array.upperBound.lexeme)) {
                                redColor();
                                printf("Type Error: ");
                                resetColor();
                                printf("Input parameter bound mismatch at line number: %d.\n", idItr->data.T.lineNo);
                                semanticError = 1;
                            }
                        }
                    }
                }

                else if (idEntry->AorP != outputEntry->AorP) {
                    redColor();
                    printf("Type Error: ");
                    resetColor();
                    printf("Input parameter type mismatch at line number: %d.\n", idItr->data.T.lineNo);
                    semanticError = 1;
                }
            }
            
            // number of parameter mismatch
            if ((idItr->sibling->data.T.tid == ASSIGNOP && outputItr->next != NULL) || (idItr->sibling->data.T.tid != ASSIGNOP && outputItr->next == NULL)) {
                redColor();
                printf("Semantic Error: ");
                resetColor();
                printf("Input parameter number mismatch at line number: %d.\n", idItr->data.T.lineNo);
                semanticError = 1;
            }

            else {
                idItr = idItr->sibling;
                outputItr = outputItr->next;
            }
        }

    }

    else if (root->TorNT == 1 && root->data.NT.ntid == expression) {
        astnode* tmp = root->children;

        // traverse new_NT's corresponding AST
        if (tmp->TorNT == 1 && tmp->data.NT.ntid == unary_op) 
            semanticChecker(tmp->sibling);

        // arithmeticOrBooleans's corresponding AST
        else 
            semanticChecker(tmp);
    }

    /*
        1. Addition allowed only for INT, REAL
        2. If one subtree has type error, just return from entire expression
    */
    else if(root->TorNT == 0 && (root->data.T.tid == MUL || root->data.T.tid == DIV || root->data.T.tid == PLUS || root->data.T.tid == MINUS)) {
        astnode* leftchild = root->children;
        astnode* rightchild = leftchild->sibling;

        if (semanticError == 2)
            return;

        // even if ID, NUM, RNUM, it will simply return back with no computation
        semanticChecker(leftchild);
        semanticChecker(rightchild);

        // left and right child AST nodes should now have stored type in the root 
        if (leftchild->datatype.tid == rightchild->datatype.tid && (leftchild->datatype.tid == INTEGER || leftchild->datatype.tid == REAL))
            root->datatype.tid = leftchild->datatype.tid;

        // semantic error - probably skip the entire expression?
        else {
            semanticError = 2;
        }
    }

    /*
        1. Logical operations only allowed on boolean
        2. If one subtree has type error, just return from entire expression
    */
    else if(root->TorNT == 0 && (root->data.T.tid == AND || root->data.T.tid == OR)) {
        astnode* leftchild = root->children;
        astnode* rightchild = leftchild->sibling;

        if (semanticError == 2)
            return;

        // even if ID, NUM, RNUM, it will simply return back with no computation
        semanticChecker(leftchild);
        semanticChecker(rightchild);

        // left and right child AST nodes should now have stored type in the root 
        if (leftchild->datatype.tid == rightchild->datatype.tid && leftchild->datatype.tid == BOOLEAN)
            root->datatype.tid = leftchild->datatype.tid;

        // semantic error - probably skip the entire expression?
        else {
            semanticError = 2;
        }
    }

    /*
        1. Relational operations allowed only for INT, REAL
        2. If one subtree has type error, just return from entire expression
    */
    else if(root->TorNT == 0 && (root->data.T.tid == GT || root->data.T.tid == GE || root->data.T.tid == LT || root->data.T.tid == LE || root->data.T.tid == EQ || root->data.T.tid == NE)) {
        astnode* leftchild = root->children;
        astnode* rightchild = leftchild->sibling;

        if (semanticError == 2)
            return;

        // even if ID, NUM, RNUM, it will simply return back with no computation
        semanticChecker(leftchild);
        semanticChecker(rightchild);

         // left and right child AST nodes should now have stored type in the root 
        if (leftchild->datatype.tid == rightchild->datatype.tid && (leftchild->datatype.tid == INTEGER || leftchild->datatype.tid == REAL))
            root->datatype.tid = leftchild->datatype.tid;

        // semantic error - probably skip the entire expression?
        else {
            semanticError = 2;
        }
    }

    /*
        1. Identifier redeclaration
    */
    else if (root->TorNT == 1 && root->data.NT.ntid == declareStmt) {
        astnode* idlist = root->children;
        astnode* tmp = idlist->children;

        while (tmp) {
            char* id = tmp->data.T.lexeme;
            idSymbolTable itr = currentIdTable;
            symbolTableIdEntry* entry = NULL;
            
            while (1) {
                entry = searchId(itr, id);

                if (entry == NULL) {
                    itr = *(itr.parent);
                    continue;
                }

                // found in symbol table - identifier has already been declared before
                redColor();
				printf("Semantic Error: ");
				resetColor();
				printf("Identifier %s at line %d has already been declared before\n", id, tmp->data.T.lineNo);
                semanticError = 1;
                break;
            }

            tmp = tmp->sibling;
        }
    }

    /* 
        1. Case variable should be declared
        2. Case variable should be integer or boolean
        3. If integer, must contain default statement
        4. If boolean, must not contain default statement
    */
    else if (root->TorNT == 1 && root->data.NT.ntid == conditionalStmt) {
        currentIdTable = root->scopeTable;
        astnode* id = root->children;
        astnode* caseStatements = id->sibling;
        astnode* default_ast = caseStatements->sibling;

        // case variable not found
        semanticChecker(id);
        // semanticError = 0;

        // must contain default statement
        if (id->datatype.tid == INTEGER) {
            caseStatements->datatype.tid == INTEGER;
            if (default_ast == NULL) {
                redColor();
                printf("Semantic Error: ");
                resetColor();
                printf("Must contain default statement at line number: %d\n", id->data.T.lineNo);
                semanticError = 1;
            }
        }

        // must not contain default statement
        else if (id->datatype.tid == BOOLEAN) {
            caseStatements->datatype.tid == BOOLEAN;
            if (default_ast != NULL) {
                redColor();
                printf("Semantic Error: ");
                resetColor();
                printf("Must not contain default statement at line number: %d\n", id->data.T.lineNo);
                semanticError = 1;
            }
        }

        // some other datatype 
        else {
            redColor();
            printf("Semantic Error: ");
            resetColor();
            printf("Invalid datatype - must be integer or boolean at line number: %d\n", id->data.T.lineNo);
            semanticError = 1;
            return;
        }

        semanticChecker(caseStatements);
    }
    
    /*
        1. Each case value variable should be integer
    */
    else if (root->TorNT == 1 && root->data.NT.ntid == caseStmts) {
        currentIdTable = root->scopeTable;
        astnode* tmp = root->children;
        while (tmp) {

            // case value of different type
            if (tmp->datatype.tid != root->datatype.tid) {
                redColor();
                printf("Type Error: ");
                resetColor();
                printf("Case value type mismatch at line number: %d\n", tmp->data.T.lineNo);
                semanticError = 1;
            }
            tmp = tmp->sibling;
            semanticChecker(tmp);
            tmp = tmp->sibling;
        }
        currentIdTable = *(currentIdTable.parent);
    }

    /*
        1. Loop variable should be declared
        2. Loop variable should be integer
        3. Loop variable shouldn't be updated - handled in assignmentStmt
    */
    else if (root->TorNT == 1 && root->data.NT.ntid == iterativeStmt && root->children->TorNT == 0 && root->children->data.T.tid == FOR) {
        currentIdTable = root->scopeTable;
        astnode* id = root->children->sibling;
        
        // identifier not found in symbol table
        semanticChecker(id);
        // semanticError = 0;

        // loop variable isn't integer data type
        if (id->datatype.tid != INTEGER) {
            redColor();
            printf("Type Error: ");
            resetColor();
            printf("Loop variable %s must be an integer at line number: %d.\n", id->data.T.lexeme, id->data.T.lineNo);
            semanticError = 1;
        }


        // identifier in symbol table - can't be updated
        else if (idFound == 1) {
        }

        astnode* stmts = id->sibling->sibling;
        semanticChecker(stmts);
        currentIdTable = *(currentIdTable.parent);
    }

    /*
        1. Expression should be of boolean type
    */
    else if (root->TorNT == 1 && root->data.NT.ntid == iterativeStmt && root->children->TorNT == 0 && root->children->data.T.tid == WHILE) {
        currentIdTable = root->scopeTable;
        astnode* abexpr = root->children->sibling;
        semanticChecker(abexpr);
        
        if (abexpr->datatype.tid != BOOLEAN) {
            redColor();
            printf("Type Error: ");
            resetColor();
            printf("While loop expression must be of boolean data type at line number: %d.\n", id->data.T.lineNo);
            semanticError = 1;
        }

        astnode* stmts = abexpr->sibling;
        semanticChecker(stmts);
        
        // set current table back to outer scope
        currentIdTable = *(currentIdTable.parent);
    }

    /*
        1. Searches for ID in current scope and all parent tables
        2. Error if identifier not found in any table  
    */
    else if (root->TorNT == 0 && root->data.T.tid == ID) {
        char* id = root->data.T.lexeme;
        idSymbolTable tmp = currentIdTable;
        symbolTableIdEntry* entry = NULL;

        while(1) {
             entry = searchId(currentIdTable, id);

            // found in current scope
            if (entry != NULL)
                break;
            
            // if reached global parent
            if (tmp.parent == NULL)
                break;
            
            // setting scope to parent scope
            tmp = *(tmp.parent);
        }

        // not found in any symbol table
        if (entry == NULL) {
            redColor();
            printf("Semantic Error: ");
            resetColor();
            printf("Identifier %s at line %d has not been declared.\n", id, root->data.T.lineNo);
            idFound = 0;
            semanticError = 1;
        }
    }
}