#include "typecheck.h"
#include "node.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int num_errors = 0;
// int num_entries = 0;
int num_methods = 0;
int errorsArray[100];
struct MethodTableEntry* method_table[50];
// struct SymbolTableEntry *symbol_table[50];
enum DataType tempType;

int compare_function(const void * num1, const void * num2) {
    if(*(int*)num1 > *(int*)num2)   return 1;
    else return -1;
}

void report_type_violation() {
    qsort(errorsArray, num_errors, sizeof(int), compare_function);
    for(int i = 0;i < num_errors; i++) {
        fprintf(stderr, "Type Violation in Line %d\n", errorsArray[i]);
    }
}

// void checkProgram(struct ASTNode * program){
//     num_entries = 0;
//     checkMain(program->children[program->num_children-1]);
// }

// void checkMain(struct ASTNode * mainClass){
//     char * nameOfClass = mainClass -> data.value.string_value;
//     checkStatement(mainClass->children[0]);
// }

// void checkStatement(struct ASTNode* statement){
//     enum NodeType statementType = statement -> node_type;
//     if (statementType == NODETYPE_PRINT || statementType == NODETYPE_PRINTLN) {
//         // All undefined types (int, boolean, String) are printable.
//         // Even if the expression is eventually undefined, no errors are
//         // reported here - undefined type is due to other type violations.
//     } else if (statementType == NODETYPE_VARDECL) {
//         struct ASTNode * varDecl = statement -> children[0];
//         enum DataType var_type = varDecl ->children[0]->data.type;
//         enum DataType expr_type = varDecl ->children[1]->data.type;
//         char * id = varDecl->data.value.string_value;
        
//         bool type_violation_exists = false;
//         // Reports an error if the variable initializer expression has a
//         // different type.
//         if (var_type == expr_type) {
//             type_violation_exists = true;
//             report_type_violation(/*TODO*/-1);
//         }
//         // Reports an error if the declared variable is declared before (i.e.,
//         // already exists in the symbol table).
//         struct SymbolTableEntry * found_entry = find_symbol(id);
//         if (found_entry != NULL) {
//             type_violation_exists = true;
//             report_type_violation(/*TODO*/-1);
            
//             // Changes the type of the variable to undefined, unless the 
//             // redefinition is the same type as the existing one.
//             if (found_entry->type != var_type) 
//                 found_entry->type = DATATYPE_UNDEFINED;
//         }
//         if (!type_violation_exists) {
//             add_to_symbol_table(id, varDecl->children[1]->data);
//         };
//     }
// }


/*
struct SymbolTableEntry * find_symbol(char* id){
    for (int i = 0; i < num_entries; ++i) {
        if(strcmp(id, symbol_table[i]->id) == 0){
            return symbol_table[i];
        }
    }
    return NULL;
}
*/

/*
void add_to_symbol_table(char* id, struct SemanticData data){
    struct SymbolTableEntry* entry = malloc(sizeof(struct SymbolTableEntry));
    entry->id =id;
    entry->type = data.type;
    symbol_table[num_entries] = entry;
    num_entries++;
}
*/

// create a new table
void create_Table(struct ASTNode* node) {
    struct TableNode* table_node = malloc(sizeof(struct TableNode));
    memset(table_node, 0, sizeof(struct TableNode));
    if (node -> node_type != NODETYPE_MAINCLASS) {
        struct TableNode* parent_table = node->parentNode->table;
    // modify the new node
        table_node->parentNode = parent_table;
    // link the new node to parent;
        parent_table->children[parent_table->num_children] = table_node;
        parent_table->num_children++;
    }
    table_node->num_symbol = 0;
    table_node->num_children = 0;
    table_node->regCount = 0;
    // add the new table to the ASTnode
    node->table = table_node;
    // printEnum(node);
}

// check redeclaration error
bool findSymbol(char *ID, struct TableNode* table) {
    if (table == NULL) {
        return false;
    }
    for (int i = 0; i < table->num_symbol; i++) {
        if (strcmp(table->currentTable[i]->id, ID) == 0) {
            return true;
        }
    }
    findSymbol(ID, table->parentNode);
}


void modifyType(char *ID, struct TableNode* table, enum DataType type) {
    if (table == NULL) return;
    for (int i = 0; i < table->num_symbol; i++) {
        if (strcmp(table->currentTable[i]->id, ID) == 0) {
            if (table->parentNode != NULL) { // Program table
                if (table->currentTable[i]->type != type) {
                    table->currentTable[i]->type = DATATYPE_UNDEFINED;
                    return;
                }    
            }
        }
    }
    modifyType(ID, table->parentNode, type);
}

