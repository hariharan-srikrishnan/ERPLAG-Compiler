#include <stdio.h>
#include <stdlib.h>

#include "ast.h"
#include "symboltableDef.h"
#include "typeExtractor.h"

int semanticError = 0;


// semantic rules and type checking
void semanticChecker(astnode* root) {

    if (root->TorNT == 1 && root->data.NT.ntid == program) {
        astnode* tmp = root->children;
        currentIdTable = *(globalIdTable.child);
        while (tmp) {
            semanticChecker(tmp);
            currentIdTable = *(currentIdTable.sibling);
            tmp = tmp->sibling;
        }
        currentIdTable = globalIdTable;
    }

    else if (root->TorNT == 1 && root->data.NT.ntid == expression) {
        
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

}