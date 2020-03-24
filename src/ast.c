#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "parserutils.h"
#include "ast.h"


// helper to create new AST node
astnode* createASTNode(t_node* node) {
    astnode* newNode = (astnode*) malloc(sizeof(astnode));
    newNode->TorNT = node->TorNT;
    newNode->data = node->data;
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

        root->ASTnode =  createASTNode(root);
        root->ASTnode->children = md_ast->ASTnode;
        md_ast->ASTnode->sibling = om_ast->ASTnode;
        om_ast->ASTnode->sibling = dm_ast->ASTnode;
        dm_ast->ASTnode->sibling = dm_ast->sibling->ASTnode;
    }

    // moduleDeclarations -> moduleDeclaration moduleDeclarations
    else if (root->TorNT == 1 && root->data.NT.ntid == moduleDeclarations && root->children->TorNT == 1 && root->children->data.NT.ntid == moduleDeclaration) {
        t_node* md_ast = root->children;
        createAST(md_ast);
        createAST(md_ast->sibling);

        md_ast->ASTnode->sibling = md_ast->sibling->ASTnode;
        root->ASTnode = md_ast->ASTnode; 
    }

    // moduleDeclarations -> EPSILON 
    else if (root->TorNT == 1 && root->data.NT.ntid == moduleDeclarations && root->children->TorNT == 0 && root->children->data.T.tid == EPSILON) {
        root->ASTnode = NULL;
    }

    // moduleDeclaration -> DECLARE MODULE ID SEMICOL  
    else if (root->TorNT == 1 && root->data.NT.ntid == moduleDeclaration) {
        root->ASTnode = createASTNode(root->children->sibling->sibling);
    }

    // otherModules -> module otherModules
    else if (root->TorNT == 1 && root->data.NT.ntid == otherModules && root->children->TorNT == 1 && root->children->data.NT.ntid == module) {
        t_node* module_ast = root->children;
        t_node* othermodule_ast = module_ast->sibling;
        createAST(module_ast);
        createAST(othermodule_ast);

        module_ast->ASTnode->sibling = othermodule_ast->ASTnode;
        root->ASTnode = module_ast->ASTnode;
    }

    // otherModules -> EPSILON 
    else if (root->TorNT == 1 && root->data.NT.ntid == otherModules && root->children->TorNT == 0 && root->children->data.T.tid == EPSILON) {
        root->ASTnode = NULL; 
    }

    // driverModule -> DRIVERDEF DRIVER PROGRAM DRIVERENDDEF moduleDef
    else if (root->TorNT == 1 && root->data.NT.ntid == driverModule) {
        t_node* moduledef_ast = root->children->sibling->sibling->sibling->sibling;
        createAST(moduledef_ast);
        root->ASTnode = moduledef_ast->ASTnode;
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

        root->ASTnode = createASTNode(root);
        id_ast->ASTnode = createASTNode(id_ast);
        root->ASTnode->children = id_ast->ASTnode;
        id_ast->ASTnode->sibling = iplist_ast->ASTnode;
        iplist_ast->ASTnode->sibling = ret_ast->ASTnode;

        if (ret_ast->ASTnode != NULL)
            ret_ast->ASTnode->sibling = moduledef_ast->ASTnode;
        
        else
            iplist_ast->ASTnode->sibling = moduledef_ast->ASTnode;
    }

    // ret -> RETURNS SQBO output_plist SQBC SEMICOL
    else if (root->TorNT == 1 && root->data.NT.ntid == ret && root->children->TorNT == 0 && root->children->data.T.tid == RETURNS) {
        t_node* oplist_ast = root->children->sibling->sibling;
        createAST(oplist_ast);
        root->ASTnode = oplist_ast->ASTnode;
    }

    // ret -> EPSILON 
    else if (root->TorNT == 1 && root->data.NT.ntid == ret && root->children->TorNT == 0 && root->children->data.T.tid == EPSILON) {
        root->ASTnode = NULL;
    }

    // input_plist -> ID COLON dataType N1
    else if (root->TorNT == 1 && root->data.NT.ntid == input_plist) {
        t_node* id_ast = root->children;
        t_node* datatype_ast = id_ast->sibling->sibling;
        t_node* n1_ast = datatype_ast->sibling;
        createAST(datatype_ast);
        createAST(n1_ast);
        
        root->ASTnode = createASTNode(root);
        id_ast->ASTnode = createASTNode(id_ast);
        root->ASTnode->children = id_ast->ASTnode;
        id_ast->ASTnode->sibling = datatype_ast->ASTnode;
        datatype_ast->ASTnode->sibling = n1_ast->ASTnode;
    }

    // N1 -> COMMA ID COLON dataType N1
    else if (root->TorNT == 1 && root->data.NT.ntid == N1 && root->children->TorNT == 0 && root->children->data.T.tid == COMMA) {
        t_node* id_ast = root->children->sibling;
        t_node* datatype_ast = id_ast->sibling->sibling;
        t_node* n1_ast = datatype_ast->sibling;
        createAST(datatype_ast);
        createAST(n1_ast);

        id_ast->ASTnode = createASTNode(id_ast);
        id_ast->ASTnode->sibling = datatype_ast->ASTnode;
        datatype_ast->ASTnode->sibling = n1_ast->ASTnode;
        root->ASTnode = id_ast->ASTnode;
    }

    // N1 -> EPSILON
    else if (root->TorNT == 1 && root->data.NT.ntid == N1 && root->children->TorNT == 0 && root->children->data.T.tid == EPSILON) {
        root->ASTnode = NULL;
    }

    // output_plist -> ID COLON type N2
    else if (root->TorNT == 1 && root->data.NT.ntid == output_plist) {
        t_node* id_ast = root->children;
        t_node* type_ast = id_ast->sibling->sibling;
        t_node* n2_ast = type_ast->sibling;
        createAST(type_ast);
        createAST(n2_ast);

        root->ASTnode = createASTNode(root);
        id_ast->ASTnode = createASTNode(id_ast);
        root->ASTnode->children = id_ast->ASTnode;
        id_ast->ASTnode->sibling = type_ast->ASTnode;
        type_ast->ASTnode->sibling = n2_ast->ASTnode;
    }

    // N2 -> COMMA ID COLON type N2
    else if (root->TorNT == 1 && root->data.NT.ntid == N2 && root->children->TorNT == 0 && root->children->data.T.tid == COMMA) {
        t_node* id_ast = root->children->sibling;
        t_node* type_ast = id_ast->sibling->sibling;
        t_node* n2_ast = type_ast->sibling;
        createAST(type_ast);
        createAST(n2_ast);
        
        id_ast->ASTnode = createASTNode(id_ast);
        id_ast->ASTnode->sibling = type_ast->ASTnode;
        type_ast->ASTnode->sibling = n2_ast->ASTnode;
        root->ASTnode = id_ast->ASTnode;
    }

    // N2 -> EPSILON
    else if (root->TorNT == 1 && root->data.NT.ntid == N2 && root->children->TorNT == 0 && root->children->data.T.tid == EPSILON) {
        root->ASTnode = NULL;
    }

    // dataType -> INTEGER
    else if (root->TorNT == 1 && root->data.NT.ntid == dataType && root->children->TorNT == 0 && root->children->data.T.tid == INTEGER) {
        root->ASTnode = createASTNode(root->children);
    }

    // dataType -> REAL
    else if (root->TorNT == 1 && root->data.NT.ntid == dataType && root->children->TorNT == 0 && root->children->data.T.tid == REAL) {
        root->ASTnode = createASTNode(root->children);
    }

    // dataType -> BOOLEAN
    else if (root->TorNT == 1 && root->data.NT.ntid == dataType && root->children->TorNT == 0 && root->children->data.T.tid == BOOLEAN) {
        root->ASTnode = createASTNode(root->children);
    }
    
    // dataType -> ARRAY SQBO range_arrays SQBC OF type
    else if (root->TorNT == 1 && root->data.NT.ntid == dataType && root->children->TorNT == 0 && root->children->data.T.tid == ARRAY) {
        t_node* ra_ast = root->children->sibling->sibling;
        t_node* type_ast = ra_ast->sibling->sibling;
        createAST(ra_ast);
        createAST(type_ast);

        root->ASTnode = ra_ast->ASTnode;
        ra_ast->ASTnode->sibling->sibling = type_ast->ASTnode; // should be sibling to index_2
    }

    // range_arrays -> index RANGEOP index
    else if (root->TorNT == 1 && root->data.NT.ntid == range_arrays) {
        t_node* index_1 = root->children;
        t_node* index_2 = index_1->sibling->sibling;
        createAST(index_1);
        createAST(index_2);   

        index_1->ASTnode->sibling = index_2->ASTnode;
        root->ASTnode = index_1->ASTnode;
    }

    // type -> INTEGER 
    else if (root->TorNT == 1 && root->data.NT.ntid == type && root->children->TorNT == 0 && root->children->data.T.tid == INTEGER) {
        root->ASTnode = createASTNode(root->children);
    }

    // type -> REAL
    else if (root->TorNT == 1 && root->data.NT.ntid == type && root->children->TorNT == 0 && root->children->data.T.tid == REAL) {
        root->ASTnode = createASTNode(root->children);
    }

    // type -> BOOLEAN
    else if (root->TorNT == 1 && root->data.NT.ntid == type && root->children->TorNT == 0 && root->children->data.T.tid == BOOLEAN) {
        root->ASTnode = createASTNode(root->children);
    }

    // moduleDef -> START statements END 
    else if (root->TorNT == 1 && root->data.NT.ntid == moduleDef) {
        t_node* stmts_ast = root->children->sibling;
        createAST(stmts_ast);
        root->ASTnode = stmts_ast->ASTnode;
    }

    // statements -> statement statements
    else if (root->TorNT == 1 && root->data.NT.ntid == statements && root->children->TorNT == 1 && root->children->data.NT.ntid == statement) {
        t_node* stmt_ast = root->children;
        t_node* stmts_ast = stmt_ast->sibling;
        createAST(stmt_ast);
        createAST(stmts_ast);

        root->ASTnode = stmt_ast->ASTnode;
        stmt_ast->ASTnode->sibling = stmts_ast->ASTnode;
    }
    
    // statements -> EPSILON 
    else if (root->TorNT == 1 && root->data.NT.ntid == statements && root->children->TorNT == 0 && root->children->data.T.tid == EPSILON) {
        root->ASTnode = NULL;
    }
    
    // statement -> ioStmt
    else if (root->TorNT == 1 && root->data.NT.ntid == statement && root->children->TorNT == 1 && root->children->data.NT.ntid == ioStmt) {
        t_node* iostmt_ast = root->children;
        createAST(iostmt_ast);
        root->ASTnode = iostmt_ast->ASTnode;
    }
    
    // statement -> simpleStmt
    else if (root->TorNT == 1 && root->data.NT.ntid == statement && root->children->TorNT == 1 && root->children->data.NT.ntid == simpleStmt) {
        t_node* simple_ast = root->children;
        createAST(simple_ast);
        root->ASTnode = simple_ast->ASTnode;
    }    
    
    // statement -> declareStmt
    else if (root->TorNT == 1 && root->data.NT.ntid == statement && root->children->TorNT == 1 && root->children->data.NT.ntid == declareStmt) {
        t_node* declare_ast = root->children;
        createAST(declare_ast);
        root->ASTnode = declare_ast->ASTnode;
    }    
    
    // statement -> conditionalStmt
    else if (root->TorNT == 1 && root->data.NT.ntid == statement && root->children->TorNT == 1 && root->children->data.NT.ntid == conditionalStmt) {
        t_node* condStmt_ast = root->children;
        createAST(condStmt_ast);
        root->ASTnode = condStmt_ast->ASTnode;
    }    
    
    // statement -> iterativeStmt
    else if (root->TorNT == 1 && root->data.NT.ntid == statement && root->children->TorNT == 1 && root->children->data.NT.ntid == iterativeStmt) {
        t_node* iterStmt_ast = root->children;
        createAST(iterStmt_ast);
        root->ASTnode = iterStmt_ast->ASTnode;
    }

    // ioStmt -> GET_VALUE BO ID BC SEMICOL
    else if (root->TorNT == 1 && root->data.NT.ntid == ioStmt && root->children->TorNT == 0 && root->children->data.T.tid == GET_VALUE) {
        t_node* id_ast = root->children->sibling->sibling;
        id_ast->ASTnode = createASTNode(id_ast);
        root->ASTnode = id_ast->ASTnode;
    }
    
    // ioStmt -> PRINT BO var BC SEMICOL 
    else if (root->TorNT == 1 && root->data.NT.ntid == ioStmt && root->children->TorNT == 0 && root->children->data.T.tid == GET_VALUE) {
        t_node* var_ast = root->children->sibling->sibling;
        createAST(var_ast);
        root->ASTnode = var_ast->ASTnode;
    }

    // boolConstt -> TRUE
    else if (root->TorNT == 1 && root->data.NT.ntid == boolConstt && root->children->TorNT == 0 && root->children->data.T.tid == TRUE) {
        root->ASTnode = createASTNode(root->children);
    }

    // boolConstt -> FALSE
    else if (root->TorNT == 1 && root->data.NT.ntid == boolConstt && root->children->TorNT == 0 && root->children->data.T.tid == FALSE) {
        root->ASTnode = createASTNode(root->children);
    }

    // var_id_num -> ID whichId
    else if (root->TorNT == 1 && root->data.NT.ntid == var_id_num && root->children->TorNT == 0 && root->children->data.T.tid == ID) {
        t_node* id_ast = root->children;
        t_node* whichid_ast = id_ast->sibling;
        createAST(whichid_ast);

        id_ast->ASTnode = createASTNode(id_ast);
        id_ast->ASTnode->sibling = whichid_ast->ASTnode;
        root->ASTnode = id_ast->ASTnode;
    }
    
    // var_id_num -> NUM
    else if (root->TorNT == 1 && root->data.NT.ntid == var_id_num && root->children->TorNT == 0 && root->children->data.T.tid == NUM) {
        root->ASTnode = createASTNode(root->children);
    }

    // var_id_num -> RNUM
    else if (root->TorNT == 1 && root->data.NT.ntid == var_id_num && root->children->TorNT == 0 && root->children->data.T.tid == RNUM) {
        root->ASTnode = createASTNode(root->children);
    }

    // var -> var_id_num
    else if (root->TorNT == 1 && root->data.NT.ntid == var && root->children->TorNT == 1 && root->children->data.NT.ntid == var_id_num) {
        t_node* varIdNum_ast = root->children;
        createAST(varIdNum_ast);

        root->ASTnode = createASTNode(root);
        root->ASTnode->children = varIdNum_ast->ASTnode;
    }

    // var -> boolConstt
    else if (root->TorNT == 1 && root->data.NT.ntid == var && root->children->TorNT == 1 && root->children->data.NT.ntid == boolConstt) {
        t_node* boolconst_ast = root->children;
        createAST(boolconst_ast);

        root->ASTnode = createASTNode(root);
        root->ASTnode->children = boolconst_ast->ASTnode;
    }

    // whichId -> SQBO index SQBC
    else if (root->TorNT == 1 && root->data.NT.ntid == var_id_num && root->children->TorNT == 0 && root->children->data.T.tid == SQBO) {
        t_node* index_ast = root->children->sibling;
        createAST(index_ast);
        root->ASTnode = index_ast->ASTnode;
    }

    // whichId -> EPSILON 
    else if (root->TorNT == 1 && root->data.NT.ntid == whichId && root->children->TorNT == 0 && root->children->data.T.tid == EPSILON) {
        root->ASTnode = NULL;
    }

    // simpleStmt -> assignmentStmt
    else if (root->TorNT == 1 && root->data.NT.ntid == simpleStmt && root->children->TorNT == 1 && root->children->data.NT.ntid == assignmentStmt) {
        t_node* assign_ast = root->children;
        createAST(assign_ast);
        root->ASTnode = assign_ast->ASTnode;
    }

    // simpleStmt -> moduleReuseStmt
    else if (root->TorNT == 1 && root->data.NT.ntid == simpleStmt && root->children->TorNT == 1 && root->children->data.NT.ntid == moduleReuseStmt) {
        t_node* moduleReuse_ast = root->children;
        createAST(moduleReuse_ast);
        root->ASTnode = moduleReuse_ast->ASTnode;
    }

    // assignmentStmt -> ID whichStmt 
    else if (root->TorNT == 1 && root->data.NT.ntid == assignmentStmt) {
        t_node* id_ast = root->children;
        t_node* whichStmt_ast = id_ast->sibling;
        id_ast->ASTnode = createASTNode(id_ast);
        whichStmt_ast->inh = id_ast->ASTnode;
        createAST(whichStmt_ast);
        
        root->ASTnode = whichStmt_ast->ASTnode;        
    }
    
    // whichStmt -> lvalueIDStmt
    else if (root->TorNT == 1 && root->data.NT.ntid == whichStmt && root->children->TorNT == 1 && root->children->data.NT.ntid == lvalueIDStmt) {
        t_node* lIdStmt_ast = root->children;
        lIdStmt_ast->inh = root->inh;
        createAST(lIdStmt_ast);
        root->ASTnode = lIdStmt_ast->ASTnode;
    }

    // whichStmt -> lvalueARRStmt
    else if (root->TorNT == 1 && root->data.NT.ntid == whichStmt && root->children->TorNT == 1 && root->children->data.NT.ntid == lvalueARRStmt) {
        t_node* lArrStmt_ast = root->children;
        lArrStmt_ast->inh = root->inh;
        createAST(lArrStmt_ast);
        root->ASTnode = lArrStmt_ast->ASTnode;
    }

    // lvalueIDStmt -> ASSIGNOP expression SEMICOL
    else if (root->TorNT == 1 && root->data.NT.ntid == lvalueIDStmt) {
        t_node* assign_ast = root->children;
        t_node* expr_ast = assign_ast->sibling;
        createAST(expr_ast);
        
        root->ASTnode = createASTNode(assign_ast);
        root->ASTnode->children = root->inh;
        root->ASTnode->children->sibling = expr_ast->ASTnode;
    }


    // lvalueARRStmt -> SQBO index SQBC ASSIGNOP expression SEMICOL
    else if (root->TorNT == 1 && root->data.NT.ntid == lvalueARRStmt) {
        t_node* index_ast = root->children->sibling;
        t_node* assign_ast = index_ast->sibling->sibling;
        t_node* expr_ast = assign_ast->sibling;
        createAST(index_ast);
        createAST(expr_ast);

        root->ASTnode = createASTNode(assign_ast);
        root->ASTnode->children = root->inh;
        root->ASTnode->children->sibling = index_ast->ASTnode;
        root->ASTnode->children->sibling->sibling = expr_ast->ASTnode;
    }

    // index -> NUM 
    else if (root->TorNT == 1 && root->data.NT.ntid == _index && root->children->TorNT == 0 && root->children->data.T.tid == NUM) {
        root->ASTnode = createASTNode(root->children);
    }
    
    // index -> ID 
    else if (root->TorNT == 1 && root->data.NT.ntid == _index && root->children->TorNT == 0 && root->children->data.T.tid == ID) {
        root->ASTnode = createASTNode(root->children);
    }

    // moduleReuseStmt -> optional USE MODULE ID WITH PARAMETERS idList SEMICOL
    else if (root->TorNT == 1 && root->data.NT.ntid == moduleReuseStmt) {
        t_node* opt_ast = root->children;
        t_node* id_ast = opt_ast->sibling->sibling->sibling;
        t_node* idlist_ast = id_ast->sibling->sibling->sibling;
        createAST(opt_ast);
        createAST(idlist_ast);
        
        id_ast->ASTnode = createASTNode(id_ast);
        opt_ast->ASTnode->sibling = id_ast->ASTnode;
        id_ast->ASTnode->sibling = idlist_ast->ASTnode;
        root->ASTnode = id_ast->ASTnode;
    }

    // optional -> SQBO idList SQBC ASSIGNOP 
    else if (root->TorNT == 1 && root->data.NT.ntid == optional && root->children->TorNT == 0 && root->children->data.NT.ntid == SQBO) {
        t_node* idlist_ast = root->children->sibling;
        t_node* assign_ast = idlist_ast->sibling->sibling;
        createAST(idlist_ast);
        
        assign_ast->ASTnode = createASTNode(assign_ast);
        root->ASTnode = idlist_ast->ASTnode;
        idlist_ast->ASTnode->sibling = assign_ast->ASTnode;
    }
    
    // optional -> EPSILON
    else if (root->TorNT == 1 && root->data.NT.ntid == optional && root->children->TorNT == 0 && root->children->data.T.tid == EPSILON) {
        root->ASTnode = NULL;
    }

    // idList -> ID N3
    else if (root->TorNT == 1 && root->data.NT.ntid == idList) {
        t_node* id_ast = root->children;
        t_node* n3_ast = id_ast->sibling;
        createAST(n3_ast);
        
        id_ast->ASTnode = createASTNode(id_ast);
        id_ast->ASTnode->sibling = n3_ast->ASTnode;
        root->ASTnode = id_ast->ASTnode;
    }

    // N3 -> COMMA ID N3
    else if (root->TorNT == 1 && root->data.NT.ntid == N3 && root->children->TorNT == 0 && root->children->data.NT.ntid == COMMA) {
        t_node* id_ast = root->children->sibling;
        t_node* n3_ast = id_ast->sibling;
        createAST(n3_ast);
        
        id_ast->ASTnode = createASTNode(id_ast);
        id_ast->ASTnode->sibling = n3_ast->ASTnode;
        root->ASTnode = id_ast->ASTnode;
    }
    
    // N3 -> EPSILON
    else if (root->TorNT == 1 && root->data.NT.ntid == N3 && root->children->TorNT == 0 && root->children->data.T.tid == EPSILON) {
        root->ASTnode = NULL;
    }

    // expression -> arithmeticOrBooleanExpr
    else if (root->TorNT == 1 && root->data.NT.ntid == expression && root->children->TorNT == 1 && root->children->data.NT.ntid == arithmeticOrBooleanExpr) {
        t_node* abexpr_ast = root->children;
        createAST(abexpr_ast);
        root->ASTnode = abexpr_ast->ASTnode;
    }

    // expression -> U
    else if (root->TorNT == 1 && root->data.NT.ntid == expression && root->children->TorNT == 1 && root->children->data.NT.ntid == U) {
        t_node* u_ast = root->children;
        createAST(u_ast);
        root->ASTnode = u_ast->ASTnode;
    }

    // U -> unary_op new_NT
    else if (root->TorNT == 1 && root->data.NT.ntid == U) {
        t_node* unaryop_ast = root->children;
        t_node* newNT_ast = unaryop_ast->sibling;
        createAST(unaryop_ast);
        createAST(newNT_ast);

        root->ASTnode = unaryop_ast->ASTnode;
        unaryop_ast->ASTnode->sibling = newNT_ast->ASTnode;
    }
    
    // new_NT -> BO arithmeticExpr BC
    else if (root->TorNT == 1 && root->data.NT.ntid == new_NT && root->children->TorNT == 0 && root->children->data.T.tid == BO) {
        t_node* arithExpr_ast = root->children->sibling;
        createAST(arithExpr_ast);
        root->ASTnode = arithExpr_ast->ASTnode;

    }

    // new_NT -> var_id_num
    else if (root->TorNT == 1 && root->data.NT.ntid == new_NT && root->children->TorNT == 1 && root->children->data.NT.ntid == var_id_num) {
        t_node* varIdNum_ast = root->children;
        createAST(root->children);
        root->ASTnode = varIdNum_ast->ASTnode;
    }

    // unary_op -> PLUS
    else if (root->TorNT == 1 && root->data.NT.ntid == unary_op && root->children->TorNT == 0 && root->children->data.NT.ntid == PLUS) {
        root->ASTnode = createASTNode(root->children);
    }

    // unary_op -> MINUS
    else if (root->TorNT == 1 && root->data.NT.ntid == unary_op && root->children->TorNT == 0 && root->children->data.NT.ntid == MINUS) {
        root->ASTnode = createASTNode(root->children);
    }

    // arithmeticOrBooleanExpr -> anyTerm N7
    else if (root->TorNT == 1 && root->data.NT.ntid == arithmeticOrBooleanExpr) {
        t_node* anyterm_ast = root->children;
        t_node* n7_ast = anyterm_ast->sibling;
        createAST(anyterm_ast);
        n7_ast->inh = anyterm_ast->ASTnode;
        createAST(n7_ast);

        root->ASTnode = n7_ast->ASTnode;
    }

    // N7 -> logicalOp anyTerm N7
    else if (root->TorNT == 1 && root->data.NT.ntid == N7 && root->children->TorNT == 1 && root->children->data.NT.ntid == logicalOp) {
        t_node* lop_ast = root->children;
        t_node* anyterm_ast = lop_ast->sibling;
        t_node* n7_ast = anyterm_ast->sibling;
        createAST(lop_ast);
        createAST(anyterm_ast);
        n7_ast->inh = anyterm_ast->ASTnode;
        createAST(n7_ast);

        root->ASTnode = lop_ast->ASTnode;
        root->ASTnode->children = root->inh;
        root->ASTnode->children->sibling = n7_ast->ASTnode;
    }

    // N7 -> EPSILON
    else if (root->TorNT == 1 && root->data.NT.ntid == N7 && root->children->TorNT == 0 && root->children->data.T.tid == EPSILON) {
        root->ASTnode = root->inh;
    }
    
    // anyTerm -> arithmeticExpr N8
    else if (root->TorNT == 1 && root->data.NT.ntid == anyTerm && root->children->TorNT == 1 && root->children->data.NT.ntid == arithmeticExpr) {
        t_node* arith_ast = root->children;
        t_node* n8_ast = arith_ast->sibling;
        createAST(arith_ast);
        n8_ast->inh = arith_ast->ASTnode;
        createAST(n8_ast);

        root->ASTnode = n8_ast->ASTnode;
    }
 
    // anyTerm -> boolConstt
    else if (root->TorNT == 1 && root->data.NT.ntid == anyTerm && root->children->TorNT == 1 && root->children->data.NT.ntid == boolConstt) {
        t_node* boolConst_ast = root->children;
        createAST(boolConst_ast);
        root->ASTnode = boolConst_ast->ASTnode;
    }

    // N8 -> relationalOp arithmeticExpr
    else if (root->TorNT == 1 && root->data.NT.ntid == N8 && root->children->TorNT == 1 && root->children->data.NT.ntid == relationalOp) {
        t_node* rop_ast = root->children;
        t_node* arith_ast = rop_ast->sibling;
        createAST(rop_ast);
        createAST(arith_ast);

        root->ASTnode = rop_ast->ASTnode;
        root->ASTnode->children = root->inh;
        root->ASTnode->children->sibling = arith_ast->ASTnode;
    }
    
    // N8 -> EPSILON
    else if (root->TorNT == 1 && root->data.NT.ntid == N8 && root->children->TorNT == 0 && root->children->data.T.tid == EPSILON) {
        root->ASTnode = root->inh;
    }


    // arithmeticExpr -> term N4
    else if (root->TorNT == 1 && root->data.NT.ntid == arithmeticExpr) {
        t_node* term_ast = root->children;
        t_node* n4_ast = term_ast->sibling;
        createAST(term_ast);
        n4_ast->inh = term_ast->ASTnode;
        createAST(n4_ast);
        
        root->ASTnode = n4_ast->ASTnode;
    }

    // N4 -> op1 term N4
    else if (root->TorNT == 1 && root->data.NT.ntid == N4 && root->children->TorNT == 1 && root->children->data.NT.ntid == op1) {
        t_node* op1_ast = root->children;
        t_node* term_ast = op1_ast->sibling;
        t_node* n4_ast = term_ast->sibling;
        createAST(op1_ast);
        createAST(term_ast);
        n4_ast->inh = term_ast->ASTnode;
        createAST(n4_ast);

        root->ASTnode = op1_ast->ASTnode;
        root->ASTnode->children = root->inh;
        root->ASTnode->children->sibling = n4_ast->ASTnode;
    }

    // N4 -> EPSILON
    else if (root->TorNT == 1 && root->data.NT.ntid == N4 && root->children->TorNT == 0 && root->children->data.T.tid == EPSILON) {
        root->ASTnode = root->inh;
    }


    // term -> factor N5
    else if (root->TorNT == 1 && root->data.NT.ntid == term) {
        t_node* factor_ast = root->children;
        t_node* n5_ast = factor_ast->sibling;
        createAST(factor_ast);
        n5_ast->inh = factor_ast->ASTnode;
        createAST(n5_ast);

        root->ASTnode = n5_ast->ASTnode;
    }


    // N5 -> op2 factor N5
    else if (root->TorNT == 1 && root->data.NT.ntid == N5 && root->children->TorNT == 1 && root->children->data.NT.ntid == op2) {
        t_node* op2_ast = root->children;
        t_node* factor_ast = op2_ast->sibling;
        t_node* n5_ast = factor_ast->sibling;
        createAST(op2_ast);
        createAST(factor_ast);
        n5_ast->inh = factor_ast->ASTnode;
        createAST(n5_ast);

        root->ASTnode = op2_ast->ASTnode;
        root->ASTnode->children = root->inh;
        root->ASTnode->children->sibling = n5_ast->ASTnode;
    }    
    
    // N5 -> EPSILON
    else if (root->TorNT == 1 && root->data.NT.ntid == N5 && root->children->TorNT == 0 && root->children->data.T.tid == EPSILON) {
        root->ASTnode = root->inh;
    }

    // factor -> BO arithmeticOrBooleanExpr BC
    else if (root->TorNT == 1 && root->data.NT.ntid == factor && root->children->TorNT == 0 && root->children->data.T.tid == BO) {
        t_node* abexpr_ast = root->children->sibling;
        createAST(abexpr_ast);
        root->ASTnode = abexpr_ast->ASTnode;
    }
    
    // factor -> var_id_num
    else if (root->TorNT == 1 && root->data.NT.ntid == factor && root->children->TorNT == 1 && root->children->data.NT.ntid == var_id_num) {
        t_node* varIdNum_ast = root->children;
        createAST(varIdNum_ast);
        root->ASTnode = varIdNum_ast->ASTnode;
    }
    
    // op1 -> PLUS
    else if (root->TorNT == 1 && root->data.NT.ntid == op1 && root->children->TorNT == 0 && root->children->data.T.tid == PLUS) {
        root->ASTnode = createASTNode(root->children);
    }

    // op1 -> MINUS
    else if (root->TorNT == 1 && root->data.NT.ntid == op1 && root->children->TorNT == 0 && root->children->data.T.tid == MINUS) {
        root->ASTnode = createASTNode(root->children);
    }

    // op2 -> MUL
    else if (root->TorNT == 1 && root->data.NT.ntid == op1 && root->children->TorNT == 0 && root->children->data.T.tid == MUL) {
        root->ASTnode = createASTNode(root->children);
    }

    // op2 -> DIV
    else if (root->TorNT == 1 && root->data.NT.ntid == op1 && root->children->TorNT == 0 && root->children->data.T.tid == DIV) {
        root->ASTnode = createASTNode(root->children);
    }

    // logicalOp -> AND
    else if (root->TorNT == 1 && root->data.NT.ntid == logicalOp && root->children->TorNT == 0 && root->children->data.T.tid == AND) {
        createASTNode(root->children);
    }

    // logicalOp -> OR
    else if (root->TorNT == 1 && root->data.NT.ntid == logicalOp && root->children->TorNT == 0 && root->children->data.T.tid == OR) {
        root->ASTnode = createASTNode(root->children);
    }

    // relationalOp -> LT
    else if (root->TorNT == 1 && root->data.NT.ntid == relationalOp && root->children->TorNT == 0 && root->children->data.T.tid == LT) {
        root->ASTnode = createASTNode(root->children);
    }

    // relationalOp -> LE 
    else if (root->TorNT == 1 && root->data.NT.ntid == relationalOp && root->children->TorNT == 0 && root->children->data.T.tid == LE) {
        root->ASTnode = createASTNode(root->children);
    }

    // relationalOp -> GT 
    else if (root->TorNT == 1 && root->data.NT.ntid == relationalOp && root->children->TorNT == 0 && root->children->data.T.tid == GT) {
        root->ASTnode = createASTNode(root->children);
    }

    // relationalOp -> GE 
    else if (root->TorNT == 1 && root->data.NT.ntid == relationalOp && root->children->TorNT == 0 && root->children->data.T.tid == GE) {
        root->ASTnode = createASTNode(root->children);
    }

    // relationalOp -> EQ 
    else if (root->TorNT == 1 && root->data.NT.ntid == relationalOp && root->children->TorNT == 0 && root->children->data.T.tid == EQ) {
        root->ASTnode = createASTNode(root->children);
    }

    // relationalOp -> NE 
    else if (root->TorNT == 1 && root->data.NT.ntid == relationalOp && root->children->TorNT == 0 && root->children->data.T.tid == NE) {
        root->ASTnode = createASTNode(root->children);
    }

    // declareStmt -> DECLARE idList COLON dataType SEMICOL -- CHECK THIS??
    else if (root->TorNT == 1 && root->data.NT.ntid == declareStmt) {
        t_node* idlist_ast = root->children->sibling;
        t_node* datatype_ast = idlist_ast->sibling->sibling;
        createAST(idlist_ast);
        createAST(datatype_ast);

        root->ASTnode = idlist_ast->ASTnode;
        idlist_ast->ASTnode->sibling = datatype_ast->ASTnode;
    }

    // conditionalStmt -> SWITCH BO ID BC START caseStmts default END  
    else if (root->TorNT == 1 && root->data.NT.ntid == conditionalStmt) {
        t_node* id_ast = root->children->sibling->sibling;
        t_node* caseStmts_ast = id_ast->sibling->sibling->sibling;
        t_node* default_ast = caseStmts_ast->sibling;
        createAST(caseStmts_ast); 
        createAST(default_ast); 
        
        id_ast->ASTnode = createASTNode(id_ast);
        id_ast->ASTnode->sibling = caseStmts_ast->ASTnode;
        caseStmts_ast->ASTnode->sibling = default_ast->ASTnode;
        root->ASTnode = id_ast->ASTnode;
    }

    // caseStmts -> CASE value COLON statements BREAK SEMICOL N9
    else if (root->TorNT == 1 && root->data.NT.ntid == caseStmts) {
        t_node* val_ast = root->children->sibling;
        t_node* stmts_ast = val_ast->sibling->sibling;
        t_node* n9_ast = stmts_ast->sibling->sibling->sibling;
        createAST(val_ast);
        createAST(stmts_ast);
        createAST(n9_ast);

        val_ast->ASTnode->sibling = stmts_ast->ASTnode;
        stmts_ast->ASTnode->sibling = n9_ast->ASTnode;
        root->ASTnode = val_ast->ASTnode;
    }

    // N9 -> CASE value COLON statements BREAK SEMICOL N9 
    else if (root->TorNT == 1 && root->data.NT.ntid == N9 && root->children->TorNT == 0 && root->children->data.T.tid == CASE) {
        t_node* val_ast = root->children->sibling;
        t_node* stmts_ast = val_ast->sibling->sibling;
        t_node* n9_ast = stmts_ast->sibling->sibling->sibling;
        createAST(val_ast);
        createAST(stmts_ast);
        createAST(n9_ast);

        val_ast->ASTnode->sibling = stmts_ast->ASTnode;
        stmts_ast->ASTnode->sibling = n9_ast->ASTnode;
        root->ASTnode = val_ast->ASTnode;
    }
    
    
    // N9 -> EPSILON
    else if (root->TorNT == 1 && root->data.NT.ntid == N9 && root->children->TorNT == 0 && root->children->data.T.tid == EPSILON) {
        root->ASTnode = NULL;
    }

    // value -> NUM 
    else if (root->TorNT == 1 && root->data.NT.ntid == value && root->children->TorNT == 0 && root->children->data.T.tid == NUM) {
        root->ASTnode = createASTNode(root->children);
    }
    
    // value -> TRUE 
    else if (root->TorNT == 1 && root->data.NT.ntid == value && root->children->TorNT == 0 && root->children->data.T.tid == TRUE) {
        root->ASTnode = createASTNode(root->children);
    }

    // value -> FALSE 
    else if (root->TorNT == 1 && root->data.NT.ntid == value && root->children->TorNT == 0 && root->children->data.T.tid == FALSE) {
        root->ASTnode = createASTNode(root->children);
    }

    // default -> DEFAULT COLON statements BREAK SEMICOL
    else if (root->TorNT == 1 && root->data.NT.ntid == _default && root->children->TorNT == 0 && root->children->data.T.tid == DEFAULT) {
        t_node* stmts_ast = root->children->sibling->sibling;
        createAST(stmts_ast);
        root->ASTnode = stmts_ast->ASTnode;
    }


    // default -> EPSILON 
    else if (root->TorNT == 1 && root->data.NT.ntid == _default && root->children->TorNT == 0 && root->children->data.T.tid == EPSILON) {
        root->ASTnode = NULL;
    }

    // iterativeStmt -> FOR BO ID IN range BC START statements END
    else if (root->TorNT == 1 && root->data.NT.ntid == iterativeStmt && root->children->TorNT == 0 && root->children->data.T.tid == FOR) {
        t_node* id_ast = root->children->sibling->sibling;
        t_node* range_ast = id_ast->sibling->sibling;
        t_node* stmts_ast = range_ast->sibling->sibling->sibling;
        createAST(range_ast);
        createAST(stmts_ast);

        id_ast->ASTnode = createASTNode(id_ast);
        id_ast->ASTnode->sibling = range_ast->ASTnode;
        range_ast->ASTnode->sibling = stmts_ast->ASTnode;
        root->ASTnode = id_ast->ASTnode;
    }
    
    // iterativeStmt -> WHILE BO arithmeticOrBooleanExpr BC START statements END
    else if (root->TorNT == 1 && root->data.NT.ntid == iterativeStmt && root->children->TorNT == 0 && root->children->data.T.tid == WHILE) {
        t_node* abexpr_ast = root->children->sibling->sibling;
        t_node* stmts_ast = abexpr_ast->sibling->sibling->sibling;
        createAST(abexpr_ast);
        createAST(stmts_ast);

        root->ASTnode = abexpr_ast->ASTnode;
        abexpr_ast->ASTnode->sibling = stmts_ast->ASTnode;
    }
    
    // range -> NUM RANGEOP NUM
    else if (root->TorNT == 1 && root->data.NT.ntid == range) {
        t_node* num_1 = root->children;
        t_node* num_2 = num_1->sibling->sibling;
        
        num_1->ASTnode = createASTNode(num_1);
        num_2->ASTnode = createASTNode(num_2);
        num_1->ASTnode->sibling = num_2->ASTnode;
        root->ASTnode = num_1->ASTnode;
    }
}


int main() {
    createAST(parseTreeRoot);
}