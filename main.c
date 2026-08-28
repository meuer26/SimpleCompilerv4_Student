// This file is licensed under the MIT License. See LICENSE for details.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "ast.h"
#include "visitor.h"
#include "symboltable.h"
#include "semantic.h"
#include "constfold.h"

extern FILE *yyin;
extern AstList program;
extern int yydebug;
int yyparse(void);

SymbolTable symbolTable;

Visitor *makeLlvmVisitor(FILE *outputFile, SymbolTable *symbolTable);
Visitor *makeX64Visitor(FILE *outputFile, SymbolTable *symbolTable);

// Emit data section / vtables before function bodies
void llvmEmitVtables(FILE *outputFile, SymbolTable *symbolTable);
void x64EmitDataAndVtables(FILE *outputFile, SymbolTable *symbolTable);

int main(int argumentCount, char *argumentValues[])
{
    const char *targetBackend = "llvm";
    const char *inputFilePath = 0;

    for (int argIndex = 1; argIndex < argumentCount; argIndex++)
    {
        if (!strncmp(argumentValues[argIndex], "--emit=", 7))
        {
            targetBackend = argumentValues[argIndex] + 7;
        }
        else
        {
            inputFilePath = argumentValues[argIndex];
        }
    }

    if (!inputFilePath)
    {
        fprintf(stderr, "usage: %s --emit=llvm|x64 <source.c>\n", argumentValues[0]);
        return 1;
    }

    yyin = fopen(inputFilePath, "r");
    if (!yyin)
    {
        fprintf(stderr, "cannot open %s\n", inputFilePath);
        return 1;
    }

    astListInit(&program);

    mkdir("./logs", 0755);
    FILE *parserLogFile = fopen("./logs/parser.log", "w");
    int savedStderrFd = -1;
    if (parserLogFile)
    {
        savedStderrFd = dup(fileno(stderr));
        fflush(stderr);
        dup2(fileno(parserLogFile), fileno(stderr));
    }

    yydebug = 1;
    yyparse();

    if (parserLogFile)
    {
        fflush(stderr);
        dup2(savedStderrFd, fileno(stderr));
        close(savedStderrFd);
        fclose(parserLogFile);
    }

    Visitor *semanticVisitor = makeSemanticVisitor(&symbolTable);
    for (int topLevelDeclIndex = 0; topLevelDeclIndex < program.count; topLevelDeclIndex++)
    {
        astAccept(program.items[topLevelDeclIndex], semanticVisitor);
    }

    constFold(&program, stderr);

    Visitor *visitor = 0;
    if (!strcmp(targetBackend, "llvm"))
    {
        llvmEmitVtables(stdout, &symbolTable);
        visitor = makeLlvmVisitor(stdout, &symbolTable);
    }
    else if (!strcmp(targetBackend, "x64"))
    {
        x64EmitDataAndVtables(stdout, &symbolTable);
        visitor = makeX64Visitor(stdout, &symbolTable);
    }
    else
    {
        fprintf(stderr, "unknown emitter: %s\n", targetBackend);
        return 1;
    }

    for (int topLevelDeclIndex = 0; topLevelDeclIndex < program.count; topLevelDeclIndex++)
    {
        astAccept(program.items[topLevelDeclIndex], visitor);
    }

    if (!strcmp(targetBackend, "x64"))
    {
        // Mark the stack non-executable so the linker does not warn:
        // "missing .note.GNU-stack section implies executable stack".
        fprintf(stdout, "\nsection .note.GNU-stack noalloc noexec nowrite progbits\n");
    }

    symPrint(&symbolTable);
    return 0;
}
