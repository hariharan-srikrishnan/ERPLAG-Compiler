#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "parser.h"
#include "parserutils.h"
#include "ast.h"


unsigned long long int parseTreeSize = 0;
unsigned long long int astSize = 0;
int parseTreeNodes = 0;
int astNodes = 0;


// helper to create new AST node
astnode* createASTNode(t_node* node) {
    astnode* newNode = (astnode*) malloc(sizeof(astnode));
    newNode->TorNT = node->TorNT;
    newNode->data = node->data;
    newNode->children = NULL;
    newNode->sibling = NULL;
    newNode->entry = NULL;
    newNode->scopeTable = NULL;
    newNode->isRedeclared = 0;
    return newNode;
}


// generate AST from parse tree
void createAST(t_node* root) {

    // program -> moduleDeclarations otherModules driverModule otherModules 
    if (root->TorNT == 1 && root->data.NT.ntid == program) {
        t_node* md_ast = root->children;
        t_node* om_ast = md_ast->sibling;
        t_node* dm_ast = om_ast->sibling;
        createAST(md_ast);
        createAST(om_ast);
        createAST(dm_ast);
        createAST(dm_ast->sibling);

        root->syn =  createASTNode(root);
        root->syn->children = md_ast->syn;

        if (md_ast->syn == NULL) {
            if (om_ast->syn == NULL) 
                root->syn->children = dm_ast->syn;

            else {
                root->syn->children = om_ast->syn;
                om_ast->syn->sibling = dm_ast->syn;
            }
        }

        else {
            if (om_ast->syn == NULL) 
                md_ast->syn->sibling = dm_ast->syn;

            else {
                md_ast->syn->sibling = om_ast->syn;
                om_ast->syn->sibling = dm_ast->syn;
            }
        }

        dm_ast->syn->sibling = dm_ast->sibling->syn;
    }

    // moduleDeclarations -> moduleDeclaration moduleDeclarations
    else if (root->TorNT == 1 && root->data.NT.ntid == moduleDeclarations && root->children->TorNT == 1 && root->children->data.NT.ntid == moduleDeclaration) {
        t_node* md_ast = root->children;
        createAST(md_ast);
        createAST(md_ast->sibling);

        md_ast->syn->sibling = md_ast->sibling->syn;
        if(root->parent->TorNT == 1 && root->parent->data.NT.ntid == moduleDeclarations) 
            root->syn = md_ast->syn; 

        else {
            root->syn = createASTNode(root);
            root->syn->children = md_ast->syn;
        }
    }

    // moduleDeclarations -> EPSILON 
    else if (root->TorNT == 1 && root->data.NT.ntid == moduleDeclarations && root->children->TorNT == 0 && root->children->data.T.tid == EPSILON) {
        root->syn = NULL;
    }

    // moduleDeclaration -> DECLARE MODULE ID SEMICOL  
    else if (root->TorNT == 1 && root->data.NT.ntid == moduleDeclaration) {
        root->syn = createASTNode(root);
        root->syn->children = createASTNode(root->children->sibling->sibling);
    }

    // otherModules -> module otherModules
    else if (root->TorNT == 1 && root->data.NT.ntid == otherModules && root->children->TorNT == 1 && root->children->data.NT.ntid == module) {
        t_node* module_ast = root->children;
        t_node* othermodule_ast = module_ast->sibling;
        createAST(module_ast);
        createAST(othermodule_ast);

        module_ast->syn->sibling = othermodule_ast->syn;
        if(root->parent->TorNT == 1 && root->parent->data.NT.ntid == otherModules)
            root->syn = module_ast->syn;

        else {
            root->syn = createASTNode(root);
            root->syn->children = module_ast->syn;
        }
    }

    // otherModules -> EPSILON 
    else if (root->TorNT == 1 && root->data.NT.ntid == otherModules && root->children->TorNT == 0 && root->children->data.T.tid == EPSILON) {
        root->syn = NULL; 
    }

    // driverModule -> DRIVERDEF DRIVER PROGRAM DRIVERENDDEF moduleDef
    else if (root->TorNT == 1 && root->data.NT.ntid == driverModule) {
        t_node* moduledef_ast = root->children->sibling->sibling->sibling->sibling;
        createAST(moduledef_ast);

        root->syn = createASTNode(root);
        root->syn->children = createASTNode(root->children->sibling);
        root->syn->children->sibling = moduledef_ast->syn;
    }

    // module -> DEF MODULE ID ENDDEF TAKES INPUT SQBO input_plist SQBC SEMICOL ret moduleDef
    else if (root->TorNT == 1 && root->data.NT.ntid == module) {
        t_node* id_ast = root->children->sibling->sibling;
        t_node* iplist_ast = id_ast->sibling->sibling->sibling->sibling->sibling;
        t_node* ret_ast = iplist_ast->sibling->sibling->sibling;
        t_node* moduledef_ast = ret_ast->sibling;
        createAST(iplist_ast);
        createAST(ret_ast);
        createAST(moduledef_ast);

        root->syn = createASTNode(root);
        id_ast->syn = createASTNode(id_ast);
        root->syn->children = id_ast->syn;
        id_ast->syn->sibling = iplist_ast->syn;
        iplist_ast->syn->sibling = ret_ast->syn;

        if (ret_ast->syn != NULL)
            ret_ast->syn->sibling = moduledef_ast->syn;
        
        else
            iplist_ast->syn->sibling = moduledef_ast->syn;
    }

    // ret -> RETURNS SQBO output_plist SQBC SEMICOL
    else if (root->TorNT == 1 && root->data.NT.ntid == ret && root->children->TorNT == 0 && root->children->data.T.tid == RETURNS) {
        t_node* oplist_ast = root->children->sibling->sibling;
        createAST(oplist_ast);
        root->syn = oplist_ast->syn;
    }

    // ret -> EPSILON 
    else if (root->TorNT == 1 && root->data.NT.ntid == ret && root->children->TorNT == 0 && root->children->data.T.tid == EPSILON) {
        root->syn = NULL;
    }

    // input_plist -> ID COLON dataType N1
    else if (root->TorNT == 1 && root->data.NT.ntid == input_plist) {
        t_node* id_ast = root->children;
        t_node* datatype_ast = id_ast->sibling->sibling;
        t_node* n1_ast = datatype_ast->sibling;
        createAST(datatype_ast);
        createAST(n1_ast);
        
        root->syn = createASTNode(root);
        id_ast->syn = createASTNode(id_ast);
        root->syn->children = id_ast->syn;
        id_ast->syn->sibling = datatype_ast->syn;
        datatype_ast->syn->sibling = n1_ast->syn;
    }

    // N1 -> COMMA ID COLON dataType N1
    else if (root->TorNT == 1 && root->data.NT.ntid == N1 && root->children->TorNT == 0 && root->children->data.T.tid == COMMA) {
        t_node* id_ast = root->children->sibling;
        t_node* datatype_ast = id_ast->sibling->sibling;
        t_node* n1_ast = datatype_ast->sibling;
        createAST(datatype_ast);
        createAST(n1_ast);

        id_ast->syn = createASTNode(id_ast);
        id_ast->syn->sibling = datatype_ast->syn;
        datatype_ast->syn->sibling = n1_ast->syn;
        root->syn = id_ast->syn;
    }

    // N1 -> EPSILON
    else if (root->TorNT == 1 && root->data.NT.ntid == N1 && root->children->TorNT == 0 && root->children->data.T.tid == EPSILON) {
        root->syn = NULL;
    }

    // output_plist -> ID COLON type N2
    else if (root->TorNT == 1 && root->data.NT.ntid == output_plist) {
        t_node* id_ast = root->children;
        t_node* type_ast = id_ast->sibling->sibling;
        t_node* n2_ast = type_ast->sibling;
        createAST(type_ast);
        createAST(n2_ast);

        root->syn = createASTNode(root);
        id_ast->syn = createASTNode(id_ast);
        root->syn->children = id_ast->syn;
        id_ast->syn->sibling = type_ast->syn;
        type_ast->syn->sibling = n2_ast->syn;
    }

    // N2 -> COMMA ID COLON type N2
    else if (root->TorNT == 1 && root->data.NT.ntid == N2 && root->children->TorNT == 0 && root->children->data.T.tid == COMMA) {
        t_node* id_ast = root->children->sibling;
        t_node* type_ast = id_ast->sibling->sibling;
        t_node* n2_ast = type_ast->sibling;
        createAST(type_ast);
        createAST(n2_ast);
        
        id_ast->syn = createASTNode(id_ast);
        id_ast->syn->sibling = type_ast->syn;
        type_ast->syn->sibling = n2_ast->syn;
        root->syn = id_ast->syn;
    }

    // N2 -> EPSILON
    else if (root->TorNT == 1 && root->data.NT.ntid == N2 && root->children->TorNT == 0 && root->children->data.T.tid == EPSILON) {
        root->syn = NULL;
    }

    // dataType -> INTEGER
    else if (root->TorNT == 1 && root->data.NT.ntid == dataType && root->children->TorNT == 0 && root->children->data.T.tid == INTEGER) {
        root->syn = createASTNode(root);
        root->syn->children = createASTNode(root->children);
    }

    // dataType -> REAL
    else if (root->TorNT == 1 && root->data.NT.ntid == dataType && root->children->TorNT == 0 && root->children->data.T.tid == REAL) {
        root->syn = createASTNode(root);
        root->syn->children = createASTNode(root->children);
    }

    // dataType -> BOOLEAN
    else if (root->TorNT == 1 && root->data.NT.ntid == dataType && root->children->TorNT == 0 && root->children->data.T.tid == BOOLEAN) {
        root->syn = createASTNode(root);
        root->syn->children = createASTNode(root->children);
    }
    
    // dataType -> ARRAY SQBO range_arrays SQBC OF type
    else if (root->TorNT == 1 && root->data.NT.ntid == dataType && root->children->TorNT == 0 && root->children->data.T.tid == ARRAY) {
        t_node* arr_ast = root->children;
        t_node* ra_ast = arr_ast->sibling->sibling;
        t_node* type_ast = ra_ast->sibling->sibling->sibling;
        createAST(ra_ast);
        createAST(type_ast);

        root->syn = createASTNode(root);
        arr_ast->syn = createASTNode(arr_ast);
        root->syn->children = arr_ast->syn;
        arr_ast->syn->sibling = ra_ast->syn;
        ra_ast->syn->sibling = type_ast->syn; 
    }

    // range_arrays -> index RANGEOP index
    else if (root->TorNT == 1 && root->data.NT.ntid == range_arrays) {
        t_node* index_1 = root->children;
        t_node* index_2 = index_1->sibling->sibling;
        createAST(index_1);
        createAST(index_2);  

        root->syn = createASTNode(root);
        root->syn->children = index_1->syn;
        index_1->syn->sibling = index_2->syn;
    }

    // type -> INTEGER 
    else if (root->TorNT == 1 && root->data.NT.ntid == type && root->children->TorNT == 0 && root->children->data.T.tid == INTEGER) {
        root->syn = createASTNode(root);
        root->syn->children = createASTNode(root->children);
    }

    // type -> REAL
    else if (root->TorNT == 1 && root->data.NT.ntid == type && root->children->TorNT == 0 && root->children->data.T.tid == REAL) {
        root->syn = createASTNode(root);
        root->syn->children = createASTNode(root->children);
    }

    // type -> BOOLEAN
    else if (root->TorNT == 1 && root->data.NT.ntid == type && root->children->TorNT == 0 && root->children->data.T.tid == BOOLEAN) {
        root->syn = createASTNode(root);
        root->syn->children = createASTNode(root->children);
    }

    // moduleDef -> START statements END 
    else if (root->TorNT == 1 && root->data.NT.ntid == moduleDef) {
        t_node* stmts_ast = root->children->sibling;
        createAST(stmts_ast);

        root->syn = createASTNode(root);
        root->syn->children = stmts_ast->syn;

        root->syn->startLineNo = root->children->data.T.lineNo;
        root->syn->endLineNo = root->children->sibling->sibling->data.T.lineNo;
    }

    // statements -> statement statements
    else if (root->TorNT == 1 && root->data.NT.ntid == statements && root->children->TorNT == 1 && root->children->data.NT.ntid == statement) {
        t_node* stmt_ast = root->children;
        t_node* stmts_ast = stmt_ast->sibling;
        createAST(stmt_ast);
        createAST(stmts_ast);

        stmt_ast->syn->sibling = stmts_ast->syn;
        if(root->parent->TorNT == 1 && root->parent->data.NT.ntid == statements)
            root->syn = stmt_ast->syn;
        
        else {
            root->syn = createASTNode(root);
            root->syn->children = stmt_ast->syn;
        }
    }
    
    // statements -> EPSILON 
    else if (root->TorNT == 1 && root->data.NT.ntid == statements && root->children->TorNT == 0 && root->children->data.T.tid == EPSILON) {
        root->syn = NULL;
    }
    
    // statement -> ioStmt
    else if (root->TorNT == 1 && root->data.NT.ntid == statement && root->children->TorNT == 1 && root->children->data.NT.ntid == ioStmt) {
        t_node* iostmt_ast = root->children;
        createAST(iostmt_ast);
        root->syn = iostmt_ast->syn;
    }
    
    // statement -> simpleStmt
    else if (root->TorNT == 1 && root->data.NT.ntid == statement && root->children->TorNT == 1 && root->children->data.NT.ntid == simpleStmt) {
        t_node* simple_ast = root->children;
        createAST(simple_ast);
        root->syn = simple_ast->syn;
    }    
    
    // statement -> declareStmt
    else if (root->TorNT == 1 && root->data.NT.ntid == statement && root->children->TorNT == 1 && root->children->data.NT.ntid == declareStmt) {
        t_node* declare_ast = root->children;
        createAST(declare_ast);
        root->syn = declare_ast->syn;
    }    
    
    // statement -> conditionalStmt
    else if (root->TorNT == 1 && root->data.NT.ntid == statement && root->children->TorNT == 1 && root->children->data.NT.ntid == conditionalStmt) {
        t_node* condStmt_ast = root->children;
        createAST(condStmt_ast);
        root->syn = condStmt_ast->syn;
    }    
    
    // statement -> iterativeStmt
    else if (root->TorNT == 1 && root->data.NT.ntid == statement && root->children->TorNT == 1 && root->children->data.NT.ntid == iterativeStmt) {
        t_node* iterStmt_ast = root->children;
        createAST(iterStmt_ast);
        root->syn= iterStmt_ast->syn;
    }

    // ioStmt -> GET_VALUE BO ID BC SEMICOL
    else if (root->TorNT == 1 && root->data.NT.ntid == ioStmt && root->children->TorNT == 0 && root->children->data.T.tid == GET_VALUE) {
        t_node* gv_ast = root->children;
        t_node* id_ast = gv_ast->sibling->sibling;

        root->syn = createASTNode(root);
        gv_ast->syn = createASTNode(gv_ast);
        id_ast->syn = createASTNode(id_ast);
        root->syn->children = gv_ast->syn;
        gv_ast->syn->sibling = id_ast->syn;
    }
    
    // ioStmt -> PRINT BO var BC SEMICOL 
    else if (root->TorNT == 1 && root->data.NT.ntid == ioStmt && root->children->TorNT == 0 && root->children->data.T.tid == PRINT) {
        t_node* pv_ast = root->children;
        t_node* var_ast = pv_ast->sibling->sibling;
        createAST(var_ast);

        root->syn = createASTNode(root);
        pv_ast->syn = createASTNode(pv_ast);
        root->syn->children = pv_ast->syn;
        pv_ast->syn->sibling = var_ast->syn;
    }

    // boolConstt -> TRUE
    else if (root->TorNT == 1 && root->data.NT.ntid == boolConstt && root->children->TorNT == 0 && root->children->data.T.tid == TRUE) {
        root->syn = createASTNode(root->children);
        root->syn->datatype.tid = BOOLEAN;
    }

    // boolConstt -> FALSE
    else if (root->TorNT == 1 && root->data.NT.ntid == boolConstt && root->children->TorNT == 0 && root->children->data.T.tid == FALSE) {
        root->syn = createASTNode(root->children);
        root->syn->datatype.tid = BOOLEAN;
    }

    // var_id_num -> ID whichId
    else if (root->TorNT == 1 && root->data.NT.ntid == var_id_num && root->children->TorNT == 0 && root->children->data.T.tid == ID) {
        t_node* id_ast = root->children;
        t_node* whichid_ast = id_ast->sibling;
        createAST(whichid_ast);

        if (whichid_ast->syn == NULL) {
            id_ast->syn = createASTNode(id_ast);
            id_ast->syn->sibling = whichid_ast->syn;
            root->syn = id_ast->syn;
        }

        else {
            root->syn = createASTNode(root);
            id_ast->syn = createASTNode(id_ast);
            root->syn->children = id_ast->syn;
            id_ast->syn->sibling = whichid_ast->syn;
        }
    }
    
    // var_id_num -> NUM
    else if (root->TorNT == 1 && root->data.NT.ntid == var_id_num && root->children->TorNT == 0 && root->children->data.T.tid == NUM) {
        root->syn = createASTNode(root->children);
        root->syn->datatype.tid = INTEGER;
    }

    // var_id_num -> RNUM
    else if (root->TorNT == 1 && root->data.NT.ntid == var_id_num && root->children->TorNT == 0 && root->children->data.T.tid == RNUM) {
        root->syn = createASTNode(root->children);
        root->syn->datatype.tid = REAL;
    }

    // var -> var_id_num
    else if (root->TorNT == 1 && root->data.NT.ntid == var && root->children->TorNT == 1 && root->children->data.NT.ntid == var_id_num) {
        t_node* varIdNum_ast = root->children;
        createAST(varIdNum_ast);

        root->syn = createASTNode(root);
        root->syn->children = varIdNum_ast->syn;
    }

    // var -> boolConstt
    else if (root->TorNT == 1 && root->data.NT.ntid == var && root->children->TorNT == 1 && root->children->data.NT.ntid == boolConstt) {
        t_node* boolconst_ast = root->children;
        createAST(boolconst_ast);

        root->syn = createASTNode(root);
        root->syn->children = boolconst_ast->syn;
    }

    // whichId -> SQBO index SQBC
    else if (root->TorNT == 1 && root->data.NT.ntid == whichId && root->children->TorNT == 0 && root->children->data.T.tid == SQBO) {
        t_node* index_ast = root->children->sibling;
        createAST(index_ast);
        root->syn = index_ast->syn;
    }

    // whichId -> EPSILON 
    else if (root->TorNT == 1 && root->data.NT.ntid == whichId && root->children->TorNT == 0 && root->children->data.T.tid == EPSILON) {
        root->syn = NULL;
    }

    // simpleStmt -> assignmentStmt
    else if (root->TorNT == 1 && root->data.NT.ntid == simpleStmt && root->children->TorNT == 1 && root->children->data.NT.ntid == assignmentStmt) {
        t_node* assign_ast = root->children;
        createAST(assign_ast);

        root->syn = createASTNode(root);
        root->syn->children = assign_ast->syn;
    }

    // simpleStmt -> moduleReuseStmt
    else if (root->TorNT == 1 && root->data.NT.ntid == simpleStmt && root->children->TorNT == 1 && root->children->data.NT.ntid == moduleReuseStmt) {
        t_node* moduleReuse_ast = root->children;
        createAST(moduleReuse_ast);

        root->syn = createASTNode(root);
        root->syn->children = moduleReuse_ast->syn;
    }

    // assignmentStmt -> ID whichStmt 
    else if (root->TorNT == 1 && root->data.NT.ntid == assignmentStmt) {
        t_node* id_ast = root->children;
        t_node* whichStmt_ast = id_ast->sibling;
        id_ast->syn = createASTNode(id_ast);
        whichStmt_ast->inh = id_ast->syn;
        createAST(whichStmt_ast);
        
        root->syn = createASTNode(root);
        root->syn->children = whichStmt_ast->syn;    
    }
    
    // whichStmt -> lvalueIDStmt
    else if (root->TorNT == 1 && root->data.NT.ntid == whichStmt && root->children->TorNT == 1 && root->children->data.NT.ntid == lvalueIDStmt) {
        t_node* lIdStmt_ast = root->children;
        lIdStmt_ast->inh = root->inh;
        createAST(lIdStmt_ast);
        root->syn = lIdStmt_ast->syn;
    }

    // whichStmt -> lvalueARRStmt
    else if (root->TorNT == 1 && root->data.NT.ntid == whichStmt && root->children->TorNT == 1 && root->children->data.NT.ntid == lvalueARRStmt) {
        t_node* lArrStmt_ast = root->children;
        lArrStmt_ast->inh = root->inh;
        createAST(lArrStmt_ast);
        root->syn = lArrStmt_ast->syn;
    }

    // lvalueIDStmt -> ASSIGNOP expression SEMICOL
    else if (root->TorNT == 1 && root->data.NT.ntid == lvalueIDStmt) {
        t_node* assign_ast = root->children;
        t_node* expr_ast = assign_ast->sibling;
        createAST(expr_ast);
        
        root->syn = createASTNode(assign_ast);
        root->syn->children = root->inh;
        root->syn->children->sibling = createASTNode(root);
        root->syn->children->sibling->children = expr_ast->syn;
    }

    // lvalueARRStmt -> SQBO index SQBC ASSIGNOP expression SEMICOL
    else if (root->TorNT == 1 && root->data.NT.ntid == lvalueARRStmt) {
        t_node* index_ast = root->children->sibling;
        t_node* assign_ast = index_ast->sibling->sibling;
        t_node* expr_ast = assign_ast->sibling;
        createAST(index_ast);
        createAST(expr_ast);

        root->syn = createASTNode(assign_ast);
        root->syn->children = root->inh;
        root->syn->children->sibling = index_ast->syn;
        index_ast->syn->sibling = createASTNode(root);
        index_ast->syn->sibling->children = expr_ast->syn;
    }

    // index -> NUM 
    else if (root->TorNT == 1 && root->data.NT.ntid == _index && root->children->TorNT == 0 && root->children->data.T.tid == NUM) {
        root->syn = createASTNode(root->children);
        root->syn->datatype.tid = INTEGER;
    }
    
    // index -> ID 
    else if (root->TorNT == 1 && root->data.NT.ntid == _index && root->children->TorNT == 0 && root->children->data.T.tid == ID) {
        root->syn = createASTNode(root->children);
    }

    // moduleReuseStmt -> optional USE MODULE ID WITH PARAMETERS idList SEMICOL
    else if (root->TorNT == 1 && root->data.NT.ntid == moduleReuseStmt) {
        t_node* opt_ast = root->children;
        t_node* id_ast = opt_ast->sibling->sibling->sibling;
        t_node* idlist_ast = id_ast->sibling->sibling->sibling;
        createAST(opt_ast);
        createAST(idlist_ast);
        
        root->syn = createASTNode(root);
        id_ast->syn = createASTNode(id_ast);
        if (opt_ast->syn == NULL) {
            root->syn->children = id_ast->syn;
        }

        else {
            root->syn->children = opt_ast->syn;
            opt_ast->syn->sibling = id_ast->syn;
        }

        id_ast->syn->sibling = idlist_ast->syn;
    }

    // optional -> SQBO idList SQBC ASSIGNOP 
    else if (root->TorNT == 1 && root->data.NT.ntid == optional && root->children->TorNT == 0 && root->children->data.NT.ntid == SQBO) {
        t_node* idlist_ast = root->children->sibling;
        t_node* assign_ast = idlist_ast->sibling->sibling;
        createAST(idlist_ast);

        root->syn = createASTNode(root);
        root->syn->children = idlist_ast->syn;
    }
    
    // optional -> EPSILON
    else if (root->TorNT == 1 && root->data.NT.ntid == optional && root->children->TorNT == 0 && root->children->data.T.tid == EPSILON) {
        root->syn = NULL;
    }

    // idList -> ID N3
    else if (root->TorNT == 1 && root->data.NT.ntid == idList) {
        t_node* id_ast = root->children;
        t_node* n3_ast = id_ast->sibling;
        createAST(n3_ast);
        
        root->syn = createASTNode(root);
        id_ast->syn = createASTNode(id_ast);
        root->syn->children = id_ast->syn;
        id_ast->syn->sibling = n3_ast->syn;
    }

    // N3 -> COMMA ID N3
    else if (root->TorNT == 1 && root->data.NT.ntid == N3 && root->children->TorNT == 0 && root->children->data.NT.ntid == COMMA) {
        t_node* id_ast = root->children->sibling;
        t_node* n3_ast = id_ast->sibling;
        createAST(n3_ast);
        
        id_ast->syn = createASTNode(id_ast);
        id_ast->syn->sibling = n3_ast->syn;
        root->syn = id_ast->syn;
    }
    
    // N3 -> EPSILON
    else if (root->TorNT == 1 && root->data.NT.ntid == N3 && root->children->TorNT == 0 && root->children->data.T.tid == EPSILON) {
        root->syn = NULL;
    }

    // expression -> arithmeticOrBooleanExpr
    else if (root->TorNT == 1 && root->data.NT.ntid == expression && root->children->TorNT == 1 && root->children->data.NT.ntid == arithmeticOrBooleanExpr) {
        t_node* abexpr_ast = root->children;
        createAST(abexpr_ast);

        root->syn = createASTNode(root);
        root->syn->children = abexpr_ast->syn;
    }

    // expression -> U
    else if (root->TorNT == 1 && root->data.NT.ntid == expression && root->children->TorNT == 1 && root->children->data.NT.ntid == U) {
        t_node* u_ast = root->children;
        createAST(u_ast);
        
        root->syn = createASTNode(root);
        root->syn->children = u_ast->syn;
    }

    // U -> unary_op new_NT
    else if (root->TorNT == 1 && root->data.NT.ntid == U) {
        t_node* unaryop_ast = root->children;
        t_node* newNT_ast = unaryop_ast->sibling;
        createAST(unaryop_ast);
        createAST(newNT_ast);

        root->syn = unaryop_ast->syn;
        unaryop_ast->syn->sibling = newNT_ast->syn;
    }
    
    // new_NT -> BO arithmeticExpr BC
    else if (root->TorNT == 1 && root->data.NT.ntid == new_NT && root->children->TorNT == 0 && root->children->data.T.tid == BO) {
        t_node* arithExpr_ast = root->children->sibling;
        createAST(arithExpr_ast);
        root->syn = arithExpr_ast->syn;
    }

    // new_NT -> var_id_num
    else if (root->TorNT == 1 && root->data.NT.ntid == new_NT && root->children->TorNT == 1 && root->children->data.NT.ntid == var_id_num) {
        t_node* varIdNum_ast = root->children;
        createAST(root->children);
        root->syn = varIdNum_ast->syn;
    }

    // unary_op -> PLUS
    else if (root->TorNT == 1 && root->data.NT.ntid == unary_op && root->children->TorNT == 0 && root->children->data.NT.ntid == PLUS) {
        root->syn = createASTNode(root);
        root->syn->children = createASTNode(root->children);
    }

    // unary_op -> MINUS
    else if (root->TorNT == 1 && root->data.NT.ntid == unary_op && root->children->TorNT == 0 && root->children->data.NT.ntid == MINUS) {
        root->syn = createASTNode(root);
        root->syn->children = createASTNode(root->children);
    }

    // arithmeticOrBooleanExpr -> anyTerm N7
    else if (root->TorNT == 1 && root->data.NT.ntid == arithmeticOrBooleanExpr) {
        t_node* anyterm_ast = root->children;
        t_node* n7_ast = anyterm_ast->sibling;
        createAST(anyterm_ast);
        n7_ast->inh = anyterm_ast->syn;
        createAST(n7_ast);

        root->syn = n7_ast->syn;
    }

    // N7 -> logicalOp anyTerm N7
    else if (root->TorNT == 1 && root->data.NT.ntid == N7 && root->children->TorNT == 1 && root->children->data.NT.ntid == logicalOp) {
        t_node* lop_ast = root->children;
        t_node* anyterm_ast = lop_ast->sibling;
        t_node* n7_ast = anyterm_ast->sibling;
        createAST(lop_ast);
        createAST(anyterm_ast);
        n7_ast->inh = anyterm_ast->syn;
        createAST(n7_ast);

        root->syn = lop_ast->syn;
        root->syn->children = root->inh;
        root->syn->children->sibling = n7_ast->syn;
    }

    // N7 -> EPSILON
    else if (root->TorNT == 1 && root->data.NT.ntid == N7 && root->children->TorNT == 0 && root->children->data.T.tid == EPSILON) {
        root->syn = root->inh;
    }
    
    // anyTerm -> arithmeticExpr N8
    else if (root->TorNT == 1 && root->data.NT.ntid == anyTerm && root->children->TorNT == 1 && root->children->data.NT.ntid == arithmeticExpr) {
        t_node* arith_ast = root->children;
        t_node* n8_ast = arith_ast->sibling;
        createAST(arith_ast);
        n8_ast->inh = arith_ast->syn;
        createAST(n8_ast);

        root->syn = n8_ast->syn;
    }
 
    // anyTerm -> boolConstt
    else if (root->TorNT == 1 && root->data.NT.ntid == anyTerm && root->children->TorNT == 1 && root->children->data.NT.ntid == boolConstt) {
        t_node* boolConst_ast = root->children;
        createAST(boolConst_ast);
        root->syn = boolConst_ast->syn;
    }

    // N8 -> relationalOp arithmeticExpr
    else if (root->TorNT == 1 && root->data.NT.ntid == N8 && root->children->TorNT == 1 && root->children->data.NT.ntid == relationalOp) {
        t_node* rop_ast = root->children;
        t_node* arith_ast = rop_ast->sibling;
        createAST(rop_ast);
        createAST(arith_ast);

        root->syn = rop_ast->syn;
        root->syn->children = root->inh;
        root->syn->children->sibling = arith_ast->syn;
    }
    
    // N8 -> EPSILON
    else if (root->TorNT == 1 && root->data.NT.ntid == N8 && root->children->TorNT == 0 && root->children->data.T.tid == EPSILON) {
        root->syn = root->inh;
    }

    // arithmeticExpr -> term N4
    else if (root->TorNT == 1 && root->data.NT.ntid == arithmeticExpr) {
        t_node* term_ast = root->children;
        t_node* n4_ast = term_ast->sibling;
        createAST(term_ast);
        n4_ast->inh = term_ast->syn;
        createAST(n4_ast);
        
        root->syn = n4_ast->syn;
    }

    // N4 -> op1 term N4
    else if (root->TorNT == 1 && root->data.NT.ntid == N4 && root->children->TorNT == 1 && root->children->data.NT.ntid == op1) {
        t_node* op1_ast = root->children;
        t_node* term_ast = op1_ast->sibling;
        t_node* n4_ast = term_ast->sibling;
        createAST(op1_ast);
        createAST(term_ast);
        n4_ast->inh = term_ast->syn;
        createAST(n4_ast);

        root->syn = op1_ast->syn;
        root->syn->children = root->inh;
        root->syn->children->sibling = n4_ast->syn;
    }

    // N4 -> EPSILON
    else if (root->TorNT == 1 && root->data.NT.ntid == N4 && root->children->TorNT == 0 && root->children->data.T.tid == EPSILON) {
        root->syn = root->inh;
    }

    // term -> factor N5
    else if (root->TorNT == 1 && root->data.NT.ntid == term) {
        t_node* factor_ast = root->children;
        t_node* n5_ast = factor_ast->sibling;
        createAST(factor_ast);
        n5_ast->inh = factor_ast->syn;
        createAST(n5_ast);

        root->syn = n5_ast->syn;
    }

    // N5 -> op2 factor N5
    else if (root->TorNT == 1 && root->data.NT.ntid == N5 && root->children->TorNT == 1 && root->children->data.NT.ntid == op2) {
        t_node* op2_ast = root->children;
        t_node* factor_ast = op2_ast->sibling;
        t_node* n5_ast = factor_ast->sibling;
        createAST(op2_ast);
        createAST(factor_ast);
        n5_ast->inh = factor_ast->syn;
        createAST(n5_ast);

        root->syn = op2_ast->syn;
        root->syn->children = root->inh;
        root->syn->children->sibling = n5_ast->syn;
    }    
    
    // N5 -> EPSILON
    else if (root->TorNT == 1 && root->data.NT.ntid == N5 && root->children->TorNT == 0 && root->children->data.T.tid == EPSILON) {
        root->syn = root->inh;
    }

    // factor -> BO arithmeticOrBooleanExpr BC
    else if (root->TorNT == 1 && root->data.NT.ntid == factor && root->children->TorNT == 0 && root->children->data.T.tid == BO) {
        t_node* abexpr_ast = root->children->sibling;
        createAST(abexpr_ast);
        root->syn = abexpr_ast->syn;
    }
    
    // factor -> var_id_num
    else if (root->TorNT == 1 && root->data.NT.ntid == factor && root->children->TorNT == 1 && root->children->data.NT.ntid == var_id_num) {
        t_node* varIdNum_ast = root->children;
        createAST(varIdNum_ast);
        root->syn = varIdNum_ast->syn;
    }
    
    // op1 -> PLUS
    else if (root->TorNT == 1 && root->data.NT.ntid == op1 && root->children->TorNT == 0 && root->children->data.T.tid == PLUS) {
        root->syn = createASTNode(root->children);
    }

    // op1 -> MINUS
    else if (root->TorNT == 1 && root->data.NT.ntid == op1 && root->children->TorNT == 0 && root->children->data.T.tid == MINUS) {
        root->syn = createASTNode(root->children);
    }

    // op2 -> MUL
    else if (root->TorNT == 1 && root->data.NT.ntid == op2 && root->children->TorNT == 0 && root->children->data.T.tid == MUL) {
        root->syn = createASTNode(root->children);
    }

    // op2 -> DIV
    else if (root->TorNT == 1 && root->data.NT.ntid == op2 && root->children->TorNT == 0 && root->children->data.T.tid == DIV) {
        root->syn = createASTNode(root->children);
    }

    // logicalOp -> AND
    else if (root->TorNT == 1 && root->data.NT.ntid == logicalOp && root->children->TorNT == 0 && root->children->data.T.tid == AND) {
        root->syn = createASTNode(root->children);
    }

    // logicalOp -> OR
    else if (root->TorNT == 1 && root->data.NT.ntid == logicalOp && root->children->TorNT == 0 && root->children->data.T.tid == OR) {
        root->syn = createASTNode(root->children);
    }

    // relationalOp -> LT
    else if (root->TorNT == 1 && root->data.NT.ntid == relationalOp && root->children->TorNT == 0 && root->children->data.T.tid == LT) {
        root->syn = createASTNode(root->children);
    }

    // relationalOp -> LE 
    else if (root->TorNT == 1 && root->data.NT.ntid == relationalOp && root->children->TorNT == 0 && root->children->data.T.tid == LE) {
        root->syn = createASTNode(root->children);
    }

    // relationalOp -> GT 
    else if (root->TorNT == 1 && root->data.NT.ntid == relationalOp && root->children->TorNT == 0 && root->children->data.T.tid == GT) {
        root->syn = createASTNode(root->children);
    }

    // relationalOp -> GE 
    else if (root->TorNT == 1 && root->data.NT.ntid == relationalOp && root->children->TorNT == 0 && root->children->data.T.tid == GE) {
        root->syn = createASTNode(root->children);
    }

    // relationalOp -> EQ 
    else if (root->TorNT == 1 && root->data.NT.ntid == relationalOp && root->children->TorNT == 0 && root->children->data.T.tid == EQ) {
        root->syn = createASTNode(root->children);
    }

    // relationalOp -> NE 
    else if (root->TorNT == 1 && root->data.NT.ntid == relationalOp && root->children->TorNT == 0 && root->children->data.T.tid == NE) {
        root->syn = createASTNode(root->children);
    }

    // declareStmt -> DECLARE idList COLON dataType SEMICOL
    else if (root->TorNT == 1 && root->data.NT.ntid == declareStmt) {
        t_node* idlist_ast = root->children->sibling;
        t_node* datatype_ast = idlist_ast->sibling->sibling;
        createAST(idlist_ast);
        createAST(datatype_ast);

        root->syn = createASTNode(root);
        root->syn->children = idlist_ast->syn;
        idlist_ast->syn->sibling = datatype_ast->syn;
    }

    // conditionalStmt -> SWITCH BO ID BC START caseStmts default END  
    else if (root->TorNT == 1 && root->data.NT.ntid == conditionalStmt) {
        t_node* id_ast = root->children->sibling->sibling;
        t_node* caseStmts_ast = id_ast->sibling->sibling->sibling;
        t_node* default_ast = caseStmts_ast->sibling;
        createAST(caseStmts_ast); 
        createAST(default_ast); 
        
        root->syn = createASTNode(root);
        id_ast->syn = createASTNode(id_ast);
        root->syn->children = id_ast->syn;
        id_ast->syn->sibling = caseStmts_ast->syn;
        caseStmts_ast->syn->sibling = default_ast->syn;

        root->syn->startLineNo = id_ast->sibling->sibling->data.T.lineNo;
        root->syn->endLineNo = default_ast->sibling->data.T.lineNo;
    }

    // caseStmts -> CASE value COLON statements BREAK SEMICOL N9
    else if (root->TorNT == 1 && root->data.NT.ntid == caseStmts) {
        t_node* val_ast = root->children->sibling;
        t_node* stmts_ast = val_ast->sibling->sibling;
        t_node* n9_ast = stmts_ast->sibling->sibling->sibling;
        createAST(val_ast);
        createAST(stmts_ast);
        createAST(n9_ast);

        root->syn = createASTNode(root);
        root->syn->children = val_ast->syn;
        val_ast->syn->sibling = stmts_ast->syn;
        stmts_ast->syn->sibling = n9_ast->syn;
    }

    // N9 -> CASE value COLON statements BREAK SEMICOL N9 
    else if (root->TorNT == 1 && root->data.NT.ntid == N9 && root->children->TorNT == 0 && root->children->data.T.tid == CASE) {
        t_node* val_ast = root->children->sibling;
        t_node* stmts_ast = val_ast->sibling->sibling;
        t_node* n9_ast = stmts_ast->sibling->sibling->sibling;
        createAST(val_ast);
        createAST(stmts_ast);
        createAST(n9_ast);

        val_ast->syn->sibling = stmts_ast->syn;
        stmts_ast->syn->sibling = n9_ast->syn;
        root->syn = val_ast->syn;
    }
    
    
    // N9 -> EPSILON
    else if (root->TorNT == 1 && root->data.NT.ntid == N9 && root->children->TorNT == 0 && root->children->data.T.tid == EPSILON) {
        root->syn = NULL;
    }

    // value -> NUM 
    else if (root->TorNT == 1 && root->data.NT.ntid == value && root->children->TorNT == 0 && root->children->data.T.tid == NUM) {
        root->syn = createASTNode(root->children);
        root->syn->datatype.tid = INTEGER;
    }
    
    // value -> TRUE 
    else if (root->TorNT == 1 && root->data.NT.ntid == value && root->children->TorNT == 0 && root->children->data.T.tid == TRUE) {
        root->syn = createASTNode(root->children);
        root->syn->datatype.tid = BOOLEAN;
    }

    // value -> FALSE 
    else if (root->TorNT == 1 && root->data.NT.ntid == value && root->children->TorNT == 0 && root->children->data.T.tid == FALSE) {
        root->syn = createASTNode(root->children);
        root->syn->datatype.tid = BOOLEAN;
    }

    // default -> DEFAULT COLON statements BREAK SEMICOL
    else if (root->TorNT == 1 && root->data.NT.ntid == _default && root->children->TorNT == 0 && root->children->data.T.tid == DEFAULT) {
        t_node* stmts_ast = root->children->sibling->sibling;
        createAST(stmts_ast);

        root->syn = createASTNode(root);
        root->syn->children = stmts_ast->syn;
        
        root->syn->startLineNo = root->children->data.T.lineNo;
        root->syn->endLineNo = stmts_ast->sibling->data.T.lineNo;
    }

    // default -> EPSILON 
    else if (root->TorNT == 1 && root->data.NT.ntid == _default && root->children->TorNT == 0 && root->children->data.T.tid == EPSILON) {
        root->syn = NULL;
    }

    // iterativeStmt -> FOR BO ID IN range BC START statements END
    else if (root->TorNT == 1 && root->data.NT.ntid == iterativeStmt && root->children->TorNT == 0 && root->children->data.T.tid == FOR) {
        t_node* for_ast = root->children;
        t_node* id_ast = for_ast->sibling->sibling;
        t_node* range_ast = id_ast->sibling->sibling;
        t_node* stmts_ast = range_ast->sibling->sibling->sibling;
        createAST(range_ast);
        createAST(stmts_ast);

        root->syn = createASTNode(root);
        for_ast->syn = createASTNode(for_ast);
        id_ast->syn = createASTNode(id_ast);
        root->syn->children = for_ast->syn;
        for_ast->syn->sibling = id_ast->syn;
        id_ast->syn->sibling = range_ast->syn;
        range_ast->syn->sibling = stmts_ast->syn;

        root->syn->startLineNo = range_ast->sibling->sibling->data.T.lineNo;
        root->syn->endLineNo = stmts_ast->sibling->data.T.lineNo;
    }
    
    // iterativeStmt -> WHILE BO arithmeticOrBooleanExpr BC START statements END
    else if (root->TorNT == 1 && root->data.NT.ntid == iterativeStmt && root->children->TorNT == 0 && root->children->data.T.tid == WHILE) {
        t_node* while_ast = root->children;
        t_node* abexpr_ast = while_ast->sibling->sibling;
        t_node* stmts_ast = abexpr_ast->sibling->sibling->sibling;
        createAST(abexpr_ast);
        createAST(stmts_ast);

        root->syn = createASTNode(root);
        while_ast->syn = createASTNode(while_ast);
        root->syn->children = while_ast->syn;
        while_ast->syn->sibling = abexpr_ast->syn;
        abexpr_ast->syn->sibling = stmts_ast->syn;

        root->syn->startLineNo = abexpr_ast->sibling->sibling->data.T.lineNo;
        root->syn->endLineNo = stmts_ast->sibling->data.T.lineNo;
    }
    
    // range -> NUM RANGEOP NUM
    else if (root->TorNT == 1 && root->data.NT.ntid == range) {
        t_node* num_1 = root->children;
        t_node* num_2 = num_1->sibling->sibling;
        
        root->syn = createASTNode(root);
        num_1->syn = createASTNode(num_1);
        num_2->syn = createASTNode(num_2);
        root->syn->children = num_1->syn;
        num_1->syn->sibling = num_2->syn;

        // not really needed here - but just for consistency
        num_1->syn->datatype.tid = INTEGER;
        num_2->syn->datatype.tid = INTEGER;
    }
}


