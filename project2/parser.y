%{
#include <stdio.h>
#include <string.h>
#include "typecheck.h"
#include "node.h"
void yyerror(char *);

extern int yylex();

// Global variables defined by lex.yy.c.
extern int yylineno;
extern char* yytext;
extern FILE *yyin;

struct ASTNode* root;
%}

// Declares all variants of semantic values. Yacc/Bison copies all variants
// to the generated header file (y.tab.h) enclosed in a C-language union
// declaration, named `YYSTYPE`. Check out the header file to see it.
%union {
    struct ASTNode* node;
    int integer;
    char * string;
}

// Left hand non-terminals. They are all associated to the `node` variant
// declared in the %union section, which is of type `ASTNode *`.
%type <node> Program MainClass MainMethod StaticVarDeclList StaticMethodDeclList StatementList VarDecl Statement Exp Type
%type <node> VarInit MoreVar StaticVarDecl StaticMethodDecl FormalHead FormalList FormalTail PrimeType
%type <node> MethodCall ExpHead  Index Explist ExpTail LeftValue EmptyRule

// Declares tokens. In the generated y.tab.h file, each token gets declared as 
// a enum constant and assigned with a unique number. These enum constants are
// used in the lex file, returned by `yylex()` to denote the symbolic tokens.

// These keyword-like tokens doesn't need to have a semantic value.
%token KW_BOOLEAN KW_CLASS KW_FALSE KW_INT MAIN KW_PUBLIC KW_TRUE KW_VOID 
%token KW_STATIC KW_STRING SYSTEM_OUT_PRINT SYSTEM_OUT_PRINTLN KW_PRIVATE 
%token KW_IF KW_ELSE KW_WHILE KW_RETURN INTEGER_PARSEINT KW_LENGTH KW_NEW
// Binary operators 
%token TOK_AND TOK_OR TOK_LESSEQL TOK_GREATEQL TOK_EQUAL TOK_NOTEQL
// These tokens have additional information aside from what kind of token it
// is, so they carry semantic information.
%token <integer> INTEGER_LITERAL
%token <string> STRING_LITERAL ID

%left TOK_OR
%left TOK_AND
%left TOK_EQUAL TOK_NOTEQL
%left '>' '<' TOK_LESSEQL TOK_GREATEQL
%left '+' '-' 
%left '*' '/'
%right '!'

%start Program

%%

Program:                
    MainClass {   
        $$ = new_node(NODETYPE_PROGRAM, yylineno);
        $$ -> newScope = false;
        root = $$;
        root->parentNode = NULL;
        add_child($$, $1);
        // traverseAST(root);
        //printf("IN Program\n");
    };

MainClass: 
    KW_CLASS ID '{' StaticVarDeclList StaticMethodDeclList MainMethod '}'
    {
        //printf("IN Main\n");
        $$ = new_node(NODETYPE_MAINCLASS, yylineno);
        $$ ->newScope = true;
        set_string_value($$, $2);
        if ($4 != NULL) {
            add_child($$, $4);
        }
        if ($5 != NULL) {
            add_child($$, $5);
        }
        add_child($$, $6);
        
    };

MainMethod:
    KW_PUBLIC KW_STATIC KW_VOID MAIN '(' KW_STRING '[' ']' ID ')' '{' StatementList '}'{
        $$ = new_node(NODETYPE_MAINMETHOD, yylineno);
        $$ ->newScope = true;
        set_string_value($$, $9);
        if ($12 != NULL) {
            add_child($$, $12);
        }
    };

StaticVarDeclList:
    StaticVarDeclList StaticVarDecl {
        $$ = new_node(NODETYPE_STATICVARDECLLIST, yylineno);
        $$ ->newScope = false;
        if ($1 != NULL) {
            add_child($$, $1);
        }
        add_child($$, $2);
    }
    | EmptyRule;
    
