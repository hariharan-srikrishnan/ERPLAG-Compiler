#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"
#include "ast.h"
#include "symboltableDef.h"
#include "symboltable.h"
#include "typeExtractor.h"

int semanticError = 0;
int idFound = 1;


// type and number check for function parameters
void matchParameters(symbolTableFuncEntry* entry, parameters* param, astnode* idlist) {
    astnode* idItr = idlist->children;
    parameters* inputItr = param;

    while (idItr) {
        idSymbolTable* tmp = currentIdTable;
        symbolTableIdEntry* idEntry = NULL;

        idSymbolTable relevantTable = entry->link;
        symbolTableIdEntry* inputEntry = searchId(relevantTable, inputItr->id.lexeme);

        while (1) {
            idEntry = searchId(*tmp, idItr->data.T.lexeme);
            if (idEntry != NULL)
                break;
            
            // not found in global symbol table as well
            if (tmp->parent == NULL)
                break;
            tmp = tmp->parent;
        }

        // identifier not in any symbol table
        if (idEntry == NULL) {
            redColor();
            printf("Semantic Error: ");
            resetColor();
            printf("Identifier %s at line number %d has not been declared.\n", idItr->data.T.lexeme, idItr->data.T.lineNo);
            semanticError = 1;
        }

        // both arrays
        else if (idEntry->AorP == 1 && inputEntry->AorP == 1) {
                
            // should be of same data type 
            if (idEntry->type.array.datatype.datatype.tid == inputEntry->type.array.datatype.datatype.tid) {

                // bound check only for static arrays
                if (idEntry->type.array.dynamicArray == 0 && inputEntry->type.array.dynamicArray == 0) {

                    // bound mismatch
                    if (atoi(idEntry->type.array.lowerBound.lexeme) != atoi(inputEntry->type.array.lowerBound.lexeme) || atoi(idEntry->type.array.upperBound.lexeme) != atoi(inputEntry->type.array.upperBound.lexeme)) {
                        redColor();
                        printf("Type Error: ");
                        resetColor();
                        printf("Input parameter %s bound mismatch at line number: %d.\n", idItr->data.T.lexeme, idItr->data.T.lineNo);
                        semanticError = 1;
                    }
                }
            }
        }

       // one is array and one is not
       else if (idEntry->AorP != inputEntry->AorP) {
            redColor();
            printf("Type Error: ");
            resetColor();
            printf("Input parameter %s type mismatch at line number: %d.\n", idItr->data.T.lexeme, idItr->data.T.lineNo);
            semanticError = 1;
       }

       // primitive and type mismatch
       else if (idEntry->type.primitive.datatype.tid != inputEntry->type.primitive.datatype.tid) {
            redColor();
            printf("Type Error: ");
            resetColor();
            printf("Input parameter %s type mismatch at line number: %d.\n", idItr->data.T.lexeme, idItr->data.T.lineNo);
            semanticError = 1;
       }

        // number of parameter mismatch
        if ((idItr->sibling == NULL && inputItr->next != NULL) || (idItr->sibling != NULL && inputItr->next == NULL)) {
            redColor();
            printf("Semantic Error: ");
            resetColor();
            printf("Input parameter number mismatch at line number: %d.\n", idItr->data.T.lineNo);
            semanticError = 1;
            return;
        }

        else {
            idItr = idItr->sibling;
            inputItr = inputItr->next;
        }
    }
}