// add new symbol to table
void addToTable(char* ID, enum DataType type, struct TableNode* table, int entryNum) {
    struct SymbolTableEntry* entry = malloc(sizeof(struct SymbolTableEntry));
    entry->id = ID;
    entry->type = type;
    entry->entryNum = entryNum;
    table->currentTable[table->num_symbol] = entry;
    table->num_symbol++;
    // printf("ADD TO TABLE: %s\n", ID);
}

// find type of a node
enum DataType findType(struct ASTNode* node) {
    if (node -> node_type == NODETYPE_PRIMETYPE) {
        return node->data.type;
    }
    findType(node->children[0]);
}


// }


// fill all the argument of a method
void findArgs(struct ASTNode* node, struct MethodTableEntry* method) {
    if (node == NULL) return;
    if (node->node_type == NODETYPE_FORMALTAIL || node->node_type == NODETYPE_FORMALLIST) {
        enum DataType temp = findType(node);
        // printf("%s %s\n", type_string(temp), node->data.value.string_value);
        method->argTypes[method->num_args] = temp;
        method->num_args++;
    }
    for (int i = 0; i < node->num_children; i++) {
        findArgs(node->children[i], method);
    }
}

int findNumArgs(char *methodName) {
    for (int i = 0; i < num_methods; i++) {
        if (strcmp(method_table[i]->methodName, methodName) == 0) {
            return method_table[i]->num_args;
        } 
    }
    return 5; // bad result
}

int checkArray(struct ASTNode* node) { // return the entry of Arrays, if the variable it's not array, return 0
    if (node == NULL) return 0;
    // printEnum(node);
    if (node->node_type == NODETYPE_LENGTH) {
        return 0;
    }
    if (node -> node_type == NODETYPE_ARRAY) { 
        if (node -> children[0]->node_type == NODETYPE_ARRAY) {
                return 2;
        } else {
            return 1;
        }
    }
    if (node->node_type == NODETYPE_NEWARRAY) { //also checking for index to be just integer
        if (node->children[1]->num_children > 1) { // index node
            enum DataType type1 = findExpType(node->children[1]->children[0]);
            enum DataType type2 = findExpType(node->children[1]->children[1]);
            if (type1 != type2 || type1 != DATATYPE_INT || node->children[1]->children[0]->node_type == NODETYPE_NEWARRAY
                || node->children[1]->children[1]->node_type == NODETYPE_NEWARRAY) {
                    //printf("Type violation 200\n");
                    insertError(node);
                }
            return 2;
        } else {
            enum DataType type1 = findExpType(node->children[1]->children[0]);
            if (type1 != DATATYPE_INT || node->children[1]->children[0]->node_type == NODETYPE_NEWARRAY) {
                //printf("Type violation 207\n");
                insertError(node);
            }
            return 1; 
        }
    }
    if (node->node_type == NODETYPE_LEFTVAL) {
        if (node->num_children == 1) {
            enum DataType type1 = findExpType(node->children[0]);
            if (type1 != DATATYPE_INT || node->children[0]->node_type == NODETYPE_NEWARRAY) {
                //printf("Type violation 217\n");
                insertError(node);
            }
            return 1;
        } if (node->num_children == 2) {
            enum DataType type1 = findExpType(node->children[0]);
            enum DataType type2 = findExpType(node->children[1]);
            if (type1 != DATATYPE_INT || type2 != type1 || node->children[0]->node_type == NODETYPE_NEWARRAY 
                    || node->children[1]->node_type == NODETYPE_NEWARRAY) {
                        //printf("Type violation 226\n");
                insertError(node);
            }
            return 2;
        }
        return 0;
    }
    checkArray(node->children[0]);
}


