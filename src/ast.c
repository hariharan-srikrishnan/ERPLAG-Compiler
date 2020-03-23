#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "parserutils.h"
#include "ast.h"


astnode* ast_root;


// helper to create new AST node
astnode* createASTNode(t_node* node) {
    astnode* newNode = (astnode*) malloc(sizeof(astnode));
    return newNode;
}


void createAST(t_node* root) {

    // program -> moduleDeclarations otherModules driverModule otherModules 
    if (root->TorNT == 1 && root->data.NT.ntid == program) {
        createAST(root->children);
        createAST(root->children->sibling);
        createAST(root->children->sibling->sibling);
        createAST(root->children->sibling->sibling->sibling);
    }

    // moduleDeclarations -> moduleDeclaration moduleDeclarations
    else if (root->TorNT == 1 && root->data.NT.ntid == moduleDeclarations && root->children->TorNT == 1 && root->children->data.NT.ntid == moduleDeclaration) {
        createAST(root->children);
        createAST(root->children->sibling);
    }

    // moduleDeclarations -> EPSILON 
    else if (root->TorNT == 1 && root->data.NT.ntid == moduleDeclarations && root->children->TorNT == 0 && root->children->data.T.tid == EPSILON) {
        return; // ???
    }

    // moduleDeclaration -> DECLARE MODULE ID SEMICOL  
    else if (root->TorNT == 1 && root->data.NT.ntid == moduleDeclaration) {
        createASTNode(root->children->sibling->sibling);
    }

    // otherModules -> module otherModules
    else if (root->TorNT == 1 && root->data.NT.ntid == otherModules && root->children->TorNT == 1 && root->children->data.NT.ntid == module) {
        createAST(root->children);
        createAST(root->children->sibling);
    }

    // otherModules -> EPSILON 
    else if (root->TorNT == 1 && root->data.NT.ntid == otherModules && root->children->TorNT == 0 && root->children->data.T.tid == EPSILON) {
        return; // ???
    }

    // driverModule -> DRIVERDEF DRIVER PROGRAM DRIVERENDDEF moduleDef
    else if (root->TorNT == 1 && root->data.NT.ntid == driverModule) {
        createAST(root->children->sibling->sibling->sibling->sibling);
    }

    // module -> DEF MODULE ID ENDDEF TAKES INPUT SQBO input_plist SQBC SEMICOL ret moduleDef
    else if (root->TorNT == 1 && root->data.NT.ntid == module) {
        t_node* iplist_ast = root->children->sibling->sibling->sibling->sibling->sibling->sibling->sibling;
        t_node* ret_ast = iplist_ast->sibling->sibling->sibling;
        createAST(iplist_ast);
        createAST(ret_ast);
        createAST(ret_ast->sibling);

        createASTNode(root->children->sibling->sibling);
    }

    // ret -> RETURNS SQBO output_plist SQBC SEMICOL
    else if (root->TorNT == 1 && root->data.NT.ntid == ret && root->children->TorNT == 0 && root->children->data.T.tid == RETURNS) {
        createAST(root->children->sibling->sibling);
    }

    // ret -> EPSILON 
    else if (root->TorNT == 1 && root->data.NT.ntid == ret && root->children->TorNT == 0 && root->children->data.T.tid == EPSILON) {
        return; // ???
    }

    // input_plist -> ID COLON dataType N1
    else if (root->TorNT == 1 && root->data.NT.ntid == input_plist) {
        t_node* id_ast = root->children;
        t_node* datatype_ast = id_ast->sibling->sibling;
        createAST(datatype_ast);
        createAST(datatype_ast->sibling);
        createASTNode(id_ast);
    }

    // N1 -> COMMA ID COLON dataType N1
    else if (root->TorNT == 1 && root->data.NT.ntid == N1 && root->children->TorNT == 0 && root->children->data.T.tid == COMMA) {
        t_node* id_ast = root->children->sibling;
        t_node* datatype_ast = id_ast->sibling->sibling;
        createAST(datatype_ast);
        createAST(datatype_ast->sibling);
        createASTNode(id_ast);
    }

    // N1 -> EPSILON
    else if (root->TorNT == 1 && root->data.NT.ntid == N1 && root->children->TorNT == 0 && root->children->data.T.tid == EPSILON) {
        return; // ???
    }

    // output_plist -> ID COLON type N2
    else if (root->TorNT == 1 && root->data.NT.ntid == output_plist) {
        t_node* id_ast = root->children;
        t_node* type_ast = id_ast->sibling->sibling;
        createAST(type_ast);
        createAST(type_ast->sibling);
        createASTNode(id_ast);
    }

    // N2 -> COMMA ID COLON type N2
    else if (root->TorNT == 1 && root->data.NT.ntid == N2 && root->children->TorNT == 0 && root->children->data.T.tid == COMMA) {
        t_node* id_ast = root->children->sibling;
        t_node* type_ast = id_ast->sibling->sibling;
        createAST(type_ast);
        createAST(type_ast->sibling);
        createASTNode(id_ast);
    }

    // N2 -> EPSILON
    else if (root->TorNT == 1 && root->data.NT.ntid == N2 && root->children->TorNT == 0 && root->children->data.T.tid == EPSILON) {
        return; // ???
    }

    // dataType -> INTEGER
    else if (root->TorNT == 1 && root->data.NT.ntid == dataType && root->children->TorNT == 0 && root->children->data.T.tid == INTEGER) {
        createASTNode(root->children);
    }

    // dataType -> REAL
    else if (root->TorNT == 1 && root->data.NT.ntid == dataType && root->children->TorNT == 0 && root->children->data.T.tid == REAL) {
        createASTNode(root->children);
    }

    // dataType -> BOOLEAN
    else if (root->TorNT == 1 && root->data.NT.ntid == dataType && root->children->TorNT == 0 && root->children->data.T.tid == BOOLEAN) {
        createASTNode(root->children);
    }
    
    // dataType -> ARRAY SQBO range_arrays SQBC OF type
    else if (root->TorNT == 1 && root->data.NT.ntid == dataType && root->children->TorNT == 0 && root->children->data.T.tid == ARRAY) {
        t_node* ra_ast = root->children->sibling->sibling;
        t_node* type_ast = ra_ast->sibling->sibling;
        createAST(ra_ast);
        createAST(type_ast);
    }

    // range_arrays -> index RANGEOP index
    else if (root->TorNT == 1 && root->data.NT.ntid == range_arrays) {
        t_node* index_1 = root->children;
        t_node* index_2 = index_1->sibling->sibling;
        createAST(index_1);
        createAST(index_2);   
    }

    // type -> INTEGER 
    else if (root->TorNT == 1 && root->data.NT.ntid == type && root->children->TorNT == 0 && root->children->data.T.tid == INTEGER) {
        createASTNode(root->children);
    }

    // type -> REAL
    else if (root->TorNT == 1 && root->data.NT.ntid == type && root->children->TorNT == 0 && root->children->data.T.tid == REAL) {
        createASTNode(root->children);
    }

    // type -> BOOLEAN
    else if (root->TorNT == 1 && root->data.NT.ntid == type && root->children->TorNT == 0 && root->children->data.T.tid == BOOLEAN) {
        createASTNode(root->children);
    }

    // moduleDef -> START statements END 
    else if (root->TorNT == 1 && root->data.NT.ntid == moduleDef) {
        createAST(root->children->sibling);
    }

    // statements -> statement statements
    else if (root->TorNT == 1 && root->data.NT.ntid == statements && root->children->TorNT == 1 && root->children->data.NT.ntid == statement) {
        t_node* s_ast = root->children;
        createAST(s_ast);
        createAST(s_ast->sibling);
    }
    
    // statements -> EPSILON 
    else if (root->TorNT == 1 && root->data.NT.ntid == statements && root->children->TorNT == 0 && root->children->data.T.tid == EPSILON) {
        return; // ???
    }
    
    // statement -> ioStmt
    else if (root->TorNT == 1 && root->data.NT.ntid == statement && root->children->TorNT == 1 && root->children->data.NT.ntid == ioStmt) {
        t_node* ios_ast = root->children;
        createAST(ios_ast);
    }
    
    // statement -> simpleStmt
    else if (root->TorNT == 1 && root->data.NT.ntid == statement && root->children->TorNT == 1 && root->children->data.NT.ntid == simpleStmt) {
        t_node* sim_ast = root->children;
        createAST(sim_ast);
    }    
    
    // statement -> declareStmt
    else if (root->TorNT == 1 && root->data.NT.ntid == statement && root->children->TorNT == 1 && root->children->data.NT.ntid == declareStmt) {
        t_node* decs_ast = root->children;
        createAST(decs_ast);
    }    
    
    // statement -> conditionalStmt
    else if (root->TorNT == 1 && root->data.NT.ntid == statement && root->children->TorNT == 1 && root->children->data.NT.ntid == conditionalStmt) {
        t_node* conds_ast = root->children;
        createAST(conds_ast);
    }    
    
    // statement -> iterativeStmt
    else if (root->TorNT == 1 && root->data.NT.ntid == statement && root->children->TorNT == 1 && root->children->data.NT.ntid == iterativeStmt) {
        t_node* iters_ast = root->children;
        createAST(iters_ast);
    }

    // ioStmt -> GET_VALUE BO ID BC SEMICOL
    else if (root->TorNT == 1 && root->data.NT.ntid == ioStmt && root->children->TorNT == 0 && root->children->data.T.tid == GET_VALUE) {
        createASTNode(root->children->sibling->sibling);
    }
    
    // ioStmt -> PRINT BO var BC SEMICOL 
    else if (root->TorNT == 1 && root->data.NT.ntid == ioStmt && root->children->TorNT == 0 && root->children->data.T.tid == GET_VALUE) {
        createASTNode(root->children->sibling->sibling);
    }

    // boolConstt -> TRUE
    else if (root->TorNT == 1 && root->data.NT.ntid == boolConstt && root->children->TorNT == 0 && root->children->data.T.tid == TRUE) {
        createASTNode(root->children);
    }

    // boolConstt -> FALSE
    else if (root->TorNT == 1 && root->data.NT.ntid == boolConstt && root->children->TorNT == 0 && root->children->data.T.tid == FALSE) {
        createASTNode(root->children);
    }

    // var_id_num -> ID whichId
    else if (root->TorNT == 1 && root->data.NT.ntid == var_id_num && root->children->TorNT == 0 && root->children->data.T.tid == ID) {
        t_node* id_ast = root->children;
        createAST(id_ast->sibling);
        createASTNode(id_ast);
    }
    
    // var_id_num -> NUM
    else if (root->TorNT == 1 && root->data.NT.ntid == var_id_num && root->children->TorNT == 0 && root->children->data.T.tid == NUM) {
        createASTNode(root->children);
    }

    // var_id_num -> RNUM
    else if (root->TorNT == 1 && root->data.NT.ntid == var_id_num && root->children->TorNT == 0 && root->children->data.T.tid == RNUM) {
        createASTNode(root->children);
    }

    // var -> var_id_num
    else if (root->TorNT == 1 && root->data.NT.ntid == var && root->children->TorNT == 1 && root->children->data.NT.ntid == var_id_num) {
        createAST(root->children);
    }

    // var -> boolConstt
    else if (root->TorNT == 1 && root->data.NT.ntid == var && root->children->TorNT == 1 && root->children->data.NT.ntid == boolConstt) {
        createAST(root->children);
    }

    // whichId -> SQBO index SQBC
    else if (root->TorNT == 1 && root->data.NT.ntid == var_id_num && root->children->TorNT == 0 && root->children->data.T.tid == SQBO) {
        createAST(root->children->sibling);
    }

    // whichId -> EPSILON 
    else if (root->TorNT == 1 && root->data.NT.ntid == whichId && root->children->TorNT == 0 && root->children->data.T.tid == EPSILON) {
        return; // ???
    }

    // simpleStmt -> assignmentStmt
    else if (root->TorNT == 1 && root->data.NT.ntid == simpleStmt && root->children->TorNT == 1 && root->children->data.NT.ntid == assignmentStmt) {
        createAST(root->children);
    }

    // simpleStmt -> moduleReuseStmt
    else if (root->TorNT == 1 && root->data.NT.ntid == simpleStmt && root->children->TorNT == 1 && root->children->data.NT.ntid == moduleReuseStmt) {
        createAST(root->children);
    }

    // assignmentStmt -> ID whichStmt 
    else if (root->TorNT == 1 && root->data.NT.ntid == assignmentStmt) {
        t_node* id_ast = root->children;
        createAST(id_ast->sibling);
        createASTNode(id_ast);
    }
    
    // whichStmt -> lvalueIDStmt
    else if (root->TorNT == 1 && root->data.NT.ntid == whichStmt && root->children->TorNT == 1 && root->children->data.NT.ntid == lvalueIDStmt) {
        createAST(root->children);
    }

    // whichStmt -> lvalueARRStmt
    else if (root->TorNT == 1 && root->data.NT.ntid == whichStmt && root->children->TorNT == 1 && root->children->data.NT.ntid == lvalueARRStmt) {
        createAST(root->children);
    }

    // lvalueIDStmt -> ASSIGNOP expression SEMICOL
    else if (root->TorNT == 1 && root->data.NT.ntid == lvalueIDStmt) {
        t_node* assign_ast = root->children;
        createAST(assign_ast->sibling);
        createASTNode(assign_ast);
    }


    // lvalueARRStmt -> SQBO index SQBC ASSIGNOP expression SEMICOL
    else if (root->TorNT == 1 && root->data.NT.ntid == lvalueARRStmt) {
        t_node* index_ast = root->children->sibling;
        t_node* assign_ast = index_ast->sibling->sibling;
        createAST(index_ast);
        createAST(assign_ast->sibling);
        createASTNode(assign_ast);
    }

    // index -> NUM 
    else if (root->TorNT == 1 && root->data.NT.ntid == _index && root->children->TorNT == 0 && root->children->data.T.tid == NUM) {
        createASTNode(root->children);
    }
    
    // index -> ID 
    else if (root->TorNT == 1 && root->data.NT.ntid == _index && root->children->TorNT == 0 && root->children->data.T.tid == ID) {
        createASTNode(root->children);
    }

    // moduleReuseStmt -> optional USE MODULE ID WITH PARAMETERS idList SEMICOL
    else if (root->TorNT == 1 && root->data.NT.ntid == moduleReuseStmt) {
        t_node* opt_ast = root->children;
        t_node* id_ast = opt_ast->sibling->sibling->sibling;
        t_node* idlist_ast = id_ast->sibling->sibling->sibling;
        createAST(opt_ast);
        createAST(idlist_ast);
        createASTNode(id_ast);
    }

    // optional -> SQBO idList SQBC ASSIGNOP 
    else if (root->TorNT == 1 && root->data.NT.ntid == optional && root->children->TorNT == 0 && root->children->data.NT.ntid == SQBO) {
        t_node* idlist_ast = root->children->sibling;
        t_node* assign_ast = idlist_ast->sibling->sibling;
        createAST(idlist_ast);
        createASTNode(assign_ast);
    }
    
    // optional -> EPSILON
    else if (root->TorNT == 1 && root->data.NT.ntid == optional && root->children->TorNT == 0 && root->children->data.T.tid == EPSILON) {
        return; // ???
    }

    // idList -> ID N3
    else if (root->TorNT == 1 && root->data.NT.ntid == idList) {
        t_node* id_ast = root->children;
        createAST(id_ast->sibling);
        createASTNode(id_ast);
    }

    // N3 -> COMMA ID N3
    else if (root->TorNT == 1 && root->data.NT.ntid == N3 && root->children->TorNT == 0 && root->children->data.NT.ntid == COMMA) {
        t_node* id_ast = root->children->sibling;
        createAST(id_ast->sibling);
        createASTNode(id_ast);
    }
    
    // N3 -> EPSILON
    else if (root->TorNT == 1 && root->data.NT.ntid == N3 && root->children->TorNT == 0 && root->children->data.T.tid == EPSILON) {
        return; // ???
    }

    // expression -> arithmeticOrBooleanExpr
    else if (root->TorNT == 1 && root->data.NT.ntid == expression && root->children->TorNT == 1 && root->children->data.NT.ntid == arithmeticOrBooleanExpr) {
        createAST(root->children);
    }

    // expression -> U
    else if (root->TorNT == 1 && root->data.NT.ntid == expression && root->children->TorNT == 1 && root->children->data.NT.ntid == U) {
        createAST(root->children);
    }

    // U -> unary_op new_NT
    else if (root->TorNT == 1 && root->data.NT.ntid == U) {
        t_node* unaryop_ast = root->children;
        createAST(unaryop_ast);
        createAST(unaryop_ast->sibling);
    }
    
    // new_NT -> BO arithmeticExpr BC
    else if (root->TorNT == 1 && root->data.NT.ntid == new_NT && root->children->TorNT == 0 && root->children->data.T.tid == BO) {
        createAST(root->children->sibling);
    }

    // new_NT -> var_id_num
    else if (root->TorNT == 1 && root->data.NT.ntid == new_NT && root->children->TorNT == 1 && root->children->data.NT.ntid == var_id_num) {
        createAST(root->children);
    }

    // unary_op -> PLUS
    else if (root->TorNT == 1 && root->data.NT.ntid == unary_op && root->children->TorNT == 0 && root->children->data.NT.ntid == PLUS) {
        createASTNode(root->children);
    }

    // unary_op -> MINUS
    else if (root->TorNT == 1 && root->data.NT.ntid == unary_op && root->children->TorNT == 0 && root->children->data.NT.ntid == MINUS) {
        createASTNode(root->children);
    }

    // arithmeticOrBooleanExpr -> anyTerm N7
    else if (root->TorNT == 1 && root->data.NT.ntid == arithmeticOrBooleanExpr) {
        t_node* at_ast = root->children;
        createAST(at_ast);
        createAST(at_ast->sibling);
    }

    // N7 -> logicalOp anyTerm N7
    else if (root->TorNT == 1 && root->data.NT.ntid == N7 && root->children->TorNT == 1 && root->children->data.NT.ntid == logicalOp) {
        t_node* lop_ast = root->children;
        t_node* at_ast = lop_ast->sibling;
        createAST(lop_ast);
        createAST(at_ast);
        createAST(at_ast->sibling);

    }

    // N7 -> EPSILON
    else if (root->TorNT == 1 && root->data.NT.ntid == N7 && root->children->TorNT == 0 && root->children->data.T.tid == EPSILON) {
        return; // ???
    }
    
    // anyTerm -> arithmeticExpr N8
    else if (root->TorNT == 1 && root->data.NT.ntid == anyTerm && root->children->TorNT == 1 && root->children->data.NT.ntid == arithmeticExpr) {
        createAST(root->children);
        createAST(root->children->sibling);
    }
 
    // anyTerm -> boolConstt
    else if (root->TorNT == 1 && root->data.NT.ntid == anyTerm && root->children->TorNT == 1 && root->children->data.NT.ntid == boolConstt) {
        createAST(root->children);
    }

    // N8 -> relationalOp arithmeticExpr
    else if (root->TorNT == 1 && root->data.NT.ntid == N8 && root->children->TorNT == 1 && root->children->data.NT.ntid == relationalOp) {
        t_node* rop_ast = root->children;
        createAST(rop_ast);
        createAST(rop_ast->sibling);
    }
    
    // N8 -> EPSILON
    else if (root->TorNT == 1 && root->data.NT.ntid == N8 && root->children->TorNT == 0 && root->children->data.T.tid == EPSILON) {
        return; // ???
    }


    // arithmeticExpr -> term N4
    else if (root->TorNT == 1 && root->data.NT.ntid == arithmeticExpr) {
        t_node* term_ast = root->children;
        createAST(term_ast);
        createAST(term_ast->sibling);
    }

    // N4 -> op1 term N4
    else if (root->TorNT == 1 && root->data.NT.ntid == N4 && root->children->TorNT == 1 && root->children->data.NT.ntid == op1) {
        t_node* op1_ast = root->children;
        t_node* term_ast = op1_ast->sibling;
        createAST(op1_ast);
        createAST(term_ast);
        createAST(term_ast->sibling);
    }

    // N4 -> EPSILON
    else if (root->TorNT == 1 && root->data.NT.ntid == N4 && root->children->TorNT == 0 && root->children->data.T.tid == EPSILON) {
        return; // ???
    }


    // term -> factor N5
    else if (root->TorNT == 1 && root->data.NT.ntid == term) {
        t_node* factor_ast = root->children;
        createAST(factor_ast);
        createAST(factor_ast->sibling);
    }


    // N5 -> op2 factor N5
    else if (root->TorNT == 1 && root->data.NT.ntid == N5 && root->children->TorNT == 1 && root->children->data.NT.ntid == op2) {
        t_node* op2_ast = root->children;
        t_node* factor_ast = root->children->sibling;
        createAST(op2_ast);
        createAST(factor_ast);
        createASTNode(factor_ast->sibling);
    }    
    
    // N5 -> EPSILON
    else if (root->TorNT == 1 && root->data.NT.ntid == N5 && root->children->TorNT == 0 && root->children->data.T.tid == EPSILON) {
        return; // ???
    }

    // factor -> BO arithmeticOrBooleanExpr BC
    else if (root->TorNT == 1 && root->data.NT.ntid == factor && root->children->TorNT == 0 && root->children->data.T.tid == BO) {
        createAST(root->children->sibling);
    }
    
    // factor -> var_id_num
    else if (root->TorNT == 1 && root->data.NT.ntid == factor && root->children->TorNT == 1 && root->children->data.NT.ntid == var_id_num) {
        createAST(root->children);
    }
    
    // op1 -> PLUS
    else if (root->TorNT == 1 && root->data.NT.ntid == op1 && root->children->TorNT == 0 && root->children->data.T.tid == PLUS) {
        createASTNode(root->children);
    }

    // op1 -> MINUS
    else if (root->TorNT == 1 && root->data.NT.ntid == op1 && root->children->TorNT == 0 && root->children->data.T.tid == MINUS) {
        createASTNode(root->children);
    }

    // op2 -> MUL
    else if (root->TorNT == 1 && root->data.NT.ntid == op1 && root->children->TorNT == 0 && root->children->data.T.tid == MUL) {
        createASTNode(root->children);
    }

    // op2 -> DIV
    else if (root->TorNT == 1 && root->data.NT.ntid == op1 && root->children->TorNT == 0 && root->children->data.T.tid == DIV) {
        createASTNode(root->children);
    }

    // logicalOp -> AND
    else if (root->TorNT == 1 && root->data.NT.ntid == logicalOp && root->children->TorNT == 0 && root->children->data.T.tid == AND) {
        createASTNode(root->children);
    }

    // logicalOp -> OR
    else if (root->TorNT == 1 && root->data.NT.ntid == logicalOp && root->children->TorNT == 0 && root->children->data.T.tid == OR) {
        createASTNode(root->children);
    }

    // relationalOp -> LT
    else if (root->TorNT == 1 && root->data.NT.ntid == relationalOp && root->children->TorNT == 0 && root->children->data.T.tid == LT) {
        createASTNode(root->children);
    }

    // relationalOp -> LE 
    else if (root->TorNT == 1 && root->data.NT.ntid == relationalOp && root->children->TorNT == 0 && root->children->data.T.tid == LE) {
        createASTNode(root->children);
    }

    // relationalOp -> GT 
    else if (root->TorNT == 1 && root->data.NT.ntid == relationalOp && root->children->TorNT == 0 && root->children->data.T.tid == GT) {
        createASTNode(root->children);
    }

    // relationalOp -> GE 
    else if (root->TorNT == 1 && root->data.NT.ntid == relationalOp && root->children->TorNT == 0 && root->children->data.T.tid == GE) {
        createASTNode(root->children);
    }

    // relationalOp -> EQ 
    else if (root->TorNT == 1 && root->data.NT.ntid == relationalOp && root->children->TorNT == 0 && root->children->data.T.tid == EQ) {
        createASTNode(root->children);
    }

    // relationalOp -> NE 
    else if (root->TorNT == 1 && root->data.NT.ntid == relationalOp && root->children->TorNT == 0 && root->children->data.T.tid == NE) {
        createASTNode(root->children);
    }

    // declareStmt -> DECLARE idList COLON dataType SEMICOL
    else if (root->TorNT == 1 && root->data.NT.ntid == declareStmt) {
        t_node* idlist_ast = root->children->sibling;
        t_node* datatype_ast = idlist_ast->sibling->sibling;
        createAST(idlist_ast);
        createAST(datatype_ast);
    }

    // conditionalStmt -> SWITCH BO ID BC START caseStmts default END  
    else if (root->TorNT == 1 && root->data.NT.ntid == conditionalStmt) {
        t_node* id_ast = root->children->sibling->sibling;
        t_node* cs_ast = id_ast->sibling->sibling->sibling;
        t_node* def_ast = cs_ast->sibling;
        createAST(cs_ast); 
        createAST(def_ast); 
        createASTNode(id_ast);
    }

    // caseStmts -> CASE value COLON statements BREAK SEMICOL N9
    else if (root->TorNT == 1 && root->data.NT.ntid == caseStmts) {
        t_node* val_ast = root->children->sibling;
        t_node* s_ast = val_ast->sibling->sibling;
        t_node* n9_ast = s_ast->sibling->sibling->sibling;
        createAST(val_ast);
        createAST(s_ast);
        createAST(n9_ast);
    }

    // N9 -> CASE value COLON statements BREAK SEMICOL N9 
    else if (root->TorNT == 1 && root->data.NT.ntid == N9 && root->children->TorNT == 0 && root->children->data.T.tid == CASE) {
        t_node* val_ast = root->children->sibling;
        t_node* s_ast = val_ast->sibling->sibling;
        t_node* n9_ast = s_ast->sibling->sibling->sibling;
        createAST(val_ast);
        createAST(s_ast);
        createAST(n9_ast);
    }
    
    
    // N9 -> EPSILON
    else if (root->TorNT == 1 && root->data.NT.ntid == N9 && root->children->TorNT == 0 && root->children->data.T.tid == EPSILON) {
        return; // ???
    }

    // value -> NUM 
    else if (root->TorNT == 1 && root->data.NT.ntid == value && root->children->TorNT == 0 && root->children->data.T.tid == NUM) {
        createASTNode(root->children);
    }
    
    // value -> TRUE 
    else if (root->TorNT == 1 && root->data.NT.ntid == value && root->children->TorNT == 0 && root->children->data.T.tid == TRUE) {
        createASTNode(root->children);
    }

    // value -> FALSE 
    else if (root->TorNT == 1 && root->data.NT.ntid == value && root->children->TorNT == 0 && root->children->data.T.tid == FALSE) {
        createASTNode(root->children);
    }

    // default -> DEFAULT COLON statements BREAK SEMICOL
    else if (root->TorNT == 1 && root->data.NT.ntid == _default && root->children->TorNT == 0 && root->children->data.T.tid == DEFAULT) {
        createAST(root->children->sibling->sibling);
    }


    // default -> EPSILON 
    else if (root->TorNT == 1 && root->data.NT.ntid == _default && root->children->TorNT == 0 && root->children->data.T.tid == EPSILON) {
        return; // ???
    }

    // iterativeStmt -> FOR BO ID IN range BC START statements END
    else if (root->TorNT == 1 && root->data.NT.ntid == iterativeStmt && root->children->TorNT == 0 && root->children->data.T.tid == FOR) {
        t_node* id_ast = root->children->sibling->sibling;
        t_node* range_ast = id_ast->sibling->sibling;
        t_node* s_ast = range_ast->sibling->sibling->sibling;
        createAST(range_ast);
        createAST(s_ast);
        createASTNode(id_ast);
    }
    
    // iterativeStmt -> WHILE BO arithmeticOrBooleanExpr BC START statements END
    else if (root->TorNT == 1 && root->data.NT.ntid == iterativeStmt && root->children->TorNT == 0 && root->children->data.T.tid == WHILE) {
        t_node* abexpr_ast = root->children->sibling->sibling;
        t_node* s_ast = abexpr_ast->sibling->sibling->sibling;
        createAST(abexpr_ast);
        createAST(s_ast);
    }
    
    // range -> NUM RANGEOP NUM
    else if (root->TorNT == 1 && root->data.NT.ntid == range) {
        createASTNode(root->children);
        createASTNode(root->children->sibling->sibling);
    }
}
