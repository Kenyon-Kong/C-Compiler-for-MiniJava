#include "codegen.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

//global
struct ARMNode *insRoot; 
struct ARMNode *insTail;
// int node->table->regCount = 2; //save space for arguments
int numVarDecl = 0;
int stringCounter = 0;
char *stringList[50];
char *staticStringList[50];
char *staticStringName[50];
int staticStringNum = 0;
int staticIntList[50];
char *staticIntName[50];
int staticIntNum = 0;

int argCounter = 0;


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
    }
}

void handleMainMethod(struct ASTNode* node) {
    struct ARMNode* asmNode = malloc(sizeof(struct ARMNode)); // for main head not including how to end
    char *temp = malloc(100);
    sprintf(temp, "main:\n  push {lr}\n  sub SP, SP, #%d\n  str r0, [SP, #0]\n  str r1, [SP, #4]", node->table->regCount *4);
    asmNode->code = temp;
    asmNode->type = INSTYPE_MAIN;
    asmNode->regIndex = 9999; // indicating no Index        
    node->asmNode = asmNode;
    asmNode->astNode = node;
    addToInsRoot(asmNode);

    struct ARMNode* endNode = malloc(sizeof(struct ARMNode));
    char *temp1 = malloc(100);
    sprintf(temp1, "add SP, SP, #%d\n  pop {pc}", node->table->regCount *4);
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
        sprintf(temp, "%s:\n  push {lr}\n  sub SP, SP, #%d", ID, node->table->regCount *4);
    } else if (numArgs == 1) {
        sprintf(temp, "%s:\n  push {lr}\n  sub SP, SP, #%d\n  str r0, [SP, #0]", ID, node->table->regCount *4);
        node->table->currentTable[0]->regOffset = 0;
    } else if (numArgs == 2) {
        sprintf(temp, "%s:\n  push {lr}\n  sub SP, SP, #%d\n  str r0, [SP, #0]\n  str r1, [SP, #4]", ID, node->table->regCount *4);
        node->table->currentTable[0]->regOffset = 0;
        node->table->currentTable[1]->regOffset = 4;
    } else if (numArgs == 3) {
        sprintf(temp, "%s:\n  push {lr}\n  sub SP, SP, #%d\n  str r0, [SP, #0]\n  str r1, [SP, #4]\n  str r2, [SP, #8]", ID, node->table->regCount *4);
        node->table->currentTable[0]->regOffset = 0;
        node->table->currentTable[1]->regOffset = 4;
        node->table->currentTable[2]->regOffset = 8;
    } else if (numArgs == 4){
        sprintf(temp, "%s:\n  push {lr}\n  sub SP, SP, #%d\n  str r0, [SP, #0]\n  str r1, [SP, #4]\n  str r2, [SP, #8]\n  str r3, [SP, #12]", ID, node->table->regCount *4);
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
    char *temp1 = malloc(100);
    sprintf(temp1, "add SP, SP, #%d\n  pop {pc}", node->table->regCount *4);
    endNode->code = temp1;
    endNode->type = INSTYPE_METHODTAIL; 
    endNode->regIndex = 9999; // indicating no Index            
    // node->asmNode = asmNode; already declared
    // asmNode->astNode = node;
    addToLinkedList(endNode);
}

void handleReturn(struct ASTNode* node) {
    if (node->children[0]->node_type == NODETYPE_OPERATION) {
        struct ARMNode* asmNode = malloc(sizeof(struct ARMNode)); // for main head not including how to end
        char *temp = malloc(100);
        sprintf(temp, "mov r0, r2");
        asmNode->code = temp;
        asmNode->type = INSTYPE_RETURN;
        asmNode->regIndex = 9999; // indicating no Index        
        node->asmNode = asmNode;
        asmNode->astNode = node;
        addToLinkedList(asmNode);
    }
}

void handleMethodCall(struct ASTNode* node) {
    char *methodName = node->data.value.string_value; 
    struct ARMNode* asmNode = malloc(sizeof(struct ARMNode));
    char *temp = malloc(100);
    sprintf(temp, "bl %s", methodName);
    asmNode->code = temp;
    asmNode->type = INSTYPE_METHODCALL;
    asmNode->regIndex = 9999; // indicating no Index        
    node->asmNode = asmNode;
    asmNode->astNode = node;
    addToLinkedList(asmNode);
}

