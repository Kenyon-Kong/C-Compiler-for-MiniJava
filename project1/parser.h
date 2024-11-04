#ifndef parser_h
#define parser_h

#include <stdio.h>

// enumerate all tokens (terminals) here
enum Token {
    TOK_OPERATORS,                                                                    // operators
    ID,
    INTEGER_LITERAL,
    STRING_LITERAL,

    KW_CLASS,// keywords
    KW_PUBLIC,
    KW_STATIC,
    KW_VOID,
    KW_MAIN,
    KW_INT,
    KW_BOOL,
    KW_STRING,
    KW_IF,
    KW_ELSE,
    KW_WHILE,
    KW_PRINTLN,
    KW_PRINT,
    KW_RETURN,
    KW_TRUE,
    KW_FALSE,
    KW_NEW,
    KW_LENGTH,

    TOK_LEFTP,
    TOK_RIGHTP,
    TOK_LEFTB,
    TOK_RIGHTB,
    TOK_LEFTC,
    TOK_RIGHTC,
    TOK_EQUAL,
    TOK_DOT,                                                                    // other terminals
    TOK_SEMICOLON,
    TOK_COMMA,
    TOK_EOF
};

void syntax_error();
void set_instream(FILE*);

#endif
