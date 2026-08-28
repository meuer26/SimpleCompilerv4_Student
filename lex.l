/* This file is licensed under the MIT License. See LICENSE for details. */

%{
#include <stdio.h>
#include <string.h>
#include "ast.h"
#include "parser.tab.h"
%}

%option noinput nounput

%%
"//"[^\n]*              { /* line comment */ }
[ \t\n]+                { /* whitespace */ }

"return"                { return TOK_RETURN; }
"int"                   { yylval.string = strdup(yytext); return TOK_TYPE; }
"{"                     { return TOK_LBRACE; }
"}"                     { return TOK_RBRACE; }
"("                     { return TOK_LPAREN; }
")"                     { return TOK_RPAREN; }
"+"                     { return TOK_ADD; }
";"                     { return TOK_SEMI; }
                        /* TODO(Assignment 1): Add tokens for `-`, `*`, `!`, `=`. */
                        /* TODO(Assignment 2): Add tokens for `++`, `do`, `while`, `<`, `if`, `else`. */
                        /* TODO(Assignment 7): Add tokens for `class`, `:`, `.`, `public:`, `public`, `virtual`, `extern "C"`. */
[0-9]+                  { yylval.number = atoi(yytext); return TOK_UINT; }
[a-zA-Z_]+              { yylval.string = strdup(yytext); return TOK_IDENTIFIER; }

%%

int yywrap(void) { return 1; }
