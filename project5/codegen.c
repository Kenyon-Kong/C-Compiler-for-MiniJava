#include "codegen.h"

#include <string.h>
#include <stdio.h>

//global
struct ARMNode *insRoot; 
struct ARMNode *insTail;
int stringCounter = 0;
char *stringList[50];
char *staticStringList[50];
char *staticStringName[50];
int staticStringNum = 0;
int staticIntList[50];
char *staticIntName[50];
int staticIntNum = 0;

int argCounter = 0; // count the number of args for current method
int ifCounter = 0; // count the number for if-else statements
int whileCounter = 0; // count the number of while statements

struct OffsetTable* offsetTableList[10];
int num_offsetTable = 0;
// traverse AST for the thrid time to generate assembly 
void thirdTraversal(struct ASTNode* node, bool isMethod) {
    if (node == NULL) return;
    for (int i = 0; i < node->num_children; i++) {
        if (isMethod == false) {
            if (node->node_type == NODETYPE_STATICMETHOD) {
                 return;
            }
            thirdTraversal(node->children[i], isMethod);
        } else {
            if (node->node_type == NODETYPE_STATICVARDECLLIST || node->node_type == NODETYPE_MAINMETHOD) {
                return;
            }
            // if (node->parentNode != NULL) {
            //     if (node->parentNode->node_type == NODETYPE_STATICMETHOD) {
            //         modifyTable(node->table);
            //     } 
            // }
            thirdTraversal(node->children[i], isMethod);
        }
    }


    if (node->node_type == NODETYPE_MAINMETHOD) {
        handleMainMethod(node);
    } else if (node->node_type == NODETYPE_STATICMETHOD) {
        handleStaticMethod(node);
    } else if (node->node_type == NODETYPE_RETURN) {
        handleReturn(node);
    } else if (node->node_type == NODETYPE_METHODCALL) {
        handleMethodCall(node);
    } else if (node->node_type == NODETYPE_OPERATION) {
        handleOperation(node);
    } else if (node->node_type == NODETYPE_EXPTYPE) {
        handleExpType(node);
    } else if (node->node_type == NODETYPE_ONLYBOOL) { // EXP -> ! Exp 
        handleBoolNot(node);
    } else if (node->node_type == NODETYPE_LEFTVAL) { 
        handleLeftVal(node);
    } else if (node->node_type == NODETYPE_VARDECL) {
        handleVarDecl(node);  
    } else if (node->node_type == NODETYPE_MOREVAR) {
        handleMoreVar(node);
    } else if (node->node_type == NODETYPE_ASSIGN) {
        handleAssign(node);
    } else if (node->node_type == NODETYPE_PRINT || node->node_type == NODETYPE_PRINTLN) {
        handlePrint(node);
    } else if (node->node_type == NODETYPE_IF) {
        handleIf(node);
    } else if (node->node_type == NODETYPE_WHILE) {
        handleWhile(node);
    } else if (node->node_type == NODETYPE_PARSEINT) {
        handleParseInt(node);
    }
}


// main functions

void handleMainMethod(struct ASTNode* node) {
    struct ARMNode* asmNode = malloc(sizeof(struct ARMNode)); // for main head not including how to end
    char *temp = malloc(200);
    sprintf(temp, "main:\n  push {lr}\n  sub SP, SP, #%d\n  str r0, [SP, #0]\n  add r1, r1, #4\n  str r1, [SP, #4]", node->offsetTable->regCount *4);
    asmNode->code = temp;
    asmNode->type = INSTYPE_MAIN;
    asmNode->regIndex = 9999; // indicating no Index        
    node->asmNode = asmNode;
    asmNode->astNode = node;
    addToInsRoot(asmNode);

    struct ARMNode* endNode = malloc(sizeof(struct ARMNode));
    char *temp1 = malloc(200);
    sprintf(temp1, "add SP, SP, #%d\n  pop {pc}", node->offsetTable->regCount *4);
    endNode->code = temp1;
    endNode->type = INSTYPE_METHODTAIL; 
    endNode->regIndex = 9999; // indicating no Index            
    // node->asmNode = asmNode; already declared
    // asmNode->astNode = node;
    endNode->astNode = node;
    addToLinkedList(endNode);
    
    node->offsetTable->num_graphNode = 0;
}

void handleStaticMethod(struct ASTNode* node) {
    argCounter = 0;
    char *ID = node->data.value.string_value;
    struct ARMNode* asmNode = malloc(sizeof(struct ARMNode)); // for method head not including how to end
    char *temp = malloc(200);
    sprintf(temp, "%s:\n  push {lr}\n  sub SP, SP, #%d", ID, node->offsetTable->regCount *4);
    asmNode->code = temp;
    asmNode->type = INSTYPE_METHOD;
    asmNode->regIndex = 9999; // indicating no Index        
    node->asmNode = asmNode;
    asmNode->astNode = node;
    int numArgs = findNumArgs(ID);
    node->offsetTable->numArgs = numArgs;
    insertMethod(asmNode);

    struct ARMNode* endNode = malloc(sizeof(struct ARMNode));
    char *temp1 = malloc(50);
    endNode->code = temp1;
    endNode->type = INSTYPE_METHODTAIL; 
    endNode->regIndex = 9999; // indicating no Index            
    // node->asmNode = asmNode; already declared
    endNode->astNode = node;
    addToLinkedList(endNode);

    node->offsetTable->num_graphNode = 0;
}

void handleReturn(struct ASTNode* node) {
    int returnReg;
    if (node->children[0]->node_type == NODETYPE_EXPTYPE) {
        if (node->children[0]->data.type != DATATYPE_STR) {
            returnReg = node->children[0]->asmNode->regIndex;
        }
    } else if (node->children[0]->node_type == NODETYPE_EXP) {
        if (node->children[0]->children[0]->node_type == NODETYPE_LEFTVAL) {
            returnReg = node->children[0]->children[0]->asmNode->regIndex;

        } 
    } else if (node->children[0]->node_type == NODETYPE_EXPMETHOD) {
        returnReg = node->children[0]->children[0]->asmNode->regIndex;
    } else if (node->children[0]->node_type == NODETYPE_OPERATION) {
       returnReg = node->children[0]->asmNode->regIndex;
    }
    char *op1 = malloc(10);
    sprintf(op1, "r0");
    char *op2 = malloc(10);
    sprintf(op2, "$t%d", returnReg);
    struct ARMNode* returnNode = malloc(sizeof(struct ARMNode));
    returnNode->op1 = op1;
    returnNode->op2 = op2;
    returnNode->DEF = op1;
    returnNode->USE[0] = op2;
    returnNode->num_def = 1;
    returnNode->num_use = 1;
    char *returnCode = malloc(30);
    sprintf(returnCode, "mov %s, %s", op1, op2);
    returnNode->code = returnCode;
    returnNode->astNode = node;
    node->asmNode = returnNode;
    returnNode->type = INSTYPE_MOVR0BACK;
    returnNode->isReturn = 9999; // hard code
    addToLinkedList(returnNode);

}

void handleIf(struct ASTNode* node) { // KW_IF '(' Exp ')' Statement KW_ELSE Statement
    int conditionIndex = 0;
    if (node->children[0]->node_type == NODETYPE_OPERATION  || node->children[0]->node_type == NODETYPE_EXPTYPE
        || node->children[0]->node_type == NODETYPE_ONLYBOOL) {
        conditionIndex = node->children[0]->asmNode->regIndex;
    } else if (node->children[0]->node_type == NODETYPE_EXPMETHOD  || node->children[0]->node_type == NODETYPE_EXP) {
        conditionIndex = node->children[0]->children[0]->asmNode->regIndex;
    }

    char *op1 = malloc(10);
    sprintf(op1, "$t%d", conditionIndex);
    char *op2 = malloc(10);
    sprintf(op2, "#0");
    struct ARMNode* cmpNode = malloc(sizeof(struct ARMNode));
    char *cmp = malloc(10);
    sprintf(cmp, "cmp %s, %s", op1, op2);
    cmpNode->op1 = op1;
    cmpNode->op2 = op2;
    cmpNode->num_use = 1;
    cmpNode->USE[0] = op1;
    cmpNode->num_def = 0;
    cmpNode->code = cmp;
    cmpNode->type = INSTYPE_CMPINT;
    cmpNode->astNode = node;

    char *branchlabel = malloc(20);
    sprintf(branchlabel, "_false_%d", ifCounter);
    struct ARMNode* branchNode = malloc(sizeof(struct ARMNode));
    char *branch = malloc(20);
    sprintf(branch, "beq %s", branchlabel);
    branchNode->num_use = 0;
    branchNode->num_def = 0;
    branchNode->code = branch;
    branchNode->type = INSTYPE_BRANCH;
    branchNode->label = branchlabel;
    branchNode->astNode = node;


    char *newlabelName = malloc(10);
    sprintf(newlabelName, "_true_%d", ifCounter);
    char *newlabel = malloc(20);
    sprintf(newlabel, "%s:", newlabelName);
    struct ARMNode* labelNode = malloc(sizeof(struct ARMNode));
    labelNode->num_use = 0;
    labelNode->num_def = 0;
    labelNode->code = newlabel;
    labelNode->type = INSTYPE_LABEL;
    labelNode->label = newlabelName;
    labelNode->astNode = node;

    // to do: insert labels and update the counter
    
    //to insert if head -> idea: insert right after the condition ends (between cond and then)
    struct ARMNode* conditionEnds;
     if (node->children[0]->node_type == NODETYPE_OPERATION || node->children[0]->node_type == NODETYPE_EXPTYPE
            || node->children[0]->node_type == NODETYPE_ONLYBOOL) {
        conditionEnds = node->children[0]->asmNode;
    } else if (node->children[0]->node_type == NODETYPE_EXPMETHOD || node->children[0]->node_type == NODETYPE_EXP ) {
        conditionEnds = node->children[0]->children[0]->asmNode;
    }
    insertAfterNode(conditionEnds, cmpNode);
    insertAfterNode(cmpNode, branchNode);
    insertAfterNode(branchNode, labelNode);

    //to insert labels between then and else -> idea: insert right after the then body ends
    struct ASTNode* thenStmt = node->children[1];
    if (thenStmt->num_children == 0) { // nothing in then body
        char *blabel = malloc(20);
        sprintf(blabel, "_endif_%d", ifCounter);
        struct ARMNode* bNode = malloc(sizeof(struct ARMNode));
        char *b = malloc(30);
        sprintf(b, "b %s", blabel);
        bNode->num_use = 0;
        bNode->num_def = 0;
        bNode->type = INSTYPE_BRANCH;
        bNode->code = b;
        bNode->label = blabel;
        bNode->astNode = node;
        insertAfterNode(labelNode, bNode);

        struct ARMNode* labelNode2 = malloc(sizeof(struct ARMNode));
        char *labelName2 = malloc(50);
        sprintf(labelName2, "_false_%d", ifCounter);
        char *label2 = malloc(50);
        sprintf(label2, "%s:", labelName2);
        labelNode2->num_use = 0;
        labelNode2->num_def = 0;
        labelNode2->code = label2;
        labelNode2->type = INSTYPE_LABEL;
        labelNode2->label = labelName2;
        labelNode2->astNode = node;
        insertAfterNode(bNode, labelNode2);
    } else {    
        struct ARMNode* thenEnds = thenStmt->asmNode;
        while(thenEnds == NULL) {
            if(thenStmt->node_type == NODETYPE_STMTLIST && thenStmt->num_children > 1) {
                thenStmt = thenStmt->children[1];
            } else {
                thenStmt = thenStmt->children[0];
            }
            
            thenEnds = thenStmt->asmNode;
        }
        char *blabel = malloc(20);
        sprintf(blabel, "_endif_%d", ifCounter);
        struct ARMNode* bNode = malloc(sizeof(struct ARMNode));
        char *b = malloc(20);
        sprintf(b, "b %s", blabel);
        bNode->num_use = 0;
        bNode->num_def = 0;
        bNode->code = b;
        bNode->type = INSTYPE_BRANCH;
        bNode->label = blabel;
        bNode->astNode = node;
        insertAfterNode(thenEnds, bNode);
        
        struct ARMNode* labelNode2 = malloc(sizeof(struct ARMNode));
        char *labelName2 = malloc(50);
        sprintf(labelName2, "_false_%d", ifCounter);
        char *label2 = malloc(50);
        sprintf(label2, "%s:", labelName2);
        labelNode2->num_use = 0;
        labelNode2->num_def = 0;
        labelNode2->code = label2;
        labelNode2->type = INSTYPE_LABEL;
        labelNode2->label = labelName2;
        labelNode2->astNode = node;
        insertAfterNode(bNode, labelNode2);
    }

    //to insert labels after else -> idea: just add to the tail of the linked list
    struct ARMNode* asmNode3 = malloc(sizeof(struct ARMNode));
    char *labelName3 = malloc(20);
    sprintf(labelName3, "_endif_%d", ifCounter);
    char *label3 = malloc(20);
    sprintf(label3, "%s:", labelName3);
    asmNode3->num_use = 0;
    asmNode3->num_def = 0;
    asmNode3->code = label3;
    asmNode3->type = INSTYPE_LABEL;
    asmNode3->label = labelName3;
    asmNode3->astNode = node;
    addToLinkedList(asmNode3);

    //update the ifCounter
    ifCounter++;
}

