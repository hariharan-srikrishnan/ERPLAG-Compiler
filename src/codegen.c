#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "symboltableDef.h"
#include "symboltable.h"
#include "typeExtractor.h"
#include "semantics.h"

// need to check NASM for maximum number of temporaries which can be generated
FILE* asmFile = NULL;
int num_temps = 0;
int num_labels = 0;

/*
    Decisions
        1. Width of integer, boolean, real? (2, 1, 4)
        2. Which registers to use for each type (AX, EAX, RAX)?
        3. How to store true and false in registers?
*/

// HOW TO ENSURE IT DOESN'T CLASH WITH CODE VARIABLES tmp1, tmp2?
char* generateTemporary(astnode* root) {
    char* temp = (char*) malloc(sizeof(char) * (3 + num_temps / 10 + 1 + 1)); // tmp + number of digits + ENDMARKER
    sprintf(temp, "tmp%d", num_temps);
    num_temps++;

    token t = root->data.T;
    strcpy(t.lexeme, temp);

    // symbolTableIdEntry* entry = createIdEntry(t, root->);
    symbolTableIdEntry entry;
    entry.id = t;
    entry.AorP = 0;
    strcpy(entry.name, temp);
    entry.offset = currentOffset;
    entry.type.primitive.datatype.tid = root->datatype.tid;

    if (root->datatype.tid == INTEGER) 
        entry.type.primitive.width = 4;
    
    
    else if (root->datatype.tid == REAL)
        entry.type.primitive.width = 8;

    // boolean
    else 
        entry.type.primitive.width = 1;
    
    currentOffset += entry.type.primitive.width;
    *currentIdTable = insertId(*currentIdTable, entry);
    root->entry = entry;
    return temp;
}


// generate labels for assembly logic
char* generateLabel() {
    char* label = (char*) malloc(sizeof(char) * (5 + num_temps / 10 + 1 + 1)); // label + number of digits + ENDMARKER
    sprintf(label, "label%d", num_labels);
    num_labels++;
    return label;
}


