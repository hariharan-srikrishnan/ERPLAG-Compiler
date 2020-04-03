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
        2. Check parent scopes for variable - HOW DO YOU COMPARE TWO SYMBOL TABLES??
            (a) Should parent and child be parents
        3. Change link from arrow to dot in function symbol table entries
        4. Print relevant semantic errors wherever semanticError = 1 -- IN PROGRESS
        5. Arrays in arithmetic expressions
        6. Set semanticError = 2 back to 0 everywhere an arithmetic expression occurs -- NOT REALLY, USE idFound
        7. Ensure for loop variable not updated - HOW TO DO FOR NESTED FOR LOOPS?
        8. If module defined but not declared
        9. The parameters being returned by a function must be assigned a value. If a parameter does not get a value assigned within the function definition, it should be reported as an error.
*/

// semantic rules and type checking
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

    else if (root->TorNT == 1 && root->data.NT.ntid == moduleDeclaration) {
        astnode* funcName = root->children;
        symbolTableFuncEntry* entry = searchFunc(funcTable, funcName->data.T.lexeme);

        // function has been declared before
        if (entry != NULL) {
            redColor();
            printf("Semantic Error: ");
            resetColor();
            printf("Function at line %d already declared\n", funcName->data.T.lineNo);
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

    else if (root->TorNT == 1 && root->data.NT.ntid == module) {
        currentIdTable = root->scopeTable;
        astnode* tmp = root->children;
        while (tmp->sibling)
            tmp = tmp->sibling;
        
        semanticChecker(tmp);
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
    
    // check if identifier present in symbol table
    else if (root->TorNT == 1 && root->data.NT.ntid == ioStmt && root->children->TorNT == 0 && root->children->data.T.tid == GET_VALUE) {
        astnode* id = root->children->sibling;
        semanticChecker(id);
        // semanticError = 0;
    }

    // check if variable, if an identifier, present in symbol table
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
                symbolTableIdEntry* entry = searchId(currentIdTable, id);

                if (entry == NULL) {
                    semanticError = 1;
                }
                
                char* indice = id_ast->sibling->data.T.lexeme; // indice we want to print
                
                // check if indice is within range
                int lb = entry->type.array.lowerBound;
                int ub = entry->type.array.upperBound;
                int indexValue = atoi(indice);

                // bound check
                if (indexValue < lb || indexValue > ub) 
                    semanticError = 1;
            }
        }
    }

    else if (root->TorNT == 1 && root->data.NT.ntid == simpleStmt) {
        semanticChecker(root->children);
    }

    // TO DO: if semantic error from expression?
    else if (root->TorNT == 1 && root->data.NT.ntid == assignmentStmt) {
        astnode* assignop = root->children;
        astnode* id = assignop->children;
        
        // not an array
        if (id->sibling->TorNT == 1 && id->sibling->data.NT.ntid == lvalueIDStmt) {
            semanticChecker(id);

            astnode* expr = id->sibling->children;
            semanticChecker(expr);

            // type mismatch
            if (expr->datatype.tid != id->datatype.tid) {
                semanticError = 1;
            }
        }

        // array
        else {
            astnode* idx = id->sibling;
            symbolTableIdEntry* entry = searchId(currentIdTable, id->data.T.lexeme);

            if (entry == NULL) {
                semanticError = 1;
                return;
            }
            
            astnode* expr = idx->sibling->children;
            semanticChecker(expr);

            // whether we can perform static bound check
            if (idx->TorNT == 0 && idx->data.T.tid == NUM && entry->AorP == 1 && entry->type.array.dynamicArray == 0) {
                int lb = entry->type.array.lowerBound;
                int ub = entry->type.array.upperBound;
                int indexValue = atoi(idx->data.T.lexeme);

                // bound check
                if (indexValue >= lb && indexValue <= ub) {

                    // type mismatch
                    if (expr->datatype.tid != entry->type.array.datatype.datatype.tid) {
                        semanticError = 1;
                    }
                }

                // out of bounds
                else {
                    semanticError = 1;
                }
            }
        }

    }

    // match the types and the number of parameters returned by a function must be the same as that of the parameters used in invoking the function.
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

        // check if function has been defined/declared
        symbolTableFuncEntry* entry = searchFunc(funcTable, functionName->data.T.lexeme);
        if (entry == NULL) {
            semanticError = 1;
            return;
        }

        // recursive call -- current symbol table is same as the one being called
        if (&currentIdTable == entry->link) {
            redColor();
            printf("Semantic Error: ");
            resetColor();
            printf("Recursion not allowed at line number: .\n", functionName->data.T.lineNo);
            semanticError = 1;
        }

        // match type and number of input parameters
        astnode* idlist = functionName->sibling;
        astnode* idItr = idlist->children;
        parameters* inputItr = entry->inputParameters;
        
        while (idItr) {
            symbolTableIdEntry* idEntry = searchId(currentIdTable, idItr->data.T.lexeme);

            // identifier not in symbol table -- CHECK FOR IDENTIFIER IN PARENT TABLES ALSO
            if (idEntry == NULL) {
                redColor();
                printf("Semantic Error: ");
                resetColor();
                printf("Identifier %s at line %d has not been declared.\n", idItr->data.T.lexeme, idItr->data.T.lineNo);
                semanticError = 1;
            }

            // data type comparison
            else if (idItr->datatype.tid == inputItr->datatype.tid) {
                idSymbolTable relevantTable = *(entry->link);
                symbolTableIdEntry* inputEntry = searchId(relevantTable, inputItr->id.lexeme);

                // arrays
                if (idEntry->AorP == 1 && inputEntry->AorP == 1) {

                    // should be of same data type 
                    if (idEntry->type.array.datatype.datatype.tid == inputEntry->type.array.datatype.datatype.tid) {

                        // bound check only for static arrays
                        if (idEntry->type.array.dynamicArray == 0 && inputEntry->type.array.dynamicArray == 0) {

                            // bound mismatch
                            if (idEntry->type.array.lowerBound != inputEntry->type.array.lowerBound || idEntry->type.array.upperBound != inputEntry->type.array.upperBound) {
                                redColor();
                                printf("Semantic Error: ");
                                resetColor();
                                printf("Input parameter bound mismatch at line number: %d.\n", idItr->data.T.lineNo);
                                semanticError = 1;
                            }
                        }
                    }
                }

                else if (idEntry->AorP != inputEntry->AorP) {
                    redColor();
                    printf("Semantic Error: ");
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
            symbolTableIdEntry* idEntry = searchId(currentIdTable, idItr->data.T.lexeme);

            // identifier not in symbol table -- CHECK FOR IDENTIFIER IN PARENT TABLES ALSO
            if (idEntry == NULL) {
                redColor();
                printf("Semantic Error: ");
                resetColor();
                printf("Identifier %s at line %d has not been declared.\n", idItr->data.T.lexeme, idItr->data.T.lineNo);
                semanticError = 1;
            }

            // data type comparison
            else if (idItr->datatype.tid == outputItr->datatype.tid) {
                idSymbolTable relevantTable = *(entry->link);
                symbolTableIdEntry* outputEntry = searchId(relevantTable, outputItr->id.lexeme);

                // arrays
                if (idEntry->AorP == 1 && outputEntry->AorP == 1) {

                    // should be of same data type 
                    if (idEntry->type.array.datatype.datatype.tid == outputEntry->type.array.datatype.datatype.tid) {

                        // bound check only for static arrays
                        if (idEntry->type.array.dynamicArray == 0 && outputEntry->type.array.dynamicArray == 0) {

                            // bound mismatch
                            if (idEntry->type.array.lowerBound != outputEntry->type.array.lowerBound || idEntry->type.array.upperBound != outputEntry->type.array.upperBound) {
                                redColor();
                                printf("Semantic Error: ");
                                resetColor();
                                printf("Input parameter bound mismatch at line number: %d.\n", idItr->data.T.lineNo);
                                semanticError = 1;
                            }
                        }
                    }
                }

                else if (idEntry->AorP != outputEntry->AorP) {
                    redColor();
                    printf("Semantic Error: ");
                    resetColor();
                    printf("Input parameter type mismatch at line number: %d.\n", idItr->data.T.lineNo);
                    semanticError = 1;
                }
            }
            
            // number of parameter mismatch
            if ((idItr->sibling->data.T.tid == ASSIGNOP && outputItr->next != NULL) || (idItr->sibling->data.T.tid == ASSIGNOP && outputItr->next == NULL)) {
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
        else {
            semanticChecker(tmp);
            // semanticError = 0;
        }
    }

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

    else if (root->TorNT == 1 && root->data.NT.ntid == declareStmt) {
        astnode* idlist = root->children;
        astnode* tmp = idlist->children;

        while (tmp) {
            char* id = tmp->data.T.lexeme;
            symbolTableIdEntry* entry = searchId(currentIdTable, id);
            
            // found in symbol table - identifier has already been declared before
            if (entry != NULL) {
                redColor();
				printf("Semantic Error: ");
				resetColor();
				printf("Identifier %s at line %d has already been declared before\n", id, tmp->data.T.lineNo);
                semanticError = 1;
            }

            tmp = tmp->sibling;
        }
    }

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

    else if (root->TorNT == 1 && root->data.NT.ntid == caseStmts) {
        currentIdTable = root->scopeTable;
        astnode* tmp = root->children;
        while (tmp) {

            // case value of different type
            if (tmp->datatype.tid != root->datatype.tid) {
                redColor();
                printf("Semantic Error: ");
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

    else if (root->TorNT == 1 && root->data.NT.ntid == iterativeStmt && root->children->TorNT == 0 && root->children->data.T.tid == FOR) {
        currentIdTable = root->scopeTable;
        astnode* id = root->children->sibling;
        
        // identifier not found in symbol table
        semanticChecker(id);
        // semanticError = 0;

        // loop variable isn't integer data type
        if (id->datatype.tid != INTEGER) {
            redColor();
            printf("Semantic Error: ");
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

    else if (root->TorNT == 1 && root->data.NT.ntid == iterativeStmt && root->children->TorNT == 0 && root->children->data.T.tid == WHILE) {
        currentIdTable = root->scopeTable;
        astnode* abexpr = root->children->sibling;
        semanticChecker(abexpr);
        // semanticError = 0;

        astnode* stmts = abexpr->sibling;
        semanticChecker(stmts);
        currentIdTable = *(currentIdTable.parent);
    }

    else if (root->TorNT == 0 && root->data.T.tid == ID) {
        char* id = root->data.T.lexeme;
        idSymbolTable tmp = currentIdTable;
        symbolTableIdEntry* entry = NULL;

        while(&tmp != &globalIdTable) {
             entry = searchId(currentIdTable, id);

            // not found in current scope
            if (entry != NULL)
                break;
            
            tmp = tmp.parent;
        }

        // check in global symbol table now 
        if (entry == NULL) {
            entry = searchId(globalIdTable, id);

            // undeclared identifier
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
}