StaticMethodDeclList:
    StaticMethodDeclList StaticMethodDecl {
        $$ = new_node(NODETYPE_STATICMETHODLIST, yylineno);
        $$ ->newScope = false;
        if ($1 != NULL) {
            add_child($$, $1);
        }
        add_child($$, $2);
    }
    | EmptyRule;

StatementList:
    StatementList Statement {
        $$ = new_node(NODETYPE_STMTLIST, yylineno);
        $$ ->newScope = false;
        if ($1 != NULL) {
            add_child($$, $1);
        }
        add_child($$, $2);
    }
    | EmptyRule;

VarDecl:
    Type ID VarInit MoreVar ';' {
        $$ = new_node(NODETYPE_VARDECL, yylineno);
        $$ ->newScope = false;
        set_string_value($$, $2);
        add_child($$, $1);
        if ($3 != NULL) {
            add_child($$, $3);
        }
        if ($4 != NULL) {
            add_child($$, $4);
        }
    };

VarInit:
    '=' Exp {
        $$ = new_node(NODETYPE_VARINIT, yylineno);
        $$ ->newScope = false;
        add_child($$, $2);
    }
    | EmptyRule;

MoreVar:
    MoreVar ',' ID VarInit {
        $$ = new_node(NODETYPE_MOREVAR, yylineno);
        $$ ->newScope = false;
        set_string_value($$, $3);
        if ($1 != NULL) {
            add_child($$, $1);
        }
        if ($4 != NULL) {
            add_child($$, $4);
        }
    }
    | EmptyRule;

StaticVarDecl:
    KW_PRIVATE KW_STATIC VarDecl {
        $$ = new_node(NODETYPE_STATICVARDECL, yylineno);
        $$ ->newScope = false;
        add_child($$, $3);
    };

StaticMethodDecl:
    KW_PUBLIC KW_STATIC Type ID '(' FormalHead ')'
        '{' StatementList '}' {
            $$ = new_node(NODETYPE_STATICMETHOD, yylineno);
            $$ ->newScope = true;
            set_string_value($$, $4);
            add_child($$, $3);
            if ($6 != NULL) {
            add_child($$, $6);
            }
            if ($9 != NULL) {
            add_child($$, $9);
        }
    };

FormalHead:
    FormalList {
        $$ = new_node(NODETYPE_FORMALHEAD, yylineno);
        $$ ->newScope = false;
        add_child($$, $1);
    }
    | EmptyRule;

FormalList:
    Type ID FormalTail {
        $$ = new_node(NODETYPE_FORMALLIST, yylineno);
        $$ ->newScope = false;
        set_string_value($$, $2);
        add_child($$, $1);
        if ($3 != NULL) {
            add_child($$, $3);
        }
    };

FormalTail:
    ',' Type ID FormalTail {
        $$ = new_node(NODETYPE_FORMALTAIL, yylineno);
        $$ ->newScope = false;
        set_string_value($$, $3);
        add_child($$, $2);
        if ($4 != NULL) {
            add_child($$, $4);
        }
    }
    | EmptyRule;

Type:
    PrimeType {
        $$ = new_node(NODETYPE_TYPE, yylineno);
        $$ ->newScope = false;
        add_child($$, $1);
    }
    | Type '[' ']' {
        $$ = new_node(NODETYPE_ARRAY, yylineno);
        $$ ->newScope = false;
        add_child($$, $1);
    };

PrimeType:                   
    KW_INT {
        $$ = new_node(NODETYPE_PRIMETYPE, yylineno);
        $$ ->newScope = false;
        $$ -> data.type = DATATYPE_INT;
    }
    | KW_BOOLEAN {
        $$ = new_node(NODETYPE_PRIMETYPE, yylineno);
        $$ ->newScope = false;
        $$ -> data.type = DATATYPE_BOOLEAN;
    }
    | KW_STRING {
        $$ = new_node(NODETYPE_PRIMETYPE, yylineno);
        $$ ->newScope = false;
        $$ -> data.type = DATATYPE_STR;
    };
                    