void handleWhile(struct ASTNode* node) { // KW_WHILE '(' Exp ')' Statement
    // to insert loop title in front of condition -> idea: find the left most asmNode of EXP condition subtree
    struct ARMNode* conditionHead;
    if (node->children[0]->node_type == NODETYPE_EXPMETHOD) {
        conditionHead = node->children[0]->children[0]->asmNode;
    } else {
        struct ASTNode* conditionExp = node->children[0];
        while (conditionExp != NULL) {
            conditionHead = conditionExp->asmNode;
            conditionExp = conditionExp->children[0];
        }
    }
    char *labelName = malloc(20);
    sprintf(labelName, "_loop_%d", whileCounter);
    char *label = malloc(30);
    sprintf(label, "%s:", labelName);
    struct ARMNode* labelNode = malloc(sizeof(struct ARMNode));
    labelNode->code = label;
    labelNode->type = INSTYPE_LABEL;
    labelNode->label = labelName;
    labelNode->num_use = 0;
    labelNode->num_def = 0;
    labelNode->astNode = node;
    struct ARMNode* prevOfConditionHead = conditionHead->prevNode;
    insertAfterNode(prevOfConditionHead, labelNode);

    // to insert compare code function for condition -> idea: insert after the root of conditon
    int conditionIndex = 0;
    if (node->children[0]->node_type == NODETYPE_OPERATION) {
        conditionIndex = node->children[0]->asmNode->regIndex;
    } else if (node->children[0]->node_type == NODETYPE_EXPMETHOD) {
        conditionIndex = node->children[0]->children[0]->asmNode->regIndex;
    }

    struct ARMNode* conditionEnds;
     if (node->children[0]->node_type == NODETYPE_OPERATION) {
        conditionEnds = node->children[0]->asmNode;
    } else if (node->children[0]->node_type == NODETYPE_EXPMETHOD) {
        conditionEnds = node->children[0]->children[0]->asmNode;
    }

    char *op1 = malloc(10);
    sprintf(op1, "$t%d", conditionIndex);
    char *op2 = malloc(10);
    sprintf(op2, "#0");
    struct ARMNode* cmpNode = malloc(sizeof(struct ARMNode));
    char *cmp = malloc(20);
    sprintf(cmp, "cmp %s, %s", op1, op2);
    cmpNode->op1 = op1;
    cmpNode->op2 = op2;
    cmpNode->num_use = 1;
    cmpNode->USE[0] = op1;
    cmpNode->num_def = 0;
    cmpNode->code = cmp;
    cmpNode->type = INSTYPE_CMPINT;
    cmpNode->astNode = node;
    insertAfterNode(conditionEnds, cmpNode);

    char *label2 = malloc(20);
    sprintf(label2, "_endloop_%d", whileCounter);
    struct ARMNode* branchNode = malloc(sizeof(struct ARMNode));
    char *branch = malloc(20);
    sprintf(branch, "beq %s", label2);
    branchNode->num_use = 0;
    branchNode->num_def = 0;
    branchNode->code = branch;
    branchNode->type = INSTYPE_BRANCH;
    branchNode->label = label2;
    branchNode->astNode = node;
    insertAfterNode(cmpNode, branchNode);

    // to insert label after loop body -> idea: just add to the tail of the current node

    struct ARMNode* bNode = malloc(sizeof(struct ARMNode));
    char *b = malloc(20);
    sprintf(b, "b %s", labelName);
    bNode->num_use = 0;
    bNode->num_def = 0;
    bNode->code = b;
    bNode->type = INSTYPE_BRANCH;
    bNode->label = labelName;
    bNode->astNode = node;
    addToLinkedList(bNode);

    struct ARMNode* labelNode2 = malloc(sizeof(struct ARMNode));
    char *uselabel = malloc(20);
    sprintf(uselabel, "%s:", label2);
    labelNode2->num_use = 0;
    labelNode2->num_def = 0;
    labelNode2->code = uselabel;
    labelNode2->type = INSTYPE_LABEL;
    labelNode2->label = label2;
    labelNode2->astNode = node;
    addToLinkedList(labelNode2);

    //update the whileCounter
    whileCounter++;
}

void handleMethodCall(struct ASTNode* node) {
    struct ARMNode* asmNode = malloc(sizeof(struct ARMNode));
    char *methodName = node->data.value.string_value; 
    struct FormalNode* formal = malloc(sizeof(struct FormalNode));
    formal->count = 0;
    opInArg(node, formal);
    int argCount[4];
    for (int i = 0;  i < formal->count; i++) {
        struct ARMNode* argNode = malloc(sizeof(struct ARMNode));
        int regOffset = node->offsetTable->regCount * 4;
        argCount[i] = regOffset;
        node->offsetTable->regCount++;

        char *op = malloc(10);
        sprintf(op, "r%d", i);

        char *code = malloc(30);
        sprintf(code, "str %s, [SP, #%d]", op, regOffset);
        argNode->code = code;
        argNode->type = INSTYPE_STRSPILL;
        argNode->astNode = node;
        argNode->num_use = 1;
        argNode->USE[0] = op;
        argNode->num_def = 0;
        addToLinkedList(argNode);
    }

    for (int i = 0; i < formal->count; i++) {
        struct ARMNode* argNode = malloc(sizeof(struct ARMNode));
         // DEF
        char *reg = malloc(10);
        sprintf(reg, "r%d", i);
        argNode->DEF = reg;
        argNode->op1 = reg;
        argNode->num_def = 1;

        asmNode->USE[i] = reg; 

        char *symbolic = malloc(10);
        sprintf(symbolic, "$t%d", formal->regList[i]);
        argNode->USE[0] = symbolic;
        argNode->op2 = symbolic;
        argNode->num_use = 1;

        char *arg = malloc(25);
        sprintf(arg, "ldr %s, %s", reg, symbolic);
        argNode->code = arg;
        argNode->type = INSTYPE_CALLERSAVED;
        argNode->astNode = node;
        addToLinkedList(argNode);
    }
    
    //use r0 - r3
    char *temp = malloc(10);
    sprintf(temp, "bl %s", methodName);
    char *r0 = malloc(10);
    sprintf(r0, "r0");
    asmNode->num_def = 1;
    asmNode->DEF = r0;
    asmNode->num_use = formal->count;
    for (int i = 0; i < formal->count; i++) {
        char *reg = malloc(10);
        sprintf(reg, "r%d", i);
        asmNode->USE[i] = reg;
    }
    asmNode->code = temp;
    asmNode->type = INSTYPE_METHODCALL;  
    asmNode->astNode = node;
    addToLinkedList(asmNode);

    // store r0 to a new register
    struct ARMNode* storeNode = malloc(sizeof(struct ARMNode));
    char *storeReg = malloc(20);
    sprintf(storeReg, "$t%d", node->offsetTable->regCount);
    char *store = malloc(20);
    sprintf(store, "mov %s, %s", storeReg, r0);
    storeNode->op1 = storeReg;
    storeNode->op2 = r0;
    storeNode->num_use = 1;
    storeNode->USE[0] = r0;
    storeNode->num_def = 1;
    storeNode->DEF = storeReg;
    storeNode->code = store;
    storeNode->regIndex = node->offsetTable->regCount;
    node->offsetTable->regCount++;
    storeNode->type = INSTYPE_MOVR0;
    storeNode->astNode = node;
    node->asmNode = storeNode;
    addToLinkedList(storeNode);

    for (int i = 0;  i < formal->count; i++) {
        struct ARMNode* argNode = malloc(sizeof(struct ARMNode));
        int regOffset = argCount[i];
        char *op = malloc(10);
        sprintf(op, "r%d", i);
        char *code = malloc(30);
        sprintf(code, "ldr %s, [SP, #%d]", op, regOffset);
        argNode->code = code;
        argNode->type = INSTYPE_LDRSPILL;
        argNode->astNode = node;
        argNode->num_use = 0;
        argNode->num_def = 1;
        argNode->DEF = op;
        addToLinkedList(argNode);
    }


}

void handleExpType(struct ASTNode* node) { // delete index for hard coding argv[0]
    if (node->parentNode->node_type == NODETYPE_ONLYINT) {
        return; // skip for signed literal, handle it somewhere else
    }
    if (node->parentNode->num_children == 1 && node->parentNode->node_type == NODETYPE_INDEX
          && node->parentNode->parentNode->node_type == NODETYPE_NEWARRAY) {
        return; // skip for one-dimensional new array (only for array declaration)
    }
    if ((node->data.type == DATATYPE_INT || node->data.type == DATATYPE_BOOLEAN)  && !isStaticVar(node)) {  // Int literal not static
        int trueVal;
        if (node->data.type == DATATYPE_BOOLEAN) {
            bool value = node->data.value.boolean_value;
            trueVal = 0;
            if (value) {
                trueVal = 1;
            }
        } else {
            trueVal = node->data.value.int_value;
        }
        
        struct ARMNode* asmNode = malloc(sizeof(struct ARMNode));

        //DEF
        char *op1 = malloc(10);
        sprintf(op1, "$t%d", node->offsetTable->regCount);
        char *op2 = malloc(10);
        sprintf(op2, "#%d", trueVal);
        asmNode->op1 = op1;
        asmNode->op2 = op2;
        asmNode->DEF = op1;
        asmNode->num_def = 1;
        asmNode->num_use = 0;
        char *temp = malloc(30);
        sprintf(temp, "mov %s, %s", op1, op2);
        asmNode->regIndex = node->offsetTable->regCount;
        node->offsetTable->regCount++;              
        asmNode->code = temp;
        asmNode->type = INSTYPE_MOVVAL;
        node->asmNode = asmNode;
        asmNode->astNode = node;
        addToLinkedList(asmNode);
    } else if (node->data.type == DATATYPE_STR && !isStaticVar(node)) {
        char *string = node->data.value.string_value;
        stringList[stringCounter] = string;
        stringCounter++;
    }
}

void handleBoolNot(struct ASTNode* node) {
    struct ARMNode* expInsNode;
    if (node->children[0]->node_type == NODETYPE_OPERATION || node->children[0]->node_type == NODETYPE_EXPTYPE) {
        expInsNode = node->children[0]->asmNode;
    } else if (node->children[0]->node_type == NODETYPE_EXPMETHOD) {
        expInsNode = node->children[0]->children[0]->asmNode;
    } else if (node->children[0]->node_type == NODETYPE_EXP) {
        if (node->children[0]->children[0]->node_type == NODETYPE_LEFTVAL
            || node->children[0]->children[0]->node_type == NODETYPE_OPERATION) {
            expInsNode = node->children[0]->children[0]->asmNode;
        }
    }
    int expIndex = expInsNode->regIndex;

    char *symbolic = malloc(10);
    sprintf(symbolic, "$t%d", expIndex);

    char *temp1 = malloc(30);
    struct ARMNode* asmNode1 = malloc(sizeof(struct ARMNode));
    sprintf(temp1, "mvn %s, %s", symbolic, symbolic);
    asmNode1->code = temp1;
    asmNode1->type = INSTYPE_MVN;

    asmNode1->num_use = 1;
    asmNode1->USE[0] = symbolic;
    asmNode1->num_def = 1;
    asmNode1->DEF = symbolic;
    asmNode1->op1 = symbolic;
    asmNode1->op2 = symbolic;
    asmNode1->astNode = node;
    addToLinkedList(asmNode1);

    char *op3 = malloc(5);
    sprintf(op3, "#1");
    char *temp2 = malloc(30);
    struct ARMNode* asmNode2 = malloc(sizeof(struct ARMNode));
    sprintf(temp2, "and %s, %s, %s", symbolic, symbolic, op3);
    asmNode2->code = temp2;
    asmNode2->type = INSTYPE_ANDVAL;

    asmNode2->num_use = 1;
    asmNode2->USE[0] = symbolic;
    asmNode2->num_def = 1;
    asmNode2->DEF = symbolic;
    asmNode2->op1 = symbolic;
    asmNode2->op2 = symbolic;
    asmNode2->op3 = op3;
    asmNode2->astNode = node;
    addToLinkedList(asmNode2);

    char *new_symbolic = malloc(10);
    sprintf(new_symbolic, "$t%d", node->offsetTable->regCount * 4);
    char *temp3 = malloc(50);
    struct ARMNode* asmNode3 = malloc(sizeof(struct ARMNode));
    sprintf(temp3, "mov %s, %s", new_symbolic, symbolic);

    asmNode3->num_def = 1;
    asmNode3->DEF = new_symbolic;
    asmNode3->num_use = 1;
    asmNode3->USE[0] = symbolic;

    asmNode3->op1 = new_symbolic;
    asmNode3->op2 = symbolic;
    asmNode3->code = temp3;
    asmNode3->type = INSTYPE_MOV;
    asmNode3->astNode = node;
    addToLinkedList(asmNode3);
    asmNode3->regIndex = node->offsetTable->regCount;
    node->offsetTable->regCount++;
           
    node->asmNode = asmNode3;
    asmNode3->astNode = node;
}

