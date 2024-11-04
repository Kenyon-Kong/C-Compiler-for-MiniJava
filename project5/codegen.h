#ifndef CODEGEN_H
#define CODEGEN_H

#include "node.h"
#include "typecheck.h"
#include <stdlib.h>
#include <stddef.h>
enum INSTYPE { INSTYPE_LDR, INSTYPE_STR, INSTYPE_MOV, INSTYPE_ADD, INSTYPE_SUB, INSTYPE_MUL, INSTYPE_PRINT, INSTYPE_PRINTLN,
                INSTYPE_MAIN, INSTYPE_METHOD, INSTYPE_METHODTAIL, INSTYPE_METHODCALL, INSTYPE_STRMETHOD,
                INSTYPE_BRANCH, INSTYPE_LABEL, INSTYPE_ENDIF,INSTYPE_ANDVAL, INSTYPE_ARRAYSTR, INSTYPE_MOVR0, INSTYPE_MOVR0BACK, 
                INSTYPE_BOOLNOT, INSTYPE_PARSEINT, INSTYPE_FIRSTINDEX, INSTYPE_SECONDINDEX, INSTYPE_MOVREG, 
                INSTYPE_STRINGCONCAT, INSTYPE_CALLERSAVED, INSTYPE_MVN, INSTYPE_AND, INSTYPE_LAST, INSTYPE_MOVCALLER, 
                INSTYPE_ORR, INSTYPE_CMP, INSTYPE_CMPINT, INSTYPE_BMOV, INSTYPE_ARRLDR, INSTYPE_MOVVAL, INSTYPE_LSL, 
                INSTYPE_LDRINT,INSTYPE_STRSPILL, INSTYPE_LDRSPILL,  };


struct ARMNode {
    char * code;
    enum INSTYPE type; // for store opeation type
    struct ARMNode *prevNode;
    struct ARMNode *nextNode;
    struct ASTNode *astNode;
    int regIndex; // the x for $tx and it's for storing
    int isReturn;

    // for optimization
    char *label; // the label for this node, only for INSTYPE_label or INTYPE_BRANCH

    int order; // the number assigned as the order of the instruction

    struct ARMNode *predecessors[5];
    int num_predecessors;
    struct ARMNode *successors[5]; 
    int num_successors;
    char *USE[4];
    int num_use;
    char *DEF;
    int num_def;

    char *LVIN[20];
    int num_lvin;
    char *LVOUT[20];
    int num_lvout;
    
    char *op1;
    char *op2;
    char *op3;
 
};

struct FormalNode {
    int count;
    int regList[10];
};

struct GraphNode {
    char* ID;
    int assignedReg;
    struct GraphNode *neighbors[50];
    int degree;
    bool colored;
    bool returnVal;
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
void optimize(); // optimize the code


//for optimization
void linkBranch(); // formulate the predecessor and successor for branch and label
void orderIns(); // assign order number to each instruction
void calculateLiveRange(); // calculate the live range for each node
void insertLvinToLvout(struct ARMNode *current, struct ARMNode *successor); // insert lvin to lvout
void buildLvin(struct ARMNode* node); // build lvin for each node
void printLiveRange();
void buildInterferenceGraph(); // build the interference graph
void createNode(struct ARMNode* node, char *ID); // create a new node for interference graph
void linkInferenceGraph(struct ARMNode* node); // link the inference graph
void printInterferenceGraph(); // print the interference graph
void colorGraph(); // color the graph
void storeOffsetTable(); // store the offset table for each method
void sortGraph(); // sort the graph
int lookUpColor(struct OffsetTable* offsetTable, char* ID); // look up the color for a ID
void reshape(); // reshape the code
void insertCalleeSaved(); // insert callee saved register




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
void modifyTable(struct TableNode* table); // update reg number if is in new StaticMethod

void insertLDRAfterNode(struct ARMNode* node1, struct ARMNode* node2);





// stack
struct ARMStackNode {
    struct ARMNode *node;
    struct ARMStackNode *nextNode;
};

bool StackEmpty(struct ARMStackNode *stack);
void pushStack(struct ARMStackNode **stack, struct ARMNode *node);
struct ARMNode* popStack(struct ARMStackNode **stack);


#endif