Statement:              
    VarDecl {
        $$ = new_node(NODETYPE_STMT, yylineno);
        $$ ->newScope = false;
        add_child($$, $1);
    }
    | '{' StatementList '}' { 
        $$ = new_node(NODETYPE_STMT, yylineno);
        $$ ->newScope = true;
        if ($2 != NULL) {
            add_child($$, $2);
        }
    }
    | KW_IF '(' Exp ')' Statement KW_ELSE Statement {
        $$ = new_node(NODETYPE_IF, yylineno);
        $$ ->newScope = false;
        add_child($$, $3);
        add_child($$, $5);
        add_child($$, $7);
    }
    | KW_WHILE '(' Exp ')' Statement {
        $$ = new_node(NODETYPE_WHILE, yylineno);
        $$ ->newScope = false;
        add_child($$, $3);
        add_child($$, $5);
    }
    | SYSTEM_OUT_PRINTLN '(' Exp ')' ';' {
        $$ = new_node(NODETYPE_PRINTLN, yylineno);
        $$ ->newScope = false;
        add_child($$, $3);
    }
    | SYSTEM_OUT_PRINT '(' Exp ')' ';' {
        $$ = new_node(NODETYPE_PRINT, yylineno);
        $$ ->newScope = false;
        add_child($$, $3);
    }
    | LeftValue '=' Exp ';' {
        $$ = new_node(NODETYPE_ASSIGN, yylineno);
        $$ ->newScope = false;
        add_child($$, $1);
        add_child($$, $3);
    }
    | KW_RETURN Exp ';' {
        $$ = new_node(NODETYPE_STMT, yylineno);
        $$ ->newScope = false;
        add_child($$, $2);
    }
    | MethodCall ';' {
        $$ = new_node(NODETYPE_STMT, yylineno);
        $$ ->newScope = false;
        add_child($$, $1);
    };

MethodCall:
    ID '(' ExpHead ')' {
        $$ = new_node(NODETYPE_METHODCALL, yylineno);
        $$ ->newScope = false;
        set_string_value($$, $1);
        if ($3 != NULL) {
            add_child($$, $3);
        }
    }
    | INTEGER_PARSEINT '(' Exp ')' {
        $$ = new_node(NODETYPE_PARSEINT, yylineno);
        $$ ->newScope = false;
        add_child($$, $3);
    };

ExpHead:
    Explist {
        $$ = new_node(NODETYPE_EXPHEAD, yylineno);
        $$ ->newScope = false;
        add_child($$, $1);
    }
    | EmptyRule;