void handleLeftVal(struct ASTNode* node) {
    if(node->num_children > 0) { // for array
        if (node->num_children == 1) { // for one dimensional array leftVal -> ID [ EXP ]
            int indexOffset; // need to *4, its just index
            if (node->children[0]->node_type == NODETYPE_OPERATION) {
                indexOffset = node->children[0]->asmNode->regIndex;
            } else if (node->children[0]->node_type == NODETYPE_EXPTYPE) {
                indexOffset = node->children[0]->asmNode->regIndex;
            } else if (node->children[0]->node_type == NODETYPE_EXP) {
                if (node->children[0]->children[0]->node_type == NODETYPE_LEFTVAL) {
                    char *ID = node->children[0]->children[0]->data.value.string_value;
                    indexOffset = findOffsetInTable(ID, node->table) / 4;
                }       
            }
            char *ID = node->data.value.string_value;
            int leftIndex = findOffsetInTable(ID, node->table);
            char *op1 = malloc(10);
            sprintf(op1, "$t%d", indexOffset);
            char *op2 = malloc(10);
            sprintf(op2, "$t%d", leftIndex / 4);
            char *op3 = malloc(10);
            sprintf(op3, "$t%d", node->offsetTable->regCount);
            char *numVal = malloc(10);
            sprintf(numVal, "#2");

            struct ARMNode* lslNode = malloc(sizeof(struct ARMNode));
            char *lsl = malloc(30);
            sprintf(lsl, "lsl %s, %s, %s", op3, op1, numVal);

            lslNode->op1 = op3;
            lslNode->op2 = op1;
            lslNode->op3 = numVal;
            lslNode->num_def = 1;
            lslNode->DEF = op3;
            lslNode->num_use = 1;
            lslNode->USE[0] = op1;

            lslNode->code = lsl;
            lslNode->regIndex = node->offsetTable->regCount;
            lslNode->type = INSTYPE_LSL;
            lslNode->astNode = node;
            addToLinkedList(lslNode);

            if (strcmp(ID, "args") == 0) {
                struct ARMNode* ldrNode = malloc(sizeof(struct ARMNode));
                char *ldr = malloc(30);
                sprintf(ldr, "ldr %s, [SP, #4]", op2);
                ldrNode->op1 = op2;
                ldrNode->DEF = op2;
                ldrNode->num_def = 1;
                ldrNode->num_use = 0;
                ldrNode->code = ldr;
                ldrNode->type = INSTYPE_LDR;
                ldrNode->astNode = node;
                addToLinkedList(ldrNode);
            }
            
            struct ARMNode* asmNode = malloc(sizeof(struct ARMNode));
            char *temp = malloc(50);
            sprintf(temp, "add %s, %s, %s", op3, op2, op3);  
            
            asmNode->op1 = op3;
            asmNode->op2 = op2;
            asmNode->op3 = op3;

            asmNode->num_def = 1;
            asmNode->DEF = op3;
            asmNode->num_use = 2;
            asmNode->USE[0] = op2;
            asmNode->USE[1] = op3; 

            asmNode->code = temp;
            asmNode->regIndex = node->offsetTable->regCount;
            node->offsetTable->regCount++;
            asmNode->type = INSTYPE_ADD;
            node->asmNode = asmNode;
            asmNode->astNode = node;
            addToLinkedList(asmNode);
        } 
    } else {
        if (!isStaticVar(node)) {
            enum DataType type = findExpType(node);
            if (node->num_children == 0 && type != DATATYPE_STR) {
                    char *ID = node->data.value.string_value; // local variable
                    int leftValReg = findOffsetInTable(ID, node->table);
                    for (int i = 0; i < node->table->arglist->num_args; i++) {
                        if (strcmp(ID, node->table->arglist->ID[i]) == 0) {
                            leftValReg = i * 4;
                            break;
                        }
                    }
                    char *symbolic = malloc(10);
                    sprintf(symbolic, "$t%d", leftValReg / 4);

                    struct ARMNode* asmNode = malloc(sizeof(struct ARMNode));
                    char *temp = malloc(30);
                    sprintf(temp, "mov %s, %s", symbolic, symbolic);
                    asmNode->regIndex = leftValReg / 4;
                    
                    asmNode->num_def = 1;
                    asmNode->DEF = symbolic;
                    asmNode->num_use = 1;
                    asmNode->USE[0] = symbolic;

                    asmNode->op1 = symbolic;
                    asmNode->op2 = symbolic;
                    asmNode->code = temp;
                    asmNode->type = INSTYPE_MOV;
                
                    node->asmNode = asmNode;
                    asmNode->astNode = node;
                    addToLinkedList(asmNode);
                }
        }
    }
    
}

void handleOperation(struct ASTNode* node) {
    char *op = node->data.value.string_value;
    // numerical operators
    if (strcmp(op, "+") == 0 || strcmp(op, "-") == 0 || strcmp(op, "*") == 0) {
        char *opeartor;
        if (strcmp(op, "+") == 0) {
            opeartor = "add";
        } else if (strcmp(op, "-") == 0) {
            opeartor = "sub";
        } else if (strcmp(op, "*") == 0) {
            opeartor = "mul";
        }
        
        int leftReg = 0;
        if (node->children[0]->node_type == NODETYPE_EXPMETHOD) {
            leftReg = node->children[0]->children[0]->asmNode->regIndex;
        } else if (node->children[0]->node_type == NODETYPE_EXP) {
            if (node->children[0]->children[0]->node_type == NODETYPE_LEFTVAL 
                && node->children[0]->children[0]->num_children == 0) {
                char *ID = node->children[0]->children[0]->data.value.string_value;
                leftReg = findOffsetInTable(ID, node->table) / 4;
                for (int i = 0; i < node->table->arglist->num_args; i++) {
                    if (strcmp(ID, node->table->arglist->ID[i]) == 0) {
                        leftReg = i;
                        break;
                    }
                }
            } else if (node->children[0]->children[0]->node_type == NODETYPE_LEFTVAL  
                    && node->children[0]->children[0]->num_children == 1) { // one dimensional array value as operand
                leftReg = node->children[0]->children[0]->asmNode->regIndex;
            } else {
                leftReg = findOpReg(node->children[0]);
            }
        } else {
            leftReg = findOpReg(node->children[0]);
        }
        int rightReg = 0;
        if (node->children[1]->node_type == NODETYPE_EXPMETHOD) {
            rightReg = node->children[1]->children[0]->asmNode->regIndex;
        } else if (node->children[1]->node_type == NODETYPE_EXP) {
            if (node->children[1]->children[0]->node_type == NODETYPE_LEFTVAL
                && node->children[1]->children[0]->num_children == 0) {
                char *ID = node->children[1]->children[0]->data.value.string_value;
                rightReg = findOffsetInTable(ID, node->table) / 4;
                for (int i = 0; i < node->table->arglist->num_args; i++) {
                    if (strcmp(ID, node->table->arglist->ID[i]) == 0) {
                        rightReg = i;
                        break;
                    }
                }
            } else if (node->children[1]->children[0]->node_type == NODETYPE_LEFTVAL  
                    && node->children[1]->children[0]->num_children == 1) { // array value as operand
                rightReg = node->children[1]->children[0]->asmNode->regIndex;
            } else {
                rightReg = findOpReg(node->children[1]);
            }
        } else {
            rightReg = findOpReg(node->children[1]);
        }
        findOpReg(node->children[1]);
        struct ARMNode* asmNode = malloc(sizeof(struct ARMNode));

        char *op1 = malloc(10);
        sprintf(op1, "$t%d", node->offsetTable->regCount);
        char *op2 = malloc(10);
        sprintf(op2, "$t%d", leftReg);
        char *op3 = malloc(10);
        sprintf(op3, "$t%d", rightReg);

        asmNode->op1 = op1;
        asmNode->op2 = op2;
        asmNode->op3 = op3;

        char *temp = malloc(60);
        sprintf(temp, "%s %s, %s, %s", opeartor, op1, op2, op3);

        asmNode->num_def = 1;
        asmNode->DEF = op1;
        asmNode->num_use = 2;
        asmNode->USE[0] = op2;
        asmNode->USE[1] = op3;

        asmNode->regIndex = node->offsetTable->regCount;
        node->offsetTable->regCount++;
          
        asmNode->code = temp;
        if (strcmp(op, "+") == 0) {
            asmNode->type = INSTYPE_ADD;
        } else if (strcmp(op, "-") == 0) {
            asmNode->type = INSTYPE_SUB;
        } else if (strcmp(op, "*") == 0) {
            asmNode->type = INSTYPE_MUL;
        }

        node->asmNode = asmNode;
        asmNode->astNode = node;
        addToLinkedList(asmNode);
    } else if (strcmp(op, "&&") == 0 || strcmp(op, "||") == 0) {
        struct ARMNode* asmNode = malloc(sizeof(struct ARMNode));
        int leftIndex;
        if (node->children[0]->node_type == NODETYPE_EXP) {
            leftIndex = node->children[0]->children[0]->asmNode->regIndex;
        } else if (node->children[0]->node_type == NODETYPE_EXPMETHOD){
            leftIndex = node->children[0]->children[0]->asmNode->regIndex;
        } else {
            leftIndex = node->children[0]->asmNode->regIndex;
        }
        int rightIndex;
        if (node->children[1]->node_type == NODETYPE_EXP) {
            rightIndex = node->children[1]->children[0]->asmNode->regIndex;
        } else if (node->children[1]->node_type == NODETYPE_EXPMETHOD){
            rightIndex = node->children[1]->children[0]->asmNode->regIndex;
        } else {
            rightIndex = node->children[1]->asmNode->regIndex;
        }
        char *andOr;
        enum INSTYPE type;
        if (strcmp(op, "&&") == 0) {
            andOr = "and";
            type = INSTYPE_AND;
        } else { // "||"
            andOr = "orr";
            type = INSTYPE_ORR;
        }

        char *op1 = malloc(10);
        sprintf(op1, "$t%d", node->offsetTable->regCount);
        char *op2 = malloc(10);
        sprintf(op2, "$t%d", leftIndex);
        char *op3 = malloc(10);
        sprintf(op3, "$t%d", rightIndex);

        asmNode->op1 = op1;
        asmNode->op2 = op2;
        asmNode->op3 = op3;


        char *temp = malloc(150);
        sprintf(temp, "%s %s, %s, %s", andOr, op1, op2, op3);

        asmNode->num_def = 1;
        asmNode->DEF = op1;
        asmNode->num_use = 2;
        asmNode->USE[0] = op2;
        asmNode->USE[1] = op3;

        asmNode->regIndex = node->offsetTable->regCount;
        node->offsetTable->regCount++;
          
        asmNode->type = type;
        asmNode->code = temp;
        node->asmNode = asmNode;
        asmNode->astNode = node;
        addToLinkedList(asmNode);
    } else { // all bool operators ">" "<" ...
        char *boolIns;
        if(strcmp(op, "<") == 0) {
            boolIns = "movlt";
        } else if(strcmp(op, ">") == 0) {
            boolIns = "movgt";
        } else if(strcmp(op, "<=") == 0) {
            boolIns = "movle";
        } else if(strcmp(op, ">=") == 0) {
            boolIns = "movge";
        } else if(strcmp(op, "==") == 0) {
            boolIns = "moveq";
        } else if(strcmp(op, "!=") == 0) {
            boolIns = "movne";
        }
        int leftIndex;
        if (node->children[0]->node_type == NODETYPE_EXP) {
            leftIndex = node->children[0]->children[0]->asmNode->regIndex;
        } else if (node->children[0]->node_type == NODETYPE_EXPMETHOD){
            leftIndex = node->children[0]->children[0]->asmNode->regIndex;
        } else {
            leftIndex = node->children[0]->asmNode->regIndex;
        }
        int rightIndex;
        if (node->children[1]->node_type == NODETYPE_EXP) {
            rightIndex = node->children[1]->children[0]->asmNode->regIndex;
        } else if (node->children[1]->node_type == NODETYPE_EXPMETHOD){
            rightIndex = node->children[1]->children[0]->asmNode->regIndex;
        } else {
            rightIndex = node->children[1]->asmNode->regIndex;
        }
        
        char *op1 = malloc(10);
        sprintf(op1, "$t%d", node->offsetTable->regCount);
        char *op2 = malloc(10);
        sprintf(op2, "$t%d", leftIndex);
        char *op3 = malloc(10);
        sprintf(op3, "$t%d", rightIndex);

        // CMP
        struct ARMNode* cmpNode = malloc(sizeof(struct ARMNode));
        char *cmp = malloc(20);
        sprintf(cmp, "cmp %s, %s", op2, op3);
        
        cmpNode->num_use = 2;
        cmpNode->USE[0] = op2;
        cmpNode->USE[1] = op3;
        cmpNode->num_def = 0;

        cmpNode->code = cmp;
        cmpNode->op1 = op2;
        cmpNode->op2 = op3;
        cmpNode->type = INSTYPE_CMP;
        cmpNode->astNode = node;
        addToLinkedList(cmpNode);


        // MOV
        struct ARMNode* movNode = malloc(sizeof(struct ARMNode));
        char *mov = malloc(20);
        char *zero = malloc(10);
        sprintf(zero, "#0");
        sprintf(mov, "mov %s, %s", op1, zero);

        movNode->num_use = 0;
        movNode->num_def = 1;
        movNode->DEF = op1;

        movNode->code = mov;
        movNode->op1 = op1;
        movNode->op2 = zero;
        movNode->type = INSTYPE_MOVVAL;
        movNode->astNode = node;
        addToLinkedList(movNode);



        // BMOV
        struct ARMNode* BmovNode = malloc(sizeof(struct ARMNode));
        char *one = malloc(10);
        sprintf(one, "#1");
        char *Bmov = malloc(50);
        sprintf(Bmov, "%s %s, %s", boolIns, op1, one);
        BmovNode->regIndex = node->offsetTable->regCount;
        BmovNode->type = INSTYPE_BMOV;
        node->offsetTable->regCount++;
        

        BmovNode->num_use = 0;
        BmovNode->num_def = 1;
        BmovNode->DEF = op1;

        BmovNode->op1 = boolIns;
        BmovNode->op2 = op1;
        BmovNode->op3 = one;
        BmovNode->code = Bmov;
        node->asmNode = BmovNode;
        BmovNode->astNode = node;
        addToLinkedList(BmovNode);
    }
    
}

