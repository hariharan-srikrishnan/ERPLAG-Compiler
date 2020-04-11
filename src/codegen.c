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
        entry.type.primitive.width = 4;
    
    currentOffset += entry.type.primitive.width;
    *currentIdTable = insertId(*currentIdTable, entry);
    root->entry = &entry;
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
        3. Floating point comparison and arithmetic?? - DONE
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
        fprintf(asmFile, "\trunTimeError: db \"Run-time error: Bound values error\", 10, 0\n"); // newline, null terminator
        fprintf(asmFile, "\n\n");
        
        fprintf(asmFile, "section .text\n");
        fprintf(asmFile, "extern _printf\n");
        fprintf(asmFile, "extern _scanf\n");
        fprintf(asmFile, "global _main\n");

        fprintf(asmFile, "_error: \n");
        fprintf(asmFile, "\tPUSH runTimeError\n");
        fprintf(asmFile, "\tCALL _printf\n");
        fprintf(asmFile, "\tMOV AL, 1\n"); // Function 1: exit()
        fprintf(asmFile, "\tMOV EBX, 0\n"); // Return code
        fprintf(asmFile, "\tINT 80h\n");
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
        fprintf(asmFile, "_main: \n");
        fprintf(asmFile, "\tPUSH EBP\n");
        fprintf(asmFile, "\tMOV EBP, ESP\n");
        generateASM(root->children->sibling); 
        fprintf(asmFile, "\tMOV EAX, 0\n");
        fprintf(asmFile, "\tRET\n");
        currentIdTable = currentIdTable->parent;
    }

    // push all parameters and all that?
    else if (root->TorNT == 1 && root->data.NT.ntid == module) {
        astnode* iplist = root->children->sibling;
        astnode* ret_ast = NULL;
        astnode* moduledef;
        astnode* funcName = root->children;

        // no return values
        if (root->children->sibling->sibling->TorNT == 1 && root->children->sibling->sibling->data.NT.ntid == moduleDef) 
            moduledef = root->children->sibling->sibling;
        
        // output parameters exist
        else {
            ret_ast = root->children->sibling->sibling;
            moduledef = ret_ast->sibling;
        }

        currentIdTable = &(root->scopeTable);

        fprintf(asmFile, "%s: \n", funcName->data.T.lexeme);
        fprintf(asmFile, "\tPUSH EBP\n");
        fprintf(asmFile, "\tMOV EBP, ESP\n");

        generateASM(moduledef);
        fprintf(asmFile, "\tLEAVE\n");
        fprintf(asmFile, "\tRET\n");

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

            fprintf(asmFile, "\tPUSH EBP - 2 - %d\n", root->entry->offset);

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

        // static arrays 
        else if (root->entry->AorP == 1 && root->entry->type.array.dynamicArray == 0) {
            int lb = atoi(root->entry->type.array.lowerBound.lexeme);
            int ub = atoi(root->entry->type.array.upperBound.lexeme);
            int length = ub - lb + 1;
            int width = root->entry->type.array.datatype.width;
            
            char* label = generateLabel();
            char* exitLabel = generateLabel();

            fprintf(asmFile, "\tMOV r8w, %d\n", lb);
            fprintf(asmFile, "\tMOV r9w, 0\n");
            fprintf(asmFile, "%s: \n", label);
            fprintf(asmFile, "\tCMP ECX, %d\n", ub);
            fprintf(asmFile, "\tJE %s\n", exitLabel);
            fprintf(asmFile, "\tPUSH EBP - 2 - r9w * %d\n", root->entry->offset, width);

            if (root->entry->type.array.datatype.datatype.tid == INTEGER || root->entry->type.array.datatype.datatype.tid == BOOLEAN) 
                fprintf(asmFile, "\tPUSH integerRead\n");

            else if (root->entry->type.array.datatype.datatype.tid == REAL) 
                fprintf(asmFile, "\tPUSH realRead\n");

            fprintf(asmFile, "\tCALL _scanf\n");
            fprintf(asmFile, "\tADD ESP, 8\n"); // remove parameters from stack
            fprintf(asmFile, "\tINC r8w\n");
            fprintf(asmFile, "\tINC r9w\n");
            fprintf(asmFile, "\tJMP %s\n", label);
            fprintf(asmFile, "%s: \n", exitLabel);
        } 

        // dynamic arrays
        else {
            char* lb = entry->type.array.lowerBound.lexeme;
            char* ub = entry->type.array.upperBound.lexeme;
            idSymbolTable* tmp = currentIdTable;
            int width = entry->type.array.datatype.width;
            char* label = generateLabel();

            // only upper bound is dynamic
            if (entry->type.array.lowerBound.tid == NUM) {
                symbolTableIdEntry* entry = NULL; 

                while (1) {
                    entry = searchId(*tmp, ub);
                    
                    // found in the symbol table
                    if (entry != NULL)
                        break;
                    
                    tmp = tmp->parent;
                }

                fprintf(asmFile, "\tMOV r8w, %d\n", atoi(lb)); // lower bound
                fprintf(asmFile, "\tMOV r9w, [EBP - 2 - %d]\n", entry->offset); // upper bound

                // dynamic bound check
                fprintf(asmFile, "\tCMP r8w, r9w\n");
                fprintf(asmFile, "\tJL _error\n");

                fprintf(asmFile, "\tMOV r10w, 0\n"); // count
                fprintf(asmFile, "%s: \n", label);
                fprintf(asmFile, "\tPUSH EBP - 2 - %d - r10w * %d\n", entry->offset, width);


                if (root->entry->type.array.datatype.datatype.tid == INTEGER || root->entry->type.array.datatype.datatype.tid == BOOLEAN) 
                    fprintf(asmFile, "\tPUSH integerRead\n");

                else if (root->entry->type.array.datatype.datatype.tid == REAL) 
                    fprintf(asmFile, "\tPUSH realRead\n");

                fprintf(asmFile, "\tCALL _scanf\n");
                fprintf(asmFile, "\tADD ESP, 8\n"); // remove param
                fprintf(asmFile, "\tINC r8w\n");
                fprintf(asmFile, "\tINC r10w\n");
                fprintf(asmFile, "\tCMP r8w, r9w\n");
                fprintf(asmFile, "\tJNE %s\n", label);
            }

            // only lower bound is dynamic
            else if (entry->type.array.upperBound.tid == NUM) {
                symbolTableIdEntry* entry = NULL; 

                while (1) {
                    entry = searchId(*tmp, lb);
                    
                    // found in the symbol table
                    if (entry != NULL)
                        break;
                    
                    tmp = tmp->parent;
                }
                
                fprintf(asmFile, "\tMOV r8w, [EBP - 2 - %d]\n", entry->offset); // lower bound
                fprintf(asmFile, "\tMOV r9w, %d\n", atoi(ub)); // upper bound

                // dynamic bound check
                fprintf(asmFile, "\tCMP r8w, r9w\n");
                fprintf(asmFile, "\tJL _error\n");

                fprintf(asmFile, "\tMOV r10w, 0\n"); // count
                fprintf(asmFile, "%s: \n", label);
                fprintf(asmFile, "\tPUSH EBP - 2 - %d - r10w * %d\n", entry->offset, width);

                if (root->entry->type.array.datatype.datatype.tid == INTEGER || root->entry->type.array.datatype.datatype.tid == BOOLEAN) 
                    fprintf(asmFile, "\tPUSH integerRead\n");

                else if (root->entry->type.array.datatype.datatype.tid == REAL) 
                    fprintf(asmFile, "\tPUSH realRead\n");

                fprintf(asmFile, "\tCALL _scanf\n");
                fprintf(asmFile, "\tADD ESP, 8\n"); // remove param
                fprintf(asmFile, "\tINC r8w\n");
                fprintf(asmFile, "\tINC r10w\n");
                fprintf(asmFile, "\tCMP r8w, r9w\n");
                fprintf(asmFile, "\tJNE %s\n", label);
            }

            else {
                symbolTableIdEntry* lowerEntry = NULL;
                symbolTableIdEntry* upperEntry = NULL;

                while (1) {
                    lowerEntry = searchId(*tmp, lb);
                    
                    // found in the symbol table
                    if (lowerEntry != NULL)
                        break;
                    
                    tmp = tmp->parent;
                }

                while (1) {
                    upperEntry = searchId(*tmp, ub);
                    
                    // found in the symbol table
                    if (upperEntry != NULL)
                        break;
                    
                    tmp = tmp->parent;
                }

                fprintf(asmFile, "\tMOV r8w, [EBP - 2 - %d]\n", lowerEntry->offset); // lower bound
                fprintf(asmFile, "\tMOV r9w, [EBP - 2 - %d]\n", upperEntry->offset); // upper bound

                // dynamic bound check
                fprintf(asmFile, "\tCMP r8w, r9w\n");
                fprintf(asmFile, "\tJL _error\n");

                fprintf(asmFile, "\tMOV r10w, 0\n"); // count
                fprintf(asmFile, "%s: \n", label);
                fprintf(asmFile, "\tPUSH EBP - 2 - %d - r10w * %d\n", lowerEntry->offset, width);

                if (root->entry->type.array.datatype.datatype.tid == INTEGER || root->entry->type.array.datatype.datatype.tid == BOOLEAN) 
                    fprintf(asmFile, "\tPUSH integerRead\n");

                else if (root->entry->type.array.datatype.datatype.tid == REAL) 
                    fprintf(asmFile, "\tPUSH realRead\n");

                fprintf(asmFile, "\tCALL _scanf\n");
                fprintf(asmFile, "\tADD ESP, 8\n"); // remove param
                fprintf(asmFile, "\tINC r8w\n");
                fprintf(asmFile, "\tINC r10w\n");
                fprintf(asmFile, "\tCMP r8w, r9w\n");
                fprintf(asmFile, "\tJNE %s\n", label);
            }
        }
    }

    else if (root->TorNT == 1 && root->data.NT.ntid == ioStmt && root->children->TorNT == 0 && root->children->data.T.tid == PRINT) {
         
        // primitive datatype
        if (root->entry->AorP == 0) {
            
            fprintf(asmFile, "\tPUSH EBP - 2 - %d\n", root->entry->offset);

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

        // static arrays 
        else if (root->entry->AorP == 1 && root->entry->type.array.dynamicArray == 0) {
            int lb = atoi(root->entry->type.array.lowerBound.lexeme);
            int ub = atoi(root->entry->type.array.upperBound.lexeme);
            int length = ub - lb + 1;
            int width = root->entry->type.array.datatype.width;
            
            char* label = generateLabel();
            char* exitLabel = generateLabel();

            fprintf(asmFile, "\tMOV r8w, %d\n", lb);
            fprintf(asmFile, "\tMOV r9w, 0\n");
            fprintf(asmFile, "%s: \n", label);
            fprintf(asmFile, "\tCMP ECX, %d\n", ub);
            fprintf(asmFile, "\tJE %s\n", exitLabel);
            fprintf(asmFile, "\tPUSH %s + r9w * %d\n", root->entry->name, width);

            if (root->entry->type.array.datatype.datatype.tid == INTEGER) 
                fprintf(asmFile, "\tPUSH integerWrite\n");

            else if (root->entry->type.array.datatype.datatype.tid == REAL) 
                fprintf(asmFile, "\tPUSH realWrite\n");

            // boolean: assume you can scan only 1 or 0
            else 
                fprintf(asmFile, "\tPUSH integerWrite\n");

            fprintf(asmFile, "\tCALL _printf\n");
            fprintf(asmFile, "\tADD ESP, 8\n"); // remove parameters from stack
            fprintf(asmFile, "\tINC r8w\n");
            fprintf(asmFile, "\tINC r9w\n");
            fprintf(asmFile, "\tJMP %s\n", label);
            fprintf(asmFile, "%s: \n", exitLabel);
        }

         // dynamic arrays
        else {
            char* lb = entry->type.array.lowerBound.lexeme;
            char* ub = entry->type.array.upperBound.lexeme;
            idSymbolTable* tmp = currentIdTable;
            int width = entry->type.array.datatype.width;
            char* label = generateLabel();

            // only upper bound is dynamic
            if (entry->type.array.lowerBound.tid == NUM) {
                symbolTableIdEntry* entry = NULL; 

                while (1) {
                    entry = searchId(*tmp, ub);
                    
                    // found in the symbol table
                    if (entry != NULL)
                        break;
                    
                    tmp = tmp->parent;
                }

                fprintf(asmFile, "\tMOV r8w, %d\n", atoi(lb)); // lower bound
                fprintf(asmFile, "\tMOV r9w, [EBP - 2 - %d]\n", entry->offset); // upper bound

                // dynamic bound check
                fprintf(asmFile, "\tCMP r8w, r9w\n");
                fprintf(asmFile, "\tJL _error\n");

                fprintf(asmFile, "\tMOV r10w, 0\n"); // count
                fprintf(asmFile, "%s: \n", label);
                fprintf(asmFile, "\tPUSH EBP - 2 - %d - r10w * %d\n", entry->offset, width);


                if (root->entry->type.array.datatype.datatype.tid == INTEGER || root->entry->type.array.datatype.datatype.tid == BOOLEAN) 
                    fprintf(asmFile, "\tPUSH integerWrite\n");

                else if (root->entry->type.array.datatype.datatype.tid == REAL) 
                    fprintf(asmFile, "\tPUSH realWrite\n");

                fprintf(asmFile, "\tCALL _printf\n");
                fprintf(asmFile, "\tADD ESP, 8\n"); // remove param
                fprintf(asmFile, "\tINC r8w\n");
                fprintf(asmFile, "\tINC r10w\n");
                fprintf(asmFile, "\tCMP r8w, r9w\n");
                fprintf(asmFile, "\tJNE %s\n", label);
            }

            // only lower bound is dynamic
            else if (entry->type.array.upperBound.tid == NUM) {
                symbolTableIdEntry* entry = NULL; 

                while (1) {
                    entry = searchId(*tmp, lb);
                    
                    // found in the symbol table
                    if (entry != NULL)
                        break;
                    
                    tmp = tmp->parent;
                }
                
                fprintf(asmFile, "\tMOV r8w, [EBP - 2 - %d]\n", entry->offset); // lower bound
                fprintf(asmFile, "\tMOV r9w, %d\n", atoi(ub)); // upper bound

                // dynamic bound check
                fprintf(asmFile, "\tCMP r8w, r9w\n");
                fprintf(asmFile, "\tJL _error\n");

                fprintf(asmFile, "\tMOV r10w, 0\n"); // count
                fprintf(asmFile, "%s: \n", label);
                fprintf(asmFile, "\tPUSH EBP - 2 - %d - r10w * %d\n", entry->offset, width);

                if (root->entry->type.array.datatype.datatype.tid == INTEGER || root->entry->type.array.datatype.datatype.tid == BOOLEAN) 
                    fprintf(asmFile, "\tPUSH integerWrite\n");

                else if (root->entry->type.array.datatype.datatype.tid == REAL) 
                    fprintf(asmFile, "\tPUSH realWrite\n");

                fprintf(asmFile, "\tCALL _printf\n");
                fprintf(asmFile, "\tADD ESP, 8\n"); // remove param
                fprintf(asmFile, "\tINC r8w\n");
                fprintf(asmFile, "\tINC r10w\n");
                fprintf(asmFile, "\tCMP r8w, r9w\n");
                fprintf(asmFile, "\tJNE %s\n", label);
            }

            else {
                symbolTableIdEntry* lowerEntry = NULL;
                symbolTableIdEntry* upperEntry = NULL;

                while (1) {
                    lowerEntry = searchId(*tmp, lb);
                    
                    // found in the symbol table
                    if (lowerEntry != NULL)
                        break;
                    
                    tmp = tmp->parent;
                }

                while (1) {
                    upperEntry = searchId(*tmp, ub);
                    
                    // found in the symbol table
                    if (upperEntry != NULL)
                        break;
                    
                    tmp = tmp->parent;
                }

                fprintf(asmFile, "\tMOV r8w, [EBP - 2 - %d]\n", lowerEntry->offset); // lower bound
                fprintf(asmFile, "\tMOV r9w, [EBP - 2 - %d]\n", upperEntry->offset); // upper bound

                // dynamic bound check
                fprintf(asmFile, "\tCMP r8w, r9w\n");
                fprintf(asmFile, "\tJL _error\n");

                fprintf(asmFile, "\tMOV r10w, 0\n"); // count
                fprintf(asmFile, "%s: \n", label);
                fprintf(asmFile, "\tPUSH EBP - 2 - %d - r10w * %d\n", lowerEntry->offset, width);

                if (root->entry->type.array.datatype.datatype.tid == INTEGER || root->entry->type.array.datatype.datatype.tid == BOOLEAN) 
                    fprintf(asmFile, "\tPUSH integerWrite\n");

                else if (root->entry->type.array.datatype.datatype.tid == REAL) 
                    fprintf(asmFile, "\tPUSH realWrite\n");

                fprintf(asmFile, "\tCALL _printf\n");
                fprintf(asmFile, "\tADD ESP, 8\n"); // remove param
                fprintf(asmFile, "\tINC r8w\n");
                fprintf(asmFile, "\tINC r10w\n");
                fprintf(asmFile, "\tCMP r8w, r9w\n");
                fprintf(asmFile, "\tJNE %s\n", label);
            }
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

                if (root->entry->type.array.datatype.datatype.tid == INTEGER || root->entry->type.array.datatype.datatype.tid == BOOLEAN) {
                    fprintf(asmFile, "\tMOV EAX, DWORD [EBP - 2 - %d]\n", id->sibling->entry->offset);
                    fprintf(asmFile, "\tMOV DWORD [EBP - 2 - %d], EAX\n", id->entry->offset);
                }

                // real
                else {
                    fprintf(asmFile, "\tMOV RAX, QWORD [EBP - 2 - %d]\n", id->sibling->entry->offset);
                    fprintf(asmFile, "\tMOV QWORD [EBP - 2 - %d], RAX\n", id->entry->offset);
                }
            }

            // assigning one array to another
            else {
                fprintf(asmFile, "\tMOV EAX, EBP - 2 - %d\n", id->sibling->entry->offset);
                fprintf(asmFile, "\tMOV EBP - 2 - %d, EAX\n", id->entry->offset);

                // char* label = generateLabel();
                // char* exitLabel = generateLabel();
                // int width = root->entry->type.array.datatype.width;

                // fprintf(asmFile, "\tMOV  ECX\n");
                // fprintf(asmFile, "\tMOV ECX, 0\n");
                // fprintf(asmFile, "%s: \n", label);
                // fprintf(asmFile, "\tCMP ECX, %d\n", length);
                // fprintf(asmFile, "\tJE %s\n", exitLabel);

                // if (root->entry->type.array.datatype.datatype.tid == INTEGER) {
                //     fprintf(asmFile, "\tMOV EAX, [%s + ECX * %d]\n", id->sibling->entry->name, width);
                //     fprintf(asmFile, "\tMOV [%s + ECX * %d], EAX\n", id->entry->name, width);
                // }

                // else if (root->entry->type.array.datatype.datatype.tid == REAL) {
                //     fprintf(asmFile, "\tMOV RAX, [%s + ECX * %d]\n", id->sibling->entry->name, width);
                //     fprintf(asmFile, "\tMOV [%s + ECX * %d], RAX\n", id->entry->name, width);
                // }

                // // boolean
                // else {
                //     fprintf(asmFile, "\tMOV EAX, [%s + ECX * %d]\n", id->sibling->entry->name, width);
                //     fprintf(asmFile, "\tMOV [%s + ECX * %d], EAX\n", id->entry->name, width);
                // }
                
                // fprintf(asmFile, "\tINC ECX\n");
                // fprintf(asmFile, "\tJMP %s\n", label);
                // fprintf(asmFile, "\tPOP ECX\n");
                // fprintf(asmFile, "%s: \n", exitLabel);
            }
            
        }

        // array with index
        else {
            astnode* idx = id->sibling;
            int width = root->entry->type.array.datatype.width;

            if (root->entry->type.array.datatype.datatype.tid == INTEGER || root->entry->type.array.datatype.datatype.tid == BOOLEAN) {
                fprintf(asmFile, "\tMOV EAX, DWORD [EBP - 2 - %d]\n", id->sibling->entry->offset);
                fprintf(asmFile, "\tMOV r8w, DWORD [EBP - 2 - %d]\n", idx->sibling->entry->offset);
                fprintf(asmFile, "\tMOV DWORD [EBP - 2 - %d  - r8w], EAX\n", id->entry->offset);
            }

            else {
                fprintf(asmFile, "\tMOV RAX, QWORD [EBP - 2 - %d]\n", id->sibling->entry->offset);
                fprintf(asmFile, "\tMOV r8d, QWORD [EBP - 2 - %d]\n", idx->sibling->entry->offset);
                fprintf(asmFile, "\tMOV QWORD [EBP - 2 - %d  - r8d], RAX\n", id->entry->offset);
            }
        }
    }

    // push paramaters onto stack in opposite order, pop based on size later
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

        astnode* idlist = functionName->sibling;
        idSymbolTable* storeCurrTable = currentIdTable;
        currentIdTable = &(functionName->scopeTable);

        symbolTableFuncEntry* entry = searchFunc(funcTable, functionName->data.T.lexeme);
        parameters* tmp;

        for (int i = 0; i < entry->numInputParams; i++) {
            tmp = entry->inputParameters;

            // reach i'th last parameter
            for(int j = 0; j < i; j++) 
                tmp = tmp->next;
            
            fprintf(asmFile, "\tPUSH DWORD [%s]\n", tmp->id.lexeme); // push all input parameters in reverse order onto stack
            // if (tmp->datatype.tid/
        }
        
        fprintf(asmFile, "\tCALL %s\n", entry->name);

        // return parameters need to be stored somehow
        if (opt != NULL) {

        }

        currentIdTable = storeCurrTable;
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
                    fprintf(asmFile, "\tISUB EAX, DWORD [EBP - 2 - %d]\n", tmp->sibling->entry->offset);
                    fprintf(asmFile, "\tMOV [EBP - 2 - %d], AX\n", tmp->sibling->entry->offset);
                }

                // real 
                else {
                    // fprintf(asmFile, "\tMOV RAX, 0\n");
                    // fprintf(asmFile, "\tISUB RAX, QWORD [EBP - 2 - %d]\n", tmp->sibling->entry->offset);
                    // fprintf(asmFile, "\tMOV  QWORD [EBP - 2 - %d], RAX\n", tmp->sibling->entry->offset);
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
                fprintf(asmFile, "\tMOV  EAX, DWORD [EBP - 2 - %d]\n", leftchild->entry->offset);
                fprintf(asmFile, "\tIADD EAX, DWORD [EBP - 2 - %d]\n", rightchild->entry->offset);
                fprintf(asmFile, "\tMOV  DWORD [EBP - 2 - %d], EAX\n", root->entry->offset);
            }

            // real
            else {
                // fprintf(asmFile, "\tFLD  QWORD [%s]\n", leftchild->entry->id.lexeme); // load (pushed on flt pt stack, st0)
                // fprintf(asmFile, "\tFADD QWORD [%s]\n", rightchild->entry->id.lexeme); // floating add (to st0)
                // fprintf(asmFile, "\tFSTP QWORD [%s]\n", tmp); // store product into c (pop flt pt stack)
            }
        }

        // subtraction
        else if (root->data.T.tid == MINUS) {

            if (root->datatype.tid == INTEGER) {
                fprintf(asmFile, "\tMOV  EAX, DWORD [EBP - 2 - %d]\n", leftchild->entry->offset);
                fprintf(asmFile, "\tISUB EAX, DWORD [EBP - 2 - %d]\n", rightchild->entry->offset);
                fprintf(asmFile, "\tMOV  DWORD [EBP - 2 - %d], EAX\n", root->entry->offset);
            }

            // real
            else {
                // fprintf(asmFile, "\tFLD  QWORD [%s]\n", leftchild->entry->id.lexeme); // load (pushed on flt pt stack, st0)
                // fprintf(asmFile, "\tFSUB QWORD [%s]\n", rightchild->entry->id.lexeme); // floating sub (to st0)
                // fprintf(asmFile, "\tFSTP QWORD [%s]\n", tmp); // store product into c (pop flt pt stack)
            }
        }

        // multiplication
        else if (root->data.T.tid == MUL) {

            if (root->datatype.tid == INTEGER) {
                fprintf(asmFile, "\tMOV  EAX, DWORD [EBP - 2 - %d]\n", leftchild->entry->offset); // load (must be eax for multiply)
                fprintf(asmFile, "\tIMUL DWORD [EBP - 2 - %d]\n", rightchild->entry->offset); // signed integer multiply
                fprintf(asmFile, "\tMOV  DWORD [EBP - 2 - %d], EAX\n", root->entry->offset); // store bottom half of product
            }

            // real
            else {
            //     fprintf(asmFile, "\tFLD  QWORD [%s]\n", leftchild->entry->id.lexeme); // load (pushed on flt pt stack, st0)
            //     fprintf(asmFile, "\tFMUL QWORD [%s]\n", rightchild->entry->id.lexeme); // floating multiply (to st0)
            //     fprintf(asmFile, "\tFSTP QWORD [%s]\n", tmp); // store product into c (pop flt pt stack)
            }
        }

        // division
        else if (root->data.T.tid == DIV) {

            if (root->datatype.tid == INTEGER) {
                fprintf(asmFile, "\tMOV  EAX, DWORD [EBP - 2 - %d]\n", leftchild->entry->offset); // load 
                fprintf(asmFile, "\tMOV  EDX, 0\n"); // load upper half of dividend with zero
                fprintf(asmFile, "\tIDIV DWORD [EBP - 2 - %d]\n", rightchild->entry->offset); // divide double register edx eax by a
                fprintf(asmFile, "\tMOV  DWORD [EBP - 2 - %d], EAX\n", root->entry->offset); // store quotient
            }

            // real
            else {
                // fprintf(asmFile, "\tFLD  QWORD [%s]\n", leftchild->entry->id.lexeme); // load (pushed on flt pt stack, st0)
                // fprintf(asmFile, "\tFDIV QWORD [%s]\n", rightchild->entry->id.lexeme); // floating div (to st0)
                // fprintf(asmFile, "\tFSTP QWORD [%s]\n", tmp); // store product into c (pop flt pt stack)
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
            fprintf(asmFile, "\tMOV EAX, DWORD [EBP - 2 - %d]\n", leftchild->entry->offset);
            fprintf(asmFile, "\tAND EAX, DWORD [EBP - 2 - %d]\n", rightchild->entry->offset);
            fprintf(asmFile, "\tMOV DWORD [EBP - 2 - %d], EAX\n", root->entry->offset);
        }

        else {
            fprintf(asmFile, "\tMOV EAX, DWORD [EBP - 2 - %d]\n", leftchild->entry->offset);
            fprintf(asmFile, "\tOR  EAX, DWORD [EBP - 2 - %d]\n", rightchild->entry->offset);
            fprintf(asmFile, "\tMOV DWORD [EBP - 2 - %d], EAX\n", root->entry->offset);
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
            fprintf(asmFile, "\tMOV EAX, DWORD [EBP - 2 - %d]\n", leftchild->entry->offset); // load 
            fprintf(asmFile, "\tCMP EAX, DWORD [EBP - 2 - %d]\n", rightchild->entry->offset); // compare 
        }

        // real number comparison - https://stackoverflow.com/questions/27528930/compare-two-numbers-in-intel-x86-assembly-nasm
        else {
            // fprintf(asmFile, "\tFLD  QWORD PTR [%s]\n", leftchild->entry->id.lexeme); // load 
            // fprintf(asmFile, "\tFCMP QWORD PTR [%s]\n", rightchild->entry->id.lexeme); // compare 
            // fprintf(asmFile, "\tWAIT\n");
            // fprintf(asmFile, "\tFSTSW AX\n");
            // fprintf(asmFile, "\tSAHF\n");
        }

        if (root->data.T.tid == GT) {
            fprintf(asmFile, "\tJG %s\n", label1);
            fprintf(asmFile, "\tJLE %s\n", label2);
        }

        else if (root->data.T.tid == GE) {
            fprintf(asmFile, "\tJGE %s\n", label1);
            fprintf(asmFile, "\tJL %s\n", label2);
        }

        else if (root->data.T.tid == LT) {
            fprintf(asmFile, "\tJL %s\n", label1);
            fprintf(asmFile, "\tJGE %s\n", label2);
        }

        else if (root->data.T.tid == LE) {
            fprintf(asmFile, "\tJLE %s\n", label1);
            fprintf(asmFile, "\tJG %s\n", label2);
        }

        else if (root->data.T.tid == EQ) {
            fprintf(asmFile, "\tJE %s\n", label1);
            fprintf(asmFile, "\tJNE %s\n", label2);
        }

        else if (root->data.T.tid == NE) {
            fprintf(asmFile, "\tJNE %s\n", label1);
            fprintf(asmFile, "\tJE %s\n", label2);
        }

        fprintf(asmFile, "%s: \n", label1); // true label
        fprintf(asmFile, "\tMOV EAX, 1\n");
        fprintf(asmFile, "\tMOV DWORD [EBP - 2 - %d], EAX\n", root->entry->offset);
        fprintf(asmFile, "\tJMP %s\n", exitLabel);
        fprintf(asmFile, "%s: \n", label2); // false label
        fprintf(asmFile, "\tMOV EAX, 0\n");
        fprintf(asmFile, "\tMOV DWORD [EBP - 2 - %d], EAX\n", root->entry->offset);
        fprintf(asmFile, "%s: \n", exitLabel);
    }

    else if (root->TorNT == 1 && root->data.NT.ntid == declareStmt) {
        astnode* idlist = root->children;
        astnode* itr = idlist->children;
        symbolTableIdEntry* entry;
        

        while (itr) {
            entry = searchId(*currentIdTable, itr->data.T.lexeme);

            if (entry->AorP == 0) {
                if (entry->type.primitive.datatype.tid == INTEGER || entry->type.primitive.datatype.tid == BOOLEAN) 
                    fprintf(asmFile, "\tPUSH DWORD [%s]\n", entry->name);

                else 
                    fprintf(asmFile, "\tPUSH QWORD [%s]\n", entry->name);
            }

            // static arrays 
            else if (entry->type.array.dynamicArray == 0) {
                int lb = atoi(entry->type.array.lowerBound.lexeme);
                int ub = atoi(entry->type.array.upperBound.lexeme);
                int width = entry->type.array.datatype.width;

                char* label = generateLabel();
                fprintf(asmFile, "\tMOV r8w, %d\n", lb); // bound
                fprintf(asmFile, "\tMOV r9w, 0\n"); // count
                fprintf(asmFile, "%s: \n", label);
                fprintf(asmFile, "\tPUSH [%s + r9w * %d]\n", entry->name, width);
                fprintf(asmFile, "\tINC r8w\n");
                fprintf(asmFile, "\tINC r9w\n");
                fprintf(asmFile, "\tCMP r8w, %d\n", ub);
                fprintf(asmFile, "\tJNE %s\n", label);
            } 

            // dynamic arrays
            else {
                char* lb = entry->type.array.lowerBound.lexeme;
                char* ub = entry->type.array.upperBound.lexeme;
                idSymbolTable* tmp = currentIdTable;
                int width = entry->type.array.datatype.width;
                char* label = generateLabel();

                // only upper bound is dynamic
                if (entry->type.array.lowerBound.tid == NUM) {
                    symbolTableIdEntry* entry = NULL; 

                    while (1) {
                        entry = searchId(*tmp, ub);
                        
                        // found in the symbol table
                        if (entry != NULL)
                            break;
                        
                        tmp = tmp->parent;
                    }

                    fprintf(asmFile, "\tMOV r8w, %d\n", atoi(lb)); // lower bound
                    fprintf(asmFile, "\tMOV r9w, [EBP - 2 - %d]\n", entry->offset); // upper bound

                    // dynamic bound check
                    fprintf(asmFile, "\tCMP r8w, r9w\n");
                    fprintf(asmFile, "\tJL _error\n");

                    fprintf(asmFile, "\tMOV r10w, 0\n"); // count
                    fprintf(asmFile, "%s: \n", label);
                    fprintf(asmFile, "\tPUSH [EBP - 2 - %d - r10w * %d]\n", entry->offset, width);
                    fprintf(asmFile, "\tINC r8w\n");
                    fprintf(asmFile, "\tINC r10w\n");
                    fprintf(asmFile, "\tCMP r8w, r9w\n");
                    fprintf(asmFile, "\tJNE %s\n", label);
                }

                // only lower bound is dynamic
                else if (entry->type.array.upperBound.tid == NUM) {
                    symbolTableIdEntry* entry = NULL; 

                    while (1) {
                        entry = searchId(*tmp, lb);
                        
                        // found in the symbol table
                        if (entry != NULL)
                            break;
                        
                        tmp = tmp->parent;
                    }
                    
                    fprintf(asmFile, "\tMOV r8w, [EBP - 2 - %d]\n", entry->offset); // lower bound
                    fprintf(asmFile, "\tMOV r9w, %d\n", atoi(ub)); // upper bound

                    // dynamic bound check
                    fprintf(asmFile, "\tCMP r8w, r9w\n");
                    fprintf(asmFile, "\tJL _error\n");

                    fprintf(asmFile, "\tMOV r10w, 0\n"); // count
                    fprintf(asmFile, "%s: \n", label);
                    fprintf(asmFile, "\tPUSH [EBP - 2 - %d - r10w * %d]\n", entry->offset, width);
                    fprintf(asmFile, "\tINC r8w\n");
                    fprintf(asmFile, "\tINC r10w\n");
                    fprintf(asmFile, "\tCMP r8w, r9w\n");
                    fprintf(asmFile, "\tJNE %s\n", label);
                }

                else {
                    symbolTableIdEntry* lowerEntry = NULL;
                    symbolTableIdEntry* upperEntry = NULL;

                    while (1) {
                        lowerEntry = searchId(*tmp, lb);
                        
                        // found in the symbol table
                        if (lowerEntry != NULL)
                            break;
                        
                        tmp = tmp->parent;
                    }

                    while (1) {
                        upperEntry = searchId(*tmp, ub);
                        
                        // found in the symbol table
                        if (upperEntry != NULL)
                            break;
                        
                        tmp = tmp->parent;
                    }

                    fprintf(asmFile, "\tMOV r8w, [EBP - 2 - %d]\n", lowerEntry->offset); // lower bound
                    fprintf(asmFile, "\tMOV r9w, [EBP - 2 - %d]\n", upperEntry->offset); // upper bound

                    // dynamic bound check
                    fprintf(asmFile, "\tCMP r8w, r9w\n");
                    fprintf(asmFile, "\tJL _error\n");

                    fprintf(asmFile, "\tMOV r10w, 0\n"); // count
                    fprintf(asmFile, "%s: \n", label);
                    fprintf(asmFile, "\tPUSH [EBP - 2 - %d - r10w * %d]\n", lowerEntry->offset, width);
                    fprintf(asmFile, "\tINC r8w\n");
                    fprintf(asmFile, "\tINC r10w\n");
                    fprintf(asmFile, "\tCMP r8w, r9w\n");
                    fprintf(asmFile, "\tJNE %s\n", label);
                }
            }
            itr = itr->sibling;
        }
    }
    
    // treating integer and boolean the same way
    else if (root->TorNT == 1 && root->data.NT.ntid == conditionalStmt) {
        currentIdTable = &(root->scopeTable);
        astnode* id = root->children;
        astnode* caseStatements = id->sibling;
        astnode* default_ast = caseStatements->sibling;
        astnode* tmp = caseStatements->children;
        
        fprintf(asmFile, "\tMOV EAX, DWORD [EBP - 2 - %d]\n", id->entry->offset);
        
        int numCases = 0;
        while (tmp) {
            numCases++;
            tmp = tmp->sibling->sibling;
        }

        tmp = caseStatements->children;
        int arr[numCases];
        char* labels[numCases];
        
        for (int i = 0; i < numCases; i++) {
            arr[i] = atoi(tmp->data.T.lexeme);
            labels[i] = generateLabel();
            fprintf(asmFile, "\tCMP EAX, %d\n", arr[i]);
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
        fprintf(asmFile, "\tINC ECX\n");
        fprintf(asmFile, "\tCMP ECX, %d\n", num2);
        fprintf(asmFile, "\tJNE %s\n", label);
        fprintf(asmFile, "\tDEC ECX\n");
        fprintf(asmFile, "\tMOV DWORD [EBP - 2 - %d], ECX\n", id->entry->offset);
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
        fprintf(asmFile, "\tMOV EAX, DWORD [EBP - 2 - %d]\n", abexpr->entry->offset);
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
        fprintf(asmFile, "\tMOV DWORD [EBP - 2 - %d], EAX\n", root->entry->offset); 
    }

    else if (root->TorNT == 0  && root->data.T.tid == RNUM) {
        char* tmp = generateTemporary(root);
        fprintf(asmFile, "\tMOV RAX, %s\n", root->data.T.lexeme); 
        fprintf(asmFile, "\tMOV QWORD [EBP - 2 - %d], RAX\n", root->entry->offset); 
    }

    else if (root->TorNT == 0  && root->data.T.tid == TRUE) {
        char* tmp = generateTemporary(root);
        fprintf(asmFile, "\tMOV EAX, 1\n"); 
        fprintf(asmFile, "\tMOV DWORD [EBP - 2 - %d], EAX\n", root->entry->offset); 
    }

    else if (root->TorNT == 0  && root->data.T.tid == FALSE) {
        char* tmp = generateTemporary(root);
        fprintf(asmFile, "\tMOV EAX, 0\n"); 
        fprintf(asmFile, "\tMOV DWORD [EBP - 2 - %d], EAX\n", root->entry->offset); 
    }
}