Exp:
    Exp TOK_AND Exp {
        $$ = new_node(NODETYPE_OPERATION, yylineno);
        $$ ->newScope = false;
        set_string_value($$, "&&");
        add_child($$, $1);
        add_child($$, $3);
    }
    | Exp TOK_OR Exp {
        $$ = new_node(NODETYPE_OPERATION, yylineno);
        $$ ->newScope = false;
        set_string_value($$, "||");
        add_child($$, $1);
        add_child($$, $3);

    }
    | Exp '<' Exp {
        $$ = new_node(NODETYPE_OPERATION, yylineno);
        $$ ->newScope = false;
        set_string_value($$, "<");
        add_child($$, $1);
        add_child($$, $3);
    }
    | Exp '>' Exp {
        $$ = new_node(NODETYPE_OPERATION, yylineno);
        $$ ->newScope = false;
        set_string_value($$, ">");
        add_child($$, $1);
        add_child($$, $3);

    }
    | Exp TOK_LESSEQL Exp {
        $$ = new_node(NODETYPE_OPERATION, yylineno);
        $$ ->newScope = false;
        set_string_value($$, "<=");
        add_child($$, $1);
        add_child($$, $3);

    }
    | Exp TOK_GREATEQL Exp {
        $$ = new_node(NODETYPE_OPERATION, yylineno);
        $$ ->newScope = false;
        set_string_value($$, ">=");
        add_child($$, $1);
        add_child($$, $3);

    }
    | Exp TOK_EQUAL Exp {
        $$ = new_node(NODETYPE_OPERATION, yylineno);
        $$ ->newScope = false;
        set_string_value($$, "==");
        add_child($$, $1);
        add_child($$, $3);

    }
    | Exp TOK_NOTEQL Exp {
        $$ = new_node(NODETYPE_OPERATION, yylineno);
        $$ ->newScope = false;
        set_string_value($$, "!=");
        add_child($$, $1);
        add_child($$, $3);

    }
    | Exp '+' Exp {
        $$ = new_node(NODETYPE_OPERATION, yylineno);
        $$ ->newScope = false;
        set_string_value($$, "+");
        add_child($$, $1);
        add_child($$, $3);
    }
    | Exp '-' Exp {
    $$ = new_node(NODETYPE_OPERATION, yylineno);
        $$ ->newScope = false;
        set_string_value($$, "-");
        add_child($$, $1);
        add_child($$, $3);

    }
    | Exp '*' Exp {
    $$ = new_node(NODETYPE_OPERATION, yylineno);
        $$ ->newScope = false;
        set_string_value($$, "*");
        add_child($$, $1);
        add_child($$, $3);

    }
    | Exp '/' Exp {
        $$ = new_node(NODETYPE_OPERATION, yylineno);
        $$ ->newScope = false;
        set_string_value($$, "/");
        add_child($$, $1);
        add_child($$, $3);

    }
    | '!' Exp {
        $$ = new_node(NODETYPE_ONLYBOOL, yylineno);
        $$ ->newScope = false;
        add_child($$, $2);
    }
    | '+' Exp {
        $$ = new_node(NODETYPE_ONLYINT, yylineno);
        $$ ->newScope = false;
        add_child($$, $2);
    }
    | '-' Exp {
        $$ = new_node(NODETYPE_ONLYINT, yylineno);
        $$ ->newScope = false;
        add_child($$, $2);
    }
    | '(' Exp ')' {
        $$ = new_node(NODETYPE_EXP, yylineno);
        $$ ->newScope = false;
        add_child($$, $2);
    }
    | LeftValue '.' KW_LENGTH {
        $$ = new_node(NODETYPE_LENGTH, yylineno);
        $$ ->newScope = false;
        add_child($$, $1);
    }
    | LeftValue {
        $$ = new_node(NODETYPE_EXP, yylineno);
        $$ ->newScope = false;
        add_child($$, $1);
    }
    | MethodCall {
        $$ = new_node(NODETYPE_EXP, yylineno);
        $$ ->newScope = false;
        add_child($$, $1);
    }
    | KW_NEW PrimeType Index {
        $$ = new_node(NODETYPE_NEWARRAY, yylineno);
        $$ ->newScope = false;
        add_child($$, $2);
        add_child($$, $3);
    }   
    | INTEGER_LITERAL {
        $$ = new_node(NODETYPE_EXPTYPE, yylineno);
        $$ ->newScope = false;
        set_int_value($$, $1);
    }
    | STRING_LITERAL {
        $$ = new_node(NODETYPE_EXPTYPE, yylineno);
        $$ ->newScope = false;
        set_string_value($$, $1);
    }
    | KW_TRUE {
        $$ = new_node(NODETYPE_EXPTYPE, yylineno);
        $$ ->newScope = false;
        set_boolean_value($$, true);
    }
    | KW_FALSE {
        $$ = new_node(NODETYPE_EXPTYPE, yylineno);
        $$ ->newScope = false;
        set_boolean_value($$, false);
    };
  /*
Operators:
    TOK_AND {
        $$ = new_node(NODETYPE_OPERATORS, yylineno);
        $$ ->newScope = false;
        set_string_value($$, "&&");
    }
    | TOK_OR {
        $$ = new_node(NODETYPE_OPERATORS, yylineno);
        $$ ->newScope = false;
        set_string_value($$, "||");
    }
    | '<' {
        $$ = new_node(NODETYPE_OPERATORS, yylineno);
        $$ ->newScope = false;
        set_string_value($$, "<");
    } 
    | '>' {
        $$ = new_node(NODETYPE_OPERATORS, yylineno);
        $$ ->newScope = false;
        set_string_value($$, ">");
    }
    | TOK_LESSEQL {
        $$ = new_node(NODETYPE_OPERATORS, yylineno);
        $$ ->newScope = false;
        set_string_value($$, "<=");
    }
    | TOK_GREATEQL {
        $$ = new_node(NODETYPE_OPERATORS, yylineno);
        $$ ->newScope = false;
        set_string_value($$, ">=");
    }
    | TOK_EQUAL {
        $$ = new_node(NODETYPE_OPERATORS, yylineno);
        $$ ->newScope = false;
        set_string_value($$, "==");
    }
    | TOK_NOTEQL {
        $$ = new_node(NODETYPE_OPERATORS, yylineno);
        $$ ->newScope = false;
        set_string_value($$, "!=");
    }
    | '+' {
        $$ = new_node(NODETYPE_OPERATORS, yylineno);
        $$ ->newScope = false;
        set_string_value($$, "+");
    }
    | '-' {
        $$ = new_node(NODETYPE_OPERATORS, yylineno);
        $$ ->newScope = false;
        set_string_value($$, "-");
    }
    | '*' {
        $$ = new_node(NODETYPE_OPERATORS, yylineno);
        $$ ->newScope = false;
        set_string_value($$, "*");
    }
    | '/' {
        $$ = new_node(NODETYPE_OPERATORS, yylineno);
        $$ ->newScope = false;
        set_string_value($$, "/");
    };
  */