void handleVarDecl(struct ASTNode* node) { 
    if (node->children[1]->children[0]->node_type == NODETYPE_EXPMETHOD) {
        struct ASTNode* expMethod = node->children[1]->children[0];
        if (expMethod->children[0]->node_type == NODETYPE_METHODCALL) {
            char *ID = expMethod->children[0]->data.value.string_value;
            enum DataType type = findTypeInMethodTable(ID);
            if (type == DATATYPE_STR) {
                return;
            }
        }
        char *ID = node->data.value.string_value;
        struct ARMNode* asmNode = malloc(sizeof(struct ARMNode));
        // memset(asmNode, 0, sizeof(struct ARMNode));
        char *temp = malloc(200);
        int offset = expMethod->children[0]->asmNode->regIndex;

        char *op1 = malloc(10);
        sprintf(op1, "$t%d", offset);
        char *op2 = malloc(5);
        sprintf(op2, "r0");

        sprintf(temp, "mov %s, %s", op1, op2);

        asmNode->op1 = op1;
        asmNode->op2 = op2;
        asmNode->num_use = 1;
        asmNode->USE[0] = op2;
        asmNode->num_def = 1;
        asmNode->DEF = op1;

        asmNode->code = temp;
        asmNode->type = INSTYPE_STRMETHOD;
        asmNode->regIndex = offset;
        // node->offsetTable->regCount++;
        node->asmNode = asmNode;
        asmNode->astNode = node;
        addToLinkedList(asmNode);
        storeOffsetToTable(ID, offset*4, node->table);
        return; // only handle methodCall
    }

    enum DataType type = findType(node->children[0]);
    if (isStaticVar(node)) { // static, not need to store
        if (type == DATATYPE_STR) {
            char *ID = node->data.value.string_value;
            char *string;
            if (node->children[1]->children[0]->node_type == NODETYPE_EXPTYPE) {
                string = node->children[1]->children[0]->data.value.string_value;
            }
            staticStringList[staticStringNum] = string;
            staticStringName[staticStringNum] = ID;
            staticStringNum++;
        } else if (type == DATATYPE_INT) {
            char *ID = node->data.value.string_value;
            int intVal;
            if (node->children[1]->children[0]->node_type == NODETYPE_EXPTYPE) {
                intVal = node->children[1]->children[0]->data.value.int_value;
            }
            staticIntList[staticIntNum] = intVal;
            staticIntName[staticIntNum] = ID;
            staticIntNum++;
        }
    } else {
        if (type == DATATYPE_INT || type == DATATYPE_BOOLEAN) {
            if (node->num_children > 1 && node->children[1]->node_type == NODETYPE_VARINIT) { // int a = 1 case
                char* ID = node->data.value.string_value;
                int rightReg;
                if (node->children[1]->children[0]->node_type == NODETYPE_EXPTYPE 
                    || node->children[1]->children[0]->node_type == NODETYPE_OPERATION) {
                    rightReg = node->children[1]->children[0]->asmNode->regIndex;
                } else if (node->children[1]->children[0]->node_type == NODETYPE_EXPMETHOD) {
                    rightReg = node->children[1]->children[0]->children[0]->asmNode->regIndex;
                } else if (node->children[1]->children[0]->node_type == NODETYPE_EXP) {
                    if (node->children[1]->children[0]->children[0]->node_type == NODETYPE_LEFTVAL) {
                        struct ASTNode* leftValNode = node->children[1]->children[0]->children[0];
                        rightReg = leftValNode->asmNode->regIndex;
                    }
                } 
                int offset = rightReg * 4;
                storeOffsetToTable(ID, offset, node->table);
                // node->offsetTable->regCount++;
            } else { // int a; case
                char* ID = node->data.value.string_value;
                int offset = node->offsetTable->regCount * 4;
                storeOffsetToTable(ID, offset, node->table);
                node->offsetTable->regCount++;
                  
            }
        } else if (type == DATATYPE_STR){
            if (node->num_children > 1 && node->children[1]->node_type == NODETYPE_VARINIT) {
                char *ID = node->data.value.string_value;
                char *string = node->children[1]->children[0]->data.value.string_value;
                int index = 0;
                for (int i = 0; i < stringCounter; i++) {
                    if (strcmp(stringList[i], string) == 0) {
                        index = i;
                    }
                }
                storeStringToTable(ID, index, node->table);
            }
        }
    }
}

void handleMoreVar(struct ASTNode* node) { // TO DO BOOL    
    enum DataType type = findTypeForMoreVar(node);
    if (isStaticVar(node)) { // more Var in Static
        if (type == DATATYPE_STR) {
            char *ID = node->data.value.string_value;
            char *string = "notInit";
            if (node->num_children == 1 && node->children[0]->node_type == NODETYPE_VARINIT) {
                if (node->children[0]->children[0]->node_type == NODETYPE_EXPTYPE) {
                    string = node->children[0]->children[0]->data.value.string_value;
                } // else need to find it
                staticStringList[staticStringNum] = string;
                staticStringName[staticStringNum] = ID;
                staticStringNum++;
            } else if (node->num_children == 2) {
                if (node->children[1]->children[0]->node_type == NODETYPE_EXPTYPE) {
                    string = node->children[1]->children[0]->data.value.string_value;
                }
                staticStringList[staticStringNum] = string;
                staticStringName[staticStringNum] = ID;
                staticStringNum++;
            }
            
        } else if (type == DATATYPE_INT) {
            if (node->num_children == 1 && node->children[0]->node_type == NODETYPE_VARINIT) {
                char *ID = node->data.value.string_value;
                int intVal;
                if (node->children[0]->children[0]->node_type == NODETYPE_EXPTYPE) {
                    intVal = node->children[0]->children[0]->data.value.int_value;
                }
                staticIntList[staticIntNum] = intVal;
                staticIntName[staticIntNum] = ID;
                staticIntNum++;
            } else if (node->num_children == 2) {
                char *ID = node->data.value.string_value;
                int intVal;
                if (node->children[1]->children[0]->node_type == NODETYPE_EXPTYPE) {
                    intVal = node->children[1]->children[0]->data.value.int_value;
                }
                staticIntList[staticIntNum] = intVal;
                staticIntName[staticIntNum] = ID;
                staticIntNum++;
            }
        }
    } else {
        if (type == DATATYPE_STR) {
            char *ID = node->data.value.string_value;
            char *string;
            if (node->children[1]->children[0]->node_type == NODETYPE_EXPTYPE) {
                string = node->children[1]->children[0]->data.value.string_value;
            }
            staticStringList[staticStringNum] = string;
            staticStringName[staticStringNum] = ID;
            staticStringNum++;
        } else if (type == DATATYPE_INT || type == DATATYPE_BOOLEAN) {
            if (node->num_children == 1 && node->children[0]->node_type == NODETYPE_VARINIT) { // , a = 1;
                char* ID = node->data.value.string_value;
                int rightReg;
                if (node->children[0]->children[0]->node_type == NODETYPE_EXPTYPE) {
                    rightReg = node->children[0]->children[0]->asmNode->regIndex;
                } else if (node->children[0]->children[0]->node_type == NODETYPE_EXPMETHOD) {
                    rightReg = node->children[0]->children[0]->children[0]->asmNode->regIndex;
                } else if (node->children[0]->children[0]->node_type == NODETYPE_EXP) {
                    if (node->children[0]->children[0]->children[0]->node_type == NODETYPE_LEFTVAL) {
                        struct ASTNode* leftValNode = node->children[0]->children[0]->children[0];
                        rightReg = leftValNode->asmNode->regIndex;
                    }
                }
                storeOffsetToTable(ID, rightReg * 4, node->table);
                // node->offsetTable->regCount++;
            } else if (node->num_children == 2){ // ..., a = 1;
                char* ID = node->data.value.string_value;
                int rightReg;
                if (node->children[1]->children[0]->node_type == NODETYPE_EXPTYPE) {
                    rightReg = node->children[1]->children[0]->asmNode->regIndex;
                } else if (node->children[1]->children[0]->node_type == NODETYPE_EXPMETHOD) {
                    rightReg = node->children[1]->children[0]->children[0]->asmNode->regIndex;
                } else if (node->children[1]->children[0]->node_type == NODETYPE_EXP) {
                    if (node->children[1]->children[0]->children[0]->node_type == NODETYPE_LEFTVAL) {
                        struct ASTNode* leftValNode = node->children[1]->children[0]->children[0];
                        rightReg = leftValNode->asmNode->regIndex;
                    }
                }
                
                storeOffsetToTable(ID, rightReg * 4, node->table);
                // node->offsetTable->regCount++;
            } else { // int a; case
                char* ID = node->data.value.string_value;
                int offset = node->offsetTable->regCount * 4;
                storeOffsetToTable(ID, offset, node->table);
                node->offsetTable->regCount++;
                  
            }
        }
        
    }   
}

void handleAssign(struct ASTNode* node) { // LeftValue '=' Exp ';'
    char* ID = node->children[0]->data.value.string_value;
    enum DataType type = findTypeInTable(ID, node->table);
    if (type != DATATYPE_STR) {
        int rightReg;
        if (node->children[1]->node_type == NODETYPE_EXP) {
            if (node->children[1]->children[0]->node_type == NODETYPE_LEFTVAL) {
                rightReg = node->children[1]->children[0]->asmNode->regIndex;
            }
        } else if (node->children[1]->node_type == NODETYPE_EXPMETHOD) {
            rightReg = node->children[1]->children[0]->asmNode->regIndex;
        } else {
            rightReg = node->children[1]->asmNode->regIndex;
        }
        int leftOffset = getIndexOfArg(ID, node) * 4;
        if (leftOffset / 4 == 9999) {
            leftOffset = findOffsetInTable(ID, node->table);
        }

        char *op1 = malloc(10);
        sprintf(op1, "$t%d", leftOffset / 4);
        char *op2 = malloc(10);
        sprintf(op2, "$t%d", rightReg);
        struct ARMNode* asmNode = malloc(sizeof(struct ARMNode));
        char *temp = malloc(50);
        sprintf(temp, "mov %s, %s", op1, op2);

        asmNode->op1 = op1;
        asmNode->op2 = op2;
        asmNode->num_def = 1;
        asmNode->DEF = op1;
        asmNode->num_use = 1;
        asmNode->USE[0] = op2;

        asmNode->code = temp;
        asmNode->type = INSTYPE_MOV;
        asmNode->regIndex = rightReg;
        node->asmNode = asmNode;
        asmNode->astNode = node;
        addToLinkedList(asmNode);
    } else {
        if (node->children[1]->node_type == NODETYPE_EXPTYPE) {
            int staticIndex = findStaticString(ID);
            if (staticIndex != 9999) {
                staticStringList[staticIndex] = "\"rewrite\""; // handling rewrite
            }
            char *string = node->children[1]->data.value.string_value;
            int index =0;
            for (int i = 0; i < stringCounter; i++) {
                if (strcmp(stringList[i], string) == 0) {
                    index = i;
                }
            }
            storeStringToTable(ID, index, node->table);
        }
    }
}

