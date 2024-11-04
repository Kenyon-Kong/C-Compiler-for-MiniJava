%option noyywrap
%option yylineno

%{
#include <stdio.h>                                                              /* for fprintf() */
#include <stdlib.h>                                                             /* for EXIT_FAILURE */

#include "parser.h"

void syntax_error() {
    fprintf(stderr, "Syntax errors in %d\n", yylineno);
    exit(EXIT_FAILURE);
}
%}

%%


    /* write regex to match all terminals */
    /* a few example are given below */



[ \t\r\n]*                  { }                                                 /* whitespace */

"//".* {
  //printf("inline_comment: %s\n", yytext);
}
"/*"([^*]|"*"+[^/*])*"*""/" {
 // printf("block comment: %s\n", yytext);
}


  /* catching ++ and -- */
"++" {
  //printf("++\n");
  
}

"--" {
  //printf("--\n");
  syntax_error();
}

  /* opeartors */
"&&" {
   //printf("&&\n");
   return TOK_AND;
}

"||" {
   //printf("||\n");
   return TOK_OPERATORS;
}

"<" {
   //printf("<\n");
   return TOK_OPERATORS;
}

">" {
   //printf(">\n");
   return TOK_OPERATORS;
}

"<=" {
   //printf("<=\n");
   return TOK_OPERATORS;
}

">=" {
   //printf(">=\n");
   return TOK_OPERATORS;
}

"==" {
   //printf("==\n");
   return TOK_OPERATORS;
}

"!=" {
   //printf("!=\n");
   return TOK_OPERATORS;
}

"+" {
   //printf("+\n");
   return TOK_OPERATORS;
}

"-" {
   //printf("-\n");
   return TOK_OPERATORS;
}

"*" {
   //printf("*\n");
   return TOK_OPERATORS;
}

"/" {
   //printf("/\n");
   return TOK_OPERATORS;
}

"class"                     { return KW_CLASS; } /* keywords */
"public"                    { return KW_PUBLIC; }
"static"                    { return KW_STATIC; }
"void"                      { return KW_VOID; }
"main"                      { return KW_MAIN; }
"int"                       {  return KW_INT; }
"boolean"                   { return KW_BOOL; }
"String"                    { return KW_STRING; }
"if"                        { return KW_IF; }
"else"                      { return KW_ELSE;}
"while"                     { return KW_WHILE; }
"System.out.println"        { return KW_PRINTLN; }
"System.out.print"          { return KW_PRINT; }
"return"                    { return KW_RETURN; }
"true"                      { return KW_TRUE; }
"false"                     { return KW_FALSE; }
"new"                       { return KW_NEW; }
"length"                    { return KW_LENGTH; }

"("                         { return TOK_LEFTP; }
")"                         { return TOK_RIGHTP; }
"["                         { return TOK_LEFTB; }
"]"                         { return TOK_RIGHTB; }
"{"                         { return TOK_LEFTC; }
"}"                         { return TOK_RIGHTC; }
"="                         { return TOK_EQUAL; }
"."                         { return TOK_DOT; }                                 /* other terminals */
";"                         { return TOK_SEMICOLON; }
","                         { return TOK_COMMA; }

<<EOF>>                     { return TOK_EOF; }
  
  /* identifiers */
[A-Za-z][A-Za-z0-9_]* {
  //printf("ID: %s\n", yytext);
  return ID;
}


  /* literals */
[0-9]+ {
  //printf("integer: %s\n", yytext);
  return INTEGER_LITERAL;
}


\"([^\\\"]+|\\.)*\" {
  //printf("string: %s\n", yytext);
  return STRING_LITERAL;
}

.                           { syntax_error(); }                                 /* all other characters */

%%

// yyin is the input stream for lex, not providing a file defaults to
// standard input
void set_instream(FILE *file) {
  yyin = file;
}
