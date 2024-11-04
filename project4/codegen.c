#include "codegen.h"
#include <stdlib.h>
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
int twoDArrayCounter = 0;

int hardCodingCounter = 0;
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
    } else if (node->node_type == NODETYPE_ONLYINT) { // +(positive) and - (negative)
        handleSigns(node);
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
    } else if (node->node_type == NODETYPE_NEWARRAY) {
        handleNewArray(node);
    } else if (node->node_type == NODETYPE_PARSEINT) {
        handleParseInt(node);
    } else if (node->node_type == NODETYPE_LENGTH) {
        handleLength(node);
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
    addToLinkedList(endNode);
}

void handleStaticMethod(struct ASTNode* node) {
    argCounter = 0;
    char *ID = node->data.value.string_value;
    struct ARMNode* asmNode = malloc(sizeof(struct ARMNode)); // for method head not including how to end
    char *temp = malloc(200);
    int numArgs = findNumArgs(ID);
    if (numArgs == 0) {
        sprintf(temp, "%s:\n  push {lr}\n  sub SP, SP, #%d", ID, node->offsetTable->regCount *4);
    } else if (numArgs == 1) {
        sprintf(temp, "%s:\n  push {lr}\n  sub SP, SP, #%d\n  str r0, [SP, #0]", ID, node->offsetTable->regCount *4);
        node->table->currentTable[0]->regOffset = 0;
    } else if (numArgs == 2) {
        sprintf(temp, "%s:\n  push {lr}\n  sub SP, SP, #%d\n  str r0, [SP, #0]\n  str r1, [SP, #4]", ID, node->offsetTable->regCount *4);
        node->table->currentTable[0]->regOffset = 0;
        node->table->currentTable[1]->regOffset = 4;
    } else if (numArgs == 3) {
        sprintf(temp, "%s:\n  push {lr}\n  sub SP, SP, #%d\n  str r0, [SP, #0]\n  str r1, [SP, #4]\n  str r2, [SP, #8]", ID, node->offsetTable->regCount *4);
        node->table->currentTable[0]->regOffset = 0;
        node->table->currentTable[1]->regOffset = 4;
        node->table->currentTable[2]->regOffset = 8;
    } else if (numArgs == 4){
        sprintf(temp, "%s:\n  push {lr}\n  sub SP, SP, #%d\n  str r0, [SP, #0]\n  str r1, [SP, #4]\n  str r2, [SP, #8]\n  str r3, [SP, #12]", ID, node->offsetTable->regCount *4);
        node->table->currentTable[0]->regOffset = 0;
        node->table->currentTable[1]->regOffset = 4;
        node->table->currentTable[2]->regOffset = 8;
        node->table->currentTable[3]->regOffset = 16;
    }
    asmNode->code = temp;
    asmNode->type = INSTYPE_METHOD;
    asmNode->regIndex = 9999; // indicating no Index        
    node->asmNode = asmNode;
    asmNode->astNode = node;
    insertMethod(asmNode);

    struct ARMNode* endNode = malloc(sizeof(struct ARMNode));
    char *temp1 = malloc(50);
    endNode->code = temp1;
    endNode->type = INSTYPE_METHODTAIL; 
    endNode->regIndex = 9999; // indicating no Index            
    // node->asmNode = asmNode; already declared
    // asmNode->astNode = node;
    addToLinkedList(endNode);
}

void handleReturn(struct ASTNode* node) {
    if (node->children[0]->node_type == NODETYPE_EXPTYPE) {
        if (node->children[0]->data.type != DATATYPE_STR) {
            node->children[0]->asmNode->isReturn = 9999; // hard code
        }
    }
    if (node->children[0]->node_type == NODETYPE_EXP) {
        if (node->children[0]->children[0]->node_type == NODETYPE_LEFTVAL) {
            node->children[0]->children[0]->asmNode->isReturn = 9999;
        } 
    }
    if (node->children[0]->node_type == NODETYPE_EXPMETHOD) {
         node->children[0]->children[0]->asmNode->isReturn = 9999;
    }
    if (node->children[0]->node_type == NODETYPE_OPERATION) {
        struct ARMNode* asmNode = malloc(sizeof(struct ARMNode)); // for main head not including how to end
        char *temp = malloc(200);
        sprintf(temp, "mov r0, r2");
        asmNode->code = temp;
        asmNode->type = INSTYPE_RETURN;
        asmNode->isReturn = 9999;
        asmNode->regIndex = 9999; // indicating no Index        
        node->asmNode = asmNode;
        asmNode->astNode = node;
        addToLinkedList(asmNode);
    } else if (node->children[0]->node_type == NODETYPE_EXPTYPE && node->children[0]->data.type == DATATYPE_STR) {
        struct ARMNode* asmNode = malloc(sizeof(struct ARMNode)); // for main head not including how to end
        char *temp = malloc(200);
        char *ID = node->children[0]->data.value.string_value;
        int msgIndex = findStringInList(ID);
        sprintf(temp, "ldr r0, =msg%d", msgIndex);
        asmNode->code = temp;
        asmNode->type = INSTYPE_RETURN;
        asmNode->isReturn = 9999;
        asmNode->regIndex = 9999; // indicating no Index        
        node->asmNode = asmNode;
        asmNode->astNode = node;
        addToLinkedList(asmNode);
    }
}

void handleIf(struct ASTNode* node) { // KW_IF '(' Exp ')' Statement KW_ELSE Statement
    struct ARMNode* asmNode = malloc(sizeof(struct ARMNode));
    char *temp = malloc(200);
    int conditionIndex = 0;
    if (node->children[0]->node_type == NODETYPE_OPERATION  || node->children[0]->node_type == NODETYPE_EXPTYPE
        || node->children[0]->node_type == NODETYPE_ONLYBOOL) {
        conditionIndex = node->children[0]->asmNode->regIndex;
    } else if (node->children[0]->node_type == NODETYPE_EXPMETHOD  || node->children[0]->node_type == NODETYPE_EXP) {
        conditionIndex = node->children[0]->children[0]->asmNode->regIndex;
    }
    if (node->children[0]->node_type == NODETYPE_EXP) {
        if (node->children[0]->children[0]->node_type == NODETYPE_LEFTVAL) {
            sprintf(temp, "cmp r0, #0\n  beq _false_%d\n_true_%d:", ifCounter, ifCounter);
        }
    } else {
        sprintf(temp, "ldr r0, [SP, #%d]\n  cmp r0, #0\n  beq _false_%d\n_true_%d:", conditionIndex * 4, ifCounter, ifCounter);
    }
    asmNode->code = temp;
    asmNode->regIndex = 9999; //no index
    asmNode->type = INSTYPE_IFHEAD;
    // to do: insert labels and update the counter
    
    //to insert if head -> idea: insert right after the condition ends (between cond and then)
    struct ARMNode* conditionEnds;
     if (node->children[0]->node_type == NODETYPE_OPERATION || node->children[0]->node_type == NODETYPE_EXPTYPE
            || node->children[0]->node_type == NODETYPE_ONLYBOOL) {
        conditionEnds = node->children[0]->asmNode;
    } else if (node->children[0]->node_type == NODETYPE_EXPMETHOD || node->children[0]->node_type == NODETYPE_EXP ) {
        conditionEnds = node->children[0]->children[0]->asmNode;
    }
    insertAfterNode(conditionEnds, asmNode);

    //to insert labels between then and else -> idea: insert right after the then body ends
    struct ASTNode* thenStmt = node->children[1];
    if (thenStmt->num_children == 0) { // nothing in then body
        struct ARMNode* asmNode2 = malloc(sizeof(struct ARMNode));
        char *temp2 = malloc(50);
        sprintf(temp2, "b _endif_%d\n_false_%d:", ifCounter, ifCounter);
        asmNode2->code = temp2;
        asmNode2->regIndex = 9999; //no index
        asmNode2->type = INSTYPE_ELSEHEAD;
        insertAfterNode(asmNode, asmNode2);
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
        struct ARMNode* asmNode2 = malloc(sizeof(struct ARMNode));
        char *temp2 = malloc(50);
        sprintf(temp2, "b _endif_%d\n_false_%d:", ifCounter, ifCounter);
        asmNode2->code = temp2;
        asmNode2->regIndex = 9999; //no index
        asmNode2->type = INSTYPE_ELSEHEAD;
        insertAfterNode(thenEnds, asmNode2);
    }

    //to insert labels after else -> idea: just add to the tail of the linked list
    struct ARMNode* asmNode3 = malloc(sizeof(struct ARMNode));
    char *temp3 = malloc(50);
    sprintf(temp3, "_endif_%d:", ifCounter);
    asmNode3->code = temp3;
    asmNode3->regIndex = 9999; //no index
    asmNode3->type = INSTYPE_ENDIF;
    node->asmNode = asmNode3;
    addToLinkedList(asmNode3);

    //update the ifCounter
    ifCounter++;
}