void handlePrint(struct ASTNode* node) {
    if (node->children[0]->node_type == NODETYPE_OPERATION) {
        char *op1 = malloc(10);
        sprintf(op1, "r0");
        char *op2 = malloc(10);
        sprintf(op2, "r1");
        char *op3 = malloc(10);
        sprintf(op1, "$t%d", node->children[0]->asmNode->regIndex * 4);

        struct ARMNode* intNode = malloc(sizeof(struct ARMNode));
        char *intIns = malloc(30);
        sprintf(intIns, "ldr %s, =integer", op1);
        intNode->code = intIns;
        intNode->type = INSTYPE_LDRINT;
        intNode->op1 = op1;
        intNode->num_def = 1;
        intNode->DEF = op1;
        intNode->num_use = 0;
        intNode->astNode = node;
        addToLinkedList(intNode);

        struct ARMNode* movNode = malloc(sizeof(struct ARMNode));
        char *mov = malloc(30);
        sprintf(mov, "mov %s, %s", op2, op3);
        movNode->code = mov;
        movNode->op1 = op2;
        movNode->op2 = op3;
        movNode->num_use = 1;
        movNode->USE[0] = op3;
        movNode->num_def = 1;
        movNode->DEF = op2;
        movNode->type = INSTYPE_MOVREG;
        movNode->astNode = node;
        addToLinkedList(movNode);

        struct ARMNode* printNode = malloc(sizeof(struct ARMNode));
        char *print= malloc(30);
        sprintf(print, "bl printf");
        printNode->code = print;
        printNode->num_use = 2;
        printNode->USE[0] = op1;
        printNode->USE[1] = op2;
        printNode->num_def = 0;
        printNode->type = INSTYPE_PRINT;
        printNode->astNode = node;
        addToLinkedList(printNode);
        
        if (node->node_type == NODETYPE_PRINTLN) {
            struct ARMNode* printLnNode = malloc(sizeof(struct ARMNode));
            char *op1 = malloc(10);
            sprintf(op1, "r0");
            char *printLn= malloc(30);
            sprintf(printLn, "ldr %s, =newline", op1);
            printLnNode->code = printLn;
            printLnNode->num_use = 0;
            printLnNode->num_def = 1;
            printLnNode->DEF = op1;
            printLnNode->type = INSTYPE_LDRINT;
            printLnNode->astNode = node;
            addToLinkedList(printLnNode);

            struct ARMNode* printLnNode2 = malloc(sizeof(struct ARMNode));
            char *printLn2= malloc(30);
            sprintf(printLn2, "bl printf");
            printLnNode2->code = printLn2;
            printLnNode2->num_use = 1;
            printLnNode2->USE[0] = op1;
            printLnNode->num_def = 0;
            printLnNode->type = INSTYPE_LDRINT;
            printLnNode2->astNode = node;
            addToLinkedList(printLnNode2);

        }
        
    } else if (node->children[0]->node_type == NODETYPE_EXPMETHOD) {
        char *op1 = malloc(10);
        sprintf(op1, "r0");
        char *op2 = malloc(10);
        sprintf(op2, "r1");
        char *op3 = malloc(10);
        sprintf(op3, "$t%d", node->children[0]->children[0]->asmNode->regIndex);

       struct ARMNode* intNode = malloc(sizeof(struct ARMNode));
        char *intIns = malloc(30);
        sprintf(intIns, "ldr %s, =integer", op1);
        intNode->code = intIns;
        intNode->type = INSTYPE_LDRINT;
        intNode->op1 = op1;
        intNode->num_def = 1;
        intNode->DEF = op1;
        intNode->num_use = 0;
        intNode->astNode = node;
        addToLinkedList(intNode);

        struct ARMNode* movNode = malloc(sizeof(struct ARMNode));
        char *mov = malloc(30);
        sprintf(mov, "mov %s, %s", op2, op3);
        movNode->code = mov;
        movNode->op1 = op2;
        movNode->op2 = op3;
        movNode->num_use = 1;
        movNode->USE[0] = op3;
        movNode->num_def = 1;
        movNode->DEF = op2;
        movNode->type = INSTYPE_MOVREG;
        movNode->astNode = node;
        addToLinkedList(movNode);

        struct ARMNode* printNode = malloc(sizeof(struct ARMNode));
        char *print= malloc(30);
        sprintf(print, "bl printf");
        printNode->code = print;
        printNode->num_use = 2;
        printNode->USE[0] = op1;
        printNode->USE[1] = op2;
        printNode->num_def = 0;
        printNode->type = INSTYPE_PRINT;
        printNode->astNode = node;
        addToLinkedList(printNode);
        
        if (node->node_type == NODETYPE_PRINTLN) {
            struct ARMNode* printLnNode = malloc(sizeof(struct ARMNode));
            char *op1 = malloc(10);
            sprintf(op1, "r0");
            char *printLn= malloc(30);
            sprintf(printLn, "ldr %s, =newline", op1);
            printLnNode->code = printLn;
            printLnNode->num_use = 0;
            printLnNode->num_def = 1;
            printLnNode->DEF = op1;
            printLnNode->type = INSTYPE_LDRINT;
            printLnNode->astNode = node;
            addToLinkedList(printLnNode);

            struct ARMNode* printLnNode2 = malloc(sizeof(struct ARMNode));
            char *printLn2= malloc(30);
            sprintf(printLn2, "bl printf");
            printLnNode2->code = printLn2;
            printLnNode2->num_use = 1;
            printLnNode2->USE[0] = op1;
            printLnNode2->num_def = 0;
            printLnNode2->type = INSTYPE_LDRINT;
            printLnNode2->astNode = node;
            addToLinkedList(printLnNode2);
        }
          
    } else if (node->children[0]->node_type == NODETYPE_EXPTYPE) { //
        char *string = node->children[0]->data.value.string_value;
        int index = findStringInList(string);
        struct ARMNode* asmNode = malloc(sizeof(struct ARMNode));
        char *temp = malloc(200);
        if (node->node_type == NODETYPE_PRINT) {
            sprintf(temp, "ldr r0, =msg%d\n  bl printf", index);
        } else {
            sprintf(temp, "ldr r0, =msg%d\n  bl printf\n  ldr r0, =newline\n  bl printf", index);
        }
        asmNode->code = temp;
        asmNode->regIndex = 9999; //no regIndex
        asmNode->num_use = 0;
        asmNode->num_def = 0;
        asmNode->type = INSTYPE_PRINTLN;
        node->asmNode = asmNode;
        asmNode->astNode = node;
        addToLinkedList(asmNode);
    } else if (node->children[0]->children[0]->node_type == NODETYPE_LEFTVAL
                && node->children[0]->children[0]->num_children == 0) { // print(a); // exp -> exp -> leftVal
        enum DataType type = findExpType(node->children[0]);
        char *ID = node->children[0]->children[0]->data.value.string_value;
        if (type == DATATYPE_STR) {
            int index = findStringIndexInTable(ID, node->table);
            struct ARMNode* asmNode = malloc(sizeof(struct ARMNode));
            int staticIndex = findStaticString(ID);
            char *temp = malloc(200);
            if (node->node_type == NODETYPE_PRINT && staticIndex == 9999) { // 9999 = not a static string
                sprintf(temp, "ldr r0, =msg%d\n  bl printf", index);
            } else if (node->node_type == NODETYPE_PRINTLN && staticIndex == 9999){
                sprintf(temp, "ldr r0, =msg%d\n  bl printf\n  ldr r0, =newline\n  bl printf", index);
            } else if (node->node_type == NODETYPE_PRINT && staticIndex != 9999 && strcmp(staticStringList[staticIndex], "\"rewrite\"") != 0 ) {
                // not re-written
                sprintf(temp, "ldr r0, =%s\n  bl printf", staticStringName[staticIndex]);
            } else if (node->node_type == NODETYPE_PRINTLN && staticIndex != 9999 && strcmp(staticStringList[staticIndex], "\"rewrite\"") != 0) {
                sprintf(temp, "ldr r0, =%s\n  bl printf\n  ldr r0, =newline\n  bl printf", staticStringName[staticIndex]);
            } else if (node->node_type == NODETYPE_PRINT && staticIndex != 9999 && strcmp(staticStringList[staticIndex], "\"rewrite\"") == 0) {
                sprintf(temp, "ldr r0, =msg%d\n  bl printf", index);
            } else {
                sprintf(temp, "ldr r0, =msg%d\n  bl printf\n  ldr r0, =newline\n  bl printf", index);
            }
            asmNode->code = temp;
            asmNode->regIndex = 9999; //no regIndex
            asmNode->num_use = 0;
            asmNode->num_def = 0;
            asmNode->type = INSTYPE_PRINT;
            node->asmNode = asmNode;
            asmNode->astNode = node;
            addToLinkedList(asmNode);
        } else if (type == DATATYPE_INT) {
            int offset = findOffsetInTable(ID, node->table);
            char *op1 = malloc(10);
            sprintf(op1, "$t%d", offset / 4);
            char *op2 = malloc(10);
            sprintf(op2, "r1");
            char *op3 = malloc(10);
            sprintf(op3, "r0");

            struct ARMNode* movNode = malloc(sizeof(struct ARMNode));
            char *mov = malloc(30);
            sprintf(mov, "mov %s, %s", op2, op1);  // mov to r1
            movNode->code = mov;
            movNode->op1 = op2;
            movNode->op2 = op1;
            movNode->num_use = 1;
            movNode->USE[0] = op1;
            movNode->num_def = 1;
            movNode->DEF = op2;
            movNode->type = INSTYPE_MOVREG;
            movNode->astNode = node;
            addToLinkedList(movNode);

            struct ARMNode* intNode = malloc(sizeof(struct ARMNode));
            char *intIns = malloc(30);
            sprintf(intIns, "ldr %s, =integer", op3);
            intNode->code = intIns;
            intNode->type = INSTYPE_LDRINT;
            intNode->op1 = op3;
            intNode->num_use = 0;
            intNode->DEF = op3;
            intNode->num_def = 1;
            intNode->astNode = node;
            addToLinkedList(intNode);

            struct ARMNode* printNode = malloc(sizeof(struct ARMNode));
            char *print= malloc(30);
            sprintf(print, "bl printf");
            printNode->code = print;
            printNode->num_use = 2;
            printNode->USE[0] = op2;
            printNode->USE[1] = op3;
            printNode->num_def = 0;
            printNode->type = INSTYPE_PRINT;
            printNode->astNode = node;
            addToLinkedList(printNode);
        
            if (node->node_type == NODETYPE_PRINTLN) {
                struct ARMNode* printLnNode = malloc(sizeof(struct ARMNode));
                char *op1 = malloc(10);
                sprintf(op1, "r0");
                char *printLn= malloc(30);
                sprintf(printLn, "ldr %s, =newline", op1);
                printLnNode->code = printLn;
                printLnNode->num_use = 0;
                printLnNode->num_def = 1;
                printLnNode->DEF = op1;
                printLnNode->type = INSTYPE_LDRINT;
                printLnNode->astNode = node;
                addToLinkedList(printLnNode);

                struct ARMNode* printLnNode2 = malloc(sizeof(struct ARMNode));
                char *printLn2= malloc(30);
                sprintf(printLn2, "bl printf");
                printLnNode2->code = printLn2;
                printLnNode2->num_use = 1;
                printLnNode2->USE[0] = op1;
                printLnNode2->num_def = 0;
                printLnNode2->type = INSTYPE_LDRINT;
                printLnNode2->astNode = node;
                addToLinkedList(printLnNode2);
            }

        }
    } else if (node->children[0]->children[0]->node_type == NODETYPE_LEFTVAL   
                && node->children[0]->children[0]->num_children == 1) { // One dimensional Array
        struct ASTNode* leftValNode = node->children[0]->children[0];
        char *ID = leftValNode->data.value.string_value;
        enum DataType arrayType = findTypeInTable(ID, node->table);
        int arrayIndexOffset = leftValNode->asmNode->regIndex;   // needs to * 4, its just index
        struct ARMNode* asmNode = malloc(sizeof(struct ARMNode));
        char *temp = malloc(200);
        if (arrayType == DATATYPE_STR) {
            if (node->node_type == NODETYPE_PRINT) {
                sprintf(temp, "ldr r0, [SP, #%d]\n  ldr r1, [r0, #0]\n  ldr r0, =string\n  bl printf", arrayIndexOffset * 4);
            } else {
                sprintf(temp, "ldr r0, [SP, #%d]\n  ldr r1, [r0, #0]\n  ldr r0, =string\n  bl printf\n  ldr r0, =newline\n  bl printf", arrayIndexOffset * 4);
            }
        } else if (arrayType == DATATYPE_INT) {
            if (node->node_type == NODETYPE_PRINT) {
                sprintf(temp, "ldr r0, [SP, #%d]\n  ldr r1, [r0, #0]\n  ldr r0, =integer\n  bl printf", arrayIndexOffset * 4);
            } else {
                sprintf(temp, "ldr r0, [SP, #%d]\n  ldr r1, [r0, #0]\n  ldr r0, =integer\n  bl printf\n  ldr r0, =newline\n  bl printf", arrayIndexOffset * 4);
            }
        }
        
        asmNode->code = temp;
        asmNode->type = INSTYPE_PRINT;
        asmNode->regIndex = 9999; //no regIndex
        asmNode->num_use = 0;
        asmNode->num_def = 0;
        node->asmNode = asmNode;
        asmNode->astNode = node;
        addToLinkedList(asmNode);
    }
}

