#ifndef CODEGEN_H
#define CODEGEN_H

#include "node.h"
#include "typecheck.h"

enum INSTYPE { INSTYPE_LDR, INSTYPE_STR, INSTYPE_MOV, INSTYPE_ADD, INSTYPE_SUB, INSTYPE_MUL, INSTYPE_PRINT,
                INSTYPE_MAIN, INSTYPE_METHOD, INSTYPE_METHODTAIL, INSTYPE_RETURN, INSTYPE_METHODCALL, INSTYPE_STRMETHOD,
                INSTYPE_BOOLOP, INSTYPE_IFHEAD, INSTYPE_ELSEHEAD, INSTYPE_ENDIF, INSTYPE_WHILEHEAD, INSTYPE_LOOPBODYHEAD, 
                INSTYPE_LOOPBODYEND, INSTYPE_NEWARRAY, INSTYPE_ARRAYDECL, INSTYPE_ARRAYLEFTVAL, INSTYPE_ARRAYSTR,
                INSTYPE_BOOLNOT, INSTYPE_PARSEINT, INSTYPE_FIRSTINDEX, INSTYPE_SECONDINDEX, INSTYPE_LENGTH, 
                INSTYPE_LENGTHDECL, INSTYPE_TWO_DIMENSION, INSTYPE_STRINGCONCAT, };

struct ARMNode {
    char * code;
    enum INSTYPE type;
    struct ARMNode *prevNode;
    struct ARMNode *nextNode;
    struct ASTNode *astNode;
    int regIndex; // the x for $tx
    int isReturn;
};

struct FormalNode {
    int count;
    int regList[10];
};

void thirdTraversal(struct ASTNode* node, bool isMethod); // main function with two rounds

// handler functions
void handleMainMethod(struct ASTNode* node);
void handleStaticMethod(struct ASTNode* node);
void handleReturn(struct ASTNode* node);
void handleIf(struct ASTNode* node);
void handleWhile(struct ASTNode* node);
void handleMethodCall(struct ASTNode* node);
void handleExpType(struct ASTNode* node);
void handleBoolNot(struct ASTNode* node);
void handleSigns(struct ASTNode* node);
void handleLeftVal(struct ASTNode* node);
void handleOperation(struct ASTNode* node);
void handleVarDecl(struct ASTNode* node);
void handleMoreVar(struct ASTNode* node);
void handleAssign(struct ASTNode* node);
void handlePrint(struct ASTNode* node);
void handleNewArray(struct ASTNode* node);
void handleParseInt(struct ASTNode* node);
void handleLength(struct ASTNode* node);



// helper funtions
void addToLinkedList(struct ARMNode *node); // add new insNode to the tail of linked list
void storeOffsetToTable(char *ID, int offset, struct TableNode* table);
int findOpReg(struct ASTNode* node);
void storeStringToTable(char *ID, int index, struct TableNode* table);
int findOffsetInTable(char *ID, struct TableNode* table); //find the offset (already *4) of a leftVal in symbol table
bool isStaticVar(struct ASTNode* node);
int findStaticString(char *ID);
int findStaticInt(char *ID);
int findStringInList(char *string); //find string in string array and return its index
int findStringIndexInTable(char *ID, struct TableNode* table);
void insertToLinkedList(struct ARMNode *node, int regIndex); // insert str to correct location
void outputASM(int arc, char *argv[]); // print to file

void addToInsRoot(struct ARMNode *node); // add main to InsRoot
void insertMethod(struct ARMNode* node); // insert method to the latest method tail
bool isArg(struct ASTNode* node); // determine if the currect node is a argument
int findNumInStaticList(char *ID); // find num in static List
enum DataType findTypeForMoreVar(struct ASTNode* node); // find the Type for moreVar
void insertAfterNode(struct ARMNode* node1, struct ARMNode* node2); // insert node2 after node1 in linked list
void updateAllChildrenTable(struct TableNode* table, int regCount); // propagate the regCount to all children table
bool arrayAsIndex(struct ASTNode* node); // check if the case ID [ LeftVal ], leftVal as an array index 
// void storeLenRegInTable(char *ID, struct TableNode* table, int offset, int dimension); // store length reg for array in symbol table
int findLenRegInTable(char *ID, struct TableNode* table, int dimension); // finds length infor in symbol table
void opInArg(struct ASTNode* node, struct FormalNode* formal); // determine if the methodCall needs to load r0 first
void modifyTable(struct TableNode* table);
#endif