/* A Bison parser, made by GNU Bison 3.5.1.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

#ifndef YY_YY_Y_TAB_H_INCLUDED
# define YY_YY_Y_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    KW_BOOLEAN = 258,
    KW_CLASS = 259,
    KW_FALSE = 260,
    KW_INT = 261,
    MAIN = 262,
    KW_PUBLIC = 263,
    KW_TRUE = 264,
    KW_VOID = 265,
    KW_STATIC = 266,
    KW_STRING = 267,
    SYSTEM_OUT_PRINT = 268,
    SYSTEM_OUT_PRINTLN = 269,
    KW_PRIVATE = 270,
    KW_IF = 271,
    KW_ELSE = 272,
    KW_WHILE = 273,
    KW_RETURN = 274,
    INTEGER_PARSEINT = 275,
    KW_LENGTH = 276,
    KW_NEW = 277,
    TOK_AND = 278,
    TOK_OR = 279,
    TOK_LESSEQL = 280,
    TOK_GREATEQL = 281,
    TOK_EQUAL = 282,
    TOK_NOTEQL = 283,
    INTEGER_LITERAL = 284,
    STRING_LITERAL = 285,
    ID = 286
  };
#endif
/* Tokens.  */
#define KW_BOOLEAN 258
#define KW_CLASS 259
#define KW_FALSE 260
#define KW_INT 261
#define MAIN 262
#define KW_PUBLIC 263
#define KW_TRUE 264
#define KW_VOID 265
#define KW_STATIC 266
#define KW_STRING 267
#define SYSTEM_OUT_PRINT 268
#define SYSTEM_OUT_PRINTLN 269
#define KW_PRIVATE 270
#define KW_IF 271
#define KW_ELSE 272
#define KW_WHILE 273
#define KW_RETURN 274
#define INTEGER_PARSEINT 275
#define KW_LENGTH 276
#define KW_NEW 277
#define TOK_AND 278
#define TOK_OR 279
#define TOK_LESSEQL 280
#define TOK_GREATEQL 281
#define TOK_EQUAL 282
#define TOK_NOTEQL 283
#define INTEGER_LITERAL 284
#define STRING_LITERAL 285
#define ID 286

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 21 "parser.y"

    struct ASTNode* node;
    int integer;
    char * string;

#line 125 "y.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_Y_TAB_H_INCLUDED  */