void handleWhile(struct ASTNode* node) { // KW_WHILE '(' Exp ')' Statement
    // to insert loop title in front of condition -> idea: find the left most asmNode of EXP condition subtree
    struct ARMNode* asmNode = malloc(sizeof(struct ARMNode));
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
    char *temp = malloc(20);
    sprintf(temp, "_loop_%d:", whileCounter);
    asmNode->code = temp;
    asmNode->regIndex = 9999; // no Index
    asmNode->type = INSTYPE_WHILEHEAD;
    struct ARMNode* prevOfConditionHead = conditionHead->prevNode;
    insertAfterNode(prevOfConditionHead, asmNode);

    // to insert compare code function for condition -> idea: insert after the root of conditon
    struct ARMNode* asmNode2 = malloc(sizeof(struct ARMNode));
    char *temp2 = malloc(200);
    int conditionIndex = 0;
    if (node->children[0]->node_type == NODETYPE_OPERATION) {
        conditionIndex = node->children[0]->asmNode->regIndex;
    } else if (node->children[0]->node_type == NODETYPE_EXPMETHOD) {
        conditionIndex = node->children[0]->children[0]->asmNode->regIndex;
    }
    sprintf(temp2, "ldr r0, [SP, #%d]\n  cmp r0, #0\n  beq _endloop_%d", conditionIndex * 4, whileCounter);
    asmNode2->code = temp2;
    asmNode2->regIndex = 9999; // no Index
    asmNode2->type = INSTYPE_LOOPBODYHEAD;

    struct ARMNode* conditionEnds;
     if (node->children[0]->node_type == NODETYPE_OPERATION) {
        conditionEnds = node->children[0]->asmNode;
    } else if (node->children[0]->node_type == NODETYPE_EXPMETHOD) {
        conditionEnds = node->children[0]->children[0]->asmNode;
    }
    insertAfterNode(conditionEnds, asmNode2);

    // to insert label after loop body -> idea: just add to the tail of the current node
    struct ARMNode* asmNode3 = malloc(sizeof(struct ARMNode));
    char *temp3 = malloc(50);
    sprintf(temp3, "b _loop_%d\n_endloop_%d:", whileCounter, whileCounter);
    asmNode3->code = temp3;
    asmNode3->regIndex = 9999; //no index
    asmNode3->type = INSTYPE_LOOPBODYEND;
    node->asmNode = asmNode3;
    addToLinkedList(asmNode3);

    //update the whileCounter
    whileCounter++;
}

void handleMethodCall(struct ASTNode* node) {
    char *methodName = node->data.value.string_value; 
    struct ARMNode* asmNode = malloc(sizeof(struct ARMNode));
    char *temp = malloc(200);
    struct FormalNode* formal = malloc(sizeof(struct FormalNode));
    formal->count = 0;
    opInArg(node, formal);
    if (node->parentNode->node_type = NODETYPE_EXPMETHOD) {
        if (node->parentNode->parentNode->node_type == NODETYPE_VARINIT) {
            if (formal->count == 0) {
                sprintf(temp, "bl %s", methodName);
            } else if (formal->count == 1) {
                sprintf(temp, "ldr r0, [SP, #%d]\n  bl %s", formal->regList[0] * 4, methodName);
            } else if (formal->count == 2) {
                sprintf(temp, "ldr r0, [SP, #%d]\n  ldr r1, [SP, #%d]\n  bl %s", formal->regList[0] * 4, formal->regList[1] * 4 , methodName);
            } else if (formal->count == 3) {
                sprintf(temp, "ldr r0, [SP, #%d]\n  ldr r1, [SP, #%d]\n  ldr r2, [SP, #%d]\n  bl %s", 
                    formal->regList[0] * 4, formal->regList[1] * 4, formal->regList[2] * 4, methodName);
            }
            asmNode->regIndex = node->offsetTable->regCount;
            node->offsetTable->regCount++;
        } else {
            if (formal->count == 0) {
                sprintf(temp, "bl %s\n  str r0, [SP, #%d]", methodName, node->offsetTable->regCount * 4);
            } else if (formal->count== 1) {
                sprintf(temp, "ldr r0, [SP, #%d]\n  bl %s\n  str r0, [SP, #%d]", formal->regList[0] * 4, methodName, node->offsetTable->regCount * 4);
            } else if (formal->count == 2) {
                sprintf(temp, "ldr r0, [SP, #%d]\n  ldr r1, [SP, #%d]\n  bl %s\n  str r0, [SP, #%d]", 
                formal->regList[0] * 4, formal->regList[1] * 4, methodName, node->offsetTable->regCount * 4);
            }
            
            asmNode->regIndex = node->offsetTable->regCount;
            node->offsetTable->regCount++;
        }
    }
    
    asmNode->code = temp;
    asmNode->type = INSTYPE_METHODCALL;  
           
    node->asmNode = asmNode;
    asmNode->astNode = node;
    addToLinkedList(asmNode);
}

void handleExpType(struct ASTNode* node) { // delete index for hard coding argv[0]
    if (node->parentNode->node_type == NODETYPE_ONLYINT) {
        return; // skip for signed literal, handle it somewhere else
    }
    if (node->parentNode->num_children == 1 && node->parentNode->node_type == NODETYPE_INDEX
          && node->parentNode->parentNode->node_type == NODETYPE_NEWARRAY) {
        return; // skip for one-dimensional new array (only for array declaration)
    }
    if (node->data.type == DATATYPE_INT  && !isStaticVar(node)) {  // Int literal not static
        int trueVal = node->data.value.int_value;
        struct ARMNode* asmNode = malloc(sizeof(struct ARMNode));
        char *temp = malloc(200);
        sprintf(temp, "ldr $t%d, =%d", node->offsetTable->regCount, trueVal);
        asmNode->regIndex = node->offsetTable->regCount;
        node->offsetTable->regCount++;              
        asmNode->code = temp;
        asmNode->type = INSTYPE_LDR;
        
        node->asmNode = asmNode;
        asmNode->astNode = node;
        addToLinkedList(asmNode);
    } else if (node->data.type == DATATYPE_STR && !isStaticVar(node)) {
        char *string = node->data.value.string_value;
        
        stringList[stringCounter] = string;
        stringCounter++;
    } else if (node->data.type == DATATYPE_BOOLEAN && !isStaticVar(node)) { // bool value store as 0 or 1
        bool value = node->data.value.boolean_value;
        int trueVal = 0;
        if (value) {
            trueVal = 1;
        }
        struct ARMNode* asmNode = malloc(sizeof(struct ARMNode));
        char *temp = malloc(200);
        sprintf(temp, "ldr $t%d, =%d", node->offsetTable->regCount, trueVal);
        asmNode->regIndex = node->offsetTable->regCount;
        node->offsetTable->regCount++;
            
        asmNode->code = temp;
        asmNode->type = INSTYPE_LDR;
        
        node->asmNode = asmNode;
        asmNode->astNode = node;
        addToLinkedList(asmNode);
    }
}

void handleBoolNot(struct ASTNode* node) {
    struct ARMNode* asmNode = malloc(sizeof(struct ARMNode));
    char *temp = malloc(200);
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
    sprintf(temp, "ldr r0, [SP, #%d]\n  mvn r0, r0\n  and r0, r0, #1", expIndex * 4);
    asmNode->code = temp;
    asmNode->type = INSTYPE_BOOLNOT;
    asmNode->regIndex = expIndex;
           
    node->asmNode = asmNode;
    asmNode->astNode = node;
    addToLinkedList(asmNode);
}

void handleSigns(struct ASTNode* node) { // to do, have not done -exp
    if (node->children[0]->node_type == NODETYPE_EXPTYPE) {
        char *sign = node->data.value.string_value;
        int trueVal = node->children[0]->data.value.int_value;
        struct ARMNode* asmNode = malloc(sizeof(struct ARMNode));
        char *temp = malloc(200);
        sprintf(temp, "ldr $t%d, =%s%d", node->offsetTable->regCount, sign, trueVal);
        asmNode->regIndex = node->offsetTable->regCount;
        node->offsetTable->regCount++;
          
        asmNode->code = temp;
        asmNode->type = INSTYPE_LDR;
        
        node->asmNode = asmNode;
        asmNode->astNode = node;
        addToLinkedList(asmNode);
    }
}