// first traversal
void firstTraversal(struct ASTNode* node) {
    if (node == NULL) return;
    if (node -> newScope == true) {
        // create a new Table
        create_Table(node);
        if (node->node_type == NODETYPE_MAINCLASS) {
            // printEnum(node->parentNode);
            node->parentNode->table = node->table;
        } else if (node ->node_type == NODETYPE_STATICMETHOD) {
            struct MethodTableEntry* entry = malloc(sizeof(struct MethodTableEntry));
            enum DataType returnType = findType(node);
            char* methodName = node->data.value.string_value;
            entry -> returnType = returnType;
            entry -> methodName = methodName;
            entry -> num_args = 0;
            findArgs(node, entry);
            method_table[num_methods] = entry;
            node->table->regCount = entry->num_args; // initialize the regCounter for each method ////
            num_methods++;
            // printf("ADD method: %s\n", methodName);
        } else if (node->node_type == NODETYPE_MAINMETHOD) {
            addToTable(node->data.value.string_value, DATATYPE_STR, node->table, 1);
            node->table->regCount = 2;
        }
    } else if (node->node_type == NODETYPE_VARDECL) {
        node->table = node->parentNode->table;
        tempType = findType(node);
        // printf("Type: %d", tempType);
        char* ID = node->data.value.string_value;
        // printf("ID: %s\n", ID);
        if (ID != NULL) {
            // printEnum(node->parentNode);
            // node->table = node->parentNode->table;
            // if (node->table == NULL) printf("NULL\n");
            if (!findSymbol(ID, node->table)) { // checking re-declare
                // add variable
                int numEntry = checkArray(node);
                addToTable(ID, tempType, node->table, numEntry);
            } else {
                //printf("Type violation 268\n");
                insertError(node);
                modifyType(ID, node->table, tempType);
            }
        }
        // } else {
        //     node->table = node->parentNode->table;
        // }
    } else if (node->node_type == NODETYPE_MOREVAR) {
        node->table = node->parentNode->table;
        char* ID = node->data.value.string_value;
        // add to table
        node -> table = node->parentNode->table;
        if (!findSymbol(ID, node->table)) { // checking re-declare
            // add variable
            addToTable(ID, tempType, node->table, 0);
            
        } else {
            // printf("%s\n", ID);
            //printf("Type violation 284\n");
            insertError(node);
            modifyType(ID, node->table, tempType);
        }
    } else if (node->node_type == NODETYPE_VARINIT) {
        node->table = node->parentNode->table;
        struct ASTNode* parent = node->parentNode;
        // printEnum(parent);
        if (parent->node_type == NODETYPE_VARDECL) {
            enum DataType leftType = findType(parent);
            int array1 = checkArray(parent);
            // printf("leftEntry: %d\n", array1);
            // printf("leftType: %s\n", type_string(leftType));
            enum DataType rightType = findExpType(node->children[0]);
            int array2 = checkArray(node);
            // printf("rightEntry: %d\n", array2);
            bool waitForSecond = isMethod(node);
            // printEnum(node->children[0]);
            // printf("rightType: %s\n ", type_string(rightType));
            if ((leftType != rightType && !waitForSecond) || (array1 != array2 && leftType == rightType)) { // check if they are the same type
                //printf("Type violation 303\n");
                insertError(node);
            }
        } else if (parent->node_type == NODETYPE_MOREVAR) {
            // node->table = node->parentNode->table;
            node->children[0]->table = node->table;
            enum DataType rightType = findExpType(node->children[0]);
            if (tempType != rightType) {
                // printf("Type violation 309\n");
                insertError(node);
            }
        }
    } else { // Outside of VarDecl
        if (node->node_type != NODETYPE_MAINCLASS && node->node_type != NODETYPE_PROGRAM) {
            node->table = node->parentNode->table;
        }
        if (node->node_type == NODETYPE_LEFTVAL) {
            // for (int i = 0; i < node->table->num_symbol; i++) {
            //     printf("%s\n", node->table->currentTable[i]->id);
            // }
            // printf("ID: %s\n", node->data.value.string_value);
            if (!findSymbol(node->data.value.string_value, node->table)) {
                //printf("Type violation 319\n");
                insertError(node);
            }

        } else if (node->node_type == NODETYPE_FORMALLIST || node->node_type == NODETYPE_FORMALTAIL) {
            enum DataType argType = findType(node);
            int argArray = checkArray(node);
            char *ID = node->data.value.string_value;
            addToTable(ID, argType, node->table, argArray);
        } else if (node->node_type == NODETYPE_PARSEINT) {
            enum DataType temp = findExpType(node->children[0]);
            if (temp != DATATYPE_STR) {
                //printf("Type violation 368\n");
                insertError(node);
            }
        }
    }
    
    for (int i = 0; i < node -> num_children; i++) {
        firstTraversal(node->children[i]);
    }
}

bool isMethod(struct ASTNode* node) { // check if the right hand side is a method
    if (node == NULL) return false;
    if (node->node_type == NODETYPE_METHODCALL) {
        return true;
    }
    for (int i = 0; i < node -> num_children; i++) {
        isMethod(node->children[i]);
    }
}

