#ifndef NODE_H
#define NODE_H
#define MAX_NUM_CHILDREN 3

#include <stdbool.h>

enum DataType { DATATYPE_UNDEFINED, DATATYPE_STR, DATATYPE_INT, DATATYPE_BOOLEAN };

// Returns the name of the given data type.
static inline const char *type_string(enum DataType t) {
    static const char *names[] = {"Undefined", "String", "Integer", "Boolean"};
    return names[t % 4];
}

struct SemanticData {
    enum DataType type;
    union value_t {
        char* string_value;
        int int_value;
        bool boolean_value;
    } value;
};


enum NodeType {
    NODETYPE_PROGRAM,
    NODETYPE_MAINCLASS,
    NODETYPE_MAINMETHOD,
    NODETYPE_STATICVARDECLLIST,
    NODETYPE_STATICMETHODLIST,
    NODETYPE_STMTLIST,
    NODETYPE_VARDECL,
    NODETYPE_VARINIT,
    NODETYPE_MOREVAR,

    NODETYPE_STATICVARDECL,
    NODETYPE_STATICMETHOD,

    NODETYPE_FORMALHEAD,
    NODETYPE_FORMALLIST,
    NODETYPE_FORMALTAIL,

    NODETYPE_PRIMETYPE,

    NODETYPE_STMT,
    NODETYPE_PRINT,
    NODETYPE_PRINTLN,
    NODETYPE_IF,
    NODETYPE_WHILE,
    NODETYPE_PARSEINT,
    NODETYPE_ASSIGN,
    NODETYPE_RETURN,

    NODETYPE_TYPE,
    NODETYPE_ARRAY,
    NODETYPE_METHODCALL,
    NODETYPE_EXPHEAD,

    NODETYPE_EXP,
    NODETYPE_EXPTYPE,
    NODETYPE_OPERATION,
    NODETYPE_ONLYBOOL, 
    NODETYPE_ONLYINT, 
    NODETYPE_LENGTH,
    NODETYPE_NEWARRAY,
    NODETYPE_INDEX,
    NODETYPE_EXPLIST,
    NODETYPE_EXPTAIL,
    NODETYPE_LEFTVAL,
    NODETYPE_EXPMETHOD,
};

struct ASTNode {
    struct ASTNode* children[MAX_NUM_CHILDREN];
    int num_children;
    int lineNum;
    struct ASTNode* parentNode;
    enum NodeType node_type;
    struct SemanticData data;
    bool newScope;

    struct TableNode *table;
    struct ARMNode *asmNode;
    struct OffsetTable *offsetTable;
};

struct TableNode{
    struct TableNode* parentNode;
    struct SymbolTableEntry* currentTable[50];
    int num_symbol;
    struct TableNode* children[10];
    int num_children;
};

struct OffsetTable{
    int regCount;
};

// Creates a new node with 0 children on the heap using `malloc()`.
struct ASTNode* new_node(enum NodeType t, int lineNum);
// Adds the given children to the parent node. Returns -1 if the capacity is full.
int add_child(struct ASTNode* parent, struct ASTNode* child);

// Sets the data of the node to the given value and the corresponding type.

void set_string_value(struct ASTNode* node, char* s);
void set_int_value(struct ASTNode* node, int i);
void set_boolean_value(struct ASTNode* node, bool b);
void traverseAST(struct ASTNode* node);
void printEnum(struct ASTNode* node);
#endif
