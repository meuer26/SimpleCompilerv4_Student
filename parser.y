%{
// This file is licensed under the MIT License. See LICENSE for details.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

int yylex(void);
void yyerror(const char *errorMessage);

extern FILE *yyin;
AstList program;
char currentClassName[20] = "";

__attribute__((unused)) static AstList concatLists(AstList prefixList, AstList suffixList)
{
    for (int suffixIndex = 0; suffixIndex < suffixList.count; suffixIndex++)
    {
        astListPush(&prefixList, suffixList.items[suffixIndex]);
    }
    return prefixList;
}
%}

%define parse.trace
%define parse.error verbose

%union
{
    int number;
    char *string;
    AstNode *node;
    AstList list;
}

%token TOK_LBRACE TOK_RBRACE TOK_RETURN TOK_ADD
%token TOK_LPAREN TOK_RPAREN TOK_SEMI
/* TODO(Assignment 1): Declare tokens for SUB, MUL, LOG_NEG, EQUAL. */
/* TODO(Assignment 2): Declare tokens for INCREMENT, DO, WHILE, LESSTHAN, IF, ELSE. */
/* TODO(Assignment 7): Declare tokens for CLASS, COLON, DOT. */

%token <number> TOK_UINT
%token <string> TOK_TYPE
%token <string> TOK_IDENTIFIER

%type <node> expression value number
%type <list> statement statementList
/* TODO(Assignment 1): Add %type declarations for any new non-terminals (e.g., variableDeclaration). */
/* TODO(Assignment 3): Add %type declarations for argument and argumentList. */
/* TODO(Assignment 7): Add %type declarations for methodList. */

%%

program:
        topLevelDeclarations
        ;

topLevelDeclarations:
        topLevelDeclarations topLevelDeclaration
        |
        ;

topLevelDeclaration:
        function
        /* TODO(Assignment 7): Add classDeclaration as another top-level alternative. */
        ;

function:
        TOK_TYPE TOK_IDENTIFIER TOK_LPAREN TOK_RPAREN TOK_LBRACE statementList TOK_RBRACE
        {
            AstNode *functionNode = funcDecl($1, $2, $6);
            astListPush(&program, functionNode);
        }
        /* TODO(Assignment 3): Add an alternative that accepts an argument list inside the parens. */
        ;

statementList:
        statement
        {
            astListInit(&$$);
            for (int innerStatementIndex = 0; innerStatementIndex < $1.count; innerStatementIndex++)
            {
                astListPush(&$$, $1.items[innerStatementIndex]);
            }
        }
        |
        statementList statement
        {
            $$ = $1;
            for (int innerStatementIndex = 0; innerStatementIndex < $2.count; innerStatementIndex++)
            {
                astListPush(&$$, $2.items[innerStatementIndex]);
            }
        }
        ;

statement:
        TOK_RETURN expression TOK_SEMI
        {
            astListInit(&$$);
            astListPush(&$$, returnStmt($2));
        }
        /* TODO(Assignment 1): Add statement forms for empty statement, expression statement, */
        /*                     variable declaration, declaration-with-assignment, assignment. */
        /* TODO(Assignment 2): Add statement forms for increment, do, while, and block.       */
        /* TODO(Assignment 2): Add statement forms for if and if/else. Both flatten to a       */
        /*   linear marker sequence: ifBegin(l,r), then-stmts, elseBegin(), [else-stmts,] endIf(). */
        ;

expression:
        value TOK_ADD expression
        {
            $$ = add($1, $3);
        }
        |
        value
        {
            $$ = $1;
        }
        |
        TOK_LPAREN expression TOK_RPAREN
        {
            $$ = $2;
        }
        /* TODO(Assignment 1): Add expression forms for SUB, MUL, and LOG_NEG. */
        ;

value:
        number
        {
            $$ = $1;
        }
        /* TODO(Assignment 1): Add a value form for identifier (variable reference). */
        /* TODO(Assignment 4): Add value forms for function calls (no-arg and with-arg). */
        /* TODO(Assignment 7): Add value forms for object method calls (no-arg and with-arg). */
        ;

number:
        TOK_UINT
        {
            $$ = intType($1);
        }
        ;

%%

void yyerror(const char *errorMessage)
{
    FILE *errorLogFile = fopen("./logs/parsing_error.log", "wb");
    if (errorLogFile)
    {
        fprintf(errorLogFile, "%s\n", errorMessage);
        fclose(errorLogFile);
    }
    fprintf(stderr, "parse error: %s\n", errorMessage);
}