enum DataType findTypeInTable(char *ID, struct TableNode* table) { // search varaible in symbol table
    if (table == NULL) {
        return DATATYPE_UNDEFINED;
    }
    for (int i = 0; i < table->num_symbol; i++) {
        if (strcmp(table->currentTable[i]->id, ID) == 0) {
            return table->currentTable[i]->type;
        }
    }
    findTypeInTable(ID, table->parentNode);
}


enum DataType findExpType(struct ASTNode* node) { // find type of a EXP node
    node->table = node->parentNode->table;
    if (node->node_type == NODETYPE_OPERATION) {
        char* op = node->data.value.string_value;
        if (strcmp(op, "&&") == 0|| strcmp(op, "||") == 0 || strcmp(op, "!=") == 0 || strcmp(op, "==") == 0 || strcmp(op, ">") == 0 ||
         strcmp(op, "<") == 0 || strcmp(op, ">=") == 0 || strcmp(op, "<=") == 0) {
            return DATATYPE_BOOLEAN;
        }
    }
    if (node->node_type == NODETYPE_EXPTYPE) { // to do
        // printf("find ExpType: %s\n", type_string(node->data.type));
        return node->data.type;
    }
    if (node->node_type == NODETYPE_PRIMETYPE) {
        return node->data.type;
    }
    if (node->node_type == NODETYPE_METHODCALL) {
        // printf("test");
        return getMethodType(node);
    }
    if (node->node_type == NODETYPE_PARSEINT) {
        return DATATYPE_INT;
    }
    if (node->node_type == NODETYPE_LEFTVAL) {
        char *ID = node->data.value.string_value;
        return findTypeInTable(ID, node->table);
    }
    if (node->node_type == NODETYPE_ONLYBOOL) {
        return DATATYPE_BOOLEAN;
    }
    if (node->node_type == NODETYPE_ONLYINT || node->node_type == NODETYPE_LENGTH) {
        return DATATYPE_INT;
    }
    for (int i = 0; i < node->num_children; i++) {
        return findExpType(node->children[i]);
    }
}


enum DataType getMethodType(struct ASTNode* node) {
    char* methodName = node->data.value.string_value;
    for (int i = 0; i < num_methods; i++) {
        if (strcmp(method_table[i]->methodName, methodName) == 0) {
            // printf("%s\n", type_string(method_table[i]->returnType));
            int argsInTable = method_table[i]->num_args;
            int count = 0;
            if (node ->num_children > 0) { // check num of calling args
                struct ASTNode* expList = node->children[0]->children[0];
                if (expList->num_children == 1) { // checking if the args type are the same 
                    enum DataType argType = findExpType(expList->children[0]); // get the only type in arg
                    if (method_table[i]->argTypes[0] != argType) {
                        printf("Type violation 390\n");
                        insertError(node);
                    }
                    count = 1;
                } else {
                    enum DataType argType = findExpType(expList->children[0]); // get the only type in arg
                    // printf("%s\n", type_string(argType));
                    // printf("%s\n", type_string(method_table[i]->argTypes[count]));
                    if (method_table[i]->argTypes[count] != argType) {
                        // printf("Type violation 399\n"); // to do !!!!!!!!!!!!!!!!! wrong in calculator
                        insertError(node);
                    }
                    expList = expList->children[1]; // turn to the first tail
                    // printEnum(expList);
                    count++;
                    while(expList -> num_children > 1) {
                        // printEnum(expList);
                        argType = findExpType(expList->children[1]); // exp is the second child
                        // printf("%s\n", type_string(argType));
                        if (method_table[i]->argTypes[count] != argType) {
                            // printf("Type violation 410\n");
                            insertError(node);
                        }
                        count++;
                        expList = expList->children[0];
                    }
                    argType = findExpType(expList->children[0]);
                    if (method_table[i]->argTypes[count] != argType) {
                        // printf("Type violation 418\n"); // to do !!!!!!!!!!!!!!!!! wrong in calculator
                        insertError(node);
                    }
                    count++;
                } 
            }
            // printf("num of calling: %d\n", count);
            if (count != argsInTable) {
                printf("Type violation 426\n");
                insertError(node);
            }
            
            return method_table[i]->returnType;
        }
    }
    return DATATYPE_UNDEFINED;
}