void handleLeftVal(struct ASTNode* node) {
    if (node->parentNode->node_type == NODETYPE_LENGTH) {
        return; // skip for length
    }
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
            struct ARMNode* asmNode = malloc(sizeof(struct ARMNode));
            char *temp = malloc(300);
            if (arrayAsIndex(node)) {
                if (node->children[0]->node_type == NODETYPE_EXP) {
                    if (node->children[0]->children[0]->node_type == NODETYPE_LEFTVAL) { // already loaded
                        if (node->children[0]->children[0]->num_children == 1) {
                            sprintf(temp, "ldr r0, [r0, #0]\n  lsl r1, r0, #2\n  ldr r0, [SP, #%d]\n  add r0, r0, r1\n  str r0, [SP, #%d]", 
                            leftIndex, node->offsetTable->regCount * 4);
                        } else if (node->children[0]->children[0]->num_children == 0) {
                            sprintf(temp, "lsl r1, r0, #2\n  ldr r0, [SP, #%d]\n  add r0, r0, r1\n  str r0, [SP, #%d]", 
                            leftIndex, node->offsetTable->regCount * 4);
                        }
                    } else {
                        sprintf(temp, "ldr r0, [SP, #%d]\n  ldr r0, [r0, #0]\n  lsl r1, r0, #2\n  ldr r0, [SP, #%d]\n  add r0, r0, r1\n  str r0, [SP, #%d]", 
                            indexOffset * 4, leftIndex, node->offsetTable->regCount * 4);
                    }
                } 
            } else {
                if (node->children[0]->node_type == NODETYPE_EXP) {
                    if (node->children[0]->children[0]->node_type == NODETYPE_LEFTVAL) { // already loaded
                        sprintf(temp, "lsl r1, r0, #2\n  ldr r0, [SP, #%d]\n  add r0, r0, r1\n  str r0, [SP, #%d]", 
                            leftIndex, node->offsetTable->regCount * 4);
                    }
                } else {
                    sprintf(temp, "ldr r0, [SP, #%d]\n  lsl r1, r0, #2\n  ldr r0, [SP, #%d]\n  add r0, r0, r1\n  str r0, [SP, #%d]", 
                            indexOffset * 4, leftIndex, node->offsetTable->regCount * 4);
                }  
            }
            asmNode->code = temp;
            asmNode->regIndex = node->offsetTable->regCount;
            node->offsetTable->regCount++;
            asmNode->type = INSTYPE_ARRAYLEFTVAL;
            node->asmNode = asmNode;
            asmNode->astNode = node;
            addToLinkedList(asmNode);

        } else if (node->num_children == 2) { //leftVal -> ID [ EXP ] [ EXP ]
            int leftExpReg;
            if (node->children[0]->node_type == NODETYPE_OPERATION) {
                leftExpReg = node->children[0]->asmNode->regIndex;
            } else if (node->children[0]->node_type == NODETYPE_EXPTYPE) {
                leftExpReg = node->children[0]->asmNode->regIndex;
            } else if (node->children[0]->node_type == NODETYPE_EXP) {
                if (node->children[0]->children[0]->node_type == NODETYPE_LEFTVAL) {
                    char *ID = node->children[0]->children[0]->data.value.string_value;
                    leftExpReg = findOffsetInTable(ID, node->table) / 4;
                }       
            }

            int rightExpReg;
            if (node->children[1]->node_type == NODETYPE_OPERATION) {
                rightExpReg = node->children[1]->asmNode->regIndex;
            } else if (node->children[1]->node_type == NODETYPE_EXPTYPE) {
                rightExpReg = node->children[1]->asmNode->regIndex;
            } else if (node->children[1]->node_type == NODETYPE_EXP) {
                if (node->children[1]->children[0]->node_type == NODETYPE_LEFTVAL) {
                    char *ID = node->children[1]->children[0]->data.value.string_value;
                    rightExpReg = findOffsetInTable(ID, node->table) / 4;
                }       
            }
            char *ID = node->data.value.string_value;
            int arrAddress = findOffsetInTable(ID, node->table);

            char *codeforFirstIndex = malloc(150);
            sprintf(codeforFirstIndex, "ldr r0, [SP, #%d]\n  lsl r0, r0, #2\n  ldr r1, [SP, #%d]\n  add r1, r1, r0", 
                    leftExpReg * 4, arrAddress);
            char *codeForSecondIndex = malloc(150);
            sprintf(codeForSecondIndex, "ldr r0, [SP, #%d]\n  lsl r0, r0, #2\n  ldr r1, [r1, #0]\n  add r1, r1, r0\n  str r1, [SP, #%d]", 
                    rightExpReg * 4, node->offsetTable->regCount * 4);
            char *beforeStore = malloc(300);
            sprintf(beforeStore, "%s\n  %s", codeforFirstIndex, codeForSecondIndex);
            
            struct ARMNode* asmNode = malloc(sizeof(struct ARMNode));
            asmNode->regIndex = node->offsetTable->regCount;
            node->offsetTable->regCount++;
            asmNode->code = beforeStore;
            asmNode->type = INSTYPE_TWO_DIMENSION;
            node->asmNode = asmNode;
            asmNode->astNode = node;
            addToLinkedList(asmNode);

        }
    } else {
        if (!isStaticVar(node)) {
            enum DataType type = findExpType(node);
            if (node->num_children == 0 && type != DATATYPE_STR) {
                    char *ID = node->data.value.string_value; // local variable
                    struct ARMNode* asmNode = malloc(sizeof(struct ARMNode));
                    char *temp = malloc(30);
                    sprintf(temp, "mov $t%d, %s", node->offsetTable->regCount, ID);
                    asmNode->regIndex = node->offsetTable->regCount;
                    node->offsetTable->regCount++;
                    
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
        

        /////////////////////////////////////////////////////////////////////////
        bool stringConcat = false;
        int leftStringIndex;
        int rightStringIndex;
        // hard coding string concat
        if (strcmp(op, "+") == 0) {
            struct ASTNode* leftExp = node->children[0];
            struct ASTNode* rightExp = node->children[1];
            
            if (leftExp->node_type == NODETYPE_EXPTYPE) {
                if (leftExp->data.type == DATATYPE_STR) {
                    char *string = leftExp->data.value.string_value;
                    leftStringIndex = findStringInList(string);
                    stringConcat = true;
                }
            }
            if (rightExp->node_type == NODETYPE_EXPTYPE) {
                if (rightExp->data.type == DATATYPE_STR) {
                    char *string = rightExp->data.value.string_value;
                    rightStringIndex = findStringInList(string);
                    stringConcat = true;
                }
            }
            if (leftExp->node_type == NODETYPE_EXP) {
                if (leftExp->children[0]->node_type == NODETYPE_LEFTVAL) {
                    char *ID = leftExp->children[0]->data.value.string_value;
                    enum DataType type = findTypeInTable(ID, node->table);
                    if (type == DATATYPE_STR) {
                        leftStringIndex = findStringIndexInTable(ID, node->table);
                        stringConcat = true;
                    }
                }
            }
            if (rightExp->node_type == NODETYPE_EXP) {
                if (rightExp->children[0]->node_type == NODETYPE_LEFTVAL) {
                    char *ID = rightExp->children[0]->data.value.string_value;
                    rightStringIndex = findStringIndexInTable(ID, node->table);
                    enum DataType type = findTypeInTable(ID, node->table);
                    if (type == DATATYPE_STR) {
                        rightStringIndex = findStringIndexInTable(ID, node->table);
                        stringConcat = true;
                    }
                }
            }
        }

        if (stringConcat) {
            struct ARMNode* asmStringNode = malloc(sizeof(struct ARMNode));
            char *stringIns = malloc(100);
            sprintf(stringIns, "ldr r0, =msg%d\n  ldr r1, =msg%d\n  bl strcat", leftStringIndex, rightStringIndex);
            asmStringNode->code = stringIns;
            asmStringNode->type = INSTYPE_STRINGCONCAT;
            asmStringNode->astNode = node;
            node->asmNode = asmStringNode;
            addToLinkedList(asmStringNode);
            return;
        }
        // only handling string concat
        /////////////////////////////////////////////////////////////////



        int leftReg = 0;
        if (node->children[0]->node_type == NODETYPE_EXPMETHOD) {
            leftReg = node->children[0]->children[0]->asmNode->regIndex;
        } else if (node->children[0]->node_type == NODETYPE_EXP) {
            if (node->children[0]->children[0]->node_type == NODETYPE_LEFTVAL 
                && node->children[0]->children[0]->num_children == 0) {
                char *ID = node->children[0]->children[0]->data.value.string_value;
                leftReg = findOffsetInTable(ID, node->table) / 4;
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
        // memset(asmNode, 0, sizeof(struct ARMNode));

        char *temp = malloc(200);
        sprintf(temp, "%s $t%d, $t%d, $t%d", opeartor, node->offsetTable->regCount, leftReg, rightReg);
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
        } else {
            leftIndex = node->children[0]->asmNode->regIndex;
        }
        int rightIndex;
        if (node->children[1]->node_type == NODETYPE_EXP) {
            rightIndex = node->children[1]->children[0]->asmNode->regIndex;
        } else {
            rightIndex = node->children[1]->asmNode->regIndex;
        }
        char *andOr;
        if (strcmp(op, "&&") == 0) {
            andOr = "and";
        } else { // "||"
            andOr = "orr";
        }
        char *temp = malloc(5050);
        sprintf(temp, "ldr r0, [SP, #%d]\n  ldr r1, [SP, #%d]\n  %s r0, r0, r1\n  str r0, [SP, #%d]", 
                leftIndex * 4, rightIndex * 4, andOr, node->offsetTable->regCount * 4);
        asmNode->regIndex = node->offsetTable->regCount;
        node->offsetTable->regCount++;
          
        asmNode->type = INSTYPE_BOOLOP;
        asmNode->code = temp;
        node->asmNode = asmNode;
        asmNode->astNode = node;
        addToLinkedList(asmNode);
    } else { // all bool operators ">" "<" ...
        struct ARMNode* asmNode = malloc(sizeof(struct ARMNode));
        char *temp = malloc(200);
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
        sprintf(temp, "cmp r0, r1\n  mov r0, #0\n  %s r0, #1\n  str r0, [SP, #%d]", boolIns, node->offsetTable->regCount * 4);
        asmNode->regIndex = node->offsetTable->regCount;
        asmNode->type = INSTYPE_BOOLOP;
        node->offsetTable->regCount++;
          
        asmNode->code = temp;
        node->asmNode = asmNode;
        asmNode->astNode = node;
        addToLinkedList(asmNode);

        // add ldr operation if needed before compare
        if (asmNode->prevNode->type != INSTYPE_LDR && asmNode->prevNode->type != INSTYPE_MOV) {
            int leftIndex = node->children[0]->asmNode->regIndex;
            int rightIndex = node->children[1]->asmNode->regIndex;
            sprintf(temp, "ldr r0, [SP, #%d]\n  ldr r1, [SP, #%d]\n  cmp r0, r1\n  mov r0, #0\n  %s r0, #1\n  str r0, [SP, #%d]", 
                        leftIndex, rightIndex, boolIns, (node->offsetTable->regCount-1) * 4);
            asmNode->code = temp;
        } else if (asmNode->prevNode->prevNode->type != INSTYPE_LDR && asmNode->prevNode->prevNode->type != INSTYPE_MOV) {
            struct ARMNode* leftChild = node->children[0]->asmNode;
            if (node->children[0]->node_type == NODETYPE_EXP) {
                leftChild = node->children[0]->children[0]->asmNode; // if exp -> leftValue
            }
            struct ARMNode* rightChild = node->children[1]->asmNode;
            if (node->children[1]->node_type == NODETYPE_EXP) {
                rightChild = node->children[1]->children[0]->asmNode; // if exp -> leftValue
            }

            if (asmNode->prevNode == leftChild) {
                int rightIndex = node->children[1]->asmNode->regIndex;
                sprintf(temp, "ldr r1, [SP, #%d]\n  cmp r0, r1\n  mov r0, #0\n  %s r0, #1\n  str r0, [SP, #%d]", 
                        rightIndex * 4, boolIns, (node->offsetTable->regCount - 1) * 4);
            } else if (asmNode->prevNode == rightChild) {
                int leftIndex = node->children[0]->asmNode->regIndex;
                sprintf(temp, "ldr r0, [SP, #%d]\n  cmp r0, r1\n  mov r0, #0\n  %s r0, #1\n  str r0, [SP, #%d]", 
                        leftIndex * 4, boolIns, (node->offsetTable->regCount - 1) * 4);
            }
            asmNode->code = temp;
        }
    }
    
}

void handleVarDecl(struct ASTNode* node) { 
    if (node->children[1]->children[0]->node_type == NODETYPE_NEWARRAY) {
        struct ASTNode* newArrayNode = node->children[1]->children[0]; 
        char *ID = node->data.value.string_value;
        int regIndex = newArrayNode->asmNode->regIndex;
        storeOffsetToTable(ID, regIndex * 4, node->table);
        return; // only handle array
    }

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
        sprintf(temp, "str r0, [SP, #%d]", offset * 4);
        asmNode->code = temp;
        asmNode->type = INSTYPE_STRMETHOD;
        asmNode->regIndex = node->offsetTable->regCount;
        // node->offsetTable->regCount++;
        node->asmNode = asmNode;
        asmNode->astNode = node;
        addToLinkedList(asmNode);
        storeOffsetToTable(ID, offset*4, node->table);
        return; // only handle methodCall
    }

    if (node->children[1]->children[0]->node_type == NODETYPE_LENGTH) {
        char *ID = node->data.value.string_value;
        struct ARMNode* asmNode = malloc(sizeof(struct ARMNode));
        char *temp = malloc(30);
        int lengthIndex = node->children[1]->children[0]->asmNode->regIndex;
        sprintf(temp, "ldr r0, [SP, #%d]\n  str r0, [SP, #%d]", lengthIndex * 4, node->offsetTable->regCount * 4);
        asmNode->code = temp;
        asmNode->regIndex = node->offsetTable->regCount;
        asmNode->type = INSTYPE_LENGTHDECL;
        node->offsetTable->regCount++;
        node->asmNode = asmNode;
        asmNode->astNode = node;
        addToLinkedList(asmNode);
        storeOffsetToTable(ID, asmNode->regIndex * 4, node->table);
        return; //only handle length
    }

    enum DataType type = findType(node->children[0]);
    if (isStaticVar(node)) {
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
                struct ARMNode* asmNode = malloc(sizeof(struct ARMNode));
                // memset(asmNode, 0, sizeof(struct ARMNode));
                char *temp = malloc(200);
                sprintf(temp, "str $t%d, [SP, #%d]", rightReg, offset);
                asmNode->code = temp;
                asmNode->type = INSTYPE_STR;
                asmNode->regIndex = rightReg;
                node->asmNode = asmNode;
                asmNode->astNode = node;
                insertToLinkedList(asmNode, rightReg);

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
                if (node->children[0]->children[0]->node_type == NODETYPE_LENGTH) {
                    char *ID = node->data.value.string_value;
                    struct ARMNode* asmNode = malloc(sizeof(struct ARMNode));
                    char *temp = malloc(30);
                    int lengthIndex = node->children[0]->children[0]->asmNode->regIndex;
                    sprintf(temp, "ldr r0, [SP, #%d]\n  str r0, [SP, #%d]", lengthIndex * 4, node->offsetTable->regCount * 4);
                    asmNode->code = temp;
                    asmNode->regIndex = node->offsetTable->regCount;
                    asmNode->type = INSTYPE_LENGTHDECL;
                    node->offsetTable->regCount++;
                    node->asmNode = asmNode;
                    asmNode->astNode = node;
                    addToLinkedList(asmNode);
                    storeOffsetToTable(ID, asmNode->regIndex * 4, node->table);
                    return; //only handle length
                }
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

                struct ARMNode* asmNode = malloc(sizeof(struct ARMNode));
                // memset(asmNode, 0, sizeof(struct ARMNode));
                char *temp = malloc(200);
                sprintf(temp, "str $t%d, [SP, #%d]", rightReg, rightReg * 4);
                asmNode->code = temp;
                asmNode->type = INSTYPE_STR;
                asmNode->regIndex = rightReg;
                node->asmNode = asmNode;
                asmNode->astNode = node;
                addToLinkedList(asmNode);

                storeOffsetToTable(ID, rightReg * 4, node->table);
                // node->offsetTable->regCount++;
            } else if (node->num_children == 2){ // ..., a = 1;
                if (node->children[1]->children[0]->node_type == NODETYPE_LENGTH) {
                    char *ID = node->data.value.string_value;
                    struct ARMNode* asmNode = malloc(sizeof(struct ARMNode));
                    char *temp = malloc(30);
                    int lengthIndex = node->children[1]->children[0]->asmNode->regIndex;
                    sprintf(temp, "ldr r0, [SP, #%d]\n  str r0, [SP, #%d]", lengthIndex * 4, node->offsetTable->regCount * 4);
                    asmNode->code = temp;
                    asmNode->regIndex = node->offsetTable->regCount;
                    asmNode->type = INSTYPE_LENGTHDECL;
                    node->offsetTable->regCount++;
                    node->asmNode = asmNode;
                    asmNode->astNode = node;
                    addToLinkedList(asmNode);
                    storeOffsetToTable(ID, asmNode->regIndex * 4, node->table);
                    return; //only handle length
                }

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
                struct ARMNode* asmNode = malloc(sizeof(struct ARMNode));
                // memset(asmNode, 0, sizeof(struct ARMNode));
                char *temp = malloc(200);
                sprintf(temp, "str $t%d, [SP, #%d]", rightReg, rightReg*4);
                asmNode->code = temp;
                asmNode->type = INSTYPE_STR;
                asmNode->regIndex = rightReg;
                node->asmNode = asmNode;
                asmNode->astNode = node;
                addToLinkedList(asmNode);

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
    if (node->children[0]->num_children > 0) { // array assignment
        int leftIndex = node->children[0]->asmNode->regIndex;
        struct ARMNode* asmNode = malloc(sizeof(struct ARMNode));
        char *temp = malloc(100);
        if (node->children[0]->num_children == 1) {
            if (node->children[1]->node_type == NODETYPE_EXPMETHOD) {
                sprintf(temp, "ldr r1, [SP, #%d]\n  str r0, [r1, #0]", leftIndex * 4);
            } else {
                if (node->children[1]->node_type == NODETYPE_OPERATION) {
                    sprintf(temp, "ldr r0, [SP, #%d]\n  str r2, [r0, #0]", leftIndex * 4);
                } else {
                    sprintf(temp, "ldr r0, [SP, #%d]\n  str r1, [r0, #0]", leftIndex * 4);
                }
                
            }
        } else {
            sprintf(temp, "str r0, [r1, #0]");
        }
        asmNode->code = temp;
        asmNode->type = INSTYPE_ARRAYSTR;
        asmNode->regIndex = leftIndex;
        node->asmNode = asmNode;
        asmNode->astNode = node;
        addToLinkedList(asmNode);
    } else {
        char* ID = node->children[0]->data.value.string_value;
        enum DataType type = findTypeInTable(ID, node->table);
        if (type != DATATYPE_STR) {
            int rightReg = node->children[1]->asmNode->regIndex;
            int leftOffset = findOffsetInTable(ID, node->table);

            struct ARMNode* asmNode = malloc(sizeof(struct ARMNode));
            // memset(asmNode, 0, sizeof(struct ARMNode));
            char *temp = malloc(200);
            sprintf(temp, "str $t%d, [SP, #%d]", rightReg, leftOffset);
            asmNode->code = temp;
            asmNode->type = INSTYPE_STR;
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
}

void handlePrint(struct ASTNode* node) {
    if (node->children[0]->node_type == NODETYPE_OPERATION) {
        struct ARMNode* asmNode = malloc(sizeof(struct ARMNode));
        char *temp = malloc(200);
        if (node->node_type == NODETYPE_PRINT) {
            sprintf(temp, "ldr r0, =integer\n  ldr r1, [SP, #%d]\n  bl printf", node->children[0]->asmNode->regIndex * 4);
        } else {
            sprintf(temp, "ldr r0, =integer\n  ldr r1, [SP, #%d]\n  bl printf\n  ldr r0, =newline\n  bl printf", 
            node->children[0]->asmNode->regIndex * 4);
        }
        asmNode->code = temp;
        asmNode->regIndex = 9999; //no regIndex
        asmNode->type = INSTYPE_PRINT;
        node->asmNode = asmNode;
        asmNode->astNode = node;
        addToLinkedList(asmNode);
    } else if (node->children[0]->node_type == NODETYPE_EXPMETHOD) {
        struct ARMNode* asmNode = malloc(sizeof(struct ARMNode));
        char *temp = malloc(200);
        if (node->node_type == NODETYPE_PRINT) {
            sprintf(temp, "str r0, [SP, #%d]\n  ldr r0, [SP, #%d]\n  bl printf", node->offsetTable->regCount * 4, node->offsetTable->regCount * 4);
        } else {
            sprintf(temp, "str r0, [SP, #%d]\n  ldr r0, =integer\n  ldr r1, [SP, #%d]\n  bl printf\n  ldr r0, =newline\n  bl printf", node->offsetTable->regCount * 4, node->offsetTable->regCount * 4);
        }
        node->offsetTable->regCount++;
          
        asmNode->code = temp;
        asmNode->regIndex = 9999; //no regIndex
        asmNode->type = INSTYPE_PRINT;
        node->asmNode = asmNode;
        asmNode->astNode = node;
        addToLinkedList(asmNode);
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
        asmNode->type = INSTYPE_PRINT;
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
            asmNode->type = INSTYPE_PRINT;
            node->asmNode = asmNode;
            asmNode->astNode = node;
            addToLinkedList(asmNode);
        } else if (type == DATATYPE_INT) {
            int offset = findOffsetInTable(ID, node->table);
            struct ARMNode* asmNode = malloc(sizeof(struct ARMNode));
            int staticIndex = findStaticInt(ID);
            char *temp = malloc(200);
            if (node->node_type == NODETYPE_PRINT && staticIndex == 9999) { // 9999 = not static
                sprintf(temp, "ldr r0, =integer\n  bl printf");
            } else if (node->node_type == NODETYPE_PRINTLN && staticIndex == 9999) {
                sprintf(temp, "ldr r0, =integer\n  bl printf\n  ldr r0, =newline\n  bl printf");
            } else if (node->node_type == NODETYPE_PRINT && staticIndex != 9999) {
                sprintf(temp, "ldr r0, =integer\n  ldr r1, =%s\n  ldr r1, [r1]\n  bl printf", staticIntName[staticIndex]);
            } else {
                sprintf(temp, "ldr r0, =integer\n  ldr r1, =%s\n  ldr r1, [r1]\n  bl printf\n  ldr r0, =newline\n  bl printf", staticIntName[staticIndex]);
            }
            asmNode->code = temp;
            asmNode->regIndex = 9999; //no regIndex
            asmNode->type = INSTYPE_PRINT;
            node->asmNode = asmNode;
            asmNode->astNode = node;
            addToLinkedList(asmNode);
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
        node->asmNode = asmNode;
        asmNode->astNode = node;
        addToLinkedList(asmNode);
    } else if (node->children[0]->children[0]->node_type == NODETYPE_LEFTVAL   
                && node->children[0]->children[0]->num_children == 2) { // two dimensional array
        struct ARMNode* asmNode = malloc(sizeof(struct ARMNode));
        char *temp = malloc(100); 
         if (node->node_type == NODETYPE_PRINT) {
            sprintf(temp, "ldr r1, [r1, #0]\n  ldr r0, =integer\n  bl printf");
        } else {
            sprintf(temp, "ldr r1, [r1, #0]\n  ldr r0, =integer\n  bl printf\n  ldr r0, =newline\n  bl printf");
        }
        asmNode->code = temp;
        asmNode->type = INSTYPE_PRINT;
        asmNode->regIndex = 9999; //no regIndex
        node->asmNode = asmNode;
        asmNode->astNode = node;
        addToLinkedList(asmNode);
    }
}

void handleNewArray(struct ASTNode* node) { // KW_NEW PrimeType Index // Index -> [ EXP ] | [ EXP ] [ EXP ]
    if (node->children[1]->num_children == 1) { // one dimensional array
        if (node->children[1]->children[0]->node_type == NODETYPE_EXPTYPE) { // simplest new int[1]
            int numEntry = node->children[1]->children[0]->data.value.int_value;
            struct ARMNode* asmNode = malloc(sizeof(struct ARMNode));
            char *insforMalloc= malloc(5050);
            int firstIndex = node->offsetTable->regCount;
            node->offsetTable->regCount++;
            int addrForArray = node->offsetTable->regCount;
            node->offsetTable->regCount++;
            sprintf(insforMalloc, "ldr r0, =%d\n  str r0, [SP, #%d]\n  add r0, r0, #1\n  lsl r0, r0, #2\n  bl malloc\n  str r0, [SP, #%d]",
                     numEntry, firstIndex * 4, addrForArray * 4);
            
            char *insforStoreLength= malloc(200);
            sprintf(insforStoreLength, "ldr r1, [SP, #%d]\n  str r1, [r0, #0]\n  add r0, r0, #4\n  str r0, [SP, #%d]", 
                     firstIndex * 4, addrForArray * 4);
            
            char *finalIns = malloc(250);
            sprintf(finalIns, "%s\n  %s", insforMalloc, insforStoreLength);

            asmNode->code = finalIns;
            asmNode->regIndex = addrForArray;
            asmNode->type = INSTYPE_NEWARRAY;
            node->asmNode = asmNode;
            asmNode->astNode = node;
            addToLinkedList(asmNode);
        } else if (node->children[1]->children[0]->node_type == NODETYPE_EXP) {
            if (node->children[1]->children[0]->children[0]->node_type == NODETYPE_LEFTVAL
                && node->children[1]->children[0]->children[0]->num_children == 0) { // new int[i], i declared int
                char *ID = node->parentNode->parentNode->data.value.string_value;
                int regIndex = findOffsetInTable(ID, node->table);
                int addrForArray = node->offsetTable->regCount;
                node->offsetTable->regCount++;
                struct ARMNode* asmNode = malloc(sizeof(struct ARMNode));
                
                char *insforMalloc= malloc(200);
                sprintf(insforMalloc, "add r0, r0, #1\n  lsl r0, r0, #2\n  bl malloc\n  str r0, [SP, #%d]", 
                        addrForArray * 4);
                char *insforStoreLength= malloc(200);
                sprintf(insforStoreLength, "ldr r1, [SP, #%d]\n  str r1, [r0, #0]\n  add r0, r0, #4\n  str r0, [SP, #%d]", 
                        regIndex * 4, addrForArray * 4);
                
                char *finalIns = malloc(200);
                sprintf(finalIns, "%s\n  %s", insforMalloc, insforStoreLength);

                asmNode->code = finalIns;
                asmNode->regIndex = addrForArray;
                asmNode->type = INSTYPE_NEWARRAY;
                node->asmNode = asmNode;
                asmNode->astNode = node;
                addToLinkedList(asmNode);
            }
        }
    } else if (node->children[1]->num_children == 2) {  // two dimensional array
        // handle the first index
        int firstIndexReg; // stores the first index offset
        struct ARMNode* asmNode1 = malloc(sizeof(struct ARMNode));
        char *insforMalloc= malloc(5050);
        if (node->children[1]->children[0]->node_type == NODETYPE_EXPTYPE) {
            firstIndexReg = node->children[1]->children[0]->asmNode->regIndex;
        } else if (node->children[1]->children[0]->node_type == NODETYPE_EXP) {
            if (node->children[1]->children[0]->children[0]->node_type == NODETYPE_LEFTVAL) {
                firstIndexReg = node->children[1]->children[0]->children[0]->asmNode->regIndex;
            }
        }
        int addrForArray = node->offsetTable->regCount;
        node->offsetTable->regCount++;
        sprintf(insforMalloc, "ldr r0, [SP, #%d]\n  add r0, r0, #1\n  lsl r0, r0, #2\n  bl malloc\n  str r0, [SP, #%d]",
                    firstIndexReg * 4, addrForArray * 4);
        
        char *insforStoreLength= malloc(200);
        sprintf(insforStoreLength, "ldr r1, [SP, #%d]\n  ldr r0, [SP, #%d]\n  str r1, [r0, #0]\n  ldr r0, [SP, #%d]\n  add r0, r0, #4\n  str r0, [SP, #%d]", 
                    firstIndexReg * 4, addrForArray * 4,addrForArray * 4, addrForArray * 4);
        
        char *finalIns = malloc(250);
        sprintf(finalIns, "%s\n  %s", insforMalloc, insforStoreLength);

        asmNode1->code = finalIns;
        asmNode1->regIndex = addrForArray;
        asmNode1->type = INSTYPE_FIRSTINDEX;
        node->asmNode = asmNode1;
        asmNode1->astNode = node;
        addToLinkedList(asmNode1); 
        // finishing handling for the first index
        /////////////////////////////////////////
        //starting to handle second index
        char *t1 = malloc(50); // the code to store t1
        int t1_reg = node->offsetTable->regCount; // the t1 instant vairable for looping
        node->offsetTable->regCount++;
        int secondIndexReg; // the reg that stores the number of second index, access each time for cmp

        // struct ASTNode* varDeclNode = node->parentNode->parentNode; // VarDecl -> VarInit-> = Exp

        sprintf(t1, "mov r0, #0\n  str r0, [SP, #%d]", t1_reg * 4);
        if (node->children[1]->children[1]->node_type == NODETYPE_EXPTYPE) {
            secondIndexReg = node->children[1]->children[1]->asmNode->regIndex;
        } else if (node->children[1]->children[1]->node_type == NODETYPE_EXP) {
            if (node->children[1]->children[1]->children[0]->node_type == NODETYPE_LEFTVAL
                && node->children[1]->children[1]->children[0]->num_children == 0) {
                struct ASTNode* leftValNode = node->children[1]->children[1]->children[0];
                char *ID = leftValNode->data.value.string_value;
                secondIndexReg = findOffsetInTable(ID, node->table) / 4;
            }
        }

        // KW_NEW PrimeType Index // Index -> [EXP] | [EXP] [EXP]

        char *loopHead = malloc(25);
        sprintf(loopHead, "_arrayLoop_%d:", twoDArrayCounter);
        char *codeForCMP = malloc(200);
        sprintf(codeForCMP, "ldr r2, [SP, #%d]\n  ldr r0, [SP, #%d]\n  cmp r2, r0\n  beq _endArrayLoop_%d",
                t1_reg * 4, firstIndexReg * 4, twoDArrayCounter);
        char *codeForMalloc = malloc(200);
        sprintf(codeForMalloc, "ldr r0, [SP, #%d]\n  add r0, r0, #1\n  lsl r0, r0, #2\n  bl malloc", 
                secondIndexReg * 4);
        char *codeForStore2D = malloc(200);
        sprintf(codeForStore2D, "ldr r1, [SP, #%d]\n  ldr r2, [SP, #%d]\n  lsl r2, r2, #2\n  add r1, r1, r2\n  str r0, [r1, #0]", 
                addrForArray * 4, t1_reg * 4);
        char *codeForStoreLength = malloc(200);
        sprintf(codeForStoreLength, "ldr r2, [SP, #%d]\n  str r2, [r0, #0]\n  add r0, r0, #4\n  str r0, [r1, #0]", secondIndexReg * 4);
        char *loopTail = malloc(200);
        sprintf(loopTail, "ldr r2, [SP, #%d]\n  add r2, r2, #1\n  str r2, [SP, #%d]\n  b _arrayLoop_%d\n_endArrayLoop_%d:", 
                 t1_reg*4, t1_reg * 4, twoDArrayCounter, twoDArrayCounter);

        char *finalCode = malloc(600);
        sprintf(finalCode, "%s\n%s\n  %s\n  %s\n  %s\n  %s\n  %s", 
                t1, loopHead, codeForCMP, codeForMalloc, codeForStore2D, codeForStoreLength, loopTail);
        struct ARMNode* asmNode = malloc(sizeof(struct ARMNode));
        asmNode->code = finalCode;
        asmNode->type = INSTYPE_SECONDINDEX;
        asmNode->regIndex = 9999; // no reg
        addToLinkedList(asmNode); 
        twoDArrayCounter++;
    }
}

void handleParseInt(struct ASTNode* node) { // exp -> expMethod -> ParseInt; ParseInt -> KW_PARSEINT (Exp)
    struct ARMNode* asmNode = malloc(sizeof(struct ARMNode));
    char *temp = malloc(5050);
    if (node->children[0]->node_type == NODETYPE_EXPTYPE) {
        char *stringVal = node->children[0]->data.value.string_value;
        int indexInStringList = findStringInList(stringVal);
        sprintf(temp, "ldr r0, =msg%d\n  bl atoi\n  str r0, [SP, #%d]", indexInStringList, node->offsetTable->regCount * 4);
    } else if (node->children[0]->node_type == NODETYPE_EXP) {
        if (node->children[0]->children[0]->node_type == NODETYPE_LEFTVAL) {
            struct ASTNode* leftValNode = node->children[0]->children[0];
            if (leftValNode->num_children == 1) { // one dimensional array
                int expIndex = leftValNode->asmNode->regIndex;
                sprintf(temp, "ldr r0, [SP, #%d]\n  ldr r0, [r0, #0]\n  bl atoi\n  str r0, [SP, #%d]",
                         expIndex * 4, node->offsetTable->regCount * 4);
            } else if (leftValNode->num_children == 0) { //string variable
                //to-do
            }
        }
    }
    asmNode->code = temp;
    asmNode->type = INSTYPE_PARSEINT;
    asmNode->regIndex = node->offsetTable->regCount;
    node->offsetTable->regCount++;
    node->asmNode = asmNode;
    asmNode->astNode = node;
    addToLinkedList(asmNode);
}

void handleLength(struct ASTNode* node) { // EXP -> LeftValue '.' KW_LENGTH
    struct ARMNode* asmNode = malloc(sizeof(struct ARMNode));
    char *temp = malloc(200);
    struct ASTNode* leftValNode = node->children[0];
    char *ID = leftValNode->data.value.string_value;
    int arrayOffset = findOffsetInTable(ID, node->table);
    int reg;
    // determine if its the right child or left child
    if ((node->parentNode->parentNode->node_type == NODETYPE_PRINT || node->parentNode->parentNode->node_type == NODETYPE_PRINTLN)) {
        reg = 1;
    } else if (node->parentNode->parentNode->children[0] == node->parentNode) { // left-child
        reg = 0;
    } else {
        reg = 1;
    }

    if (leftValNode->num_children == 0) { // LEFTVAL -> ID | LEFTVAL -> ID [ EXP ]
        sprintf(temp, "ldr r%d, [SP, #%d]\n  ldr r%d, [r%d, #-4]\n  str r%d, [SP, #%d]",
             reg, arrayOffset, reg, reg, reg, node->offsetTable->regCount * 4);
    } else if (leftValNode->num_children == 1) {
        if (leftValNode->children[0]->node_type == NODETYPE_EXPTYPE) {
            int regNum = leftValNode->children[0]->data.value.int_value;
            sprintf(temp, "ldr r%d, [SP, #%d]\n  ldr r%d, [r%d, #%d]\n  ldr r%d, [r%d, #-4]\n  str r%d, [SP, #%d]",
             reg, arrayOffset, reg, reg, regNum * 4, reg, reg, reg, node->offsetTable->regCount * 4);
        } else if (leftValNode->children[0]->node_type == NODETYPE_EXP) {
            if (leftValNode->children[0]->children[0]->node_type == NODETYPE_LEFTVAL) {
                char *ID1 = leftValNode->children[0]->children[0]->data.value.string_value;
                int leftValReg = findOffsetInTable(ID1, node->table);
                sprintf(temp, "ldr r1, [SP, #%d]\n  ldr r1, [r1, r0]\n  ldr r1, [r1, #-4]\n  str r1, [SP, #%d]", 
                        leftValReg, node->offsetTable->regCount * 4);
            }
        }
    }
    asmNode->regIndex = node->offsetTable->regCount;
    node->offsetTable->regCount++;
    asmNode->code = temp;
    asmNode->type = INSTYPE_LENGTH;
    node->asmNode = asmNode;
    asmNode->astNode = node;
    addToLinkedList(asmNode);
}
//

// helper functions


void modifyTable(struct TableNode* table) {
    for (int i = 0; i < table->num_symbol; i++) {
        table->currentTable[i]->regOffset = i * 4;
    }
}


void opInArg(struct ASTNode* node, struct FormalNode* formal) {
    if (node == NULL) return;
    if ((node->node_type == NODETYPE_OPERATION  && node->parentNode->node_type == NODETYPE_EXPLIST)
        || (node->node_type == NODETYPE_OPERATION  && node->parentNode->node_type == NODETYPE_EXPTAIL)
        || (node->node_type == NODETYPE_EXPTYPE &&  node->parentNode->node_type == NODETYPE_EXPLIST)
        || (node->node_type == NODETYPE_OPERATION  && node->parentNode->node_type == NODETYPE_EXPTAIL)) {
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
        node->prevNode = insTail;
        insTail = insTail->nextNode;
    }
}

void addToInsRoot(struct ARMNode *node) {
    node->nextNode = insRoot;
    node->prevNode = NULL;
    insRoot->prevNode = node;
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


void reshapeASM() {
    struct ARMNode *temp = insRoot;
    while(temp != NULL) {
        if (temp->type == INSTYPE_METHODCALL) {
            argCounter = 0;
        } else if (temp->type == INSTYPE_MOV) { //change mov to ldr, for declared variable
            if (isArg(temp->astNode)) {
                char *ID = temp->astNode->data.value.string_value;
                int staticIndex = findNumInStaticList(ID);
                if (staticIndex != 9999) { // is not a static num
                    char *tempCode = malloc(200);
                    sprintf(tempCode, "ldr r%d, =%s\n  ldr r%d, [r%d]", 
                        argCounter, staticIntName[staticIndex], argCounter, argCounter);
                    argCounter++;
                    temp->code = tempCode;
                } else {
                    int offset = findOffsetInTable(ID, temp->astNode->table);
                    char *tempCode = malloc(200);
                    sprintf(tempCode, "ldr r%d, [SP, #%d]", argCounter, offset);
                    argCounter++;
                    temp->code = tempCode;
                }
                
            } else {
                char *ID = temp->astNode->data.value.string_value;
                int reg;
                if ((temp->astNode->parentNode->parentNode->node_type == NODETYPE_PRINT || temp->astNode->parentNode->parentNode->node_type == NODETYPE_PRINTLN)) {
                    reg = 1;
                } else if (temp->astNode->parentNode->parentNode->children[0] == temp->astNode->parentNode) { // left-child
                    reg = 0;
                } else {
                    reg = 1;
                }
                int staticIndex = findNumInStaticList(ID);
                if (staticIndex != 9999) { // is not a static int
                    char *tempCode = malloc(200);
                    sprintf(tempCode, "ldr r%d, =%s\n  ldr r%d, [r%d]\n  str r%d, [SP, #%d]", 
                            reg, staticIntName[staticIndex], reg, reg, reg, temp->regIndex * 4);
                    temp->code = tempCode;
                } else {
                    int offset = findOffsetInTable(ID, temp->astNode->table);
                    char *tempCode = malloc(200);
                    sprintf(tempCode, "ldr r%d, [SP, #%d]\n  str r%d, [SP, #%d]",
                             reg, offset, reg, temp->regIndex * 4);
                    temp->code = tempCode;
                }
            }
        } else if (temp->type == INSTYPE_LDR) { // change ldr to ldr, for integer literal
            if (isArg(temp->astNode)) {
                int eqlIndex = 0;
                for(int i = 0; i < strlen(temp->code); i++) {
                    if (temp->code[i] == '=') {
                        eqlIndex = i + 1;
                        break;
                    }
                }
                char *constLiteral = malloc(500);
                strncpy(constLiteral, temp->code + eqlIndex, strlen(temp->code) - eqlIndex);
                char *tempCode = malloc(200);
                sprintf(tempCode, "ldr r%d, =%s\n  str r%d, [SP, #%d]", argCounter, constLiteral, argCounter, temp->regIndex * 4);
                argCounter++;
                temp->code = tempCode;
            } else {
                int reg;
                if (temp->astNode->parentNode->children[0] == temp->astNode) { // left-child
                    reg = 0;
                } else{
                    reg = 1;
                }
                if (temp->astNode->parentNode->node_type == NODETYPE_ASSIGN) {
                    struct ASTNode* assign =temp->astNode->parentNode;
                    if (assign->children[0]->num_children == 2) {
                        reg = 0;
                    } 
                }
                // get #constant literal
                int eqlIndex = 0;
                for(int i = 0; i < strlen(temp->code); i++) {
                    if (temp->code[i] == '=') {
                        eqlIndex = i + 1;
                        break;
                    }
                }
                char *constLiteral = malloc(10);
                strncpy(constLiteral, temp->code + eqlIndex, strlen(temp->code) - eqlIndex);
                char *tempCode = malloc(200);
                // if (reg == 0 && moreOperation(temp->astNode->parentNode->children[1])) {
                //     sprintf(tempCode, "ldr r%d, =%s\n  str r0, [SP, #%d]", reg, constLiteral, temp->regIndex *4);
                // } else if (temp->nextNode->type == INSTYPE_METHODCALL || temp->nextNode->type == INSTYPE_MOV
                //             || temp->nextNode->type == INSTYPE_IFHEAD || temp->nextNode->type == INSTYPE_ARRAYLEFTVAL) {
                //     sprintf(tempCode, "ldr r%d, =%s\n  str r0, [SP, #%d]", reg, constLiteral, temp->regIndex *4);
                // } else {
                //     sprintf(tempCode, "ldr r%d, =%s", reg, constLiteral);
                // }
                sprintf(tempCode, "ldr r%d, =%s\n  str r%d, [SP, #%d]", reg, constLiteral, reg, temp->regIndex * 4);
                temp->code = tempCode;
            }
        } else if (temp->type == INSTYPE_ADD || temp->type == INSTYPE_SUB || temp->type == INSTYPE_MUL) {
            struct ASTNode *node = temp->astNode;
            int firstComma = 0;
            int secondComma = 0;
            for (int i =0; i<strlen(temp->code); i++) {
                if (firstComma != 0 && temp->code[i] == ',') {
                    secondComma = i;
                } else if (temp->code[i] == ',') {
                    firstComma = i;
                }
            }
            char *leftReg_string = malloc(500);
            strncpy(leftReg_string, temp->code + firstComma + 4, secondComma - (firstComma + 4));
            char *rightReg_string = malloc(500);
            strncpy(rightReg_string, temp->code + secondComma + 4, strlen(temp->code) - (secondComma + 4));


            int leftOffset = atoi(leftReg_string) * 4;
            int rightOffset = atoi(rightReg_string) * 4;
            char *op;
            if (temp->type == INSTYPE_ADD) {
                op = "add";
            } else if (temp->type == INSTYPE_SUB) {
                op = "sub";
            } else if (temp->type == INSTYPE_MUL) {
                op = "mul";
            }

            char *tempCode = malloc(200);
            if (temp->prevNode->type != INSTYPE_LDR && temp->prevNode->type != INSTYPE_MOV) {
                if (temp->astNode->children[0]->node_type == NODETYPE_EXP  
                    && temp->astNode->children[0]->node_type == NODETYPE_EXP) {
                    if (temp->astNode->children[0]->children[0]->node_type == NODETYPE_LEFTVAL
                        && temp->astNode->children[1]->children[0]->node_type == NODETYPE_LEFTVAL) {
                        sprintf(tempCode, "ldr r0, [SP, #%d]\n  ldr r0, [r0, #0]\n  ldr r1, [SP, #%d]\n  ldr r1, [r1, #0]\n  %s r2, r0, r1\n  str r2, [SP, #%d]",
                        leftOffset, rightOffset, op, (temp->regIndex) * 4);
                    } else if (temp->astNode->children[0]->children[0]->node_type == NODETYPE_LEFTVAL 
                        && temp->astNode->children[0]->children[0]->num_children == 1) {
                        sprintf(tempCode, "ldr r0, [SP, #%d]\n  ldr r0, [r0, #0]\n  ldr r1, [SP, #%d]\n  %s r2, r0, r1\n  str r2, [SP, #%d]",
                        leftOffset, rightOffset, op, (temp->regIndex) * 4);
                    } else if (temp->astNode->children[1]->children[0]->node_type == NODETYPE_LEFTVAL
                        && temp->astNode->children[1]->children[0]->num_children == 1) {
                        sprintf(tempCode, "ldr r0, [SP, #%d]\n  ldr r1, [SP, #%d]\n  ldr r1, [r1, #0]\n  %s r2, r0, r1\n  str r2, [SP, #%d]",
                            leftOffset, rightOffset, op, (temp->regIndex) * 4);
                    } else {
                        sprintf(tempCode, "ldr r0, [SP, #%d]\n  ldr r1, [SP, #%d]\n  %s r2, r0, r1\n  str r2, [SP, #%d]",
                            leftOffset, rightOffset, op, (temp->regIndex) * 4);
                    }
                } else {
                    sprintf(tempCode, "ldr r0, [SP, #%d]\n  ldr r1, [SP, #%d]\n  %s r2, r0, r1\n  str r2, [SP, #%d]",
                        leftOffset, rightOffset, op, (temp->regIndex) * 4);
                }
            } else if ((temp->prevNode->type == INSTYPE_LDR || temp->prevNode->prevNode->type == INSTYPE_MOV) 
                        && temp->prevNode->type != INSTYPE_METHODCALL && 
                        (temp->prevNode->prevNode->type != INSTYPE_LDR && temp->prevNode->prevNode->type != INSTYPE_MOV)) {
                if (temp->astNode->children[0]->node_type == NODETYPE_EXPTYPE) { // left-child
                    sprintf(tempCode, "ldr r1, [SP, #%d]\n  %s r2, r0, r1\n  str r2, [SP, #%d]",
                    rightOffset, op, (temp->regIndex) * 4);
                } else {
                    sprintf(tempCode, "ldr r0, [SP, #%d]\n  %s r2, r0, r1\n  str r2, [SP, #%d]",
                    leftOffset, op, (temp->regIndex) * 4);
                }
            } else if (temp->prevNode->type == INSTYPE_METHODCALL) {
                int notMethod;
                if (rightOffset != 0) {
                    notMethod = rightOffset;
                } else {
                    notMethod = leftOffset;
                }
                sprintf(tempCode, "ldr r1, [SP, #%d]\n  %s r2, r0, r1\n  str r2, [SP, #%d]",
                        notMethod, op, (temp->regIndex) * 4);
            } else {
                sprintf(tempCode, "%s r2, r0, r1\n  str r2, [SP, #%d]", op, (temp->regIndex) * 4);
            }
            temp->code = tempCode;
        } else if (temp->type == INSTYPE_STR) {
            char *code = temp->code;
            int firstComma = 0;
            int firstHash = 0;
            for(int i = 0; i < strlen(code); i++) {
                if (code[i] == ',' && firstComma == 0) {
                    firstComma = i;
                }
                if (code[i] == '#') {
                    firstHash = i;
                }
            }
            char *reg = malloc(10);
            strncpy(reg, code + 6, firstComma - 6);
            int txOffset = atoi(reg) * 4;
            char *offset_string = malloc(500);
            strncpy(offset_string, code + firstHash + 1, (strlen(code)-1) - (firstHash+1));
            
            char *tempCode = malloc(100);
            if (temp->prevNode->type != INSTYPE_LDR) {
                sprintf(tempCode, "ldr r0, [SP, #%d]\n  str r0, [SP, #%s]", txOffset, offset_string);
            } else {
                char reg_string = temp->prevNode->code[5];
                sprintf(tempCode, "str r%c, [SP, #%s]", reg_string, offset_string);
            }
            temp->code = tempCode;
        } else if (temp->type == INSTYPE_PRINT) {
            if (temp->prevNode->type == INSTYPE_STRINGCONCAT) {
                char *newIns = malloc(100);
                if (temp->astNode->node_type == NODETYPE_PRINT) {
                    sprintf(newIns, "ldr r1, =string\n  bl printf");
                } else {
                    sprintf(newIns, "ldr r1, =string\n  bl printf\n  ldr r0, =newline\n  bl printf");
                }
                temp->code = newIns;
            }
        }
        temp = temp->nextNode;
    }
}

void outputASM(int arc, char *argv[]) {
    reshapeASM();
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
            if (temp->type == INSTYPE_MAIN || temp->type == INSTYPE_METHOD || temp->type == INSTYPE_ENDIF
                || temp->type == INSTYPE_WHILEHEAD) {
                fprintf(fp, "%s\n", code);
            } else if (temp->isReturn == 9999) {
                fprintf(fp, "  %s\n  add SP, SP, #%d\n  pop {pc}\n", code, temp->astNode->offsetTable->regCount * 4);
            } else {
                fprintf(fp, "  %s\n", code);
            }
            temp = temp->nextNode;
        }
        fclose(fp);
        fp = NULL;
    }
}
