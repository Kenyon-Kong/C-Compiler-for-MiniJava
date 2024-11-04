#ifndef CODEGEN_H
#define CODEGEN_H

#include "node.h"
#include "typecheck.h"

enum INSTYPE { INSTYPE_LDR, INSTYPE_STR, INSTYPE_MOV, INSTYPE_ADD, INSTYPE_SUB, INSTYPE_MUL, INSTYPE_PRINT,
                INSTYPE_MAIN, INSTYPE_METHOD, INSTYPE_METHODTAIL, INSTYPE_RETURN, INSTYPE_METHODCALL, INSTYPE_STRMETHOD};

struct ARMNode {
    char * code;
    enum INSTYPE type;
    struct ARMNode *prevNode;
    struct ARMNode *nextNode;
    struct ASTNode *astNode;
    int regIndex; // the x for $tx
};

void addToLinkedList(struct ARMNode *node); // add new insNode to the tail of linked list
void thirdTraversal(struct ASTNode* node, bool isMethod); // main function with two rounds


void handleMainMethod(struct ASTNode* node);
void handleStaticMethod(struct ASTNode* node);
void handleReturn(struct ASTNode* node);
void handleMethodCall(struct ASTNode* node);
void handleExpType(struct ASTNode* node);
void handleLeftVal(struct ASTNode* node);
void handleOperation(struct ASTNode* node);
void handleVarDecl(struct ASTNode* node);
void handleMoreVar(struct ASTNode* node);
void handleAssign(struct ASTNode* node);
void handlePrint(struct ASTNode* node);

void storeOffsetToTable(char *ID, int offset, struct TableNode* table);
int findOpReg(struct ASTNode* node);
void storeStringToTable(char *ID, int index, struct TableNode* table);
int findOffsetInTable(char *ID, struct TableNode* table);
bool isStaticVar(struct ASTNode* node);
int findStaticString(char *ID);
int findStaticInt(char *ID);
int findStringInList(char *string);
int findStringIndexInTable(char *ID, struct TableNode* table);
void insertToLinkedList(struct ARMNode *node, int regIndex); // insert str to correct location
void outputASM(int arc, char *argv[]); // print to file

void addToInsRoot(struct ARMNode *node); // add main to InsRoot
void insertMethod(struct ARMNode* node); // insert method to the latest method tail
bool isArg(struct ASTNode* node); // determine if the currect node is a argument
int findNumInStaticList(char *ID); // find num in static List
enum DataType findTypeForMoreVar(struct ASTNode* node); // find the Type for moreVar
#endif