void handleParseInt(struct ASTNode* node) { // exp -> expMethod -> ParseInt; ParseInt -> KW_PARSEINT (Exp)
    struct ARMNode* asmNode = malloc(sizeof(struct ARMNode));
    if (node->children[0]->node_type == NODETYPE_EXP) {
        if (node->children[0]->children[0]->node_type == NODETYPE_LEFTVAL) {
            struct ASTNode* leftValNode = node->children[0]->children[0];
            if (leftValNode->num_children == 1) { // one dimensional array
                int expIndex = leftValNode->asmNode->regIndex;
                char *op1 = malloc(10);
                sprintf(op1, "$t%d", expIndex);
                char *op2 = malloc(10);
                sprintf(op2, "r0");
                char *temp = malloc(20);
                sprintf(temp, "ldr %s, [%s, #0]", op2, op1);
                struct ARMNode* ldrNode = malloc(sizeof(struct ARMNode));
                ldrNode->num_use = 1;
                ldrNode->USE[0] = op1;
                ldrNode->num_def = 1;
                ldrNode->DEF = op2;

                ldrNode->code = temp;
                ldrNode->type = INSTYPE_ARRLDR;
                ldrNode->op1 = op2;
                ldrNode->op2 = op1;
                ldrNode->astNode = node;
                addToLinkedList(ldrNode);
            }
        }
    }

    char *op2 = malloc(10);
    sprintf(op2, "r0");
    struct ARMNode* atoiNode = malloc(sizeof(struct ARMNode));
    char *atoiChar = malloc(15);
    sprintf(atoiChar, "bl atoi");
    atoiNode->num_use = 1;
    atoiNode->USE[0] = op2;
    atoiNode->num_def = 0;
    atoiNode->code = atoiChar;
    atoiNode->type = INSTYPE_PARSEINT;
    atoiNode->regIndex = node->offsetTable->regCount;
    node->offsetTable->regCount++;
    node->asmNode = atoiNode;
    atoiNode->astNode = node;
    addToLinkedList(atoiNode);
}

void outputASM(int arc, char *argv[]) {
    optimize();
    if (arc > 1) {
        int indexOfDot = 0;
        // get filename
        for (int i =0; i < strlen(argv[1]); i++) {
            if (argv[1][i] == '.') {
                indexOfDot = i;
            }
        }
        char* filename = malloc(indexOfDot + 3);
        strncpy(filename, argv[1], indexOfDot);
        // done getting fileName
        filename[indexOfDot] = '.';
        filename[indexOfDot + 1] = 's';
        filename[indexOfDot + 2 ] = '\0';
        FILE *fp;
        fp = fopen(filename, "w");

        fprintf(fp, ".section .data\n");
        for (int i = 0; i < staticIntNum; i++) {
            fprintf(fp, "%s: .word %d\n", staticIntName[i], staticIntList[i]);
        }
        for (int i = 0; i < staticStringNum; i++) { // for static String
            fprintf(fp, "%s: .asciz %s\n", staticStringName[i], staticStringList[i]);
        }
        for (int i = 0; i < stringCounter; i++) { // for all other String
            fprintf(fp, "msg%d: .asciz %s\n", i, stringList[i]);
        }

        fprintf(fp, ".section .text\n");
        fprintf(fp, "integer: .asciz \"%%d\"\n");
        fprintf(fp, "string: .asciz \"%%s\"\n");
        fprintf(fp, "newline: .asciz \"\\n\"\n");
        fprintf(fp, "\n");
        fprintf(fp, ".global main\n");
        fprintf(fp, ".balign 4\n");

        struct ARMNode* temp = insRoot;
        while (temp != NULL) {
            char *code = temp->code;
            if (temp->type == INSTYPE_MAIN || temp->type == INSTYPE_METHOD || temp->type == INSTYPE_LABEL) {
                fprintf(fp, "%s\n", code);
            } else if (temp->type == INSTYPE_LAST) {
                fprintf(fp, "  %s\n  add SP, SP, #%d\n  pop {pc}\n", code, offsetTableList[1]->regCount * 4);
            } else {
                fprintf(fp, "  %s\n", code);
            }
            temp = temp->nextNode;
        }
        fclose(fp);
        fp = NULL;
    }
}


void optimize() {
    linkBranch();
    orderIns();
    calculateLiveRange();
    printLiveRange();
    buildInterferenceGraph();
    colorGraph();
    // printInterferenceGraph();
    reshape();
    insertCalleeSaved();
}

void reshape() {
    struct ARMNode* temp = insRoot;
    while (temp != NULL) {
        struct OffsetTable* offsetTable = temp->astNode->offsetTable;
        if (temp->type == INSTYPE_MOV) {
            char *op1 = temp->op1;
            char *op2 = temp->op2;
            int reg1 = lookUpColor(offsetTable, op1);
            int reg2 = lookUpColor(offsetTable, op2);
            char *realCode = malloc(20);
            sprintf(realCode, "mov r%d, r%d", reg1, reg2);
            temp->code = realCode;
        } else if (temp->type == INSTYPE_CMPINT) {
            char *op1 = temp->op1;
            int reg1 = lookUpColor(offsetTable, op1);
            char *realCode = malloc(20);
            sprintf(realCode, "cmp r%d, %s", reg1, temp->op2);
            temp->code = realCode;
        } else if (temp->type == INSTYPE_CALLERSAVED) {
            char *op2  = temp->op2;
            int reg2 = lookUpColor(offsetTable, op2);
            char *realCode = malloc(20);
            sprintf(realCode, "mov %s, r%d", temp->op1, reg2);
            temp->code = realCode;
        } else if (temp->type == INSTYPE_MOVVAL) {
            char *op1 = temp->op1;
            char *op2 = temp->op2;
            int reg1 = lookUpColor(offsetTable, op1);
            char *realCode = malloc(20);
            sprintf(realCode, "mov r%d, %s", reg1, op2);
            temp->code = realCode;
        } else if (temp->type == INSTYPE_MVN) {
            char *op1 = temp->op1;
            int reg1 = lookUpColor(offsetTable, op1);
            char *realCode = malloc(20);
            sprintf(realCode, "mvn r%d, r%d", reg1, reg1);
            temp->code = realCode;
        } else if (temp->type == INSTYPE_ANDVAL) {
            char *op1 = temp->op1;
            char *op3 = temp->op3;
            int reg1 = lookUpColor(offsetTable, op1);
            char *realCode = malloc(20);
            sprintf(realCode, "and r%d, r%d, %s", reg1, reg1, op3);
            temp->code = realCode;
        } else if (temp->type == INSTYPE_LSL) {
            char *op1 = temp->op1;
            char *op2 = temp->op2;
            char *op3 = temp->op3;
            int reg1 = lookUpColor(offsetTable, op1);
            int reg2 = lookUpColor(offsetTable, op2);
            char *realCode = malloc(20);
            sprintf(realCode, "lsl r%d, r%d, %s", reg1, reg2, op3);
            temp->code = realCode;
        } else if (temp->type == INSTYPE_LDR) {
            char *op1 = temp->op1;
            int reg1 = lookUpColor(offsetTable, op1);
            char *realCode = malloc(20);
            sprintf(realCode, "ldr r%d, [SP, #4]", reg1);
            temp->code = realCode;
        } else if (temp->type == INSTYPE_ADD || temp->type == INSTYPE_SUB || temp->type == INSTYPE_MUL) {
            char *op1 = temp->op1;
            char *op2 = temp->op2;
            char *op3 = temp->op3;
            int reg1 = lookUpColor(offsetTable, op1);
            int reg2 = lookUpColor(offsetTable, op2);
            int reg3 = lookUpColor(offsetTable, op3);
            char *realCode = malloc(30);
            if (temp->type == INSTYPE_ADD) {
                sprintf(realCode, "add r%d, r%d, r%d", reg1, reg2, reg3);
            } else if (temp->type == INSTYPE_SUB) {
                sprintf(realCode, "sub r%d, r%d, r%d", reg1, reg2, reg3);
            } else if (temp->type == INSTYPE_MUL) {
                sprintf(realCode, "mul r%d, r%d, r%d", reg1, reg2, reg3);
            }
            temp->code = realCode;
        } else if (temp->type == INSTYPE_AND || temp->type == INSTYPE_ORR) {
            char *op1 = temp->op1;
            char *op2 = temp->op2;
            char *op3 = temp->op3;
            int reg1 = lookUpColor(offsetTable, op1);
            int reg2 = lookUpColor(offsetTable, op2);
            int reg3 = lookUpColor(offsetTable, op3);
            char *realCode = malloc(30);
            if (temp->type == INSTYPE_AND) {
                sprintf(realCode, "and r%d, r%d, r%d", reg1, reg2, reg3);
            } else if (temp->type == INSTYPE_ORR) {
                sprintf(realCode, "orr r%d, r%d, r%d", reg1, reg2, reg3);
            }
            temp->code = realCode;
        } else if (temp->type == INSTYPE_CMP) {
            char *op1 = temp->op1;
            char *op2 = temp->op2;
            int reg1 = lookUpColor(offsetTable, op1);
            int reg2 = lookUpColor(offsetTable, op2);
            char *realCode = malloc(20);
            sprintf(realCode, "cmp r%d, r%d", reg1, reg2);
            temp->code = realCode;
        } else if (temp->type == INSTYPE_BMOV) {
            char *operator = temp->op1;
            char *op2 = temp->op2;
            char *op3 = temp->op3;
            int reg2 = lookUpColor(offsetTable, op2);
            char *realCode = malloc(20);
            sprintf(realCode, "%s r%d, %s", operator, reg2, op3);
            temp->code = realCode;
        } else if (temp->type == INSTYPE_STRMETHOD) {
            char *op1 = temp->op1;
            char *op2 = temp->op2;
            int reg1 = lookUpColor(offsetTable, op1);
            char *realCode = malloc(20);
            sprintf(realCode, "mov r%d, %s", reg1, op2);
            temp->code = realCode;
        } else if (temp->type == INSTYPE_MOVREG) {
            char *op1 = temp->op1;
            char *op2 = temp->op2;
            int reg2 = lookUpColor(offsetTable, op2);
            char *realCode = malloc(20);
            sprintf(realCode, "mov %s, r%d", op1, reg2);
            temp->code = realCode;
        } else if (temp->type == INSTYPE_ARRLDR) {
            char *op1 = temp->op1;
            char *op2 = temp->op2;
            int reg2 = lookUpColor(offsetTable, op2);
            char *realCode = malloc(20);
            sprintf(realCode, "ldr %s, [r%d, #0]", op1, reg2);
            temp->code = realCode;
        } else if (temp->type == INSTYPE_MOVR0) {
            char *op1 = temp->op1;
            int reg1 = lookUpColor(offsetTable, op1);
            char *realCode = malloc(20);
            sprintf(realCode, "mov r%d, r0", reg1);
            temp->code = realCode;
        } else if (temp->type == INSTYPE_MOVR0BACK) {
            char *op2 = temp->op2;
            int reg2 = lookUpColor(offsetTable, op2);
            char *realCode = malloc(20);
            sprintf(realCode, "mov r0, r%d", reg2);
            temp->code = realCode;
        }
        temp = temp->nextNode;
    }
}

void insertCalleeSaved() {
    if (num_offsetTable == 1) {
        return;
    }
    int regList[10];
    int numRegUsed = 0; 
    for (int i = 0; i < offsetTableList[1]->num_graphNode; i++) {
        if (offsetTableList[1]->graph[i]->assignedReg >= offsetTableList[1]->numArgs) {
            bool flag = false;
            for (int j = 0; j < numRegUsed; j++) {
                if (regList[j] == offsetTableList[1]->graph[i]->assignedReg) {
                    flag = true;
                    break;
                }
            }
            if (!flag && offsetTableList[1]->graph[i]->assignedReg != 0) {
                regList[numRegUsed] = offsetTableList[1]->graph[i]->assignedReg;
                numRegUsed++;
            }
        }
    }

    if (numRegUsed == 0) {
        struct ARMNode* temp = insRoot;
        while (temp != NULL) {
            if (temp->type == INSTYPE_MOVR0BACK) {
                temp->type = INSTYPE_LAST;
            }
            temp = temp->nextNode;
        }
    }

    struct ARMNode* temp = insRoot;
    while (temp != NULL) {
        if (temp->type == INSTYPE_METHOD) {
            for (int i = 0; i < numRegUsed; i++) {
                struct ARMNode* strNode = malloc(sizeof(struct ARMNode*));
                char *str = malloc(40);
                sprintf(str, "str r%d, [SP, #%d]", regList[i], i *4);
                insertLDRAfterNode(temp, strNode);
                strNode->code = str;
                strNode->type = INSTYPE_STR;
            }
        }
        if (temp->type == INSTYPE_MOVR0BACK) {
             for (int i = 0; i < numRegUsed; i++) {
                struct ARMNode* ldrNode = malloc(sizeof(struct ARMNode*));
                char *ldr = malloc(30);
                sprintf(ldr, "ldr r%d, [SP, #%d]", regList[i], i *4);
                insertLDRAfterNode(temp, ldrNode);
                if (i == 0) {
                    ldrNode->type = INSTYPE_LAST; 
                } else {
                    ldrNode->type = INSTYPE_LDR;
                }
                ldrNode->code = ldr;
            }
        }
        temp = temp->nextNode;
    }
}

