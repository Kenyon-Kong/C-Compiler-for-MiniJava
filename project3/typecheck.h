#ifndef TYPE_CHECK_H
#define TYPE_CHECK_H

#include "node.h"

struct SymbolTableEntry{
    char * id;
    enum DataType type;
    int entryNum;
    
    int regOffset;
    int stringIndex;
};

struct MethodTableEntry {
    enum DataType returnType;
    char * methodName;
    enum DataType argTypes[10];
    int num_args;
};
// Adds an entry to the symbol table.
// void add_to_symbol_table(char* id, struct SemanticData SemanticData);

// Looks for an entry in the symbol table with the given name.
// Returns NULL if there is no such entry.
// struct SymbolTableEntry * find_symbol(char* id);

// void checkProgram(struct ASTNode* program);
// void checkMain(struct ASTNode* mainClass);
// void checkStatement(struct ASTNode* statement);
void report_type_violation(); 
void create_Table(struct ASTNode* node);
bool findSymbol(char *ID, struct TableNode* table);
void addToTable(char* ID, enum DataType type, struct TableNode* table, int entryNum);
enum DataType findType(struct ASTNode* node);
void firstTraversal(struct ASTNode* node);
void traverseTable(struct TableNode* table);
void findArgs(struct ASTNode* node, struct MethodTableEntry* method);
enum DataType findTypeInTable(char *ID, struct TableNode* table);
void traverseMethod();
void modifyType(char *ID, struct TableNode* table, enum DataType type);
enum DataType findExpType(struct ASTNode* node);
void secondTraversal(struct ASTNode* node);
void insertError(struct ASTNode* node);
bool isMethod(struct ASTNode* node);
enum DataType getMethodType(struct ASTNode* node);
extern int num_errors;
extern int num_entries;

int findNumArgs(char *methodName); // used in codegen

#endif