/*
    TODO
        1. Change symbol tables so the temporaries can be handled (using scopeTable) - DONE?
        2. Link temporaries to symbol table somehow - create relevant offsets (DONE?)
        3. Floating point comparison?
        4. Module reuse statement
        5. Code generation for dynamic type checking
*/
void generateASM(astnode* root) {
    
    if(root == NULL) 
		return;

    if (semanticError == 1)
        exit(1);

    if (root->TorNT == 1 && root->data.NT.ntid == program) {
        fprintf(asmFile, "section .data\n");
        fprintf(asmFile, "\tintegerRead: db \"%%d\", 0\n");
        fprintf(asmFile, "\tintegerWrite: db \"%%d\", 10, 0\n"); // newline, null terminator
        fprintf(asmFile, "\trealRead: db \"%%ld\", 0\n");
        fprintf(asmFile, "\trealWrite: db \"%%ld\", 10, 0\n"); // newline, null terminator
        fprintf(asmFile, "\n\n");
        
        fprintf(asmFile, "section .text\n");
        fprintf(asmFile, "extern _printf\n");
        fprintf(asmFile, "extern _scanf\n");
        fprintf(asmFile, "\n\n");

        currentIdTable = globalIdTable;
        astnode* tmp = root->children;
        while (tmp) {
            generateASM(tmp);
            tmp = tmp->sibling;
        }
        currentIdTable = globalIdTable;
    }

    else if (root->TorNT == 1 && root->data.NT.ntid == otherModules) {
        astnode* tmp = root->children;
        while (tmp) {
            generateASM(tmp);
            tmp = tmp->sibling;
        }
    }

    else if (root->TorNT == 1 && root->data.NT.ntid == driverModule) {
        currentIdTable = &(root->scopeTable);
        generateASM(root->children->sibling);
        currentIdTable = currentIdTable->parent;
    }

    // push all parameters and all that?
    else if (root->TorNT == 1 && root->data.NT.ntid == module) {
        astnode* iplist = root->children->sibling;
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

        currentIdTable = &(root->scopeTable);
        generateASM(moduledef);

        currentIdTable = currentIdTable->parent;
    }

    else if (root->TorNT == 1 && root->data.NT.ntid == moduleDef) {
        generateASM(root->children);
    }

    else if (root->TorNT == 1 && root->data.NT.ntid == statements) {
        astnode* tmp = root->children;
        while(tmp) {
            generateASM(tmp);
            tmp = tmp->sibling;
        }
    }

    // reference: https://stackoverflow.com/questions/10973650/how-to-use-scanf-in-nasm
    else if (root->TorNT == 1 && root->data.NT.ntid == ioStmt && root->children->TorNT == 0 && root->children->data.T.tid == GET_VALUE) {

        // primitive datatype
        if (root->entry->AorP == 0) {

            fprintf(asmFile, "\tPUSH %s\n", root->entry->name);

            if (root->entry->type.primitive.datatype.tid == INTEGER) 
                fprintf(asmFile, "\tPUSH integerRead\n");

            else if (root->entry->type.primitive.datatype.tid == REAL) 
                fprintf(asmFile, "\tPUSH realRead\n");

            // boolean: assume you can scan only 1 or 0
            else 
                fprintf(asmFile, "\tPUSH integerRead\n");
           
            fprintf(asmFile, "\tCALL _scanf\n");
            fprintf(asmFile, "\tADD ESP, 8\n"); // remove parameters from stack
        }

        // arrays - how to handle dynamic arrays?
        else if (root->entry->AorP == 1 && root->entry->type.array.dynamicArray == 0) {
            int lb = atoi(root->entry->type.array.lowerBound.lexeme);
            int ub = atoi(root->entry->type.array.upperBound.lexeme);
            int length = ub - lb + 1;
            int width = root->entry->type.array.datatype.width;
            
            char* label = generateLabel();
            char* exitLabel = generateLabel();

            fprintf(asmFile, "\tPUSH ECX\n");
            fprintf(asmFile, "\tMOV ECX, 0\n");
            fprintf(asmFile, "%s: \n", label);
            fprintf(asmFile, "\tCMP ECX, %d\n", length);
            fprintf(asmFile, "\tJE %s\n", exitLabel);
            fprintf(asmFile, "\tPUSH %s + ECX * %d\n", root->entry->name, width);

            if (root->entry->type.array.datatype.datatype.tid == INTEGER) 
                fprintf(asmFile, "\tPUSH integerRead\n");

            else if (root->entry->type.array.datatype.datatype.tid == REAL) 
                fprintf(asmFile, "\tPUSH realRead\n");

            // boolean: assume you can scan only 1 or 0
            else 
                fprintf(asmFile, "\tPUSH integerRead\n");

            fprintf(asmFile, "\tCALL _scanf\n");
            fprintf(asmFile, "\tADD ESP, 8\n"); // remove parameters from stack
            fprintf(asmFile, "\tINC ECX\n");
            fprintf(asmFile, "\tJMP %s\n", label);
            fprintf(asmFile, "\tPOP ECX\n");
            fprintf(asmFile, "%s: \n", exitLabel);
        } 
    }

    else if (root->TorNT == 1 && root->data.NT.ntid == ioStmt && root->children->TorNT == 0 && root->children->data.T.tid == PRINT) {
         
        // primitive datatype
        if (root->entry->AorP == 0) {
            
            fprintf(asmFile, "\tPUSH %s\n", root->entry->name);

            if (root->entry->type.primitive.datatype.tid == INTEGER) 
                fprintf(asmFile, "\tPUSH integerWrite\n");
            

            else if (root->entry->type.primitive.datatype.tid == REAL) 
                fprintf(asmFile, "\tPUSH realWrite\n");

            // boolean: assume you can scan only 1 or 0
            else 
                fprintf(asmFile, "\tPUSH integerWrite\n");
            
            fprintf(asmFile, "\tCALL _printf\n");
            fprintf(asmFile, "\tADD ESP, 8\n"); // remove parameters from stack
        }

        // arrays - how to handle dynamic arrays?
        else if (root->entry->AorP == 1 && root->entry->type.array.dynamicArray == 0) {
            int lb = atoi(root->entry->type.array.lowerBound.lexeme);
            int ub = atoi(root->entry->type.array.upperBound.lexeme);
            int length = ub - lb + 1;
            int width = root->entry->type.array.datatype.width;

            char* label = generateLabel();
            char* exitLabel = generateLabel();

            fprintf(asmFile, "\tPUSH ECX\n");
            fprintf(asmFile, "\tMOV ECX, 0\n");
            fprintf(asmFile, "%s: \n", label);
            fprintf(asmFile, "\tCMP ECX, %d\n", length);
            fprintf(asmFile, "\tJE %s\n", exitLabel);
            fprintf(asmFile, "\tPUSH %s + ECX * %d\n", root->entry->name, width);

            if (root->entry->type.array.datatype.datatype.tid == INTEGER) 
                fprintf(asmFile, "\tPUSH integerWrite\n");
            

            else if (root->entry->type.array.datatype.datatype.tid == REAL)
                fprintf(asmFile, "\tPUSH realWrite\n");

            // boolean: assume you can scan only 1 or 0
            else 
                fprintf(asmFile, "\tPUSH integerWrite\n");

            fprintf(asmFile, "\tCALL _printf\n");
            fprintf(asmFile, "\tADD ESP, 8\n"); // remove parameters from stack
            fprintf(asmFile, "\tINC ECX\n");
            fprintf(asmFile, "\tJMP %s\n", label);
            fprintf(asmFile, "\tPOP ECX\n");
            fprintf(asmFile, "%s: \n", exitLabel);
        }
    }

    else if (root->TorNT == 1 && root->data.NT.ntid == simpleStmt) {
        generateASM(root->children);
    }
    
    else if (root->TorNT == 1 && root->data.NT.ntid == assignmentStmt) {
        astnode* assignop = root->children;
        astnode* id = assignop->children;
        
        // not an array
        if (id->sibling->TorNT == 1 && id->sibling->data.NT.ntid == lvalueIDStmt) {

            if (root->entry->AorP == 0) {

                if (root->entry->type.array.datatype.datatype.tid == INTEGER) {
                    fprintf(asmFile, "\tMOV EAX, [%s]\n", id->sibling->entry->name);
                    fprintf(asmFile, "\tMOV [%s], EAX\n", id->entry->name);
                }

                else if (root->entry->type.array.datatype.datatype.tid == REAL) {
                    fprintf(asmFile, "\tMOV RAX, [%s]\n", id->sibling->entry->name);
                    fprintf(asmFile, "\tMOV [%s], RAX\n", id->entry->name);
                }

                // boolean
                else if {
                    fprintf(asmFile, "\tMOV EAX, [%s]\n", id->sibling->entry->name);
                    fprintf(asmFile, "\tMOV [%s], EAX\n", id->entry->name);
                }
            }

            // assigning one array to another
            else {
                char* label = generateLabel();
                char* exitLabel = generateLabel();
                int width = root->entry->type.array.datatype.width;

                fprintf(asmFile, "\tPUSH ECX\n");
                fprintf(asmFile, "\tMOV ECX, 0\n");
                fprintf(asmFile, "%s: \n", label);
                fprintf(asmFile, "\tCMP ECX, %d\n", length);
                fprintf(asmFile, "\tJE %s\n", exitLabel);

                if (root->entry->type.array.datatype.datatype.tid == INTEGER) {
                    fprintf(asmFile, "\tMOV EAX, [%s + ECX * %d]\n", id->sibling->entry->name, width);
                    fprintf(asmFile, "\tMOV [%s + ECX * %d], EAX\n", id->entry->name, width);
                }

                else if (root->entry->type.array.datatype.datatype.tid == REAL) {
                    fprintf(asmFile, "\tMOV RAX, [%s + ECX * %d]\n", id->sibling->entry->name, width);
                    fprintf(asmFile, "\tMOV [%s + ECX * %d], RAX\n", id->entry->name, width);
                }

                // boolean
                else {
                    fprintf(asmFile, "\tMOV EAX, [%s + ECX * %d]\n", id->sibling->entry->name, width);
                    fprintf(asmFile, "\tMOV [%s + ECX * %d], EAX\n", id->entry->name, width);
                }
                
                fprintf(asmFile, "\tINC ECX\n");
                fprintf(asmFile, "\tJMP %s\n", label);
                fprintf(asmFile, "\tPOP ECX\n");
                fprintf(asmFile, "%s: \n", exitLabel);
            }
            
        }

        // array with index
        else {
            astnode* idx = id->sibling;
            int width = root->entry->type.array.datatype.width;

            if (root->entry->type.array.datatype.datatype.tid == INTEGER) {
                fprintf(asmFile, "\tMOV EAX, [%s]\n", id->sibling->entry->name);
                fprintf(asmFile, "\tMOV [%s + %s * %d], EAX\n", id->entry->name, idx->entry->name, width);
            }

            else if (root->entry->type.array.datatype.datatype.tid == REAL) {
                fprintf(asmFile, "\tMOV RAX, [%s]\n", id->sibling->entry->name);
                fprintf(asmFile, "\tMOV [%s + %s * %d], RAX\n", id->entry->name, idx->entry->name, width);
            }

            // boolean
            else {
                fprintf(asmFile, "\tMOV EAX, [%s]\n", id->sibling->entry->name);
                fprintf(asmFile, "\tMOV [%s + %s * %d], EAX\n", id->entry->name, idx->entry->name, width);
            }
        }
    }

    // push paramaters onto stack in opposite order, pop based on size later
    else if (root->TorNT == 1 && root->data.NT.ntid == moduleReuseStmt) {

    }

    else if (root->TorNT == 1 && root->data.NT.ntid == expression) {
        astnode* tmp = root->children;
        
        if (tmp->TorNT == 1 && tmp->data.NT.ntid == unary_op) {
            generateASM(tmp->sibling);
            root->entry = tmp->sibling->entry;

            // need to negate the result
            if (tmp->children->data.T.tid == MINUS) {
                
                if (tmp->entry->type.array.datatype.datatype.tid == INTEGER) {
                    fprintf(asmFile, "\tMOV EAX, 0\n");
                    fprintf(asmFile, "\tISUB EAX, [%s]\n", tmp->sibling->entry->name);
                    fprintf(asmFile, "\tMOV [%s], EAX\n", tmp->sibling->entry->name);
                }

                // real - unary op with boolean is just absurd
                else {
                    fprintf(asmFile, "\tMOV RAX, 0\n");
                    fprintf(asmFile, "\tISUB RAX, [%s]\n", tmp->sibling->entry->name);
                    fprintf(asmFile, "\tMOV [%s], RAX\n", tmp->sibling->entry->name);
                }
            }
        }
        
        else {
            generateASM(tmp);
            root->entry = tmp->entry;
        }
    }

    // TODO: Link temporary to symbol table - DONE?
    else if(root->TorNT == 0 && (root->data.T.tid == MUL || root->data.T.tid == DIV || root->data.T.tid == PLUS || root->data.T.tid == MINUS)) {
        astnode* leftchild = root->children;
        astnode* rightchild = leftchild->sibling;

        generateASM(leftchild);
        generateASM(rightchild);

        char* tmp = generateTemporary(root);

        // addition
        if (root->data.T.tid == PLUS) {
            
            // integer arithmetic
            if (root->datatype.tid == INTEGER) {
                fprintf(asmFile, "\tMOV EAX, [%s]\n", leftchild->entry->id.lexeme);
                fprintf(asmFile, "\tIADD EAX, [%s]\n", rightchild->entry.id.lexeme);
                fprintf(asmFile, "\tMOV [%s], EAX\n", tmp);
            }

            // real
            else {
                fprintf(asmFile, "\tFLD  QWORD [%s]\n", leftchild->entry->id.lexeme); // load (pushed on flt pt stack, st0)
                fprintf(asmFile, "\tFIADD QWORD [%s]\n", rightchild->entry->id.lexeme); // floating add (to st0)
                fprintf(asmFile, "\tFSTP QWORD [%s]\n", tmp); // store product into c (pop flt pt stack)
            }
        }

        // subtraction
        else if (root->data.T.tid == MINUS) {

            if (root->datatype.tid == INTEGER) {
                fprintf(asmFile, "\tMOV EAX, [%s]\n", leftchild->entry->id.lexeme);
                fprintf(asmFile, "\tISUB EAX, [%s]\n", rightchild->entry->id.lexeme);
                fprintf(asmFile, "\tMOV [%s], EAX\n", tmp);
            }

            // real
            else {
                fprintf(asmFile, "\tFLD  QWORD [%s]\n", leftchild->entry->id.lexeme); // load (pushed on flt pt stack, st0)
                fprintf(asmFile, "\tFISUB QWORD [%s]\n", rightchild->entry->id.lexeme); // floating sub (to st0)
                fprintf(asmFile, "\tFSTP QWORD [%s]\n", tmp); // store product into c (pop flt pt stack)
            }
        
        // multiplication
        else if (root->data.T.tid == MUL) {

            if (root->datatype.tid == INTEGER) {
                fprintf(asmFile, "\tMOV  EAX, [%s]\n", leftchild->entry->id.lexeme); // load (must be eax for multiply)
                fprintf(asmFile, "\tIMUL DWORD [%s]\n", rightchild->entry->id.lexeme); // signed integer multiply
                fprintf(asmFile, "\tMOV  [%s], EAX\n", tmp); // store bottom half of product
            }

            // real
            else {
                fprintf(asmFile, "\tFLD  QWORD [%s]\n", leftchild->entry->id.lexeme); // load (pushed on flt pt stack, st0)
                fprintf(asmFile, "\tFIMUL QWORD [%s]\n", rightchild->entry->id.lexeme); // floating multiply (to st0)
                fprintf(asmFile, "\tFSTP QWORD [%s]\n", tmp); // store product into c (pop flt pt stack)
            }
        }

        // division
        else if (root->data.T.tid == DIV) {

            if (root->datatype.tid == INTEGER) {
                fprintf(asmFile, "\tMOV  EAX, [%s]\n", leftchild->entry->id.lexeme); // load 
                fprintf(asmFile, "\tMOV  EDX, 0\n", leftchild->entry->id.lexeme); // load upper half of dividend with zero
                fprintf(asmFile, "\tIDIV DWORD [%s]\n", rightchild->entry->id.lexeme); // divide double register edx eax by a
                fprintf(asmFile, "\tMOV  [%s], EAX\n", tmp); // store quotient
            }

            // real
            else {
                fprintf(asmFile, "\tFLD  QWORD [%s]\n", leftchild->entry->id.lexeme); // load (pushed on flt pt stack, st0)
                fprintf(asmFile, "\tFIDIV QWORD [%s]\n", rightchild->entry->id.lexeme); // floating div (to st0)
                fprintf(asmFile, "\tFSTP QWORD [%s]\n", tmp); // store product into c (pop flt pt stack)
            }
        }
    }

    else if(root->TorNT == 0 && (root->data.T.tid == AND || root->data.T.tid == OR)) {
        astnode* leftchild = root->children;
        astnode* rightchild = leftchild->sibling;

        generateASM(leftchild);
        generateASM(rightchild);

        char* tmp = generateTemporary(root);

        if (root->data.T.tid == AND) {
            fprintf(asmFile, "\tMOV EAX, [%s]\n", leftchild->entry->id.lexeme);
            fprintf(asmFile, "\tAND EAX, [%s]\n", rightchild->entry.id.lexeme);
            fprintf(asmFile, "\tMOV [%s], EAX\n", tmp);
        }

        else {
            fprintf(asmFile, "\tMOV EAX, [%s]\n", leftchild->entry->id.lexeme);
            fprintf(asmFile, "\tOR  EAX, [%s]\n", rightchild->entry.id.lexeme);
            fprintf(asmFile, "\tMOV [%s], EAX\n", tmp);
        }
    }

    else if(root->TorNT == 0 && (root->data.T.tid == GT || root->data.T.tid == GE || root->data.T.tid == LT || root->data.T.tid == LE || root->data.T.tid == EQ || root->data.T.tid == NE)) {
        astnode* leftchild = root->children;
        astnode* rightchild = leftchild->sibling;

        generateASM(leftchild);
        generateASM(rightchild);

        char* tmp = generateTemporary(root);
        char* label1 = generateLabel();
        char* label2 = generateLabel();
        char* exitLabel = generateLabel();

        if (leftchild->datatype.tid == INTEGER) {
            fprintf(asmFile, "\tMOV  EAX, [%s]\n", leftchild->entry->id.lexeme); // load 
            fprintf(asmFile, "\tCMP  EAX, [%s]\n", rightchild->entry->id.lexeme); // compare 

            if (root->data.T.tid == GT) {
                fprintf(asmFile, "\tJG %s\n", label1);
                fprintf(asmFile, "\tJLE %s\n", label2);
            }

            if (root->data.T.tid == GE) {
                fprintf(asmFile, "\tJGE %s\n", label1);
                fprintf(asmFile, "\tJL %s\n", label2);
            }

            if (root->data.T.tid == LT) {
                fprintf(asmFile, "\tJL %s\n", label1);
                fprintf(asmFile, "\tJGE %s\n", label2);
            }

            if (root->data.T.tid == LE) {
                fprintf(asmFile, "\tJLE %s\n", label1);
                fprintf(asmFile, "\tJG %s\n", label2);
            }

            if (root->data.T.tid == EQ) {
                fprintf(asmFile, "\tJE %s\n", label1);
                fprintf(asmFile, "\tJNE %s\n", label2);
            }

            if (root->data.T.tid == NE) {
                fprintf(asmFile, "\tJNE %s\n", label1);
                fprintf(asmFile, "\tJE %s\n", label2);
            }

            fprintf(asmFile, "%s: \n", label1); // true label
            fprintf(asmFile, "\tMOV EAX, 1\n");
            fprintf(asmFile, "\tMOV [%s], EAX\n", tmp);
            fprintf(asmFile, "\tJMP %s\n", exitLabel);
            fprintf(asmFile, "%s: \n", label2); // false label
            fprintf(asmFile, "\tMOV EAX, 0\n");
            fprintf(asmFile, "\tMOV [%s], EAX\n", tmp);
            fprintf(asmFile, "%s: \n", exitLabel);
        }

        // floating point comparison
        else {

        }

    }
    
    // treating integer and boolean the same way
    else if (root->TorNT == 1 && root->data.NT.ntid == conditionalStmt) {
        currentIdTable = &(root->scopeTable);
        astnode* id = root->children;
        astnode* caseStatements = id->sibling;
        astnode* default_ast = caseStatements->sibling;
        astnode* tmp = caseStatements->children;
        
        fprintf(asmFile, "\tMOV EAX, [%s]\n", id->entry->name);
        
        int numCases = 0;
        while (tmp) {
            numCases++;
            tmp = tmp->sibling->sibling;
        }

        tmp = caseStatements->children;
        int arr[numCases];
        char* labels[numCases];
        
        for (int i = 0; i < num_cases; i++) {
            arr[i] = atoi(tmp->data.T.lexeme);
            labels[i] = generateLabel();
            fprintf(asmFile, "\tCMP [%s], EAX\n");
            fprintf(asmFile, "\tJE %s\n", labels[i]);
        }

        char* exitLabel = generateLabel();
        fprintf(asmFile, "\tJMP %s\n", exitLabel);

        for (int i = 0; i < numCases; i++) {
            tmp = tmp->sibling;
            fprintf(asmFile, "%s: \n", labels[i]);
            generateASM(tmp);
            tmp = tmp->sibling->sibling;
        }

        fprintf(asmFile, "%s: \n", exitLabel);
        if (default_ast) 
            generateASM(default_ast);

        currentIdTable = currentIdTable->parent;
    }

    else if (root->TorNT == 1 && root->data.NT.ntid == iterativeStmt && root->children->TorNT == 0 && root->children->data.T.tid == FOR) {
        currentIdTable = &(root->scopeTable);
        astnode* id = root->children->sibling;
        astnode* range_ast = root->children->sibling->sibling;
        astnode* stmts = root->children->sibling->sibling->sibling;
        
        int num1 = atoi(range_ast->children->data.T.lexeme);
        int num2 = atoi(range_ast->children->sibling->data.T.lexeme);
        char* label = generateLabel();

        fprintf(asmFile, "\tPUSH ECX\n");
        fprintf(asmFile, "\tMOV ECX, %d\n", num1);
        fprintf(asmFile, "%s: \n", label);

        generateASM(stmts);
        fprintf(asmFile, "\tINC ECX\n")
        fprintf(asmFile, "\tCMP ECX, %d\n", num2);
        fprintf(asmFile, "\tJNE %s\n", label);
        fprintf(asmFile, "\tPOP ECX\n");
        currentIdTable = currentIdTable->parent;
    }

    else if (root->TorNT == 1 && root->data.NT.ntid == iterativeStmt && root->children->TorNT == 0 && root->children->data.T.tid == WHILE) {
        currentIdTable = &(root->scopeTable);
        astnode* abexpr = root->children->sibling;
        astnode* stmts = abexpr->sibling;

        char* label = generateLabel();
        char* exitLabel = generateLabel();

        fprintf(asmFile, "%s: \n", label);
        generateASM(abexpr);
        fprintf(asmFile, "\tMOV EAX, [%s]\n", abexpr->entry->name);
        fprintf(asmFile, "\tCMP EAX, 1\n");
        fprintf(asmFile, "\tJNE %s\n", exitLabel);
        
        generateASM(stmts);
        fprintf(asmFile, "\tJMP %s\n", label);
        fprintf(asmFile, "%s: \n", exitLabel);

        currentIdTable = currentIdTable->parent;
    }

    else if (root->TorNT == 0  && root->data.T.tid == NUM) {
        char* tmp = generateTemporary(root);
        fprintf(asmFile, "\tMOV EAX, %s\n", root->data.T.lexeme); 
        fprintf(asmFile, "\tMOV [%s], EAX\n", tmp); 
    }

    else if (root->TorNT == 0  && root->data.T.tid == RNUM) {
        char* tmp = generateTemporary(root);
        fprintf(asmFile, "\tMOV RAX, %s\n", root->data.T.lexeme); 
        fprintf(asmFile, "\tMOV [%s], RAX\n", tmp); 
    }

    else if (root->TorNT == 0  && root->data.T.tid == TRUE) {
        char* tmp = generateTemporary(root);
        fprintf(asmFile, "\tMOV EAX, 1\n", root->data.T.lexeme); 
        fprintf(asmFile, "\tMOV [%s], EAX\n", tmp); 
    }

    else if (root->TorNT == 0  && root->data.T.tid == FALSE) {
        char* tmp = generateTemporary(root);
        fprintf(asmFile, "\tMOV EAX, 0\n", root->data.T.lexeme); 
        fprintf(asmFile, "\tMOV [%s], EAX\n", tmp); 
    }
}