#include <stddef.h>                                                             /* for NULL */
#include <stdlib.h>                                                             /* for EXIT_SUCCESS and EXIT_FAILURE */
#include <stdbool.h>
#include "parser.h"


// yylex is used to get tokens from lex
extern int yylex();
int current_token;
void program();
void mainClass();
void stmtList();
void stmt();
void varDecl();
void varInit();
void moreVar();
void primeType();
void type();
void typeTail();
void expression();
void expList();
void lengthTail();
void expTail();
void index1();
void indexTail();
void leftValue();
void leftTail();


// advance current token
static void consume() {
    current_token = yylex();
}

// return true if current token is expected
// otherwise return false
static bool peek(int expected_token) {
    if(current_token == expected_token) {
        return true;
    }
    return false;
}

static void match(int expect) {
  if(current_token != expect) {
   syntax_error();
  } else {
    consume();
  }
}

// write code to perform recursive descent based on the grammar after
// rewriting it

void program() {
  mainClass();
}

void mainClass() {
  match(KW_CLASS);
  match(ID);
  match(TOK_LEFTC);
  match(KW_PUBLIC);
  match(KW_STATIC);
  match(KW_VOID);
  match(KW_MAIN);
  match(TOK_LEFTP);
  match(KW_STRING);
  match(TOK_LEFTB);
  match(TOK_RIGHTB);
  match(ID);
  match(TOK_RIGHTP);
  match(TOK_LEFTC);
  stmtList(); // stmtlist //
  match(TOK_RIGHTC);
  match(TOK_RIGHTC);
}

void stmtList() {
  //printf("stmtList\n");
  if(peek(KW_INT) || peek(KW_BOOL) || peek(KW_STRING) || peek(KW_IF)
      || peek(KW_IF) || peek(KW_WHILE) || peek(KW_PRINTLN) || peek(KW_PRINT) || peek(ID)
      || peek(KW_RETURN)) {
    stmt();
    stmtList();
  } else {
    return; //null rule
  }
}

void varDecl() {
  //printf("varDecl\n");
  type();
  match(ID);
  varInit();
  moreVar();
  match(TOK_SEMICOLON);//printf("varDecl OUT\n");
}

void varInit() {
  //printf("varInit\n");
  if(peek(TOK_EQUAL)) {
    match(TOK_EQUAL);
    expression();
  } else {
    return; //null rule
  }
}

void moreVar() {
  //printf("moreVar\n");
  if(peek(TOK_COMMA)) {
    match(TOK_COMMA);
    match(ID);
    varInit();
    moreVar();
  } else {
    return; //null rule
  }
}

void primeType() {
  //printf("PrimitiveType\n");
  if(peek(KW_INT)) {
    match(KW_INT);
  } else if(peek(KW_BOOL)) {
    match(KW_BOOL);
  } else if(peek(KW_STRING)) {
    match(KW_STRING);
  } else {
    syntax_error;
  }
}

void type() {
  //printf("Type\n");
  primeType();
  typeTail();
}

void typeTail() {
  //printf("TypeTail\n");
  if(peek(TOK_LEFTB)) {
    match(TOK_LEFTB);
    match(TOK_RIGHTB);
    typeTail();
  } else {
    return; // null rule
  }
}

void stmt() {
  //printf("stmt\n");
  if(peek(KW_INT) || peek(KW_BOOL) || peek(KW_STRING)) {
    varDecl(); // FIRST(varDecl) = {int, bool, string}
  } else if(peek(TOK_LEFTC)) {
    match(TOK_LEFTC);
    stmtList();
    match(TOK_RIGHTC);
  } else if(peek(KW_IF)) {
    match(KW_IF);
    match(TOK_LEFTP);
    expression();
    match(TOK_RIGHTP);
    stmt();
    match(KW_ELSE);
    stmt();
  } else if(peek(KW_WHILE)) {
    match(KW_WHILE);
    match(TOK_LEFTP);
    expression();
    match(TOK_RIGHTP);
    stmt();
  } else if(peek(KW_PRINTLN)) {
    match(KW_PRINTLN);
    match(TOK_LEFTP);
    expression();
    match(TOK_RIGHTP);
    match(TOK_SEMICOLON);
  } else if(peek(KW_PRINT)) {
    match(KW_PRINT);
    match(TOK_LEFTP);
    expression();
    match(TOK_RIGHTP);
    match(TOK_SEMICOLON);
  } else if(peek(ID)) { // FIRST(leftValue) = {ID}
    leftValue();
    match(TOK_EQUAL);
    expression();
    match(TOK_SEMICOLON);
  } else if(peek(KW_RETURN)) {
    match(KW_RETURN);
    expression();
    match(TOK_SEMICOLON);
  } else {
    syntax_error();
  }
}

void expression() {
  if(peek(ID) || peek(INTEGER_LITERAL) || peek(STRING_LITERAL) || peek(KW_TRUE)
      || peek(KW_FALSE) || peek(KW_NEW)) {
    expList();
    expTail();
  } else if(peek(TOK_LEFTP)){
    match(TOK_LEFTP);
    expression();
    match(TOK_RIGHTP);
  } else {
    syntax_error();
  }
}

void expList() {
  if(peek(ID)) { // FIRST(leftValue) = {ID}
    leftValue();
    lengthTail();
  } else if(peek(INTEGER_LITERAL)) {
    match(INTEGER_LITERAL);
  } else if(peek(STRING_LITERAL)) {
    match(STRING_LITERAL);
  } else if(peek(KW_TRUE)) {
    match(KW_TRUE);
  } else if(peek(KW_FALSE)) {
    match(KW_FALSE);
  } else if(peek(KW_NEW)) {
    match(KW_NEW);
    primeType();
    index1();
  }
}

void lengthTail() {
  if(peek(TOK_DOT)) {
    match(TOK_DOT);
    match(KW_LENGTH);
  } else {
    return; // null rule
  }
}

void expTail() {
  if(peek(TOK_OPERATORS)) {
     match(TOK_OPERATORS);
     expression();
     expTail();
  } else {
    return; // null rule
  }
}

void index1() {
  match(TOK_LEFTB);
  expression();
  match(TOK_RIGHTB);
  indexTail();
}

void indexTail() {
  if(peek(TOK_LEFTB)) {
    match(TOK_LEFTB);
    expression();
    match(TOK_RIGHTB);
    indexTail();
  } else {
    return; // null rule
  }
}

void leftValue() {
  match(ID);
  leftTail();
}

void leftTail() {
  if(peek(TOK_LEFTB)) {
    match(TOK_LEFTB);
    expression();
    match(TOK_RIGHTB);
    leftTail();
  } else {
    return; // null rule
  }
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        char *path = argv[1];
        FILE *file = fopen(path, "r");
        if(file == NULL) {
            perror("Cannot open file");
            return EXIT_FAILURE;
        }

        set_instream(file);
    }

    // call the function to match the start non terminal here
    consume();
    program();
    if(!peek(TOK_EOF)) {
      syntax_error();
    }
    return EXIT_SUCCESS;
}