// compute size of parseTree
void computeParseTreeSize(t_node* root) {

    if (root == NULL)
        return;
    
    parseTreeSize += sizeof(root);
    parseTreeNodes++;
    t_node* tmp = root->children;
    while (tmp) {
        computeParseTreeSize(tmp);
        tmp = tmp->sibling;
    }
}


// compute size of AST
void computeASTSize(astnode* root) {

    if (root == NULL)
        return;

    astSize += sizeof(root);
    astNodes++;
    astnode* tmp = root->children;
    while (tmp) {
        computeASTSize(tmp);
        tmp = tmp->sibling;
    }
}


// traversal of AST
void printAST(astnode* root) {
    // NULL node
    if (root == NULL)
        return;

    // non-terminal
    if (root->TorNT == 1)
        printf("%s\n", root->data.NT.name);

    // token
    else
        printf("%s\n", root->data.T.lexeme);
    
    astnode* tmp = root->children;
    while (tmp != NULL) {
        printAST(tmp);
        tmp = tmp->sibling;
    }
}


// int main(int argc, char* argv[]) {
//     parserfp = fopen("grammar.txt", "r");
//     readGrammar(parserfp);
//     fclose(parserfp);

//     computeFirstAndFollowSets();
//     initializeParseTree();
//     createParseTable();

//     fp = fopen(argv[1], "r");
//     if (fp == NULL)
//         printf("NULL");
//     getStream(fp);
//     parseInputSourceCode(argv[1]);
//     printParseTree(argv[2]);
//     fclose(fp);

//     createAST(parseTreeRoot);
//     printAST(parseTreeRoot->syn);
// }