// semantic rules and type checking by traversing AST
void semanticChecker(astnode* root) {

    if (root == NULL)
        return;

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
            semanticChecker(tmp);
            tmp = tmp->sibling;
        }
        //printf("Completed module declarations\n");
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
        //printf("In driver module\n");
        currentIdTable = &(root->scopeTable);
        semanticChecker(root->children->sibling);
        currentIdTable = currentIdTable->parent;
        //printf("Completed driver module\n");
    }

    /*
        1. Function redefinition
        2. Output parameters should be updated (if they exist) - see isUpdated symbol table node variable
    */
    else if (root->TorNT == 1 && root->data.NT.ntid == module) {
        currentIdTable = &(root->scopeTable);
        astnode* funcName = root->children;

        symbolTableFuncEntry* entry = searchFunc(funcTable, funcName->data.T.lexeme);
        
        // function has already been defined once
        if (entry->definitionLineNo != funcName->data.T.lineNo) {
            redColor();
            printf("Semantic Error: ");
            resetColor();
            printf("Function at line %d already defined, at line number: %d.\n", funcName->data.T.lineNo, entry->definitionLineNo);
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
            astnode* itr = ret_ast->children;
            while (itr) {

                idSymbolTable* tmp = currentIdTable;
                symbolTableIdEntry* entry = NULL; 

                while (1) {
                    entry = searchId(*tmp, itr->data.T.lexeme);
                    
                    // found in the symbol table
                    if (entry != NULL)
                        break;
                    
                    // not found in global symbol table as well
                    if (tmp->parent == NULL)
                        break;
                    tmp = tmp->parent;
                }

                if (entry == NULL) {
                    redColor();
                    printf("Semantic Error: ");
                    resetColor();
                    printf("Identifier %s at line number %d has not been declared.\n", itr->data.T.lexeme, root->data.T.lineNo);
                    semanticError = 1;
                    idFound = 1;
                }

                else 
                    itr->entry = entry;

                // output parameter hasn't been updated
                if (entry->isUpdated == 0) {
                    redColor();
                    printf("Semantic Error: ");
                    resetColor();
                    printf("Output parameter %s not assigned a value in function defined at line number: %d.\n", itr->data.T.lexeme, funcName->data.T.lineNo);
                    semanticError = 1;
                }

                // next node is a type node in output_plist
                itr = itr->sibling->sibling;
            }
        }

        currentIdTable = currentIdTable->parent;
    }

    else if (root->TorNT == 1 && root->data.NT.ntid == moduleDef) {
        //printf("In moduleDef\n");
        semanticChecker(root->children);
        //printf("Completed moduleDef\n");
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
        // printf("In get value\n");
        astnode* id = root->children->sibling;
        idSymbolTable* tmp = currentIdTable;
        symbolTableIdEntry* entry = NULL;

        while(1) {
             entry = searchId(*tmp, id->data.T.lexeme);

            // found in current scope
            if (entry != NULL)
                break;
            
            // if reached global parent
            if (tmp->parent == NULL)
                break;
            
            // setting scope to parent scope
            tmp = tmp->parent;
        }

        // not found in any symbol table
            if (entry == NULL) {
                redColor();
                printf("Semantic Error: ");
                resetColor();
                printf("Identifier %s at line number %d has not been declared.\n", id->data.T.lexeme, id->data.T.lineNo);
                semanticError = 1;
                return;
            }

            entry->isUpdated = 1;
            id->entry = entry;
    }

    /*
        1. Identifier declared
        2. Bound check for arrays
    */
    else if (root->TorNT == 1 && root->data.NT.ntid == ioStmt && root->children->TorNT == 0 && root->children->data.T.tid == PRINT) {
        
        // check if array or not
        if (root->TorNT == 0 && root->children->sibling->children->data.T.tid == ID) {
            
            astnode* id_ast = root->children->sibling->children;
            // not an array 
            if (id_ast->sibling == NULL) {
                
                // identifier
                semanticChecker(id_ast);
                // semanticError = 0;
            }

            else {
                // array
                char* id = id_ast->data.T.lexeme;
                idSymbolTable* tmp = currentIdTable;
                symbolTableIdEntry* entry = NULL; 

                while (1) {
                    entry = searchId(*tmp, id);
                    
                    // found in the symbol table
                    if (entry != NULL)
                        break;
                    
                    // not found in global symbol table as well
                    if (tmp->parent == NULL)
                        break;
                    tmp = tmp->parent;
                }

                if (entry == NULL) {
                    redColor();
                    printf("Semantic Error: ");
                    resetColor();
                    printf("Identifier %s at line number %d has not been declared.\n", id, root->data.T.lineNo);
                    semanticError = 1;
                    return;
                }

                id_ast->entry = entry;

                // can static type check be done
                if (id_ast->sibling->TorNT == 0 && id_ast->sibling->datatype.tid == INTEGER && entry->AorP == 1 && entry->type.array.dynamicArray == 0) {
                    char* indice = id_ast->sibling->data.T.lexeme; // indice we want to print
                    printf("%s\n", indice); 
                    
                    // check if indice is within range
                    int lb = atoi(entry->type.array.lowerBound.lexeme);
                    int ub = atoi(entry->type.array.upperBound.lexeme);
                    int indexValue = atoi(indice);
                    printf("%d\n", indexValue);

                    // bound check
                    if (indexValue < lb || indexValue > ub) {
                        redColor();
                        printf("Type Error: ");
                        resetColor();
                        printf("Index %d out of bounds at line number: %d.\n", indexValue, id_ast->data.T.lineNo);
                        semanticError = 1;               
                    }

                }

                // index is not an integer
                else if (id_ast->sibling->TorNT == 0 && id_ast->sibling->datatype.tid != INTEGER) {
                    redColor();
                    printf("Semantic Error: ");
                    resetColor();
                    printf("Index is not an integer at line number: %d.\n", id_ast->data.T.lineNo);
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
        2. Can't update loop variable - see loopVariable symbol table variable
        3. Bound check
    */
    else if (root->TorNT == 1 && root->data.NT.ntid == assignmentStmt) {
        //printf("In assignment stmt\n");
        astnode* assignop = root->children;
        astnode* id = assignop->children;
        
        // not an array
        if (id->sibling->TorNT == 1 && id->sibling->data.NT.ntid == lvalueIDStmt) {
            idSymbolTable* tmp = currentIdTable;
            symbolTableIdEntry* entry = NULL; 

            while (1) {
                entry = searchId(*tmp, id->data.T.lexeme);
                
                // found in the symbol table
                if (entry != NULL)
                    break;
                
                // not found in global symbol table as well
                if (tmp->parent == NULL)
                    break;
                tmp = tmp->parent;
            }

            if (entry == NULL) {
                redColor();
                printf("Semantic Error: ");
                resetColor();
                printf("Identifier %s at line number %d has not been declared.\n", id->data.T.lexeme, id->data.T.lineNo);
                semanticError = 1;
                idFound = 1;
                return;
            }
            
            entry->isUpdated = 1;
            id->entry = entry;

            // cannot update loop variable
            if (entry->loopVariable == 1) {
                redColor();
                printf("Semantic Error: ");
                resetColor();
                printf("Cannot update loop variable at line number: %d.\n", id->data.T.lineNo);
                semanticError = 1;
            }
            
            astnode* expr = id->sibling->children;
            semanticChecker(expr);

            if (expr->datatype.tid == EPSILON) 
                return;
            
            // array and not an array
            if (entry->AorP == 1 && expr->datatype.tid != ARRAY) {
                redColor();
                printf("Type Error: ");
                resetColor();
                printf("Array assignment mismatch at line number: %d.\n", id->data.T.lineNo);
                semanticError = 1;
            }

            // type mismatch
            else if (entry->AorP == 0 && entry->type.primitive.datatype.tid != expr->datatype.tid) {
                redColor();
                printf("Type Error: ");
                resetColor();
                printf("Assignment type mismatch at line number: %d.\n", id->data.T.lineNo);
                semanticError = 1;
            }  
        }

        // array
        else {
            astnode* idx = id->sibling;
            idSymbolTable* tmp = currentIdTable;
            symbolTableIdEntry* entry = NULL; 

            while (1) {
                entry = searchId(*tmp, id->data.T.lexeme);
                
                // found in the symbol table
                if (entry != NULL)
                    break;
                
                // not found in global symbol table as well
                if (tmp->parent == NULL)
                    break;
                tmp = tmp->parent;
            }

            if (entry == NULL) {
                redColor();
                printf("Semantic Error: ");
                resetColor();
                printf("Identifier %s at line number %d has not been declared.\n", id->data.T.lexeme, root->data.T.lineNo);
                semanticError = 1;
                idFound = 1;
                return;
            }
            
            entry->isUpdated = 1;
            id->entry = entry;
            astnode* expr = idx->sibling->children;
            semanticChecker(expr);

            if (expr->datatype.tid == EPSILON) 
                return;

            // whether we can perform static bound check
            if (idx->TorNT == 0 && idx->data.T.tid == NUM && entry->AorP == 1 && entry->type.array.dynamicArray == 0) {
                int lb = atoi(entry->type.array.lowerBound.lexeme);
                int ub = atoi(entry->type.array.upperBound.lexeme);
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
                printf("Index is not an integer at line number: %d.\n", id->data.T.lineNo);
                semanticError = 1;               
            }
        }
        
        idFound = 1;
        //printf("Completed assignment stmt\n");
    }

    /* 
        1. Function declared but not defined
        2. If declared, then declaration line number < current function line Number < definition line number
        3. If not declared, should be defined before 
        4. Input parameter type and number check - matchParameters()
        5. Output parameter type and number check - matchParameters()
    */
    else if (root->TorNT == 1 && root->data.NT.ntid == moduleReuseStmt) {
        //printf("In module reuse stmt\n");
        astnode *functionName, *opt;
        if (root->children->TorNT == 1 && root->children->data.NT.ntid == optional) {
            opt = root->children;
            functionName = opt->sibling;
        }

        else {
            opt = NULL;
            functionName = root->children;
        }

        astnode* idlist = functionName->sibling;
        symbolTableFuncEntry* entry = searchFunc(funcTable, functionName->data.T.lexeme);
        int currLineNo = functionName->data.T.lineNo;

        // called function not defined
        if (entry == NULL || entry->definitionLineNo == -1) {
            redColor();
            printf("Semantic Error: ");
            resetColor();
            printf("Function %s has not been defined, called at line number: %d.\n", functionName->data.T.lexeme, currLineNo);
            semanticError = 1;
            return;
        }

        // recursive call -- current symbol table is same as the one being called
        if (currentIdTable == &(entry->link)) {
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
        matchParameters(entry, entry->inputParameters, idlist);
        
        parameters* outputItr = entry->outputParameters;
        
        // parameters used to invoke function call should exist in symbol table (see optional rule of AST)
        if (opt != NULL) {
            astnode* itr = opt->children->children;

            while (itr) {
                idSymbolTable* tmp = currentIdTable;
                symbolTableIdEntry* entry = NULL;

                while(1) {
                    entry = searchId(*tmp, itr->data.T.lexeme);

                    // found in current scope
                    if (entry != NULL)
                        break;
                    
                    // if reached global parent
                    if (tmp->parent == NULL)
                        break;
                    
                    // setting scope to parent scope
                    tmp = tmp->parent;
                }

                // not found in any symbol table
                if (entry == NULL) {
                    redColor();
                    printf("Semantic Error: ");
                    resetColor();
                    printf("Identifier %s at line number %d has not been declared.\n", itr->data.T.lexeme, itr->data.T.lineNo);
                    semanticError = 1;
                }
                
                else {
                    entry->isUpdated = 1;
                    itr->entry = entry;
                }

                itr = itr->sibling;
            }
        }
        
        // has return type, but not being assigned or vice-versa
        if ((opt == NULL && outputItr != NULL) || (opt != NULL && outputItr == NULL)) {
            redColor();
            printf("Semantic Error: ");
            resetColor();
            printf("Output parameter(s) number mismatch at line number: %d.\n", functionName->data.T.lineNo);
            semanticError = 1;
            return;
        }

        // check type and number of output parameters
        if (opt != NULL) {
            idlist = opt->children;
            matchParameters(entry, outputItr, idlist);
        }
    }

    else if (root->TorNT == 1 && root->data.NT.ntid == expression) {
        astnode* tmp = root->children;

        // traverse new_NT's corresponding AST
        if (tmp->TorNT == 1 && tmp->data.NT.ntid == unary_op) {
            //printf("In unary op\n");
            semanticChecker(tmp->sibling);
            root->datatype.tid = tmp->sibling->datatype.tid;
            //printf("Completed unary op\n");
        }

        // arithmeticOrBooleans's corresponding AST
        else {
            semanticChecker(tmp);
            root->datatype.tid = tmp->datatype.tid;
        }
    }

    /*
        1. Addition allowed only for INT, REAL
        2. If one subtree has type error, just return from entire expression
    */
    else if(root->TorNT == 0 && (root->data.T.tid == MUL || root->data.T.tid == DIV || root->data.T.tid == PLUS || root->data.T.tid == MINUS)) {
        astnode* leftchild = root->children;
        astnode* rightchild = leftchild->sibling;

        // even if ID, NUM, RNUM, it will simply return back with no computation
        semanticChecker(leftchild);
        semanticChecker(rightchild);

         // there is a type error somewhere already
        if (leftchild->datatype.tid == EPSILON || rightchild->datatype.tid == EPSILON) {
            root->datatype.tid = EPSILON;
            return;
        }

        if (idFound == 0) {
            root->datatype.tid = EPSILON;
            return;
        }

        // left and right child AST nodes should now have stored type in the root 
        if (leftchild->datatype.tid == rightchild->datatype.tid && (leftchild->datatype.tid == INTEGER || leftchild->datatype.tid == REAL))
            root->datatype.tid = leftchild->datatype.tid;

        // semantic error - probably skip the entire expression?
        else {
            redColor();
            printf("Type Error: ");
            resetColor();
            printf("Type mismatch at line number: %d.\n", root->data.T.lineNo);
            semanticError = 1;
            root->datatype.tid = EPSILON;
        }
    }

    /*
        1. Logical operations only allowed on boolean
        2. If one subtree has type error, just return from entire expression
    */
    else if(root->TorNT == 0 && (root->data.T.tid == AND || root->data.T.tid == OR)) {
        astnode* leftchild = root->children;
        astnode* rightchild = leftchild->sibling;

        // even if ID, NUM, RNUM, it will simply return back with no computation
        semanticChecker(leftchild);
        semanticChecker(rightchild);

        // there is a type error somewhere already
        if (leftchild->datatype.tid == EPSILON || rightchild->datatype.tid == EPSILON) {
            root->datatype.tid = EPSILON;
            return;
        }

        if (idFound == 0) {
            root->datatype.tid = EPSILON;
            return;
        }

        // left and right child AST nodes should now have stored type in the root 
        if (leftchild->datatype.tid == rightchild->datatype.tid && leftchild->datatype.tid == BOOLEAN)
            root->datatype.tid = leftchild->datatype.tid;

        // semantic error - probably skip the entire expression?
        else {
            redColor();
            printf("Type Error: ");
            resetColor();
            printf("Type mismatch at line number: %d.\n", root->data.T.lineNo);
            semanticError = 1;
            root->datatype.tid = EPSILON;
        }
    }

    /*
        1. Relational operations allowed only for INT, REAL
        2. If one subtree has type error, just return from entire expression
    */
    else if(root->TorNT == 0 && (root->data.T.tid == GT || root->data.T.tid == GE || root->data.T.tid == LT || root->data.T.tid == LE || root->data.T.tid == EQ || root->data.T.tid == NE)) {
        astnode* leftchild = root->children;
        astnode* rightchild = leftchild->sibling;

        // even if ID, NUM, RNUM, it will simply return back with no computation
        semanticChecker(leftchild);
        semanticChecker(rightchild);

        // there is a type error somewhere already
         if (leftchild->datatype.tid == EPSILON || rightchild->datatype.tid == EPSILON) {
            root->datatype.tid = EPSILON;
            return;
         }

        if (idFound == 0) {
            root->datatype.tid = EPSILON;
            return;
        }

         // left and right child AST nodes should now have stored type in the root 
        if (leftchild->datatype.tid == rightchild->datatype.tid && (leftchild->datatype.tid == INTEGER || leftchild->datatype.tid == REAL))
            root->datatype.tid = BOOLEAN;

        // semantic error - probably skip the entire expression?
        else {
            redColor();
            printf("Type Error: ");
            resetColor();
            printf("Type mismatch at line number: %d.\n", root->data.T.lineNo);
            semanticError = 1;
            root->datatype.tid = EPSILON;
        }
    }

    /*
        1. Identifier redeclaration - see isRedeclared AST node variable
        2. Dynamic Array - bound identifiers aren't declared
        3. Dynamic Array - bound identifiers aren't integers
        4. Static Array - lower bound < upper bound 
    */
    else if (root->TorNT == 1 && root->data.NT.ntid == declareStmt) {
        //printf("In declare stmt\n");
        astnode* idlist = root->children;
        astnode* itr = idlist->children;

        while (itr) {
            char* id = itr->data.T.lexeme;
            symbolTableIdEntry* entry = searchId(*currentIdTable, id);
            itr->entry = entry;

            // found in current symbol table - but a redeclaration
            if (itr->isRedeclared == 1) {
                redColor();
                printf("Semantic Error: ");
                resetColor();
                printf("Identifier %s at line %d has already been declared before.\n", id, itr->data.T.lineNo);
                semanticError = 1;
            }

            // arrays - a first time declaration
            else if (entry->AorP == 1) {
                token lowerBound = entry->type.array.lowerBound;
                token upperBound = entry->type.array.upperBound;
                astnode* lb = root->children->sibling->children;
                astnode* ub = root->children->sibling->children;

                // static array
                if (entry->type.array.dynamicArray == 0) {
                    int lower = atoi(entry->type.array.lowerBound.lexeme);
                    int upper = atoi(entry->type.array.upperBound.lexeme);

                    // bounds not in increasing order 
                    if (lower >= upper) {
                        redColor();
                        printf("Semantic Error: ");
                        resetColor();
                        printf("Lower bound %d is greater than upper bound %d at line number: %d.\n", lower, upper, itr->data.T.lineNo);
                        semanticError = 1;
                        *currentIdTable = removeId(*currentIdTable, id);
                    }
                }

                // check if lower bound identifier declared
                if (lowerBound.tid == ID) {
                    idSymbolTable* tmp = currentIdTable;
                    symbolTableIdEntry* boundEntry = NULL;
                    char* boundId = lb->data.T.lexeme;

                    while(1) {
                        boundEntry = searchId(*tmp, boundId);

                        // found in current scope
                        if (boundEntry != NULL)
                            break;
                        
                        // if reached global parent
                        if (tmp->parent == NULL)
                            break;
                        
                        // setting scope to parent scope
                        tmp = tmp->parent;
                    }

                    // lower bound identifier not declared
                    if (boundEntry == NULL) {
                        redColor();
                        printf("Semantic Error: ");
                        resetColor();
                        printf("Array bound %s has not been declared at line number: %d.\n", boundId, itr->data.T.lineNo);
                        semanticError = 1;
                        *currentIdTable = removeId(*currentIdTable, id);
                    }

                    // lower bound identifier not of integer data type
                    else if (boundEntry->AorP != 0 || boundEntry->type.primitive.datatype.tid != INTEGER) {
                        redColor();
                        printf("Semantic Error: ");
                        resetColor();
                        printf("Array bound %s is not of integer type at line number: %d.\n", boundId, itr->data.T.lineNo);
                        semanticError = 1;
                        *currentIdTable = removeId(*currentIdTable, id);
                    }
                }

                // check if upper bound identifier declared
                if (upperBound.tid == ID) {
                    idSymbolTable* tmp = currentIdTable;
                    symbolTableIdEntry* boundEntry = NULL;
                    char* boundId = ub->data.T.lexeme;

                    while(1) {
                        boundEntry = searchId(*tmp, boundId);

                        // found in current scope
                        if (boundEntry != NULL)
                            break;
                        
                        // if reached global parent
                        if (tmp->parent == NULL)
                            break;
                        
                        // setting scope to parent scope
                        tmp = tmp->parent;
                    }

                    // lower bound identifier not declared
                    if (boundEntry == NULL) {
                        redColor();
                        printf("Semantic Error: ");
                        resetColor();
                        printf("Array bound %s has not been declared at line number: %d.\n", boundId, itr->data.T.lineNo);
                        semanticError = 1;
                        *currentIdTable = removeId(*currentIdTable, id);
                    }

                    // lower bound identifier not of integer data type
                    else if (boundEntry->AorP != 0 || boundEntry->type.primitive.datatype.tid != INTEGER) {
                        redColor();
                        printf("Semantic Error: ");
                        resetColor();
                        printf("Array bound %s is not of integer type at line number: %d.\n", boundId, itr->data.T.lineNo);
                        semanticError = 1;
                        *currentIdTable = removeId(*currentIdTable, id);
                    }
                }
            }

            itr = itr->sibling;
        }
        //printf("Completed declare stmt\n");
    }

    /* 
        1. Case variable should be declared
        2. Case variable should be integer or boolean
        3. If integer, must contain default statement
        4. If boolean, must not contain default statement
    */
    else if (root->TorNT == 1 && root->data.NT.ntid == conditionalStmt) {
        currentIdTable = &(root->scopeTable);
        astnode* id = root->children;
        astnode* caseStatements = id->sibling;
        astnode* default_ast = caseStatements->sibling;

        // case variable not found
        semanticChecker(id);
        // semanticError = 0;

        // must contain default statement
        if (id->datatype.tid == INTEGER) {
            caseStatements->datatype.tid = INTEGER;
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
            caseStatements->datatype.tid = BOOLEAN;
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
        currentIdTable = &(root->scopeTable);
        astnode* tmp = root->children;
        while (tmp) {

            // case value of different type
            if (tmp->datatype.tid != root->datatype.tid) {
                redColor();
                printf("Type Error: ");
                resetColor();
                printf("Case value type mismatch at line number: %d\n", tmp->data.T.lineNo);
                //printf("%s vs %s", tmp->); //incomplete
                semanticError = 1;
            }
            tmp = tmp->sibling;
            semanticChecker(tmp);
            tmp = tmp->sibling;
        }
        currentIdTable = currentIdTable->parent;
    }

    /*
        1. Loop variable should be declared
        2. Loop variable should be integer
        3. Loop variable shouldn't be updated - handled in assignmentStmt
    */
    else if (root->TorNT == 1 && root->data.NT.ntid == iterativeStmt && root->children->TorNT == 0 && root->children->data.T.tid == FOR) {
        currentIdTable = &(root->scopeTable);
        astnode* id = root->children->sibling;
        idSymbolTable* tmp = currentIdTable;
        symbolTableIdEntry* entry = NULL;

        while(1) {
             entry = searchId(*tmp, id->data.T.lexeme);

            // found in current scope
            if (entry != NULL)
                break;
            
            // if reached global parent
            if (tmp->parent == NULL)
                break;
            
            // setting scope to parent scope
            tmp = tmp->parent;
        }

        // not found in any symbol table
        if (entry == NULL) {
            redColor();
            printf("Semantic Error: ");
            resetColor();
            printf("Identifier %s at line number %d has not been declared.\n", id->data.T.lexeme, id->data.T.lineNo);
            idFound = 0;
            semanticError = 1;
        }
        
        else 
            id->entry = entry;

        // loop variable isn't integer data type
        if (entry->AorP != 0 || entry->type.primitive.datatype.tid != INTEGER) {
            redColor();
            printf("Type Error: ");
            resetColor();
            printf("Loop variable %s must be an integer at line number: %d.\n", id->data.T.lexeme, id->data.T.lineNo);
            semanticError = 1;
        }

         else 
            entry->loopVariable = 1;
        
        astnode* stmts = id->sibling->sibling;
        semanticChecker(stmts);
        currentIdTable = currentIdTable->parent;
    }

    /*
        1. Expression should be of boolean type
    */
    else if (root->TorNT == 1 && root->data.NT.ntid == iterativeStmt && root->children->TorNT == 0 && root->children->data.T.tid == WHILE) {
        currentIdTable = &(root->scopeTable);
        int currLineNo = root->children->data.T.lineNo;
        astnode* abexpr = root->children->sibling;
        semanticChecker(abexpr);
        
        if (abexpr->datatype.tid != EPSILON && abexpr->datatype.tid != BOOLEAN) {
            redColor();
            printf("Type Error: ");
            resetColor();
            printf("While loop expression must be of boolean data type at line number: %d.\n", currLineNo);
            semanticError = 1;
        }
        
        astnode* stmts = abexpr->sibling;
        semanticChecker(stmts);
        
        // set current table back to outer scope
        currentIdTable = currentIdTable->parent;
    }

    /*
        1. Searches for ID in current scope and all parent tables
        2. Error if identifier not found in any table  
    */
    else if (root->TorNT == 0 && root->data.T.tid == ID) {
        char* id = root->data.T.lexeme;
        idSymbolTable* tmp = currentIdTable;
        symbolTableIdEntry* entry = NULL;

        while(1) {
             entry = searchId(*tmp, id);

            // found in current scope
            if (entry != NULL)
                break;
            
            // if reached global parent
            if (tmp->parent == NULL)
                break;
            
            // setting scope to parent scope
            tmp = tmp->parent;
        }

        // not found in any symbol table
        if (entry == NULL) {
            redColor();
            printf("Semantic Error: ");
            resetColor();
            printf("Identifier %s at line number %d has not been declared.\n", id, root->data.T.lineNo);
            idFound = 0;
            semanticError = 1;
        }

        else {
            root->entry = entry;
            if (entry->AorP == 0)
                root->datatype = entry->type.primitive.datatype;
            
            else 
                root->datatype.tid = ARRAY;

        }
    }
}