//



// helper functions

// write me a function: traverse the linked list from InsRoot, and if the node is in INSTYPE_BRANCH, find its corresponding
// node with type INSTYPE_LABEL, and let the next node of the INSTYPE_LABEL has predesessor to the INSTYPE_BRANCH node
// and let the INSTYPE_BRANCH node has successor to the INSTYPE_LABEL node
void linkBranch() {
    struct ARMNode* temp = insRoot;
    while (temp != NULL) {
        if (temp->type == INSTYPE_BRANCH) {
            struct ARMNode* labelNode = insRoot;
            while (labelNode != NULL) {
                if (labelNode->type == INSTYPE_LABEL) {
                    if (strcmp(labelNode->label, temp->label) == 0) {
                        struct ARMNode* nextTolabel = labelNode->nextNode;
                        struct ARMNode* prevToBranch = temp->prevNode;
                        nextTolabel->predecessors[nextTolabel->num_predecessors] = prevToBranch;
                        nextTolabel->num_predecessors++;
                        prevToBranch->successors[prevToBranch->num_successors] = nextTolabel;
                        prevToBranch->num_successors++;
                        break;
                    }
                }
                labelNode = labelNode->nextNode;
            }
        }
        temp = temp->nextNode;
    }
}

void orderIns() {
    int count = 0;
    struct ARMNode *temp = insRoot;
    while (temp != NULL) {
        temp->order = count;
        count++;
        temp = temp->nextNode;
    }
}

// write me a function to calculate the live in and live out set for each node
void calculateLiveRange() {
    struct ARMStackNode* stack = NULL;
    pushStack(&stack, insTail);
    bool non_stop = true;
    while(non_stop) {
        struct ARMNode* current = popStack(&stack);
        if (current == NULL) break;
        if (current->type == INSTYPE_METHODTAIL) {
            pushStack(&stack, current->prevNode);
            continue;
        }
        for (int i = 0; i < current->num_successors; i++) {
            insertLvinToLvout(current, current->successors[i]);
        }
        int beforeLvin = current->num_lvin;
        buildLvin(current);
        int afterLvin = current->num_lvin;
        if (current->type == INSTYPE_MAIN) {
            non_stop = false;
        } else if (current->type == INSTYPE_METHOD) {
            pushStack(&stack, current->prevNode);
        } else if (current->num_predecessors == 2) {
            if (current->predecessors[0]->order > current->predecessors[1]->order) {
                pushStack(&stack, current->predecessors[1]);
                pushStack(&stack, current->predecessors[0]);
            } else {
                pushStack(&stack, current->predecessors[0]);
                pushStack(&stack, current->predecessors[1]);
            }
        } else if (afterLvin == beforeLvin && beforeLvin != 0) {
            continue;
        } else  {
            pushStack(&stack, current->predecessors[0]);
        }
    }
}

void insertLvinToLvout(struct ARMNode *current, struct ARMNode *successor) {
    for (int i = 0; i < successor->num_lvin; i++) {
        for (int j = 0; j < current->num_lvout; j++) {
            if (strcmp(successor->LVIN[i], current->LVOUT[j]) == 0) {
                break;
            } else if (j == current->num_lvout - 1) {
                current->LVOUT[current->num_lvout] = successor->LVIN[i];
                current->num_lvout++;
            }
        }
        if (current->num_lvout == 0) {
            current->LVOUT[current->num_lvout] = successor->LVIN[i];
            current->num_lvout++;
        }
    }
}

void buildLvin(struct ARMNode* node) {
    // (U USE)
    for (int i = 0; i < node->num_use; i++) {
        bool isExist = false;
        for (int j = 0; j < node->num_lvin; j++) {
            if (strcmp(node->USE[i], node->LVIN[j]) == 0) {
                isExist = true;
                break;
            }
        }
        if (!isExist) {
            node->LVIN[node->num_lvin] = node->USE[i];
            node->num_lvin++;
        }
    }
    // ( U (LVOUT - DEF) )
    if (node->num_def == 1) {
        for (int i = 0; i < node->num_lvout; i++) {
            if (strcmp(node->DEF, node->LVOUT[i]) != 0) {
                bool isExist = false;
                for (int j = 0; j < node->num_lvin; j++) {
                    if (strcmp(node->LVOUT[i], node->LVIN[j]) == 0) {
                        isExist = true;
                        break;
                    }
                }
                if (!isExist) {
                    node->LVIN[node->num_lvin] = node->LVOUT[i];
                    node->num_lvin++;
                }
            }
        }
    } else {
        for (int i = 0; i < node->num_lvout; i++) {  
            bool isExist = false;
            for (int j = 0; j < node->num_lvin; j++) {
                if (strcmp(node->LVOUT[i], node->LVIN[j]) == 0) {
                    isExist = true;
                    break;
                }
            }
            if (!isExist) {
                node->LVIN[node->num_lvin] = node->LVOUT[i];
                node->num_lvin++;
            }
        }
    }
}

// write me a function to print every node's live in and live out set and its use and def set
void printLiveRange() {
    struct ARMNode* temp = insRoot;
    while (temp != NULL) {
        printf("node: %s\n", temp->code);
        printf("  use: ");
        for (int i = 0; i < temp->num_use; i++) {
            printf("%s ", temp->USE[i]);
        }
        printf("\n");
        printf("  def: ");
        for (int i = 0; i < temp->num_def; i++) {
            printf("%s ", temp->DEF);
        }
        printf("\n");
        printf("  live in: ");
        for (int i = 0; i < temp->num_lvin; i++) {
            printf("%s ", temp->LVIN[i]);
        }
        printf("\n");
        printf("  live out: ");
        for (int i = 0; i < temp->num_lvout; i++) {
            printf("%s ", temp->LVOUT[i]);
        }
        printf("\n");
        temp = temp->nextNode;
    }
}

void buildInterferenceGraph() {
    struct ARMNode* temp = insRoot;
    while (temp != NULL) {
        for (int i = 0; i < temp->num_lvout; i++) {
            createNode(temp, temp->LVOUT[i]);
        }
        for (int i = 0; i < temp->num_lvin; i++) {
            createNode(temp, temp->LVIN[i]);
        }
        linkInferenceGraph(temp);
        temp = temp->nextNode;
    }
    return;
}

void createNode(struct ARMNode* node, char *ID) {
    for (int i = 0; i < node->astNode->offsetTable->num_graphNode; i++) {
        if (strcmp(node->astNode->offsetTable->graph[i]->ID, ID) == 0) {
            return;
        }
    }
    struct GraphNode* newNode = malloc(sizeof(struct GraphNode));
    newNode->ID = ID;
    newNode->degree = 0;
    if (node->prevNode == NULL) {
        newNode->returnVal = false;
    } else if (node->prevNode->type == INSTYPE_METHODCALL || node->prevNode->type == INSTYPE_PARSEINT) {
        newNode->returnVal = true;
    } else {
        newNode->returnVal = false;
    }
    newNode->colored = false;
    node->astNode->offsetTable->graph[node->astNode->offsetTable->num_graphNode] = newNode;
    node->astNode->offsetTable->num_graphNode++;
}

void linkInferenceGraph(struct ARMNode* node) {
    for (int i = 0; i < node->num_lvin; i++) {
        char *symbolic = node->LVIN[i];
        for (int j = 0; j < node->astNode->offsetTable->num_graphNode; j++) {
            if (strcmp(symbolic, node->astNode->offsetTable->graph[j]->ID) == 0) {
                struct GraphNode *currentNode = node->astNode->offsetTable->graph[j];
                for (int k = 0; k < node->num_lvin; k++) {
                    if (k == i) continue;
                    char *symbolic2 = node->LVIN[k];
                    for (int l = 0; l < node->astNode->offsetTable->num_graphNode; l++) {
                        if (strcmp(symbolic2, node->astNode->offsetTable->graph[l]->ID) == 0) {
                            struct GraphNode *currentNode2 = node->astNode->offsetTable->graph[l];
                            bool isExist = false;
                            for (int m = 0; m < currentNode2->degree; m++) {
                                if (strcmp(currentNode2->neighbors[m]->ID, symbolic) == 0) {
                                    isExist = true;
                                    break;
                                }
                            }
                            if (!isExist) {
                                currentNode2->neighbors[currentNode2->degree] = currentNode;
                                currentNode2->degree++;
                            }
                        }
                    }
                }
            }
        }
    }
    for (int i = 0; i < node->num_lvout; i++) {
        char *symbolic = node->LVOUT[i];
        for (int j = 0; j < node->astNode->offsetTable->num_graphNode; j++) {
            if (strcmp(symbolic, node->astNode->offsetTable->graph[j]->ID) == 0) {
                struct GraphNode *currentNode = node->astNode->offsetTable->graph[j];
                for (int k = 0; k < node->num_lvout; k++) {
                    if (k == i) continue;
                    char *symbolic2 = node->LVOUT[k];
                    for (int l = 0; l < node->astNode->offsetTable->num_graphNode; l++) {
                        if (strcmp(symbolic2, node->astNode->offsetTable->graph[l]->ID) == 0) {
                            struct GraphNode *currentNode2 = node->astNode->offsetTable->graph[l];
                            bool isExist = false;
                            for (int m = 0; m < currentNode2->degree; m++) {
                                if (strcmp(currentNode2->neighbors[m]->ID, symbolic) == 0) {
                                    isExist = true;
                                    break;
                                }
                            }
                            if (!isExist) {
                                currentNode2->neighbors[currentNode2->degree] = currentNode;
                                currentNode2->degree++;
                            }
                        }
                    }
                }
            }
        }
    }
}

void printInterferenceGraph() {
    for (int j = 0; j < num_offsetTable; j++) {
        struct OffsetTable* offsetTable = offsetTableList[j];
        for (int i = 0; i < offsetTable->num_graphNode; i++) {
            printf("node: %s\n", offsetTable->graph[i]->ID);
            printf("  degree: %d\n", offsetTable->graph[i]->degree);
            printf("  neighbors: ");
            for (int j = 0; j < offsetTable->graph[i]->degree; j++) {
                printf("%s ", offsetTable->graph[i]->neighbors[j]->ID);
            }
            printf("\n");
        }
        printf("end of %d table\n", j);
        printf("\n");
    }
}

void colorGraph() {
    storeOffsetTable();
    sortGraph();
    for (int i =0; i < num_offsetTable; i++) {
        for (int j = 0; j < offsetTableList[i]->num_graphNode; j++) {
            char *ID = offsetTableList[i]->graph[j]->ID;
            if (strcmp(ID, "r0") == 0) {
                offsetTableList[i]->graph[j]->assignedReg = 0;
                offsetTableList[i]->graph[j]->colored = true;
            } else if (strcmp(ID, "r1") == 0) {
                offsetTableList[i]->graph[j]->assignedReg = 1;
                offsetTableList[i]->graph[j]->colored = true;
            } else if (strcmp(ID, "r2") == 0) {
                offsetTableList[i]->graph[j]->assignedReg = 2;
                offsetTableList[i]->graph[j]->colored = true;
            } else {
                offsetTableList[i]->graph[j]->assignedReg = -1;
            }   
        }
    }
    for (int i = 1; i < num_offsetTable; i++) {
        struct OffsetTable* offsetTable = offsetTableList[i];
        char *argList[offsetTable->numArgs];
        for (int k = 0; k < offsetTable->numArgs; k++) {
                char *argReg = malloc(10);
                sprintf(argReg, "$t%d", k);
                argList[k] = argReg;
        }
        for (int j = 0; j < offsetTableList[i]->num_graphNode; j++) {
            int numArgs = offsetTableList[i]->num_graphNode;
            char *ID = offsetTableList[i]->graph[j]->ID;
            for (int k = 0; k < offsetTable->numArgs; k++) {
                if (strcmp(ID, argList[k]) == 0) {
                    offsetTableList[i]->graph[j]->assignedReg = k;
                    offsetTableList[i]->graph[j]->colored = true;
                    break;
                }
            }    
        }
    }


    for (int i = 0; i < num_offsetTable; i++) {
        struct OffsetTable* offsetTable = offsetTableList[i];
        int color = offsetTable->numArgs - 1;
        for (int j = 0; j < offsetTable->num_graphNode; j++) {
            struct GraphNode* currentNode = offsetTable->graph[j];
            if (currentNode->returnVal == true) {
                for (int k = 4; k <= 12; k++) {
                    for (int l = 0; l < currentNode->degree; l++) {
                        if (currentNode->neighbors[l]->assignedReg == k) {
                            break;
                        } else {
                            if (l == currentNode->degree - 1 && currentNode->colored == false) {
                                currentNode->assignedReg = k;
                                currentNode->colored = true;
                                color = k;
                                break;
                            }
                        }
                    }
                }
                if (!currentNode->colored) {
                    color++;
                    currentNode->assignedReg = color;
                }
            } else {
                for (int k = 0; k <= color; k++) {
                    for (int l = 0; l < currentNode->degree; l++) {
                        if (currentNode->neighbors[l]->assignedReg == k) {
                            break;
                        } else {
                            if (l == currentNode->degree - 1 && currentNode->colored == false) {
                                currentNode->assignedReg = k;
                                currentNode->colored = true;
                                break;
                            }
                        }
                    }
                }
                if (!currentNode->colored) {
                    color++;
                    currentNode->assignedReg = color;
                }
            }
            
        }
    }
    return;
}