void insertError(struct ASTNode* node) { // insert new error into error array
    // int lineNum = node->lineNum;
    // bool appeared = false;
    // for(int i = 0; i < num_errors; i++) {
    //     if (errorsArray[i] == lineNum) {
    //         appeared = true;
    //     }
    // }
    // if (appeared == false) {
    // }
    errorsArray[num_errors] = node->lineNum;
    num_errors++;
}

int findDimension(char *ID, struct TableNode* table) { // find array dimension 
    if (table == NULL) return 0;
    for (int i = 0; i < table->num_symbol; i++) {
        char * temp = table->currentTable[i]->id;
        if (strcmp(table->currentTable[i]->id, ID) == 0) {
            return table->currentTable[i]->entryNum;
        }
        // printf("%s, %s\n", temp, ID);
    }
    findDimension(ID, table->parentNode);
}

void secondTraversal(struct ASTNode* node) { // second traversal
    if (node == NULL) return;
    if (node->node_type == NODETYPE_OPERATION) { // Exp op Exp don't need to check == and != since all type works
        char* op = node->data.value.string_value;
        if (strcmp(op, "||") == 0 || strcmp(op, "&&") == 0) {
            enum DataType type1 = findExpType(node->children[0]);
            enum DataType type2 = findExpType(node->children[1]);
            if (type1 != DATATYPE_BOOLEAN || type2 != DATATYPE_BOOLEAN) {
                //printf("Type violation 467\n");
                insertError(node);
            }
        } else if (strcmp(op, "+") == 0) { // to do compare operations 
            enum DataType type1 = findExpType(node->children[0]);
            enum DataType type2 = findExpType(node->children[1]);
            //printf("1: %s, 2: %s\n", type_string(type1), type_string(type2));
            if (type1 != type2 || type1 == DATATYPE_BOOLEAN) {
                //printf("Type violation 475\n");
                insertError(node);
            }
            if (checkArray(node->children[0]) != checkArray(node->children[1])) {
                //printf("Type violation 479\n");
                insertError(node);
            }
        } else if (strcmp(op, "-") == 0 || strcmp(op, "*") == 0 || strcmp(op, "/") == 0 || strcmp(op, ">") == 0
                    || strcmp(op, "<") == 0 || strcmp(op, "<=") == 0 || strcmp(op, ">=") == 0) {
            enum DataType type1 = findExpType(node->children[0]);
            enum DataType type2 = findExpType(node->children[1]);
            // printf("1: %s, 2: %s\n", type_string(type1), type_string(type2));
            if (type1 != type2 || type1 != DATATYPE_INT) {
                //printf("Type violation 530\n");
                insertError(node);
            }
            if (checkArray(node->children[0]) != checkArray(node->children[1])) {
                //printf("Type violation 534\n");
                insertError(node);
            }
        }
    } else if (node->node_type == NODETYPE_VARINIT) {
        struct ASTNode* parent = node->parentNode;
        // printEnum(parent);
        if (parent->node_type == NODETYPE_VARDECL) { // to do if return type is array, checking method returns
            enum DataType leftType = findType(parent);
            enum DataType rightType = findExpType(node->children[0]);
            // printEnum(node->children[0]);
            // printf("leftType: %s\n ", type_string(leftType));
            // printf("rightType: %s\n ", type_string(rightType));
            if (leftType != rightType) { // check if they are the same type
                // printf("Type violation 506\n");
                insertError(node);
            }
        }
    } else if (node->node_type == NODETYPE_ASSIGN) {
        enum DataType leftType = findTypeInTable(node->children[0]->data.value.string_value, node->table);
        enum DataType rightType = findExpType(node->children[1]);
        // printf("leftType: %s\n ", type_string(leftType));
        // printf("rightType: %s\n ", type_string(rightType));
        if (leftType != rightType) { // check if they are the same type
                //printf("Type violation 514\n");
                insertError(node);
            }
    } else if (node->node_type == NODETYPE_IF || node->node_type == NODETYPE_WHILE) {
        enum DataType temp = findExpType(node->children[0]);
        if (temp != DATATYPE_BOOLEAN) {
            //printf("Type violation 564\n");
            insertError(node);
        }
    } else if (node->node_type == NODETYPE_LENGTH) {
        // printf("111");
        // printf("%s\n", node->table->currentTable[0]->id);
        char *ID = node->children[0]->data.value.string_value;
        // printf("%s\n", ID);
        if (findDimension(ID, node->table) == 0) {
            //printf("Type violation 570\n");
            insertError(node);
        } else if (findDimension(ID, node->table) == 1) {
            if (node->children[0]->num_children == 1) {
                //printf("Type violation 578\n");
                insertError(node);
            }
        } else {
            if (node->children[0]->num_children == 2) {
                //printf("Type violation 583\n");
                insertError(node);
            }
        }
    } else if (node->node_type == NODETYPE_ONLYINT) {
        enum DataType temp = findExpType(node->children[0]);
        if (temp != DATATYPE_INT) {
            // printf("%s, NOT INT\n", type_string(temp));
            //printf("Type violation 577\n");
            insertError(node);
        }
    } else if (node->node_type == NODETYPE_ONLYBOOL) {
        enum DataType temp = findExpType(node->children[0]);
        if (temp != DATATYPE_BOOLEAN) {
            // printf("%s, NOT Bool\n", type_string(temp));
            //printf("Type violation 584\n");
            insertError(node);
        }
    }
    for (int i = 0; i < node->num_children; i++) {
        secondTraversal(node->children[i]);
    }
}