void handleExpType(struct ASTNode* node) {
    if (node->parentNode->node_type != NODETYPE_LEFTVAL) {
        if (node->data.type == DATATYPE_INT  && !isStaticVar(node)) {  // Int literal not static
            int trueVal = node->data.value.int_value;
            struct ARMNode* asmNode = malloc(sizeof(struct ARMNode));
            // memset(asmNode, 0, sizeof(struct ARMNode));
            char *temp = malloc(100);
            sprintf(temp, "ldr $t%d, =%d", node->table->regCount, trueVal);
            asmNode->regIndex = node->table->regCount;
            node->table->regCount++;
            asmNode->code = temp;
            asmNode->type = INSTYPE_LDR;
            
            node->asmNode = asmNode;
            asmNode->astNode = node;
            addToLinkedList(asmNode);
        } else if (node->data.type == DATATYPE_STR && !isStaticVar(node)) {
            char *string = node->data.value.string_value;
            stringList[stringCounter] = string;
            stringCounter++;
        }
    }   
}

void handleLeftVal(struct ASTNode* node) {
    if (!isStaticVar(node)) {
        enum DataType type = findExpType(node);
        if (node->num_children == 0 && type != DATATYPE_STR) {
                char *ID = node->data.value.string_value; // local variable
                struct ARMNode* asmNode = malloc(sizeof(struct ARMNode));
                // memset(asmNode, 0, sizeof(struct ARMNode));
                char *temp = malloc(100);
                sprintf(temp, "mov $t%d, %s", node->table->regCount, ID);
                asmNode->regIndex = node->table->regCount;
                node->table->regCount++;
                asmNode->code = temp;
                asmNode->type = INSTYPE_MOV;
            
                node->asmNode = asmNode;
                asmNode->astNode = node;
                addToLinkedList(asmNode);
            }
    }
}

void handleOperation(struct ASTNode* node) {
    char *op = node->data.value.string_value;
    char *opeartor;
    if (strcmp(op, "+") == 0) {
        opeartor = "add";
    } else if (strcmp(op, "-") == 0) {
        opeartor = "sub";
    } else if (strcmp(op, "*") == 0) {
        opeartor = "mul";
    }

    int leftReg;
    if (node->children[0]->node_type == NODETYPE_EXPMETHOD) {
        leftReg = 0;
    } else {
        leftReg = findOpReg(node->children[0]);
    }   
    int rightReg;
    if (node->children[1]->node_type == NODETYPE_EXPMETHOD) {
        rightReg = 0;
    } else {
        rightReg = findOpReg(node->children[1]);
    }
     findOpReg(node->children[1]);
    struct ARMNode* asmNode = malloc(sizeof(struct ARMNode));
    // memset(asmNode, 0, sizeof(struct ARMNode));

    char *temp = malloc(100);
    sprintf(temp, "%s $t%d, $t%d, $t%d", opeartor, node->table->regCount, leftReg, rightReg);
    asmNode->regIndex = node->table->regCount;
    node->table->regCount++;

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
}