Index: 
    '[' Exp ']' {
        $$ = new_node(NODETYPE_INDEX, yylineno);
        $$ ->newScope = false;
        add_child($$, $2);
    }
    | '[' Exp ']' '[' Exp ']' {
        $$ = new_node(NODETYPE_INDEX, yylineno);
        $$ ->newScope = false;
        add_child($$, $2);
        add_child($$, $5);
    };

Explist:
    Exp ExpTail {
        $$ = new_node(NODETYPE_EXPLIST, yylineno);
        $$ ->newScope = false;
        add_child($$, $1);
        if ($2 != NULL) {
            add_child($$, $2);
        }  
    };

ExpTail:
    ExpTail ',' Exp {
        $$ = new_node(NODETYPE_EXPTAIL, yylineno);
        $$ ->newScope = false;
        if ($1 != NULL) {
            add_child($$, $1);
        }
        add_child($$, $3);
    }
    | EmptyRule;

LeftValue:
    ID {
        $$ = new_node(NODETYPE_LEFTVAL, yylineno);
        $$ ->newScope = false;
        set_string_value($$, $1);
    }
    | ID '[' Exp ']' {
        $$ = new_node(NODETYPE_LEFTVAL, yylineno);
        $$ ->newScope = false;
        set_string_value($$, $1);
        add_child($$, $3);
    }
    | ID '[' Exp ']' '[' Exp ']' {
        $$ = new_node(NODETYPE_LEFTVAL, yylineno);
        $$ ->newScope = false;
        set_string_value($$, $1);
        add_child($$, $3);
        add_child($$, $6);
    };

EmptyRule:
    {
        $$ = NULL;
    }
    ;
%%

void yyerror(char* s) {
    fprintf(stderr, "Syntax errors in line %d\n", yylineno);
}


int main(int argc, char* argv[])
{
    yyin = fopen( argv[1], "r" );
    //traverseAST(root);
    // Checks for syntax errors and constructs AST
    if (yyparse() != 0)
        return 1;
    traverseAST(root);
    firstTraversal(root);
    secondTraversal(root);
    //printEnum(root);
    // traverseTable(root->table);
    // traverseMethod();
    report_type_violation();
    // Traverse the AST to check for semantic errors if no syntac errors
    //checkProgram(root);
    
    return 0;
}