// for testing
void printEnum(struct ASTNode* node) {
    const char* nodeType[] = {"NODETYPE_PROGRAM", "NODETYPE_MAINCLASS", "MainMethod", "NODETYPE_STATICVARDECLLIST","NODETYPE_STATICMETHODLIST", "NODETYPE_STMTLIST",
    "NODETYPE_VARDECL","NODETYPE_VARINIT","NODETYPE_MOREVAR",
    "NODETYPE_STATICVARDECL","NODETYPE_STATICMETHOD","NODETYPE_FORMALHEAD","NODETYPE_FORMALLIST",
    "NODETYPE_FORMALTAIL","NODETYPE_PRIMETYPE","NODETYPE_STMT","NODETYPE_PRINT",
    "NODETYPE_PRINTLN", "NODETYPE_IF",
    "NODETYPE_WHILE","NODETYPE_PARSEINT","NODETYPE_ASSIGN", "NODETYPE_RETURN", "NODETYPE_TYPE", "NODETYPE_ARRAY", "NODETYPE_METHODCALL",
    "NODETYPE_EXPHEAD", "NODETYPE_EXP", "NODETYPE_EXPTYPE", "NODETYPE_OPERATION", 
    "NODETYPE_ONLYBOOL", "NODETYPE_ONLYINT", "NODETYPE_LENGTH", "NODETYPE_NEWARRAY", "NODETYPE_INDEX", "NODETYPE_EXPLIST", "NODETYPE_EXPTAIL", "NODETYPE_LEFTVAL", "NODETYPE_EXPMETHOD"};
    printf("%s: childNum %d\n", nodeType[node->node_type], node->num_children);
    // if (node->parentNode != NULL) {
    //     printf("Parent: %s\n", nodeType[node->parentNode->node_type]);
    // }
}

// print the AST tree
void traverseAST(struct ASTNode* node) {
    if (node == NULL) return;
    printEnum(node);
    for (int i = 0; i < node -> num_children; i++) {
        traverseAST(node->children[i]);
    }
}

// print the tree of Symbol table
void traverseTable(struct TableNode* table) {
    if (table == NULL) return;
    // printf("number of symbols: %d\n", table->num_symbol);
    // if (table->parentNode != NULL) printf("parentNode: %d\n", table->parentNode->num_symbol);
    for (int i = 0; i < table->num_symbol; i++) {\
        printf("Type: %s ID: %s NumEntry: %d\n", type_string(table->currentTable[i]->type), table->currentTable[i]->id, table->currentTable[i]->entryNum);
    }
    printf("number of children: %d\n", table->num_children);
    printf("\n");
    for (int i = 0; i < table->num_children; i++) {
        traverseTable(table->children[i]);
    }
}

void traverseMethod() {
    for (int i = 0; i < num_methods; i++) {
        printf("MethodName: %s, argNum: %d, returnType: %d\n", method_table[i]->methodName, 
                method_table[i]->num_args, method_table[i]->returnType);
        for (int j = 0; j < method_table[i]->num_args; j++) {
            if (method_table[i]->argTypes[j] == DATATYPE_STR) {
                printf("%d: String ", j);
            } else if (method_table[i]->argTypes[j] == DATATYPE_INT) {
                printf("%d: Int ", j);
            } else if (method_table[i]->argTypes[j] == DATATYPE_BOOLEAN) {
                printf("%d: Boolean ", j);
            }
        }
        printf("\n");
    }
}
// }