void handleVarDecl(struct ASTNode* node) {
    if (node->children[1]->children[0]->node_type == NODETYPE_EXPMETHOD) {
        char *ID = node->data.value.string_value;
        struct ARMNode* asmNode = malloc(sizeof(struct ARMNode));
        // memset(asmNode, 0, sizeof(struct ARMNode));
        char *temp = malloc(100);
        int offset = node->table->regCount;
        sprintf(temp, "str r0, [SP, #%d]", offset * 4);
        asmNode->code = temp;
        asmNode->type = INSTYPE_STRMETHOD;
        asmNode->regIndex = node->table->regCount;
        // node->table->regCount++;
        node->asmNode = asmNode;
        asmNode->astNode = node;
        addToLinkedList(asmNode);
        storeOffsetToTable(ID, offset*4, node->table);
        return;
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
        if (type != DATATYPE_STR) {
            if (node->num_children > 1 && node->children[1]->node_type == NODETYPE_VARINIT) { // int a = 1 case
                char* ID = node->data.value.string_value;
                int rightReg = node->children[1]->children[0]->asmNode->regIndex;
                int offset = rightReg * 4;
                struct ARMNode* asmNode = malloc(sizeof(struct ARMNode));
                // memset(asmNode, 0, sizeof(struct ARMNode));
                char *temp = malloc(100);
                sprintf(temp, "str $t%d, [SP, #%d]", rightReg, offset);
                asmNode->code = temp;
                asmNode->type = INSTYPE_STR;
                asmNode->regIndex = rightReg;
                node->asmNode = asmNode;
                asmNode->astNode = node;
                insertToLinkedList(asmNode, rightReg);

                storeOffsetToTable(ID, offset, node->table);
                // node->table->regCount++;
            } else { // int a; case
                char* ID = node->data.value.string_value;
                int offset = node->table->regCount * 4;
                storeOffsetToTable(ID, offset, node->table);
                node->table->regCount++;
            }
        } else {
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

void handleMoreVar(struct ASTNode* node) {
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
        } else if (type == DATATYPE_INT) {
            if (node->num_children == 1 && node->children[0]->node_type == NODETYPE_VARINIT) { // , a = 1; 
                char* ID = node->data.value.string_value;
                int offset = (node->table->regCount - 1) * 4;
                int rightReg = node->children[0]->children[0]->asmNode->regIndex;

                struct ARMNode* asmNode = malloc(sizeof(struct ARMNode));
                // memset(asmNode, 0, sizeof(struct ARMNode));
                char *temp = malloc(100);
                sprintf(temp, "str $t%d, [SP, #%d]", rightReg, offset);
                asmNode->code = temp;
                asmNode->type = INSTYPE_STR;
                asmNode->regIndex = rightReg;
                node->asmNode = asmNode;
                asmNode->astNode = node;
                addToLinkedList(asmNode);

                storeOffsetToTable(ID, offset, node->table);
                // node->table->regCount++;
            } else if (node->num_children == 2){ // ..., a = 1;
                char* ID = node->data.value.string_value;
                int offset = (node->table->regCount - 1) * 4;
                int rightReg = node->children[1]->children[0]->asmNode->regIndex;

                struct ARMNode* asmNode = malloc(sizeof(struct ARMNode));
                // memset(asmNode, 0, sizeof(struct ARMNode));
                char *temp = malloc(100);
                sprintf(temp, "str $t%d, [SP, #%d]", rightReg, offset);
                asmNode->code = temp;
                asmNode->type = INSTYPE_STR;
                asmNode->regIndex = rightReg;
                node->asmNode = asmNode;
                asmNode->astNode = node;
                addToLinkedList(asmNode);

                storeOffsetToTable(ID, offset, node->table);
                // node->table->regCount++;
            } else { // int a; case
                char* ID = node->data.value.string_value;
                int offset = node->table->regCount * 4;
                storeOffsetToTable(ID, offset, node->table);
                node->table->regCount++;
            }
        }
        
    }   
}

void handleAssign(struct ASTNode* node) {
    char* ID = node->children[0]->data.value.string_value;
    enum DataType type = findTypeInTable(ID, node->table);
    if (type != DATATYPE_STR) {
        int rightReg = node->children[1]->asmNode->regIndex;
        int leftOffset = findOffsetInTable(ID, node->table);

        struct ARMNode* asmNode = malloc(sizeof(struct ARMNode));
        // memset(asmNode, 0, sizeof(struct ARMNode));
        char *temp = malloc(100);
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

void handlePrint(struct ASTNode* node) {
    if (node->children[0]->node_type == NODETYPE_EXPMETHOD) {
        struct ARMNode* asmNode = malloc(sizeof(struct ARMNode));
        char *temp = malloc(100);
        if (node->node_type == NODETYPE_PRINT) {
            sprintf(temp, "str r0, [SP, #%d]\n  ldr r0, [SP, #%d]\n  bl printf", node->table->regCount * 4, node->table->regCount * 4);
        } else {
            sprintf(temp, "str r0, [SP, #%d]\n  ldr r0, =integer\n  ldr r1, [SP, #%d]\n  bl printf\n  ldr r0, =newline\n  bl printf", node->table->regCount * 4, node->table->regCount * 4);
        }
        node->table->regCount++;
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
        char *temp = malloc(100);
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
                && node->children[0]->children[0]->num_children == 0) { // print(a);
        enum DataType type = findExpType(node->children[0]);
        char *ID = node->children[0]->children[0]->data.value.string_value;
        if (type == DATATYPE_STR) {
            int index = findStringIndexInTable(ID, node->table);
            struct ARMNode* asmNode = malloc(sizeof(struct ARMNode));
            int staticIndex = findStaticString(ID);
            char *temp = malloc(100);
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
            char *temp = malloc(100);
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
    } else { //print(arg[0]);
        struct ARMNode* asmNode = malloc(sizeof(struct ARMNode));
        char *temp = malloc(100);
        if (node->node_type == NODETYPE_PRINT) {
            sprintf(temp, "ldr r0, =string\n  ldr r1, [r1, #4]\n  bl printf");
        } else {
            sprintf(temp, "ldr r0, =string\n  ldr r1, [r1, #4]\n  bl printf\n  ldr r0, =newline\n  bl printf");
        }
        asmNode->code = temp;
        asmNode->type = INSTYPE_PRINT;
        asmNode->regIndex = 9999; //no regIndex
        node->asmNode = asmNode;
        asmNode->astNode = node;
        addToLinkedList(asmNode);
    }
}


bool isArg(struct ASTNode* node) {
    while (node != NULL) {
        if (node->node_type == NODETYPE_EXPLIST || node->node_type == NODETYPE_EXPHEAD) {
            return true;
        }
        node = node->parentNode;
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
                }
                node->nextNode = next;
                node->prevNode = temp;
                break;
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
    for (int i = 0; i < table->num_symbol; i++) {
        if (strcmp(table->currentTable[i]->id, ID) == 0) {
            return table->currentTable[i]->regOffset;
        }
    }
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
        if (temp->type == INSTYPE_MOV) { //change mov to ldr, for declared variable
            if (isArg(temp->astNode)) {
                char *ID = temp->astNode->data.value.string_value;
                int staticIndex = findNumInStaticList(ID);
                if (staticIndex != 9999) { // not a static num
                    char *tempCode = malloc(100);
                    sprintf(tempCode, "ldr r%d, =%s\n  ldr r%d, [r%d]", argCounter, staticIntName[staticIndex], argCounter, argCounter);
                    argCounter++;
                    temp->code = tempCode;
                } else {
                    int offset = findOffsetInTable(ID, temp->astNode->table);
                    char *tempCode = malloc(100);
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
                if (staticIndex != 9999) { // is a static int
                    char *tempCode = malloc(100);
                    sprintf(tempCode, "ldr r%d, =%s\n  ldr r%d, [r%d]", reg, staticIntName[staticIndex], reg, reg);
                    temp->code = tempCode;
                } else {
                    int offset = findOffsetInTable(ID, temp->astNode->table);
                    char *tempCode = malloc(100);
                    sprintf(tempCode, "ldr r%d, [SP, #%d]", reg, offset);
                    temp->code = tempCode;
                }
            }
        } else if (temp->type == INSTYPE_LDR) { // change ldr to ldr, for integer literal
            if (isArg(temp->astNode)) {
                char *constLiteral = malloc(10);
                strncpy(constLiteral, temp->code + 10, strlen(temp->code) - 10);
                char *tempCode = malloc(100);
                sprintf(tempCode, "ldr r%d, =%s", argCounter, constLiteral);
                argCounter++;
                temp->code = tempCode;
            } else {
                int reg;
                if (temp->astNode->parentNode->children[0] == temp->astNode) { // left-child
                    reg = 0;
                } else {
                    reg = 1;
                }
                // get #constant literal
                char *constLiteral = malloc(10);
                strncpy(constLiteral, temp->code + 10, strlen(temp->code) - 10);
                char *tempCode = malloc(100);
                if (reg == 0 && moreOperation(temp->astNode->parentNode->children[1])) {
                    sprintf(tempCode, "ldr r%d, =%s\n  str r0, [SP, #%d]", reg, constLiteral, temp->regIndex *4);
                } else if (temp->nextNode->type == INSTYPE_METHODCALL) {
                    sprintf(tempCode, "ldr r%d, =%s\n  str r0, [SP, #%d]", reg, constLiteral, temp->regIndex *4);
                } else {
                    sprintf(tempCode, "ldr r%d, =%s", reg, constLiteral);
                }
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
            char *leftReg_string = malloc(10);
            strncpy(leftReg_string, temp->code + firstComma + 4, secondComma - (firstComma + 4));
            char *rightReg_string = malloc(10);
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
            char *tempCode = malloc(100);
            if (temp->prevNode->type != INSTYPE_LDR && temp->prevNode->type != INSTYPE_METHODCALL && temp->prevNode->type != INSTYPE_MOV) {
                sprintf(tempCode, "ldr r0, [SP, #%d]\n  ldr r1, [SP, #%d]\n  %s r2, r0, r1\n  str r2, [SP, #%d]",
                        leftOffset, rightOffset, op, (temp->regIndex) * 4);
            } else if ((temp->prevNode->type == INSTYPE_LDR || temp->prevNode->prevNode->type == INSTYPE_MOV) 
                        && temp->prevNode->type != INSTYPE_METHODCALL && 
                        (temp->prevNode->prevNode->type != INSTYPE_LDR && temp->prevNode->prevNode->type != INSTYPE_MOV)) {
                int reg;
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
            char *offset_string = malloc(10);
            strncpy(offset_string, code + firstHash + 1, (strlen(code)-1) - (firstHash+1));
            
            char *tempCode = malloc(100);
            if (temp->prevNode->type != INSTYPE_LDR) {
                sprintf(tempCode, "ldr r0, [SP, #%d]\n  str r0, [SP, #%s]", txOffset, offset_string);
            } else {
                char reg_string = temp->prevNode->code[5];
                sprintf(tempCode, "str r%c, [SP, #%s]", reg_string, offset_string);
            }
            temp->code = tempCode;
        } if (temp->type == INSTYPE_METHODCALL) {
            argCounter = 0;
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
            if (temp->type == INSTYPE_MAIN || temp->type == INSTYPE_METHOD) {
                fprintf(fp, "%s\n", code);
            } else {
                fprintf(fp, "  %s\n", code);
            }
            temp = temp->nextNode;
        }
        fclose(fp);
        fp = NULL;
    }
}
