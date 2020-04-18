/*
Anirudh S Chakravarthy - 2017A7PS1195P
Hariharan Srikrishnan  - 2017A7PS0134P
Honnesh Rohmetra	   - 2016B2A70770P
Praveen Ravirathinam   - 2017A7PS1174P
*/

/*
    Good references for NASM 64-bit
        1. Really good tutorial: https://cs.lmu.edu/~ray/notes/nasmtutorial/
        2. Floating Point: https://my.eng.utah.edu/~cs4400/sse-fp.pdf
        3. Addressing modes: https://en.wikipedia.org/wiki/X86#Addressing_modes
        4. https://cs.brown.edu/courses/cs033/docs/guides/x64_cheatsheet.pdf
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "symboltableDef.h"
#include "symboltable.h"
#include "typeExtractor.h"
#include "semantics.h"


FILE* asmFile = NULL;
int num_temps = 0;
int num_labels = 0;


int findNextOffset(idSymbolTable* table) {
    int max = 0;
    idNode* itr;
    symbolTableIdEntry entry;
    for (int i = 0; i < table->hashSize; i++) {
        itr = table->list[i].head;
        while (itr) {
            if (itr->entry.offset > max) {
                max = itr->entry.offset;
                entry = itr->entry;
            }
            itr = itr->next;
        }
    }

    idSymbolTable* tmp = table->child;
    while (tmp) {
        for (int i = 0; i < tmp->hashSize; i++) {
            itr = tmp->list[i].head;
            while (itr) {
                if (itr->entry.offset > max) {
                    max = itr->entry.offset;
                    entry = itr->entry;
                }
                itr = itr->next;
            }
        }
        tmp = tmp->sibling;
    }

    if (entry.AorP == 0) 
        max += entry.type.primitive.width;

    else if (entry.type.array.dynamicArray == 0) {
        int lb = atoi(entry.type.array.lowerBound.lexeme);
        int ub = atoi(entry.type.array.upperBound.lexeme);
        max += (ub - lb + 1) * entry.type.array.datatype.width;
    }

    else 
        max += 8;

    return max;
}


// generate temporary variables for arithmetic expressions
char* generateTemporary(astnode* root) {
    char* temp = (char*) malloc(sizeof(char) * (4 + num_temps / 10 + 1 + 1)); // tmp + number of digits + ENDMARKER
    sprintf(temp, "_tmp%d", num_temps);
    num_temps++;

    token t = root->data.T;
    strcpy(t.lexeme, temp);

    symbolTableIdEntry* entry = (symbolTableIdEntry*) malloc(sizeof(symbolTableIdEntry));
    entry->id = t;
    entry->AorP = 0;
    strcpy(entry->name, temp);
    entry->type.primitive.datatype.tid = root->datatype.tid;
    entry->type.primitive.width = 8;

    entry->offset = findNextOffset(currentIdTable);
    fprintf(asmFile, "\tSUB RSP, %d\n", entry->type.primitive.width);
    
    *currentIdTable = insertId(*currentIdTable, *entry);
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
        3. Floating point comparison and arithmetic?? - DONE
        4. Module reuse statement - DONE
        5. Code generation for dynamic type checking - DONE
        6. Real numbers for dynamic/arrays - DONE
        7. Dynamic bound check between arrays - DONE
*/
void generateASM(astnode* root) {
    
    if(root == NULL) 
		return;

    if (semanticError == 1)
        exit(1);

    if (root->TorNT == 1 && root->data.NT.ntid == program) {

        // Data defintions
        fprintf(asmFile, "section .data\n\n");
        fprintf(asmFile, "integerRead db \"%%d\", 0\n");
        fprintf(asmFile, "integerWrite db \"%%d\", 10, 0\n"); // newline, null terminator
        fprintf(asmFile, "realRead db \"%%lf\", 0\n");
        fprintf(asmFile, "realWrite db \"%%lf\", 10, 0\n"); // newline, null terminator
        
        // fancy printing formats
        fprintf(asmFile, "intMessage db \"Enter an integer\", 10, 0\n");
        fprintf(asmFile, "realMessage db \"Enter an real:\", 10, 0\n");
        fprintf(asmFile, "boolMessage db \"Enter a boolean (1 or 0 only):\", 10, 0\n");

        fprintf(asmFile, "intArrMessage db \"Enter integer array for range %%d to %%d:\", 10, 0\n");
        fprintf(asmFile, "realArrMessage db \"Enter real array for range %%d to %%d:\", 10, 0\n");
        fprintf(asmFile, "boolArrMessage db \"Enter boolean array for range %%d to %%d (1 or 0 only):\", 10, 0\n");

        // true false messages
        fprintf(asmFile, "trueMessage db \"True\", 10, 0\n");
        fprintf(asmFile, "falseMessage db \"False\", 10,  0\n");

        
        // standard error message 
        fprintf(asmFile, "runTimeError db \"Run-time error: Bound values error\", 10, 0\n"); // newline, null terminator
        fprintf(asmFile, "\n\n");

        // code segment
        fprintf(asmFile, "section .text\n");
        fprintf(asmFile, "extern printf\n");
        fprintf(asmFile, "extern scanf\n");
        fprintf(asmFile, "global main\n\n");

        // error handling - sys_exit system call
        fprintf(asmFile, "_error: \n");
        fprintf(asmFile, "\tMOV RDI, runTimeError\n");
        fprintf(asmFile, "\tMOV AL, 0\n");
        fprintf(asmFile, "\tCALL printf\n");
        fprintf(asmFile, "\tMOV RAX, 60\n");
        fprintf(asmFile, "\tMOV RDI, 0\n");
        fprintf(asmFile, "\tSYSCALL\n");
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
        currentIdTable = root->scopeTable;
        currentOffset = 0;

        fprintf(asmFile, "main: \n");
        fprintf(asmFile, "\tPUSH RBP\n");
        fprintf(asmFile, "\tMOV RBP, RSP\n");

        // push local variables
        int maxOffset = findNextOffset(currentIdTable);
        fprintf(asmFile, "\tSUB RSP, %d\n", maxOffset);

        generateASM(root->children->sibling); 
        fprintf(asmFile, "\tMOV RSP, RBP\n"); 
        fprintf(asmFile, "\tPOP RBP\n"); 
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

        currentIdTable = root->scopeTable;
        currentOffset = 0;
        symbolTableFuncEntry* entry = searchFunc(funcTable, funcName->data.T.lexeme);

        fprintf(asmFile, "%s: \n", entry->name);
        fprintf(asmFile, "\tPUSH RBP\n");
        fprintf(asmFile, "\tMOV RBP, RSP\n"); 

        // push local variables
        int maxOffset = findNextOffset(currentIdTable);
        fprintf(asmFile, "\tSUB RSP, %d\n", maxOffset);

        parameters* tmp = entry->outputParameters;
        int outputSize = 0;

        while (tmp) {
             symbolTableIdEntry* entry = searchId(*currentIdTable, tmp->id.lexeme);

             if (entry->AorP == 0)  
                outputSize += entry->type.primitive.width;

            else if (entry->type.array.dynamicArray == 0) {
                int lb = atoi(entry->type.array.lowerBound.lexeme);
                int ub = atoi(entry->type.array.upperBound.lexeme);
                outputSize += (ub - lb + 1) * entry->type.array.datatype.width;
            }

            // What would be the case for dynamic arrays as return? not sure.
            else 
                outputSize += 8;
            
            tmp = tmp->next;
        }

       // move input parameters from shared space into offset locations
        tmp = entry->inputParameters;
        int i = 0;
        while (tmp) {
            symbolTableIdEntry* entry = searchId(*currentIdTable, tmp->id.lexeme);

            if (entry->AorP == 0) {
                fprintf(asmFile, "\tMOV RAX, QWORD [RBP + %d + 16 + %d]\n", outputSize, i);
                fprintf(asmFile, "\tMOV QWORD [RBP - 16 - %d], RAX\n", entry->offset);
                i += entry->type.primitive.width;
            }

            else if (entry->type.array.dynamicArray == 0) {
                int lb = atoi(entry->type.array.lowerBound.lexeme);
                int ub = atoi(entry->type.array.upperBound.lexeme);
                char* label = generateLabel();
                char* exitLabel = generateLabel();

                fprintf(asmFile, "\tMOV R8, %d\n", lb);
                fprintf(asmFile, "\tMOV R9, 0\n");
                fprintf(asmFile, "%s: \n", label);
                fprintf(asmFile, "\tCMP R8, %d\n", ub);
                fprintf(asmFile, "\tJG %s\n", exitLabel);

                fprintf(asmFile, "\tMOV RAX, QWORD [RBP + 16 + %d + %d * R9]\n", outputSize, entry->offset);
                fprintf(asmFile, "\tMOV QWORD [RBP - 16 - %d - R9 * %d], RAX\n", entry->offset, entry->type.array.datatype.width);

                fprintf(asmFile, "\tINC R8\n");
                fprintf(asmFile, "\tINC R9\n");
                fprintf(asmFile, "\tJMP %s\n", label);
                fprintf(asmFile, "%s: \n", exitLabel);
                i += (ub - lb + 1) * entry->type.array.datatype.width;
            }

            tmp = tmp->next;
        }
 
        // execute the statements of the module
        generateASM(moduledef);

        // place output parameters back into shared space
        if (ret_ast != NULL) {
            tmp = entry->outputParameters;
            i = 0;
            while (tmp) {
                symbolTableIdEntry* entry = searchId(*currentIdTable, tmp->id.lexeme);

                // primitive
                if (entry->AorP == 0) {
                    fprintf(asmFile, "\tMOV RAX, QWORD [RBP - 16 - %d]\n", entry->offset);
                    fprintf(asmFile, "\tMOV QWORD [RBP + 16 + %d], RAX\n", i);
                    i += entry->type.primitive.width;
                }
                
                // static array
                else if (entry->type.array.dynamicArray == 0) {
                    int lb = atoi(entry->type.array.lowerBound.lexeme);
                    int ub = atoi(entry->type.array.upperBound.lexeme);
                    char* label = generateLabel();
                    char* exitLabel = generateLabel();

                    fprintf(asmFile, "\tMOV R8, %d\n", lb);
                    fprintf(asmFile, "\tMOV R9, 0\n");
                    fprintf(asmFile, "%s: \n", label);
                    fprintf(asmFile, "\tCMP R8, %d\n", ub);
                    fprintf(asmFile, "\tJG %s\n", exitLabel);

                    fprintf(asmFile, "\tMOV RAX, QWORD [RBP - 16 - %d - R9 * %d]\n", entry->offset, entry->type.array.datatype.width);
                    fprintf(asmFile, "\tMOV QWORD [RBP + 16 + %d + R9 * %d], RAX\n", i, entry->type.array.datatype.width);

                    fprintf(asmFile, "\tINC R8\n");
                    fprintf(asmFile, "\tINC R9\n");
                    fprintf(asmFile, "\tJMP %s\n", label);
                    fprintf(asmFile, "%s: \n", exitLabel);
                    i += root->entry->type.array.datatype.width;
                }

                // dynamic arrays
                else {
                    char* lb = entry->type.array.lowerBound.lexeme;
                    char* ub = entry->type.array.upperBound.lexeme;
                    idSymbolTable* tmp = currentIdTable;
                    int width = entry->type.array.datatype.width;
                    char* label = generateLabel();
                    char* exitLabel = generateLabel();

                    // if lowerbound is dynamic
                    if (entry->type.array.lowerBound.tid == ID) {
                        symbolTableIdEntry* lbentry = NULL; 

                        while (1) {
                            lbentry = searchId(*tmp, lb);
                            
                            // found in the symbol table
                            if (lbentry != NULL)
                                break;
                            
                            tmp = tmp->parent;
                        }
                        fprintf(asmFile, "\tMOV R8, QWORD [RBP - 16 - %d]\n", lbentry->offset);
                    }

                    else 
                        fprintf(asmFile, "\tMOV R8, %d\n", atoi(lb));
                    
                    
                    // if upperbound is dynamic
                    if (entry->type.array.upperBound.tid == ID) {
                        symbolTableIdEntry* ubentry = NULL; 
                        tmp = currentIdTable;

                        while (1) {
                            ubentry = searchId(*tmp, ub);
                            
                            // found in the symbol table
                            if (ubentry != NULL)
                                break;
                            
                            tmp = tmp->parent;
                        }
                        fprintf(asmFile, "\tMOV R9, QWORD [RBP - 16 - %d]\n", ubentry->offset);
                    }
                    
                    else 
                        fprintf(asmFile, "\tMOV R9, %d\n", atoi(ub));


                    // dynamic bound check - lower bound > upper bound
                    fprintf(asmFile, "\tCMP R8, R9\n");
                    fprintf(asmFile, "\tJG _error\n");

                    fprintf(asmFile, "\tMOV R10, 0\n"); // count
                    fprintf(asmFile, "%s: \n", label);
                    fprintf(asmFile, "\tCMP R8, R9\n");
                    fprintf(asmFile, "\tJG %s\n", exitLabel);

                    fprintf(asmFile, "\tMOV RAX, QWORD [RBP - 16 - %d - R10 * %d]\n", entry->offset, entry->type.array.datatype.width);
                    fprintf(asmFile, "\tMOV QWORD [RBP + 16 + %d + R10 * %d], RAX\n", i, entry->type.array.datatype.width);

                    fprintf(asmFile, "\tINC R8\n");
                    fprintf(asmFile, "\tINC R10\n");
                    fprintf(asmFile, "\tJMP %s\n", label);
                    fprintf(asmFile, "%s: \n", exitLabel);
                    i += root->entry->type.array.datatype.width;
                }
                tmp = tmp->next;
            }
        }
        
        fprintf(asmFile, "\tMOV RSP, RBP\n"); 
        fprintf(asmFile, "\tPOP RBP\n"); 
        fprintf(asmFile, "\tRET\n\n");

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

    // reference: https://stackoverflow.com/questions/26889692/nasm-x86-64-scanf-segmentation-fault
    else if (root->TorNT == 1 && root->data.NT.ntid == ioStmt && root->children->TorNT == 0 && root->children->data.T.tid == GET_VALUE) {
        symbolTableIdEntry* entry = root->children->sibling->entry;

        // primitive datatype
        if (entry->AorP == 0) {

            char* exitLabel1 = generateLabel();
            char* exitLabel2 = generateLabel();

            fprintf(asmFile, "\tMOV RAX, RSP\n");
            fprintf(asmFile, "\tMOV RDX, 0\n");
            fprintf(asmFile, "\tMOV R9, 16\n");
            fprintf(asmFile, "\tIDIV R9\n");
            fprintf(asmFile, "\tCMP RDX, 0\n");
            fprintf(asmFile, "\tJE %s\n", exitLabel1);
            fprintf(asmFile, "\tSUB RSP, 8\n");
            fprintf(asmFile, "\tMOV R8, 1\n");
            fprintf(asmFile, "%s: \n", exitLabel1);

            if (entry->type.primitive.datatype.tid == INTEGER)
                fprintf(asmFile, "\tLEA RDI, [intMessage]\n");
            
            else if (entry->type.primitive.datatype.tid == BOOLEAN)
                fprintf(asmFile, "\tLEA RDI, [boolMessage]\n");

            else 
                fprintf(asmFile, "\tLEA RDI, [realMessage]\n");
            
            fprintf(asmFile, "\tMOV AL, 0\n");
            fprintf(asmFile, "\tCALL printf\n");

            if (entry->type.primitive.datatype.tid == INTEGER || entry->type.primitive.datatype.tid == BOOLEAN) 
                fprintf(asmFile, "\tLEA RDI, [integerRead]\n");

            else 
                fprintf(asmFile, "\tLEA RDI, [realRead]\n");

            fprintf(asmFile, "\tLEA RSI, [RBP - 16 - %d]\n", entry->offset);
            fprintf(asmFile, "\tMOV AL, 0\n");
            fprintf(asmFile, "\tCALL scanf\n");
            fprintf(asmFile, "\tCMP R8D, 1\n");
            fprintf(asmFile, "\tJNE %s\n", exitLabel2);
            fprintf(asmFile, "\tADD RSP, 8\n");
            fprintf(asmFile, "%s: \n", exitLabel2);
        }

        // static arrays 
        else if (entry->AorP == 1 && entry->type.array.dynamicArray == 0) {
            int lb = atoi(entry->type.array.lowerBound.lexeme);
            int ub = atoi(entry->type.array.upperBound.lexeme);
            int width = entry->type.array.datatype.width;
            
            char* label = generateLabel();
            char* exitLabel = generateLabel();

            char* exitLabel1 = generateLabel();
            char* exitLabel2 = generateLabel();

            fprintf(asmFile, "\tMOV RAX, RSP\n");
            fprintf(asmFile, "\tMOV RDX, 0\n");
            fprintf(asmFile, "\tMOV R9, 16\n");
            fprintf(asmFile, "\tIDIV R9\n");
            fprintf(asmFile, "\tCMP RDX, 0\n");
            fprintf(asmFile, "\tJE %s\n", exitLabel1);
            fprintf(asmFile, "\tSUB RSP, 8\n");
            fprintf(asmFile, "\tMOV R13, 1\n");
            fprintf(asmFile, "%s: \n", exitLabel1);

            fprintf(asmFile, "\tMOV RSI, %d\n", lb);
            fprintf(asmFile, "\tMOV RDX, %d\n", ub);

             if (entry->type.array.datatype.datatype.tid == INTEGER) 
                fprintf(asmFile, "\tLEA RDI, [intArrMessage]\n");
            
            else if (entry->type.array.datatype.datatype.tid == BOOLEAN) 
                fprintf(asmFile, "\tLEA RDI, [boolArrMessage]\n");

            else 
                fprintf(asmFile, "\tLEA RDI, [realMessage]\n");

            fprintf(asmFile, "\tMOV AL, 0\n");
            fprintf(asmFile, "\tCALL printf\n");

            fprintf(asmFile, "\tMOV R8, %d\n", lb);
            fprintf(asmFile, "\tMOV R9, 0\n");
            fprintf(asmFile, "%s: \n", label);
            fprintf(asmFile, "\tCMP R8, %d\n", ub);
            fprintf(asmFile, "\tJG %s\n", exitLabel);

            fprintf(asmFile, "\tPUSH R8\n");
            fprintf(asmFile, "\tPUSH R9\n");
            fprintf(asmFile, "\tMOV RAX, R9\n");
            fprintf(asmFile, "\tNEG RAX\n");
            fprintf(asmFile, "\tLEA RSI, [RBP + RAX * %d - 16 - %d]\n",  width, entry->offset);

            if (entry->type.array.datatype.datatype.tid == INTEGER || entry->type.array.datatype.datatype.tid == BOOLEAN) 
                fprintf(asmFile, "\tLEA RDI, [integerRead]\n");

            else if (entry->type.array.datatype.datatype.tid == REAL) 
                fprintf(asmFile, "\tLEA RDI, [realRead]\n");

            fprintf(asmFile, "\tMOV AL, 0\n");
            fprintf(asmFile, "\tCALL scanf\n");
            fprintf(asmFile, "\tPOP R9\n");
            fprintf(asmFile, "\tPOP R8\n");
            fprintf(asmFile, "\tINC R8\n");
            fprintf(asmFile, "\tINC R9\n");
            fprintf(asmFile, "\tJMP %s\n", label);
            fprintf(asmFile, "%s: \n", exitLabel);

            fprintf(asmFile, "\tCMP R13, 1\n");
            fprintf(asmFile, "\tJNE %s\n", exitLabel2);
            fprintf(asmFile, "\tADD RSP, 8\n");
            fprintf(asmFile, "%s: \n", exitLabel2);
        } 

        // dynamic arrays
        else {
            char* lb = entry->type.array.lowerBound.lexeme;
            char* ub = entry->type.array.upperBound.lexeme;
            idSymbolTable* tmp = currentIdTable;
            int width = entry->type.array.datatype.width;
            char* label = generateLabel();
            char* exitLabel = generateLabel();

            if (entry->type.array.lowerBound.tid == ID) {
                symbolTableIdEntry* lbentry = NULL; 
         
                while (1) {
                    lbentry = searchId(*tmp, lb);
                    
                    // found in the symbol table
                    if (lbentry != NULL)
                        break;
                    
                    tmp = tmp->parent;
                }
                fprintf(asmFile, "\tMOV R8, QWORD [RBP - 16 - %d]\n", lbentry->offset);
            }

            else 
                fprintf(asmFile, "\tMOV R8, %d\n", atoi(lb));


             if (entry->type.array.upperBound.tid == ID) {
                symbolTableIdEntry* ubentry = NULL; 
                tmp = currentIdTable;

                while (1) {
                    ubentry = searchId(*tmp, ub);
                    
                    // found in the symbol table
                    if (ubentry != NULL)
                        break;
                    
                    tmp = tmp->parent;
                }
                fprintf(asmFile, "\tMOV R9, QWORD [RBP - 16 - %d]\n", ubentry->offset);
            }

            else 
                fprintf(asmFile, "\tMOV R9, %d\n", atoi(ub));


            // dynamic bound check
            fprintf(asmFile, "\tCMP R8, R9\n");
            fprintf(asmFile, "\tJG _error\n");

            fprintf(asmFile, "\tMOV R10, 0\n"); // count
            fprintf(asmFile, "%s: \n", label);
            fprintf(asmFile, "\tCMP R8, R9\n");
            fprintf(asmFile, "\tJG %s\n", exitLabel);
            fprintf(asmFile, "\tMOV RAX, R10\n");
            fprintf(asmFile, "\tNEG RAX\n");
            fprintf(asmFile, "\tLEA RSI, [RBP + RAX * %d - 16 - %d]\n", width, entry->offset);

            if (entry->type.array.datatype.datatype.tid == INTEGER || entry->type.array.datatype.datatype.tid == BOOLEAN) 
                fprintf(asmFile, "\tLEA RDI, [integerRead]\n");

            else if (entry->type.array.datatype.datatype.tid == REAL) 
                fprintf(asmFile, "\tLEA RDI, [realRead]\n");

            fprintf(asmFile, "\tMOV AL, 0\n");
            fprintf(asmFile, "\tCALL scanf\n");
            fprintf(asmFile, "\tINC R8\n");
            fprintf(asmFile, "\tINC R10\n");
            fprintf(asmFile, "\tJMP %s\n", label);
            fprintf(asmFile, "%s: \n", exitLabel);
        }
    }

    else if (root->TorNT == 1 && root->data.NT.ntid == ioStmt && root->children->TorNT == 0 && root->children->data.T.tid == PRINT) {
        symbolTableIdEntry* entry = root->children->sibling->children->entry;

        // primitive datatype
        if (entry->AorP == 0) {
            fprintf(asmFile, "\tMOV RSI, QWORD [RBP - 16 - %d]\n", entry->offset);

            if (entry->type.primitive.datatype.tid == INTEGER) 
                fprintf(asmFile, "\tLEA RDI, [integerWrite]\n");
            
            else if (entry->type.primitive.datatype.tid == BOOLEAN) {
                char* trueLabel = generateLabel();
                char* falseLabel = generateLabel();
                char* exitLabel = generateLabel();

                fprintf(asmFile, "\tMOV RSI, 0\n");
                fprintf(asmFile, "\tCMP QWORD [RBP - 16 - %d], 1\n", entry->offset);
                fprintf(asmFile, "\tJE %s\n", trueLabel);
                fprintf(asmFile, "\tJMP %s\n", falseLabel);

                fprintf(asmFile, "%s: \n", trueLabel);
                fprintf(asmFile, "\tLEA RDI, [trueMessage]\n");
                fprintf(asmFile, "\tJMP %s\n", exitLabel);

                fprintf(asmFile, "%s: \n", falseLabel);
                fprintf(asmFile, "\tLEA RDI, [falseMessage]\n");
                fprintf(asmFile, "%s: \n", exitLabel);
            }
            
            else if (entry->type.primitive.datatype.tid == REAL) 
                fprintf(asmFile, "\tLEA RDI, [realWrite]\n");

            fprintf(asmFile, "\tMOV AL, 0\n");
            fprintf(asmFile, "\tCALL printf\n");
        }

        // static arrays 
        else if (entry->AorP == 1 && entry->type.array.dynamicArray == 0) {
            int lb = atoi(entry->type.array.lowerBound.lexeme);
            int ub = atoi(entry->type.array.upperBound.lexeme);
            int width = entry->type.array.datatype.width;
            
            char* label = generateLabel();
            char* exitLabel = generateLabel();

            fprintf(asmFile, "\tMOV R8, %d\n", lb);
            fprintf(asmFile, "\tMOV R9, 0\n");
            fprintf(asmFile, "%s: \n", label);
            fprintf(asmFile, "\tCMP R8, %d\n", ub);
            fprintf(asmFile, "\tJG %s\n", exitLabel);
            fprintf(asmFile, "\tPUSH R8\n");
            fprintf(asmFile, "\tPUSH R9\n");
            fprintf(asmFile, "\tMOV RAX, R9\n");
            fprintf(asmFile, "\tNEG RAX\n");
            fprintf(asmFile, "\tMOV RSI, [RBP + RAX * %d - 16 - %d]\n", width, entry->offset);

            if (entry->type.array.datatype.datatype.tid == INTEGER || entry->type.array.datatype.datatype.tid == BOOLEAN) 
                fprintf(asmFile, "\tLEA RDI, [integerWrite]\n");

            else 
                fprintf(asmFile, "\tLEA RDI, [realWrite]\n");

            fprintf(asmFile, "\tMOV AL, 0\n");
            fprintf(asmFile, "\tCALL printf\n");
            fprintf(asmFile, "\tPOP R9\n");
            fprintf(asmFile, "\tPOP R8\n");
            fprintf(asmFile, "\tINC R8\n");
            fprintf(asmFile, "\tINC R9\n");
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
            char* exitLabel = generateLabel();

            // if lower bound is dynamic
            if (entry->type.array.lowerBound.tid == ID) {
                symbolTableIdEntry* lbentry = NULL; 

                while (1) {
                    lbentry = searchId(*tmp, lb);
                    
                    // found in the symbol table
                    if (lbentry != NULL)
                        break;
                    
                    tmp = tmp->parent;
                }
                fprintf(asmFile, "\tMOV R8, QWORD [RBP - 16 - %d]\n", lbentry->offset);
            }

            else 
                fprintf(asmFile, "\tMOV R8, %d\n", atoi(lb)); // lower bound

            // if upper bound is dynamic
            if (entry->type.array.upperBound.tid == ID) {
                symbolTableIdEntry* ubentry = NULL; 
                tmp = currentIdTable;

                while (1) {
                    ubentry = searchId(*tmp, ub);
                    
                    // found in the symbol table
                    if (ubentry != NULL)
                        break;
                    
                    tmp = tmp->parent;
                }
                fprintf(asmFile, "\tMOV R9, QWORD [RBP - 16 - %d]\n", ubentry->offset);
            }
            
            else 
                fprintf(asmFile, "\tMOV R9, %d\n", atoi(ub)); // upper bound

            // dynamic bound check
            fprintf(asmFile, "\tCMP R8, R9\n");
            fprintf(asmFile, "\tJG _error\n");

            fprintf(asmFile, "\tMOV R10, 0\n"); // count
            fprintf(asmFile, "%s: \n", label);
            fprintf(asmFile, "\tCMP R8, R9\n");
            fprintf(asmFile, "\tJG %s\n", exitLabel);
            fprintf(asmFile, "\tMOV RAX, R10\n");
            fprintf(asmFile, "\tNEG RAX\n");
            fprintf(asmFile, "\tMOV RSI, [RBP + RAX * %d - 16 - %d]\n", width, entry->offset);

            if (entry->type.array.datatype.datatype.tid == INTEGER || entry->type.array.datatype.datatype.tid == BOOLEAN) 
                fprintf(asmFile, "\tLEA RDI, [integerWrite]\n");

            else if (entry->type.array.datatype.datatype.tid == REAL) 
                fprintf(asmFile, "\tLEA RDI, [realWrite]\n");

            fprintf(asmFile, "\tMOV AL,0\n");
            fprintf(asmFile, "\tCALL printf\n");
            fprintf(asmFile, "\tINC R8n");
            fprintf(asmFile, "\tINC R10\n");
            fprintf(asmFile, "\tJMP %s\n", label);
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
            astnode* expr = id->sibling->children;
            generateASM(expr);

            if (id->entry->AorP == 0) {
                fprintf(asmFile, "\tMOV RAX, QWORD [RBP - 16 - %d]\n", expr->entry->offset);
                fprintf(asmFile, "\tMOV QWORD [RBP - 16 - %d], RAX\n", id->entry->offset);
            }

            // assigning one static array to another
            else if (id->entry->type.array.dynamicArray == 0 && expr->entry->type.array.dynamicArray == 0) {
                int lb = atoi(id->entry->type.array.lowerBound.lexeme);
                int ub = atoi(id->entry->type.array.upperBound.lexeme);
                int width = id->entry->type.array.datatype.width;
                
                char* label = generateLabel();
                char* exitLabel = generateLabel();
                
                fprintf(asmFile, "\tMOV R8, %d\n", lb);
                fprintf(asmFile, "\tMOV R9, 0\n");
                fprintf(asmFile, "%s: \n", label);
                fprintf(asmFile, "\tCMP R8, %d\n", ub);
                fprintf(asmFile, "\tMOV RAX, QWORD [RBP - 16 - %d - %d * R9]\n", expr->entry->offset, width);
                fprintf(asmFile, "\tMOV QWORD [RBP - 16 - %d - %d * R9], RAX\n", id->entry->offset, width);

                fprintf(asmFile, "\tINC R8\n");
                fprintf(asmFile, "\tINC R9\n");
                fprintf(asmFile, "\tJMP %s\n", label);
                fprintf(asmFile, "%s: \n", exitLabel);
            }

            // dynamic array - LOTS OF CASES
            else {
                char* label = generateLabel();
                char* exitLabel = generateLabel();
                int width = id->entry->type.array.datatype.width;

                // for LHS of assignment statement first
                if (id->entry->type.array.lowerBound.tid == ID) {
                    symbolTableIdEntry* entry = NULL; 
                    idSymbolTable* tmp = currentIdTable;

                    while (1) {
                        entry = searchId(*tmp, id->entry->type.array.lowerBound.lexeme);
                        
                        // found in the symbol table
                        if (entry != NULL)
                            break;
                        
                        tmp = tmp->parent;
                    }

                    fprintf(asmFile, "\tMOV R8, QWORD [RBP - 16 - %d]\n", entry->offset);
                }

                else 
                    fprintf(asmFile, "\tMOV R8, %d\n", atoi(id->entry->type.array.lowerBound.lexeme));


                // dynamic upper bound
                if (id->entry->type.array.upperBound.tid == ID) {
                    symbolTableIdEntry* entry = NULL; 
                    idSymbolTable* tmp = currentIdTable;

                    while (1) {
                        entry = searchId(*tmp, id->entry->type.array.upperBound.lexeme);
                        
                        // found in the symbol table
                        if (entry != NULL)
                            break;
                        
                        tmp = tmp->parent;
                    }

                    fprintf(asmFile, "\tMOV R9, QWORD [RBP - 16 - %d]\n", entry->offset);
                }

                else 
                    fprintf(asmFile, "\tMOV R9, %d\n", atoi(id->entry->type.array.upperBound.lexeme));

                 // check if LHS bounds are valid
                fprintf(asmFile, "\tCMP R8, R9\n");
                fprintf(asmFile, "\tJG _error\n");

                // now for RHS of assignment statement
                if (expr->entry->type.array.lowerBound.tid == ID) {
                    symbolTableIdEntry* entry = NULL; 
                    idSymbolTable* tmp = currentIdTable;

                    while (1) {
                        entry = searchId(*tmp, expr->entry->type.array.lowerBound.lexeme);
                        
                        // found in the symbol table
                        if (entry != NULL)
                            break;
                        
                        tmp = tmp->parent;
                    }

                    fprintf(asmFile, "\tMOV R10, QWORD [RBP - 16 - %d]\n", entry->offset);
                }

                else 
                    fprintf(asmFile, "\tMOV R10, %d\n", atoi(expr->entry->type.array.lowerBound.lexeme));


                // dynamic upper bound
                if (expr->entry->type.array.upperBound.tid == ID) {
                    symbolTableIdEntry* entry = NULL; 
                    idSymbolTable* tmp = currentIdTable;

                    while (1) {
                        entry = searchId(*tmp, expr->entry->type.array.upperBound.lexeme);
                        
                        // found in the symbol table
                        if (entry != NULL)
                            break;
                        
                        tmp = tmp->parent;
                    }

                    fprintf(asmFile, "\tMOV R11, QWORD [RBP - 16 - %d]\n", entry->offset);
                }

                else 
                    fprintf(asmFile, "\tMOV R11, %d\n", atoi(expr->entry->type.array.upperBound.lexeme));


                // check if RHS bounds are valid
                fprintf(asmFile, "\tCMP R10, R11\n");
                fprintf(asmFile, "\tJG _error\n");
                
                // check whether array's have same range
                fprintf(asmFile, "\tMOV R12, R9\n");
                fprintf(asmFile, "\tSUB R12, R8\n"); // R12 = R9 - R8 = length of LHS array
                fprintf(asmFile, "\tMOV R13, R11\n");
                fprintf(asmFile, "\tSUB R13, R10\n"); // R13 = R11 - R10 = length of RHS array
                fprintf(asmFile, "\tCMP R12, R13\n");
                fprintf(asmFile, "\tJNE _error\n");
                
                fprintf(asmFile, "\tMOV R12, 0\n");
                fprintf(asmFile, "%s: \n", label);
                fprintf(asmFile, "\tCMP R10, R11\n");
                fprintf(asmFile, "\tJG %s\n", exitLabel);
                fprintf(asmFile, "\tMOV RAX, QWORD [RBP - 16 - %d - %d * R12]\n", expr->entry->offset, width);
                fprintf(asmFile, "\tMOV QWORD [RBP - 16 - %d - %d * R12], RAX\n", id->entry->offset, width);

                fprintf(asmFile, "\tINC R10\n");
                fprintf(asmFile, "\tINC R12\n");
                fprintf(asmFile, "\tJMP %s\n", label);
                fprintf(asmFile, "%s: \n", exitLabel);
            }   
        }

        // array with index
        else {
            astnode* idx = id->sibling;
            astnode* expr = idx->sibling->children;
            generateASM(idx);
            generateASM(expr);
            int width = id->entry->type.array.datatype.width;

            // dynamic lower bound 
            if (id->entry->type.array.lowerBound.tid == ID) {
                symbolTableIdEntry* entry = NULL; 
                idSymbolTable* tmp = currentIdTable;

                while (1) {
                    entry = searchId(*tmp, id->entry->type.array.lowerBound.lexeme);
                    
                    // found in the symbol table
                    if (entry != NULL)
                        break;
                    
                    tmp = tmp->parent;
                }

                fprintf(asmFile, "\tMOV R8, QWORD [RBP - 16 - %d]\n", entry->offset);
            }

            else 
                fprintf(asmFile, "\tMOV R8, %d\n", atoi(id->entry->type.array.lowerBound.lexeme));


            // dynamic upper bound
            if (id->entry->type.array.upperBound.tid == ID) {
                symbolTableIdEntry* entry = NULL; 
                idSymbolTable* tmp = currentIdTable;

                while (1) {
                    entry = searchId(*tmp, id->entry->type.array.upperBound.lexeme);
                    
                    // found in the symbol table
                    if (entry != NULL)
                        break;
                    
                    tmp = tmp->parent;
                }

                fprintf(asmFile, "\tMOV R9, QWORD [RBP - 16 - %d]\n", entry->offset);
            }

            else 
                fprintf(asmFile, "\tMOV R9, %d\n", atoi(id->entry->type.array.upperBound.lexeme));

            // check if bounds are valid
            fprintf(asmFile, "\tCMP R8, R9\n");
            fprintf(asmFile, "\tJG _error\n");

            fprintf(asmFile, "\tMOV R10, QWORD [RBP - 16 - %d]\n", idx->entry->offset);

            // check if index within bounds
            fprintf(asmFile, "\tCMP R8, R10\n");
            fprintf(asmFile, "\tJG _error\n");
            fprintf(asmFile, "\tCMP R9, R10\n");
            fprintf(asmFile, "\tJL _error\n");

            fprintf(asmFile, "\tMOV RAX, QWORD [RBP - 16 - %d]\n", expr->entry->offset);
            fprintf(asmFile, "\tSUB R10, R8\n");
            fprintf(asmFile, "\tNEG R10\n");
            fprintf(asmFile, "\tMOV QWORD [RBP + R10 * %d - 16 - %d], RAX\n", width, id->entry->offset);
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

        symbolTableFuncEntry* entry = searchFunc(funcTable, functionName->data.T.lexeme);

        // push all input parameters onto stack in reverse order 
        int numInputParams = entry->numInputParams;
        for (int i = numInputParams - 1; i >= 0; i--) {
            astnode* tmp = idlist->children;

            // go to i'th last parameter
            for (int j = 0; j < i; j++)
                tmp = tmp->sibling;
            
            idSymbolTable* itr = currentIdTable;
            symbolTableIdEntry* idEntry = NULL;

            while (1) {
                idEntry = searchId(*itr, tmp->data.T.lexeme);

                if (idEntry != NULL)
                    break;
                
                itr = itr->parent;
            }

            if (idEntry->AorP == 0) 
                fprintf(asmFile, "\tPUSH QWORD [RBP - 16 - %d]\n", idEntry->offset); 

            else if (idEntry->type.array.dynamicArray == 0) {
                int lb = atoi(idEntry->type.array.lowerBound.lexeme);
                int ub = atoi(idEntry->type.array.upperBound.lexeme);
                int width = idEntry->type.array.datatype.width;

                char* label = generateLabel();
                char* exitLabel = generateLabel();

                fprintf(asmFile, "\tMOV R8, %d\n", lb);
                fprintf(asmFile, "\tMOV R9, 0\n");
                fprintf(asmFile, "%s: \n", label);
                fprintf(asmFile, "\tCMP R8, %d\n", ub);
                fprintf(asmFile, "\tJG %s\n", exitLabel);
                fprintf(asmFile, "\tPUSH QWORD [RBP - 16 - %d - R9 * %d]\n", idEntry->offset, width); 
                fprintf(asmFile, "\tINC R8\n");
                fprintf(asmFile, "\tINC R9\n");
                fprintf(asmFile, "\tJMP %s\n", label);
                fprintf(asmFile, "%s: \n", exitLabel);
            }

            else {
                int width = idEntry->type.array.datatype.width;
                char* label = generateLabel();
                char* exitLabel = generateLabel();

                if (idEntry->type.array.lowerBound.tid == ID) {
                    symbolTableIdEntry* entry = NULL; 
                    idSymbolTable* tmp = currentIdTable;

                    while (1) {
                        entry = searchId(*tmp, idEntry->type.array.lowerBound.lexeme);
                        
                        // found in the symbol table
                        if (entry != NULL)
                            break;
                        
                        tmp = tmp->parent;
                    }

                    fprintf(asmFile, "\tMOV R8, QWORD [RBP - 16 - %d]\n", entry->offset);
                }

                else 
                    fprintf(asmFile, "\tMOV R8, %d\n", atoi(idEntry->type.array.lowerBound.lexeme));


                // dynamic upper bound
                if (idEntry->type.array.upperBound.tid == ID) {
                    symbolTableIdEntry* entry = NULL; 
                    idSymbolTable* tmp = currentIdTable;

                    while (1) {
                        entry = searchId(*tmp, idEntry->type.array.upperBound.lexeme);
                        
                        // found in the symbol table
                        if (entry != NULL)
                            break;
                        
                        tmp = tmp->parent;
                    }

                    fprintf(asmFile, "\tMOV R9, QWORD [RBP - 16 - %d]\n", entry->offset);
                }

                else 
                    fprintf(asmFile, "\tMOV R9, %d\n", atoi(idEntry->type.array.upperBound.lexeme));


                // check if bounds are valid
                fprintf(asmFile, "\tCMP R8, R9\n");
                fprintf(asmFile, "\tJG _error\n");
                
                fprintf(asmFile, "\tMOV R10, 0\n");
                fprintf(asmFile, "%s: \n", label);
                fprintf(asmFile, "\tCMP R8, R9\n");
                fprintf(asmFile, "\tJG %s\n", exitLabel);
                fprintf(asmFile, "\tPUSH QWORD [RBP - 16 - %d - %d * R10]\n", idEntry->offset, width);

                fprintf(asmFile, "\tINC R8\n");
                fprintf(asmFile, "\tINC R10\n");
                fprintf(asmFile, "\tJMP %s\n", label);
                fprintf(asmFile, "%s: \n", exitLabel);
            }
        }

        // allocate space for callee place return values - push output parameters onto stack in reverse order
        int numOutputParams = entry->numOutputParams;
        for (int i = numOutputParams - 1; i >= 0; i--) {
            astnode* tmp = opt->children->children;

            for (int j = 0; j < i; j++) 
                tmp = tmp->sibling;

            idSymbolTable* itr = currentIdTable;
            symbolTableIdEntry* idEntry = NULL;

            while (1) {
                idEntry = searchId(*itr, tmp->data.T.lexeme);

                if (idEntry != NULL)
                    break;

                itr = itr->parent;
            }

            if (idEntry->AorP == 0) 
                fprintf(asmFile, "\tPUSH QWORD [RBP - 16 - %d]\n", idEntry->offset); 

            else if (idEntry->type.array.dynamicArray == 0) {
                int lb = atoi(idEntry->type.array.lowerBound.lexeme);
                int ub = atoi(idEntry->type.array.upperBound.lexeme);
                int width = idEntry->type.array.datatype.width;

                char* label = generateLabel();
                char* exitLabel = generateLabel();

                fprintf(asmFile, "\tMOV R8, %d\n", lb);
                fprintf(asmFile, "\tMOV R9, 0\n");
                fprintf(asmFile, "%s: \n", label);
                fprintf(asmFile, "\tCMP R8, %d\n", ub);
                fprintf(asmFile, "\tJG %s\n", exitLabel);
                fprintf(asmFile, "\tPUSH QWORD [RBP - 16 - %d - R9 * %d]\n", idEntry->offset, width); 
                fprintf(asmFile, "\tINC R8\n");
                fprintf(asmFile, "\tINC R9\n");
                fprintf(asmFile, "\tJMP %s\n", label);
                fprintf(asmFile, "%s: \n", exitLabel);
            }

            else {
                int width = idEntry->type.array.datatype.width;
                char* label = generateLabel();
                char* exitLabel = generateLabel();

                if (idEntry->type.array.lowerBound.tid == ID) {
                    symbolTableIdEntry* entry = NULL; 
                    idSymbolTable* tmp = currentIdTable;

                    while (1) {
                        entry = searchId(*tmp, idEntry->type.array.lowerBound.lexeme);
                        
                        // found in the symbol table
                        if (entry != NULL)
                            break;
                        
                        tmp = tmp->parent;
                    }

                    fprintf(asmFile, "\tMOV R8, QWORD [RBP - 16 - %d]\n", entry->offset);
                }

                else 
                    fprintf(asmFile, "\tMOV R8, %d\n", atoi(idEntry->type.array.lowerBound.lexeme));


                // dynamic upper bound
                if (idEntry->type.array.upperBound.tid == ID) {
                    symbolTableIdEntry* entry = NULL; 
                    idSymbolTable* tmp = currentIdTable;

                    while (1) {
                        entry = searchId(*tmp, idEntry->type.array.upperBound.lexeme);
                        
                        // found in the symbol table
                        if (entry != NULL)
                            break;
                        
                        tmp = tmp->parent;
                    }

                    fprintf(asmFile, "\tMOV R9, QWORD [RBP - 16 - %d]\n", entry->offset);
                }

                else 
                    fprintf(asmFile, "\tMOV R9, %d\n", atoi(idEntry->type.array.upperBound.lexeme));


                // check if bounds are valid
                fprintf(asmFile, "\tCMP R8, R9\n");
                fprintf(asmFile, "\tJG _error\n");
                
                fprintf(asmFile, "\tMOV R10, 0\n");
                fprintf(asmFile, "%s: \n", label);
                fprintf(asmFile, "\tCMP R8, R9\n");
                fprintf(asmFile, "\tJG %s\n", exitLabel);
                fprintf(asmFile, "\tPUSH QWORD [RBP - 16 - %d - %d * R10]\n", idEntry->offset, width);

                fprintf(asmFile, "\tINC R8\n");
                fprintf(asmFile, "\tINC R10\n");
                fprintf(asmFile, "\tJMP %s\n", label);
                fprintf(asmFile, "%s: \n", exitLabel);
            }
        }
        
        fprintf(asmFile, "\tCALL %s\n", entry->name);

        // copy output parameters back to their actual place (in normal order)
        for (int i = 0; i < numOutputParams; i++) {
            astnode* tmp = opt->children->children;

            for (int j = 0; j < i; j++)
                tmp = tmp->sibling;
            
            idSymbolTable* itr = currentIdTable;
            symbolTableIdEntry* idEntry = NULL;

            while (1) {
                idEntry = searchId(*itr, tmp->data.T.lexeme);

                if (idEntry != NULL)
                    break;

                itr = itr->parent;
            }

            if (idEntry->AorP == 0) {
                fprintf(asmFile, "\tMOV RAX, QWORD [RSP]\n");
                fprintf(asmFile, "\tMOV QWORD [RBP - 16 - %d], RAX\n", idEntry->offset);
                fprintf(asmFile, "\tADD RSP, %d\n", idEntry->type.primitive.width);
            }

            else if (idEntry->type.array.dynamicArray == 0) {
                int lb = atoi(idEntry->type.array.lowerBound.lexeme);
                int ub = atoi(idEntry->type.array.upperBound.lexeme);
                int width = idEntry->type.array.datatype.width;

                char* label = generateLabel();
                char* exitLabel = generateLabel();

                fprintf(asmFile, "\tMOV R8, %d\n", lb);
                fprintf(asmFile, "\tMOV R9, 0\n");
                fprintf(asmFile, "%s: \n", label);
                fprintf(asmFile, "\tCMP R8, %d\n", ub);
                fprintf(asmFile, "\tJG %s\n", exitLabel);
                fprintf(asmFile, "\tMOV RAX, QWORD [RSP]\n");
                fprintf(asmFile, "\tMOV QWORD [RBP - 16 - %d - R9 * %d], RAX\n", idEntry->offset, width); 
                fprintf(asmFile, "\tADD RSP, %d\n", width);
                fprintf(asmFile, "\tINC R8\n");
                fprintf(asmFile, "\tINC R9\n");
                fprintf(asmFile, "\tJMP %s\n", label);
                fprintf(asmFile, "%s: \n", exitLabel);
            }

            else {
                int width = idEntry->type.array.datatype.width;
                char* label = generateLabel();
                char* exitLabel = generateLabel();

                if (idEntry->type.array.lowerBound.tid == ID) {
                    symbolTableIdEntry* entry = NULL; 
                    idSymbolTable* tmp = currentIdTable;

                    while (1) {
                        entry = searchId(*tmp, idEntry->type.array.lowerBound.lexeme);
                        
                        // found in the symbol table
                        if (entry != NULL)
                            break;
                        
                        tmp = tmp->parent;
                    }

                    fprintf(asmFile, "\tMOV R8, QWORD [RBP - 16 - %d]\n", entry->offset);
                }

                else 
                    fprintf(asmFile, "\tMOV R8, %d\n", atoi(idEntry->type.array.lowerBound.lexeme));


                // dynamic upper bound
                if (idEntry->type.array.upperBound.tid == ID) {
                    symbolTableIdEntry* entry = NULL; 
                    idSymbolTable* tmp = currentIdTable;

                    while (1) {
                        entry = searchId(*tmp, idEntry->type.array.upperBound.lexeme);
                        
                        // found in the symbol table
                        if (entry != NULL)
                            break;
                        
                        tmp = tmp->parent;
                    }

                    fprintf(asmFile, "\tMOV R9, QWORD [RBP - 16 - %d]\n", entry->offset);
                }

                else 
                    fprintf(asmFile, "\tMOV R9, %d\n", atoi(idEntry->type.array.upperBound.lexeme));


                // check if bounds are valid
                fprintf(asmFile, "\tCMP R8, R9\n");
                fprintf(asmFile, "\tJG _error\n");

                fprintf(asmFile, "\tMOV R10, 0\n");
                fprintf(asmFile, "%s: \n", label);
                fprintf(asmFile, "\tCMP R8, R9\n");
                fprintf(asmFile, "\tJG %s\n", exitLabel);
                fprintf(asmFile, "\tMOV QWORD [RBP - 16 - %d - R10 * %d], RSP\n", idEntry->offset, width); 
                fprintf(asmFile, "\tADD RSP, %d\n", width);
                fprintf(asmFile, "\tINC R8\n");
                fprintf(asmFile, "\tINC R10\n");
                fprintf(asmFile, "\tJMP %s\n", label);
                fprintf(asmFile, "%s: \n", exitLabel);
            }
        }

        currentIdTable = currentIdTable->parent;
    }

    else if (root->TorNT == 1 && root->data.NT.ntid == expression) {
        astnode* tmp = root->children;
        
        if (tmp->TorNT == 1 && tmp->data.NT.ntid == unary_op) {
            generateASM(tmp->sibling);
            root->entry = tmp->sibling->entry;

            // need to negate the result
            if (tmp->children->data.T.tid == MINUS) {
                
                if (tmp->entry->type.array.datatype.datatype.tid == INTEGER) {
                    fprintf(asmFile, "\tMOV RAX, 0\n");
                    fprintf(asmFile, "\tSUB RAX, QWORD [RBP - 16 - %d]\n", tmp->sibling->entry->offset);
                    fprintf(asmFile, "\tMOV QWORD [RBP - 16 - %d], RAX\n", tmp->sibling->entry->offset);
                }

                // real 
                else {
                    fprintf(asmFile, "\tMOVQ  XMM0, 0\n");
                    fprintf(asmFile, "\tSUBSD XMM0, QWORD [RBP - 16 - %d]\n", tmp->sibling->entry->offset);
                    fprintf(asmFile, "\tMOVQ  QWORD [RBP - 16 - %d], XMM0\n", tmp->sibling->entry->offset);
                }
            }
        }

        else {
            generateASM(tmp);
            root->entry = tmp->entry;
        }
    }
        
    else if(root->TorNT == 0 && (root->data.T.tid == MUL || root->data.T.tid == DIV || root->data.T.tid == PLUS || root->data.T.tid == MINUS)) {
        astnode* leftchild = root->children;
        astnode* rightchild = leftchild->sibling;

        generateASM(leftchild);
        generateASM(rightchild);

        char* tmp = generateTemporary(root);

        if (leftchild->TorNT == 1 && leftchild->data.NT.ntid == var_id_num) {
            symbolTableIdEntry* idEntry = leftchild->entry;
            astnode* idx = leftchild->children->sibling;

            int width = idEntry->type.array.datatype.width;
            char* label = generateLabel();
            char* exitLabel = generateLabel();

            if (idEntry->type.array.lowerBound.tid == ID) {
                symbolTableIdEntry* entry = NULL; 
                idSymbolTable* tmp = currentIdTable;

                while (1) {
                    entry = searchId(*tmp, idEntry->type.array.lowerBound.lexeme);
                    
                    // found in the symbol table
                    if (entry != NULL)
                        break;
                    
                    tmp = tmp->parent;
                }

                fprintf(asmFile, "\tMOV R8, QWORD [RBP - 16 - %d]\n", entry->offset);
            }

            else 
                fprintf(asmFile, "\tMOV R8, %d\n", atoi(idEntry->type.array.lowerBound.lexeme));


            // dynamic upper bound
            if (idEntry->type.array.upperBound.tid == ID) {
                symbolTableIdEntry* entry = NULL; 
                idSymbolTable* tmp = currentIdTable;

                while (1) {
                    entry = searchId(*tmp, idEntry->type.array.upperBound.lexeme);
                    
                    // found in the symbol table
                    if (entry != NULL)
                        break;
                    
                    tmp = tmp->parent;
                }

                fprintf(asmFile, "\tMOV R9, QWORD [RBP - 16 - %d]\n", entry->offset);
            }

            else 
                fprintf(asmFile, "\tMOV R9, %d\n", atoi(idEntry->type.array.upperBound.lexeme));


            // check if bounds are valid
            fprintf(asmFile, "\tCMP R8D, R9D\n");
            fprintf(asmFile, "\tJG _error\n");

            if (idx->data.T.tid == ID)
                fprintf(asmFile, "\tMOV R10D, [RBP - 16 - %d]\n", idx->entry->offset);
            
            else 
                fprintf(asmFile, "\tMOV R10D, %s\n", idx->data.T.lexeme);

            // check if index within bounds
            fprintf(asmFile, "\tCMP R8D, R10D\n");
            fprintf(asmFile, "\tJG _error\n");
            fprintf(asmFile, "\tCMP R9D, R10D\n");
            fprintf(asmFile, "\tJL _error\n");
            
            // store left offset in register
            fprintf(asmFile, "\tMOVSX R11, R10D\n");
            fprintf(asmFile, "\tSUB R11, R8\n");
            fprintf(asmFile, "\tMOV RAX, R11\n");
            fprintf(asmFile, "\tNEG RAX\n");
            fprintf(asmFile, "\tMOV R11, QWORD [RBP + RAX * %d - 16 - %d]\n", width, idEntry->offset);
        }

        else 
            fprintf(asmFile, "\tMOV R11, QWORD [RBP - 16 - %d]\n", leftchild->entry->offset);

        if (rightchild->TorNT == 1 && rightchild->data.NT.ntid == var_id_num) {
            symbolTableIdEntry* idEntry = rightchild->entry;
            astnode* idx = rightchild->children->sibling;
            
            if (idx->data.T.tid == ID)
                fprintf(asmFile, "\tMOV R10D, [RBP - 16 - %d]\n", idx->entry->offset);
            
            else 
                fprintf(asmFile, "\tMOV R10, %s\n", idx->data.T.lexeme);

            int width = idEntry->type.array.datatype.width;
            char* label = generateLabel();
            char* exitLabel = generateLabel();

            if (idEntry->type.array.lowerBound.tid == ID) {
                symbolTableIdEntry* entry = NULL; 
                idSymbolTable* tmp = currentIdTable;

                while (1) {
                    entry = searchId(*tmp, idEntry->type.array.lowerBound.lexeme);
                    
                    // found in the symbol table
                    if (entry != NULL)
                        break;
                    
                    tmp = tmp->parent;
                }

                fprintf(asmFile, "\tMOV R8, QWORD [RBP - 16 - %d]\n", entry->offset);
            }

            else 
                fprintf(asmFile, "\tMOV R8, %d\n", atoi(idEntry->type.array.lowerBound.lexeme));


            // dynamic upper bound
            if (idEntry->type.array.upperBound.tid == ID) {
                symbolTableIdEntry* entry = NULL; 
                idSymbolTable* tmp = currentIdTable;

                while (1) {
                    entry = searchId(*tmp, idEntry->type.array.upperBound.lexeme);
                    
                    // found in the symbol table
                    if (entry != NULL)
                        break;
                    
                    tmp = tmp->parent;
                }

                fprintf(asmFile, "\tMOV R9, QWORD [RBP - 16 - %d]\n", entry->offset);
            }

            else 
                fprintf(asmFile, "\tMOV R9, %d\n", atoi(idEntry->type.array.upperBound.lexeme));


            // check if bounds are valid
            fprintf(asmFile, "\tCMP R8D, R9D\n");
            fprintf(asmFile, "\tJG _error\n");

            // check if index within bounds
            fprintf(asmFile, "\tCMP R8D, R10D\n");
            fprintf(asmFile, "\tJG _error\n");
            fprintf(asmFile, "\tCMP R9D, R10D\n");
            fprintf(asmFile, "\tJL _error\n");
            
            // store left offset in register
            fprintf(asmFile, "\tMOVSX R12, R10D\n");
            fprintf(asmFile, "\tSUB R12, R8\n");
            fprintf(asmFile, "\tMOV RAX, R12\n");
            fprintf(asmFile, "\tNEG RAX\n");
            fprintf(asmFile, "\tMOV R12, QWORD [RBP + RAX * %d - 16 - %d]\n", width, idEntry->offset);
        }

        else 
            fprintf(asmFile, "\tMOV R12, QWORD [RBP - 16 - %d]\n", rightchild->entry->offset);
        


        // addition
        if (root->data.T.tid == PLUS) {

            // integer arithmetic
            if (root->datatype.tid == INTEGER) {
                fprintf(asmFile, "\tMOV  RAX, R11\n");
                fprintf(asmFile, "\tADD  RAX, R12\n");
                fprintf(asmFile, "\tMOV  QWORD [RBP - 16 - %d], RAX\n", root->entry->offset);
            }

            // real
            else {
                fprintf(asmFile, "\tMOVQ  XMM0, QWORD [RBP - 16 - %d]\n", leftchild->entry->offset); 
                fprintf(asmFile, "\tADDSD XMM0, QWORD [RBP - 16 - %d]\n", rightchild->entry->offset); 
                fprintf(asmFile, "\tMOVQ  QWORD [RBP - 16 - %d], XMM0\n", root->entry->offset); 
            }
        }

        // subtraction
        else if (root->data.T.tid == MINUS) {

            if (root->datatype.tid == INTEGER) {
                fprintf(asmFile, "\tMOV  RAX, R11\n");
                fprintf(asmFile, "\tSUB  RAX, R12\n");
                fprintf(asmFile, "\tMOV  QWORD [RBP - 16 - %d], RAX\n", root->entry->offset);
            }

            // real
            else {
                fprintf(asmFile, "\tMOVQ  XMM0, QWORD [RBP - 16 - %d]\n", leftchild->entry->offset); 
                fprintf(asmFile, "\tSUBSD XMM0, QWORD [RBP - 16 - %d]\n", rightchild->entry->offset); 
                fprintf(asmFile, "\tMOVQ  QWORD [RBP - 16 - %d], XMM0\n", root->entry->offset); 
            }
        }

        // multiplication
        else if (root->data.T.tid == MUL) {

            if (root->datatype.tid == INTEGER) {
                fprintf(asmFile, "\tMOV   RAX, R11\n"); 
                fprintf(asmFile, "\tIMUL  R12\n"); 
                fprintf(asmFile, "\tMOV   QWORD [RBP - 16 - %d], RAX\n", root->entry->offset); 
            } 

            // real
            else {
                fprintf(asmFile, "\tMOVQ  XMM0, QWORD [RBP - 16 - %d]\n", leftchild->entry->offset); 
                fprintf(asmFile, "\tMULSD XMM0, QWORD [RBP - 16 - %d]\n", rightchild->entry->offset); 
                fprintf(asmFile, "\tMOVQ  QWORD [RBP - 16 - %d], XMM0\n", root->entry->offset); 
            }
        }

        // division
        else if (root->data.T.tid == DIV) {

            if (root->datatype.tid == INTEGER) {
                fprintf(asmFile, "\tMOV   RAX, R11\n"); 
                fprintf(asmFile, "\tMOV   RDX, 0\n"); 
                fprintf(asmFile, "\tIDIV  R12\n"); 
                fprintf(asmFile, "\tMOV   QWORD [RBP - 16 - %d], RAX\n", root->entry->offset); 
            }

            // real
            else {
                fprintf(asmFile, "\tMOVQ  XMM0, QWORD [RBP - 16 - %d]\n", leftchild->entry->offset); 
                fprintf(asmFile, "\tDIVSD XMM0, QWORD [RBP - 16 - %d]\n", rightchild->entry->offset); 
                fprintf(asmFile, "\tMOVQ  QWORD [RBP - 16 - %d], XMM0\n", root->entry->offset); 
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
            fprintf(asmFile, "\tMOV RAX, QWORD [RBP - 16 - %d]\n", leftchild->entry->offset);
            fprintf(asmFile, "\tAND RAX, QWORD [RBP - 16 - %d]\n", rightchild->entry->offset);
            fprintf(asmFile, "\tMOV QWORD [RBP - 16 - %d], RAX\n", root->entry->offset);
        }

        else {
            fprintf(asmFile, "\tMOV RAX, QWORD [RBP - 16 - %d]\n", leftchild->entry->offset);
            fprintf(asmFile, "\tOR  RAX, QWORD [RBP - 16 - %d]\n", rightchild->entry->offset);
            fprintf(asmFile, "\tMOV QWORD [RBP - 16 - %d], RAX\n", root->entry->offset);
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
            fprintf(asmFile, "\tMOV RAX, QWORD [RBP - 16 - %d]\n", leftchild->entry->offset); // load 
            fprintf(asmFile, "\tCMP RAX, QWORD [RBP - 16 - %d]\n", rightchild->entry->offset); // compare 
        }

        else {
            fprintf(asmFile, "\tMOV   XMM0, QWORD [RBP - 16 - %d]\n", leftchild->entry->offset); // load 
            fprintf(asmFile, "\tCMPSD XMM0, QWORD [RBP - 16 - %d]\n", rightchild->entry->offset); // compare 
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
        fprintf(asmFile, "\tMOV RAX, 1\n");
        fprintf(asmFile, "\tMOV QWORD [RBP - 16 - %d], RAX\n", root->entry->offset);
        fprintf(asmFile, "\tJMP %s\n", exitLabel);
        fprintf(asmFile, "%s: \n", label2); // false label
        fprintf(asmFile, "\tMOV RAX, 0\n");
        fprintf(asmFile, "\tMOV QWORD [RBP - 16 - %d], RAX\n", root->entry->offset);
        fprintf(asmFile, "%s: \n", exitLabel);
    }
    
    // treating integer and boolean the same way
    else if (root->TorNT == 1 && root->data.NT.ntid == conditionalStmt) {
        currentIdTable = root->scopeTable;
        astnode* id = root->children;
        astnode* caseStatements = id->sibling;
        astnode* default_ast = caseStatements->sibling;
        astnode* tmp = caseStatements->children;

        fprintf(asmFile, "\tMOV R8D, [RBP - 16 - %d]\n", id->entry->offset);
        
        // find number of case statements first
        int numCases = 0;
        while (tmp) {
            numCases++;
            tmp = tmp->sibling->sibling;
        }

        tmp = caseStatements->children;
        int arr[numCases];
        char* labels[numCases];
        
        // store value and label for each case statement
        for (int i = 0; i < numCases; i++) {
            arr[i] = atoi(tmp->data.T.lexeme);
            labels[i] = generateLabel();
            fprintf(asmFile, "\tCMP R8D, %d\n", arr[i]);
            fprintf(asmFile, "\tJE %s\n", labels[i]);
            tmp = tmp->sibling->sibling;
        }

        char* exitLabel = generateLabel();
        char* defaultLabel = generateLabel();
        
        if (default_ast) 
            fprintf(asmFile, "\tJMP %s\n", defaultLabel);

        else
            fprintf(asmFile, "\tJMP %s\n", exitLabel);

        // assembly code for each statement
        tmp = caseStatements->children;
        for (int i = 0; i < numCases; i++) {
            tmp = tmp->sibling;
            fprintf(asmFile, "%s: \n", labels[i]);
            generateASM(tmp);
            tmp = tmp->sibling;
            fprintf(asmFile, "\tJMP %s\n", exitLabel);
        }

        // exit or default condition
        if (default_ast) {
            fprintf(asmFile, "%s: \n", defaultLabel);
            generateASM(default_ast->children);
        }

        fprintf(asmFile, "%s: \n", exitLabel);
        currentIdTable = currentIdTable->parent;
    }

    else if (root->TorNT == 1 && root->data.NT.ntid == iterativeStmt && root->children->TorNT == 0 && root->children->data.T.tid == FOR) {
        currentIdTable = root->scopeTable;
        astnode* id = root->children->sibling;
        astnode* range_ast = root->children->sibling->sibling;
        astnode* stmts = root->children->sibling->sibling->sibling;

        int num1 = atoi(range_ast->children->data.T.lexeme);
        int num2 = atoi(range_ast->children->sibling->data.T.lexeme);
        char* label = generateLabel();
        char* exitLabel = generateLabel();

        fprintf(asmFile, "\tMOV R15, %d\n", num1);
        fprintf(asmFile, "%s: \n", label);
        fprintf(asmFile, "\tMOV QWORD [RBP - 16 - %d], R15\n", id->entry->offset);
        fprintf(asmFile, "\tCMP R15, %d\n", num2);
        fprintf(asmFile, "\tJG %s\n", exitLabel);

        generateASM(stmts);
        fprintf(asmFile, "\tINC R15\n");
        fprintf(asmFile, "\tJMP %s\n", label);
        fprintf(asmFile, "%s: \n", exitLabel);
        currentIdTable = currentIdTable->parent;
    }

    else if (root->TorNT == 1 && root->data.NT.ntid == iterativeStmt && root->children->TorNT == 0 && root->children->data.T.tid == WHILE) {
        currentIdTable = root->scopeTable;
        astnode* abexpr = root->children->sibling;
        astnode* stmts = abexpr->sibling;

        char* label = generateLabel();
        char* exitLabel = generateLabel();

        fprintf(asmFile, "%s: \n", label);
        generateASM(abexpr);
        fprintf(asmFile, "\tMOV RAX, QWORD [RBP - 16 - %d]\n", abexpr->entry->offset);
        fprintf(asmFile, "\tCMP RAX, 1\n");
        fprintf(asmFile, "\tJNE %s\n", exitLabel);
        
        generateASM(stmts);
        fprintf(asmFile, "\tJMP %s\n", label);
        fprintf(asmFile, "%s: \n", exitLabel);

        currentIdTable = currentIdTable->parent;
    }

    else if (root->TorNT == 0  && root->data.T.tid == NUM) {
        char* tmp = generateTemporary(root);
        fprintf(asmFile, "\tMOV QWORD [RBP - 16 - %d], %s\n", root->entry->offset, root->data.T.lexeme); 
    }

    else if (root->TorNT == 0  && root->data.T.tid == RNUM) {
        char* tmp = generateTemporary(root);
        fprintf(asmFile, "\tMOV QWORD [RBP - 16 - %d], %s\n", root->entry->offset, root->data.T.lexeme); 
    }

    else if (root->TorNT == 0  && root->data.T.tid == TRUE) {
        char* tmp = generateTemporary(root);
        fprintf(asmFile, "\tMOV RAX, 1\n"); 
        fprintf(asmFile, "\tMOV QWORD [RBP - 16 - %d], RAX\n", root->entry->offset); 
    }

    else if (root->TorNT == 0  && root->data.T.tid == FALSE) {
        char* tmp = generateTemporary(root);
        fprintf(asmFile, "\tMOV RAX, 0\n"); 
        fprintf(asmFile, "\tMOV QWORD [RBP - 16 - %d], RAX\n", root->entry->offset); 
    }
}