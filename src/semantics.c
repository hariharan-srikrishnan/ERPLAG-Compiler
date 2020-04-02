#include <stdio.h>
#include <stdlib.h>

#include "ast.h"
#include "symboltableDef.h"
#include "symboltable.h"
#include "typeExtractor.h"

int semanticError = 0;


/*
    TO-DO:
        1. Arithmetic expressions
        2. Check parent scopes for variable
        3. Change link from arrow to dot in function symbol table entries
        4. Print relevant semantic errors
        5. Arrays in arithmetic expressions
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
        char* id = root->children->sibling->data.T.lexeme;
        symbolTableIdEntry* entry = searchId(currentIdTable, id);
        
        if (entry == NULL) {
            semanticError = 1;
        }
    }

    // check if variable, if an identifier, present in symbol table
    else if (root->TorNT == 1 && root->data.NT.ntid == ioStmt && root->children->TorNT == 0 && root->children->data.T.tid == PRINT) {
        
        // check if array or not
        if (root->TorNT == 0 && root->children->sibling->children->data.T.tid == ID) {
            
            astnode* id_ast = root->children->sibling->children;
            // check if array or not
            if (id_ast->sibling == NULL) {
                
                // identifier
                char *id = id_ast->data.T.lexeme;
                symbolTableIdEntry* entry = searchId(currentIdTable, id);
                
                if (entry == NULL) {
                    semanticError = 1;
                    return;
                }
            }

            else {
                // array
                char *id = id_ast->data.T.lexeme;
                symbolTableIdEntry* entry = searchId(currentIdTable, id);

                if (entry == NULL) {
                    semanticError = 1;
                    return;
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
            symbolTableIdEntry* entry = searchId(currentIdTable, id->data.T.lexeme);
            
            if (entry == NULL) {
                semanticError = 1;
                return;
            }

            astnode* expr = id->sibling->children;
            semanticChecker(expr);

            // type mismatch
            if (expr->datatype.tid != entry->type.primitive.datatype.tid) {
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

                else {
                    semanticError = 1;
                }
            }
        }

    }

    // match thhe types and the number of parameters returned by a function must be the same as that of the parameters used in invoking the function.
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

        if (opt != NULL) {
            astnode* tmp = opt->children;
            while (tmp) {
                symbolTableIdEntry* entry = searchId(currentIdTable, tmp->data.T.lexeme);
                
                if (entry == NULL) {
                    semanticError = 1;
                }
                tmp = tmp->sibling;
            }
        }

        // check if function has been defined/declared
        symbolTableFuncEntry* entry = searchFunc(funcTable, functionName->data.T.lexeme);
        if (entry == NULL) {
            semanticError = 1;
            return;
        }

        // check if identifiers are in symbol table
        astnode* idlist = functionName->sibling;
        astnode* tmp = idlist->children;
        while (tmp) {
            symbolTableIdEntry* entry = searchId(currentIdTable, tmp->data.T.lexeme);
            
            if (entry == NULL) {
                semanticError = 1;
            }

            tmp = tmp->sibling;
        }

    }

    else if (root->TorNT == 1 && root->data.NT.ntid == expression) {
        astnode* tmp = root->children;

        // unaryop -- (arithmeticExpr / var_id_num)
        if (tmp->TorNT == 1 && tmp->data.NT.ntid == unary_op) {
            if (root->children->sibling->TorNT == 0 && root->children->sibling->data.T.tid == ID) {
                char* idName = root->children->sibling->data.T.lexeme;
                symbolTableIdEntry* entry = searchId(currentIdTable, idName);
                
                if (entry == NULL) {
                    semanticError = 1;
                    return;
                }

                // TODO: HANDLE ARRAYS
                root->datatype = entry->type.primitive.datatype;

            }

            else if (root->children->sibling->TorNT == 0 && root->children->sibling->data.T.tid == NUM) {
                root->datatype.tid = INTEGER;
                root->datatype.lineNo = root->children->sibling->data.T.lineNo; 
            }

            else if (root->children->sibling->TorNT == 0 && root->children->sibling->data.T.tid == RNUM) {
                root->datatype.tid = REAL;
                root->datatype.lineNo = root->children->sibling->data.T.lineNo; 
            }

            else {
                semanticChecker(tmp->sibling);
                // root->datatype = 
            }

        }
        
        // arithhmetic or boolean expressions
        else {
            semanticChecker(tmp);
            // root->datatype = 
        }
    }

    else if(root->TorNT == 0 && (root->data.T.tid == MUL || root->data.T.tid == DIV || root->data.T.tid == PLUS || root->data.T.tid == MINUS)) {
        astnode* tmp = root->children;
        token type = tmp->data.T;

        while (tmp->sibling->TorNT == 0 && tmp->sibling->data.T.tid != ID) {
            tmp = tmp->sibling->children;

            // type mismatch
            if (type.tid != tmp->data.T.tid) {
                semanticError = 1;
                return;
            }

            type = tmp->data.T;
        }

        if (tmp->sibling->TorNT == 0 && tmp->sibling->data.T.tid == ID) {

            // type mismatch
            if (type.tid != tmp->sibling->data.T.tid) {
                semanticError = 1;
            }
        }
    }

    else if (root->TorNT == 1 && root->data.NT.ntid == declareStmt) {
        astnode* idlist = root->children;
        astnode* tmp = idlist->children;

        while (tmp) {
            symbolTableIdEntry* entry = searchId(currentIdTable, tmp->data.T.lexeme);
            
            // found in symbol table - identifier has already been declared before
            if (entry != NULL) {
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
        symbolTableIdEntry* entry = searchId(currentIdTable, id->data.T.lexeme);
        if (entry == NULL) {
            semanticError = 1;
        }

        // must contain default statement
        if (entry->AorP == 0 && entry->type.primitive.datatype.tid == INTEGER) {
            if (default_ast == NULL) {
                semanticError = 1;
            }
        }

        // must not contain default statement
        else if (entry->AorP == 0 && entry->type.primitive.datatype.tid == BOOLEAN) {
            if (default_ast != NULL) {
                semanticError = 1;
            }
        }

        // some other datatype 
        else {
            semanticError = 1;
        }

        semanticChecker(caseStatements);
    }

    else if (root->TorNT == 1 && root->data.NT.ntid == caseStmts) {
        currentIdTable =root->scopeTable;
        astnode* tmp = root->children;
        while (tmp) {
            tmp = tmp->sibling;
            semanticChecker(tmp);
            tmp = tmp->sibling;
        }
        currentIdTable = *(currentIdTable.parent);
    }

    else if (root->TorNT == 1 && root->data.NT.ntid == iterativeStmt && root->children->TorNT == 0 && root->children->data.T.tid == FOR) {
        currentIdTable = root->scopeTable;
        astnode* id = root->children->sibling;
        symbolTableIdEntry* entry = searchId(currentIdTable, id->data.T.lexeme);

        // identifier not found in symbol table
        if (entry == NULL) {
            semanticError = 1;
        }

        astnode* stmts = id->sibling->sibling;
        semanticChecker(stmts);
        currentIdTable = *(currentIdTable.parent);
    }

    else if (root->TorNT == 1 && root->data.NT.ntid == iterativeStmt && root->children->TorNT == 0 && root->children->data.T.tid == WHILE) {
        currentIdTable = root->scopeTable;
            semanticError = 1;
        }

        astnode* stmts = id->sibling->sibling;
        semanticChecker(stmts);
        currentIdTable = *(currentIdTable.parent);
    }

    else if (root->TorNT == 1 && root->data.NT.ntid == iterativeStmt && root->children->TorNT == 0 && root->children->data.T.tid == WHILE) {
        currentIdTable = root->scopeTable;
        astnode* abexpr = root->children->sibling;

        astnode* stmts = abexpr->sibling;
        semanticChecker(stmts);
        currentIdTable = *(currentIdTable.parent);
    }
}