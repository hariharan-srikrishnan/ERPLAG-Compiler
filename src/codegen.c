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


/*
    TODO
        1. Change symbol tables so the temporaries can be handled (using scopeTable) - DONE?
        2. Link temporaries to symbol table somehow - create relevant offsets (DONE?)
        3. Move integer, real to registers - DONE
        4. What to do after CMP
        5. Floating point comparison?
*/
void generateASM(astnode* root) {
    
    if(root == NULL) 
		return;

    if (semanticError == 1)
        exit(1);

    if (root->TorNT == 1 && root->data.NT.ntid == program) {
        fprintf(asmFile, "extern _printf\n");
        fprintf(asmFile, "extern _scanf\n");
    }

    else if (root->TorNT == 1 && root->data.NT.ntid == moduleDeclarations) {
    }

    else if (root->TorNT == 1 && root->data.NT.ntid == moduleDeclaration) {
    }

    else if (root->TorNT == 1 && root->data.NT.ntid == otherModules) {
    }

    else if (root->TorNT == 1 && root->data.NT.ntid == driverModule) {
    }

    else if (root->TorNT == 1 && root->data.NT.ntid == module) {
    }

    else if (root->TorNT == 1 && root->data.NT.ntid == moduleDef) {
    }

    else if (root->TorNT == 1 && root->data.NT.ntid == statements) {
    }

    else if (root->TorNT == 1 && root->data.NT.ntid == ioStmt && root->children->TorNT == 0 && root->children->data.T.tid == GET_VALUE) {
    }

    else if (root->TorNT == 1 && root->data.NT.ntid == ioStmt && root->children->TorNT == 0 && root->children->data.T.tid == PRINT) {
    }

    else if (root->TorNT == 1 && root->data.NT.ntid == simpleStmt) {
    }
    
    else if (root->TorNT == 1 && root->data.NT.ntid == assignmentStmt) {
    }

    else if (root->TorNT == 1 && root->data.NT.ntid == moduleReuseStmt) {
    }

    else if (root->TorNT == 1 && root->data.NT.ntid == expression) {
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
                fprintf(asmFile, "\tADD EAX, [%s]\n", rightchild->entry.id.lexeme);
                fprintf(asmFile, "\tMOV [%s], EAX\n", tmp);
            }

            // real
            else {
                fprintf(asmFile, "\tFLD  QWORD [%s]\n", leftchild->entry->id.lexeme); // load (pushed on flt pt stack, st0)
                fprintf(asmFile, "\tFADD QWORD [%s]\n", rightchild->entry->id.lexeme); // floating add (to st0)
                fprintf(asmFile, "\tFSTP QWORD [%s]\n", tmp); // store product into c (pop flt pt stack)
            }
        }

        // subtraction
        else if (root->data.T.tid == MINUS) {

            if (root->datatype.tid == INTEGER) {
                fprintf(asmFile, "\tMOV EAX, [%s]\n", leftchild->entry->id.lexeme);
                fprintf(asmFile, "\tSUB EAX, [%s]\n", rightchild->entry->id.lexeme);
                fprintf(asmFile, "\tMOV [%s], EAX\n", tmp);
            }

            // real
            else {
                fprintf(asmFile, "\tFLD  QWORD [%s]\n", leftchild->entry->id.lexeme); // load (pushed on flt pt stack, st0)
                fprintf(asmFile, "\tFSUB QWORD [%s]\n", rightchild->entry->id.lexeme); // floating sub (to st0)
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
                fprintf(asmFile, "\tFMUL QWORD [%s]\n", rightchild->entry->id.lexeme); // floating multiply (to st0)
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
                fprintf(asmFile, "\tFDIV QWORD [%s]\n", rightchild->entry->id.lexeme); // floating div (to st0)
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
            fprintf(asmFile, "\tMOV AH, [%s]\n", leftchild->entry->id.lexeme);
            fprintf(asmFile, "\tAND AH, [%s]\n", rightchild->entry.id.lexeme);
            fprintf(asmFile, "\tMOV [%s], AH\n", tmp);
        }

        else {
            fprintf(asmFile, "\tMOV AH, [%s]\n", leftchild->entry->id.lexeme);
            fprintf(asmFile, "\tOR  AH, [%s]\n", rightchild->entry.id.lexeme);
            fprintf(asmFile, "\tMOV [%s], AH\n", tmp);
        }
    }

    else if(root->TorNT == 0 && (root->data.T.tid == GT || root->data.T.tid == GE || root->data.T.tid == LT || root->data.T.tid == LE || root->data.T.tid == EQ || root->data.T.tid == NE)) {
        astnode* leftchild = root->children;
        astnode* rightchild = leftchild->sibling;

        generateASM(leftchild);
        generateASM(rightchild);

        char* tmp = generateTemporary(root);

        if (leftchild->datatype.tid == INTEGER) {
            fprintf(asmFile, "\tMOV  EAX, [%s]\n", leftchild->entry->id.lexeme); // load 
            fprintf(asmFile, "\tCMP  EAX, [%s]\n", rightchild->entry->id.lexeme); // compare 
            fprintf(asmFile, "\tMOV  [%s], EAX\n", tmp); // load 
        }

        else {

        }

    }
    
    else if (root->TorNT == 1 && root->data.NT.ntid == declareStmt) {
    }

    else if (root->TorNT == 1 && root->data.NT.ntid == conditionalStmt) {
    }

    else if (root->TorNT == 1 && root->data.NT.ntid == caseStmts) {
    }

    else if (root->TorNT == 1 && root->data.NT.ntid == iterativeStmt && root->children->TorNT == 0 && root->children->data.T.tid == FOR) {
    }

    else if (root->TorNT == 1 && root->data.NT.ntid == iterativeStmt && root->children->TorNT == 0 && root->children->data.T.tid == WHILE) {
    }

	else if (root->TorNT == 0 && root->data.T.tid == ID) {
        
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
        fprintf(asmFile, "\tMOV AH, 1\n", root->data.T.lexeme); 
        fprintf(asmFile, "\tMOV [%s], AH\n", tmp); 
    }

    else if (root->TorNT == 0  && root->data.T.tid == FALSE) {
        char* tmp = generateTemporary(root);
        fprintf(asmFile, "\tMOV AH, 0\n", root->data.T.lexeme); 
        fprintf(asmFile, "\tMOV [%s], AH\n", tmp); 
    }
}