void storeOffsetTable() {
    struct ARMNode* temp = insRoot;
    offsetTableList[num_offsetTable] = temp->astNode->offsetTable;
    temp = temp->nextNode;
    num_offsetTable++;
    while (temp != NULL) {
        if (temp->type == INSTYPE_METHOD) {
            offsetTableList[num_offsetTable] = temp->astNode->offsetTable;
            num_offsetTable++;
        }
        temp = temp->nextNode;
    }
}

void sortGraph() {
    // for each offset table in the offset table list, sort the graph based on the degree from large to small
    for (int i = 0; i < num_offsetTable; i++) {
        struct OffsetTable* offsetTable = offsetTableList[i];
        for (int j = 0; j < offsetTable->num_graphNode; j++) {
            for (int k = j + 1; k < offsetTable->num_graphNode; k++) {
                if (offsetTable->graph[j]->degree < offsetTable->graph[k]->degree) {
                    struct GraphNode* temp = offsetTable->graph[j];
                    offsetTable->graph[j] = offsetTable->graph[k];
                    offsetTable->graph[k] = temp;
                }
            }
        }
    }
}

int lookUpColor(struct OffsetTable* offsetTable, char* ID) {
    for (int i = 0; i < offsetTable->num_graphNode; i++) {
        if (strcmp(offsetTable->graph[i]->ID, ID) == 0) {
            return offsetTable->graph[i]->assignedReg;
        }
    }
}







void modifyTable(struct TableNode* table) {
    for (int i = 0; i < table->num_symbol; i++) {
        table->currentTable[i]->regOffset = i * 4;
    }
}


void opInArg(struct ASTNode* node, struct FormalNode* formal) {
    if (node == NULL) return;
    if ((node->node_type == NODETYPE_OPERATION  && node->parentNode->node_type == NODETYPE_EXPLIST)
        || (node->node_type == NODETYPE_EXPTYPE  && node->parentNode->node_type == NODETYPE_EXPTAIL)
        || (node->node_type == NODETYPE_EXPTYPE &&  node->parentNode->node_type == NODETYPE_EXPLIST)
        || (node->node_type == NODETYPE_OPERATION  && node->parentNode->node_type == NODETYPE_EXPTAIL)
        || (node->node_type == NODETYPE_LEFTVAL && node->parentNode->parentNode->node_type == NODETYPE_EXPLIST)
        || (node->node_type == NODETYPE_LEFTVAL && node->parentNode->parentNode->node_type == NODETYPE_EXPTAIL)) {
        formal->regList[formal->count] = node->asmNode->regIndex;
        formal->count++;
    } else if (node->node_type == NODETYPE_LEFTVAL) {
        if (node->parentNode->parentNode->node_type == NODETYPE_EXPLIST || node->parentNode->parentNode->node_type == NODETYPE_EXPTAIL) {
            formal->regList[formal->count] = node->asmNode->regIndex;
            formal->count++;
        }
    }
    for (int i = 0; i < node->num_children; i++) {
        opInArg(node->children[i], formal);
    }
}

bool arrayAsIndex(struct ASTNode* node) { // check if the case ID [ LeftVal ], leftVal as an array index s children
    node = node->children[0];
    while (node != NULL) {
        if (node->node_type == NODETYPE_LEFTVAL) {
            return true;
        }
        node = node->children[0];
    }
    return false;
}

void insertAfterNode(struct ARMNode* node1, struct ARMNode* node2) { //inserting node2 after node1
    struct ARMNode* next = node1->nextNode;
    node1->nextNode = node2;
    node1->successors[node1->num_successors] = node2;
    node1->num_successors++;

    node2->prevNode = node1;
    node2->predecessors[node2->num_predecessors] = node1;
    node2->num_predecessors++;
    if (next != NULL) {
        next->prevNode = node2;
        next->predecessors[next->num_predecessors] = node2;
        next->num_predecessors++;
        node2->nextNode = next;
        node2->successors[node2->num_successors] = next;
        node2->num_successors++;
    } else {
        insTail = node2;
    }
}

void insertLDRAfterNode(struct ARMNode* node1, struct ARMNode* node2) { //inserting node2 after node1
    struct ARMNode* next = node1->nextNode;
    node1->nextNode = node2;
    node2->prevNode = node1;
    if (next != NULL) {
        next->prevNode = node2;
        node2->nextNode = next;
    } else {
        insTail = node2;
    }
}




bool isArg(struct ASTNode* node) {
    // while (node != NULL) {
    //     if (node->node_type == NODETYPE_EXPLIST || node->node_type == NODETYPE_EXPHEAD) {
    //         return true;
    //     }
    //     node = node->parentNode;
    // }
    if (node->parentNode->node_type == NODETYPE_EXPTAIL) {
        if (node->parentNode->parentNode->parentNode->node_type == NODETYPE_EXPHEAD) {
            return true;
        }
    }
    if (node->parentNode->parentNode != NULL) {
        if (node->parentNode->parentNode->node_type == NODETYPE_EXPHEAD) {
            return true;
        }
    }
    return false;
}


int findStaticString(char *ID) {
    for (int i = 0; i < staticStringNum; i++) {
        if (strcmp(ID, staticStringName[i]) == 0) {
            return i;
        } 
    }
    return 9999;
}

int findStaticInt(char *ID) {
    for (int i = 0; i < staticIntNum; i++) {
        if (strcmp(ID, staticIntName[i]) == 0) {
            return i;
        } 
    }
    return 9999;
}

int findOpReg(struct ASTNode* node) {
    while(node != NULL) {
        if (node->node_type == NODETYPE_EXPTYPE || node->node_type == NODETYPE_OPERATION ||
             node->node_type == NODETYPE_LEFTVAL) {
            return node->asmNode->regIndex;
        }
        if (node->node_type == NODETYPE_EXPMETHOD) {
            return node->children[0]->asmNode->regIndex;
        }
        node = node->children[0];
    }
}

bool isStaticVar(struct ASTNode* node) { // check if the node is outside mainMethod
    bool result = false;
    while(node->node_type != NODETYPE_PROGRAM) {
        if (node->node_type == NODETYPE_STATICVARDECLLIST) {
            result = true;
        }
        node = node->parentNode;
    }
    return result;
}

enum DataType findTypeForMoreVar(struct ASTNode* node) {
    while (node != NULL) {
        if (node->node_type == NODETYPE_VARDECL) {
            return findType(node);
        }
        node = node->parentNode;
    }
}

int findStringInList(char *string) {
    for (int i = 0; i < stringCounter; i++) {
        if (strcmp(string, stringList[i]) == 0) {
            return i;
        }
    }
    return 9999; // not in static
}

int findNumInStaticList(char *ID) {
    for (int i = 0; i < staticIntNum; i++) {
        if (strcmp(ID, staticIntName[i]) == 0) {
            return i;
        }
    }
    return 9999; // not in static
}

void insertMethod(struct ARMNode* node) {
    struct ARMNode* temp = insTail;
    while (temp != NULL) {
        if (temp->type == INSTYPE_METHODTAIL) {
           struct ARMNode* next = temp->nextNode;
           node->nextNode = next;
           node->prevNode = temp;
           temp->nextNode = node;
           if (next != NULL) {
                next->prevNode = node;
           }
           break;
        }
        temp = temp->prevNode;
    }
}

void insertToLinkedList(struct ARMNode *node, int regIndex) {
    struct ARMNode *temp = insRoot;
    while (temp != NULL) {
        if (temp->regIndex == regIndex) {
            if (temp->type == INSTYPE_LDR || temp->type == INSTYPE_ADD 
                || temp->type == INSTYPE_MUL || temp->type == INSTYPE_SUB) {
                struct ARMNode *next = temp->nextNode;
                temp->nextNode = node;
                if (next != NULL) {
                    next->prevNode = node;
                } else {
                    insTail = node;
                }
                node->nextNode = next;
                node->prevNode = temp;
                break;
            } else {
                temp = temp->nextNode;
            } 
        } else {
            temp = temp->nextNode;
        }
    }
}

void addToLinkedList(struct ARMNode *node) {
    if (insRoot == NULL) {
        insRoot = node;
        insTail = node;
    } else {
        insTail->nextNode = node;
        insTail->successors[insTail->num_successors] = node;
        insTail->num_successors++;
        node->prevNode = insTail;
        node->predecessors[insTail->num_predecessors] = insTail;
        insTail = insTail->nextNode;
    }
}

void addToInsRoot(struct ARMNode *node) {
    node->nextNode = insRoot;
    node->successors[node->num_successors] = insRoot;
    node->num_successors++;
    node->prevNode = NULL;
    insRoot->prevNode = node;
    insRoot->predecessors[insRoot->num_predecessors] = node;
    insRoot->num_predecessors++;
    insRoot = node;
}

void storeOffsetToTable(char *ID, int offset, struct TableNode* table) {
    for (int i = 0; i < table->num_symbol; i++) {
        if (strcmp(table->currentTable[i]->id, ID) == 0) {
            table->currentTable[i]->regOffset = offset;
        }
    }
}

void storeStringToTable(char *ID, int index, struct TableNode* table) {
    if (table == NULL) return;
    for (int i = 0; i < table->num_symbol; i++) {
        if (strcmp(table->currentTable[i]->id, ID) == 0) {
            table->currentTable[i]->stringIndex = index; 
        }
    }
    storeStringToTable(ID, index, table->parentNode);
}

int findOffsetInTable(char *ID, struct TableNode* table) {
    if (table == NULL) {
        return 9999; // error
    }
    for (int i = 0; i < table->num_symbol; i++) {
        if (strcmp(table->currentTable[i]->id, ID) == 0) {
            return table->currentTable[i]->regOffset;
        }
    }
    findOffsetInTable(ID, table->parentNode);
}

int findLenRegInTable(char *ID, struct TableNode* table, int dimension) {  
    if (table == NULL) {    // dimension 1 == store for first index, 2 == for second
        return 9999; //error 
    }
    for (int i = 0; i < table->num_symbol; i++) {
        if (strcmp(table->currentTable[i]->id, ID) == 0) {
            if (dimension == 1) {
                return table->currentTable[i]->firstIndexLen;
            } else if (dimension == 2) {
                return table->currentTable[i]->secondIndexLen;
            }
        }
    }
    findLenRegInTable(ID, table->parentNode, dimension);
}

int findStringIndexInTable(char *ID, struct TableNode* table) {
    for (int i = 0; i < table->num_symbol; i++) {
        if (strcmp(table->currentTable[i]->id, ID) == 0) {
            return table->currentTable[i]->stringIndex;
        }
    }
    findStringIndexInTable(ID, table->parentNode);
}

bool moreOperation(struct ASTNode* node) {
    while (node != NULL) {
        if (node->node_type == NODETYPE_OPERATION) {
            return true;
        }
        node = node->children[0];
    }
    return false;
}



bool StackEmpty(struct ARMStackNode *stack) {
    if (stack == NULL) {
        return true;
    }
    return false;
}

void pushStack(struct ARMStackNode **stack, struct ARMNode *node) {
    struct ARMStackNode *newNode = malloc(sizeof(struct ARMStackNode));
    newNode->node = node;
    newNode->nextNode = *stack;
    *stack = newNode;
}

struct ARMNode* popStack(struct ARMStackNode **stack) {
    if (StackEmpty(*stack)) {
        return NULL;
    }
    struct ARMStackNode *temp = *stack;
    *stack = (*stack)->nextNode;
    struct ARMNode *node = temp->node;
    free(temp);
    return node;
}