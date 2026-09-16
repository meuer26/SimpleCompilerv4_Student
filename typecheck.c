// This file is licensed under the MIT License. See LICENSE for details.

#include <stdio.h>
#include <string.h>
#include "ast.h"
#include "visitor.h"
#include "symboltable.h"
#include "typecheck.h"

// One pass over the AST that fills `lastType` with the type of the just-
// visited expression. ADD/SUB/MUL inspect their operands' lastType and
// report an error when the two disagree.
typedef struct
{
    SymbolTable *symbolTable;
    int currentFunctionOrMethodIndex;
    char lastType[20];
    int errorCount;
} TypeCheckContext;

static TypeCheckContext typeCheckContext;

static void setLastType(TypeCheckContext *context, const char *type)
{
    strncpy(context->lastType, type, sizeof context->lastType - 1);
    context->lastType[sizeof context->lastType - 1] = '\0';
}

// Resolve a variable reference (LVAR/ARG/OBJ) within the current function
// scope. Writes the declared type into `outType`. Returns "<unknown>" if
// nothing matches — the operand-mismatch check still works on the string.
static void resolveVariableType(SymbolTable *symbolTable, const char *variableName,
                                int functionIndex, char *outType)
{
    for (int entryIndex = 0; entryIndex < symbolTable->count; entryIndex++)
    {
        SymbolEntry *entry = &symbolTable->entries[entryIndex];
        if (entry->parentIndex != functionIndex)
        {
            continue;
        }
        if (!strcmp(entry->entryType, "ASSIGN"))
        {
            continue;
        }
        if (!strcmp(entry->symbolName, variableName))
        {
            strncpy(outType, entry->symbolType, 19);
            outType[19] = '\0';
            return;
        }
    }
    strcpy(outType, "<unknown>");
}

static void resolveFunctionReturnType(SymbolTable *symbolTable, const char *functionName,
                                      char *outType)
{
    for (int entryIndex = 0; entryIndex < symbolTable->count; entryIndex++)
    {
        SymbolEntry *entry = &symbolTable->entries[entryIndex];
        if (!strcmp(entry->entryType, "FUNC") && !strcmp(entry->symbolName, functionName))
        {
            strncpy(outType, entry->symbolType, 19);
            outType[19] = '\0';
            return;
        }
    }
    strcpy(outType, "<unknown>");
}

// ----- expression visitors: each sets context->lastType -----

static void visitInt(Visitor *visitor, AstNode *node)
{
    TypeCheckContext *context = visitor->context;
    (void)node;
    setLastType(context, "int");
}

static void visitVarRef(Visitor *visitor, AstNode *node)
{
    TypeCheckContext *context = visitor->context;
    resolveVariableType(context->symbolTable, node->name,
                        context->currentFunctionOrMethodIndex, context->lastType);
}

static void checkBinaryArithmetic(Visitor *visitor, AstNode *node, const char *operatorLabel)
{
    TypeCheckContext *context = visitor->context;

    astAccept(node->lOperand, visitor);
    char leftType[20];
    strncpy(leftType, context->lastType, sizeof leftType - 1);
    leftType[sizeof leftType - 1] = '\0';

    astAccept(node->rOperand, visitor);
    char rightType[20];
    strncpy(rightType, context->lastType, sizeof rightType - 1);
    rightType[sizeof rightType - 1] = '\0';

    if (strcmp(leftType, rightType) != 0)
    {
        fprintf(stderr, "type error: %s of '%s' and '%s'\n", operatorLabel, leftType, rightType);
        context->errorCount++;
        setLastType(context, "<error>");
    }
    else
    {
        setLastType(context, leftType);
    }
}

static void visitAdd(Visitor *visitor, AstNode *node) { checkBinaryArithmetic(visitor, node, "add"); }
static void visitSub(Visitor *visitor, AstNode *node) { checkBinaryArithmetic(visitor, node, "sub"); }
static void visitMul(Visitor *visitor, AstNode *node) { checkBinaryArithmetic(visitor, node, "mul"); }

static void visitLogNeg(Visitor *visitor, AstNode *node)
{
    TypeCheckContext *context = visitor->context;
    astAccept(node->rOperand, visitor);
    setLastType(context, "int");
}

static void visitFuncCall(Visitor *visitor, AstNode *node)
{
    TypeCheckContext *context = visitor->context;
    if (node->hasArgument)
    {
        astAccept(node->rOperand, visitor);
    }
    resolveFunctionReturnType(context->symbolTable, node->name, context->lastType);
}

static void visitMethodCall(Visitor *visitor, AstNode *node)
{
    TypeCheckContext *context = visitor->context;
    if (node->hasArgument)
    {
        astAccept(node->rOperand, visitor);
    }
    // Method's mangled name was stored in node->name at AST construction; here we
    // recompute it from receiver + method to look up the METHOD entry's return type.
    char mangledMethodName[40];
    snprintf(mangledMethodName, sizeof mangledMethodName, "%s_%s",
             node->className ? node->className : "", node->methodName ? node->methodName : "");
    setLastType(context, "<unknown>");
    for (int entryIndex = 0; entryIndex < context->symbolTable->count; entryIndex++)
    {
        SymbolEntry *entry = &context->symbolTable->entries[entryIndex];
        if (!strcmp(entry->entryType, "METHOD") && !strcmp(entry->symbolName, mangledMethodName))
        {
            setLastType(context, entry->symbolType);
            return;
        }
    }
}

// ----- non-expression visitors: recurse where needed, no type to set -----

static void visitDecl(Visitor *visitor, AstNode *node) { (void)visitor; (void)node; }

static void visitDeclAssign(Visitor *visitor, AstNode *node)
{
    astAccept(node->rOperand, visitor);
}

static void visitAssign(Visitor *visitor, AstNode *node)
{
    astAccept(node->rOperand, visitor);
}

static void visitIncrement(Visitor *visitor, AstNode *node) { (void)visitor; (void)node; }
static void visitDoBegin(Visitor *visitor, AstNode *node) { (void)visitor; (void)node; }

static void visitWhile(Visitor *visitor, AstNode *node)
{
    astAccept(node->lOperand, visitor);
    astAccept(node->rOperand, visitor);
}

static void visitIfBegin(Visitor *visitor, AstNode *node)
{
    astAccept(node->lOperand, visitor);
    astAccept(node->rOperand, visitor);
}

static void visitElseBegin(Visitor *visitor, AstNode *node) { (void)visitor; (void)node; }
static void visitEndIf(Visitor *visitor, AstNode *node) { (void)visitor; (void)node; }

static void visitReturn(Visitor *visitor, AstNode *node)
{
    astAccept(node->rOperand, visitor);
}

static void visitObjDecl(Visitor *visitor, AstNode *node) { (void)visitor; (void)node; }
static void visitArgDecl(Visitor *visitor, AstNode *node) { (void)visitor; (void)node; }

static void visitFuncDecl(Visitor *visitor, AstNode *node)
{
    TypeCheckContext *context = visitor->context;
    int previousFunctionIndex = context->currentFunctionOrMethodIndex;
    context->currentFunctionOrMethodIndex = node->symbolTableIndex;
    for (int bodyIndex = 0; bodyIndex < node->body.count; bodyIndex++)
    {
        astAccept(node->body.items[bodyIndex], visitor);
    }
    context->currentFunctionOrMethodIndex = previousFunctionIndex;
}

static void visitMethodDecl(Visitor *visitor, AstNode *node)
{
    TypeCheckContext *context = visitor->context;
    int previousFunctionIndex = context->currentFunctionOrMethodIndex;
    context->currentFunctionOrMethodIndex = node->symbolTableIndex;
    for (int bodyIndex = 0; bodyIndex < node->body.count; bodyIndex++)
    {
        astAccept(node->body.items[bodyIndex], visitor);
    }
    context->currentFunctionOrMethodIndex = previousFunctionIndex;
}

static void visitClassDecl(Visitor *visitor, AstNode *node) { (void)visitor; (void)node; }

Visitor *makeTypeCheckVisitor(SymbolTable *symbolTable)
{
    static Visitor typeCheckVisitor;

    typeCheckContext.symbolTable = symbolTable;
    typeCheckContext.currentFunctionOrMethodIndex = -1;
    typeCheckContext.lastType[0] = '\0';
    typeCheckContext.errorCount = 0;

    typeCheckVisitor.context = &typeCheckContext;
    typeCheckVisitor.visitInt = visitInt;
    typeCheckVisitor.visitVarRef = visitVarRef;
    typeCheckVisitor.visitAdd = visitAdd;
    typeCheckVisitor.visitSub = visitSub;
    typeCheckVisitor.visitMul = visitMul;
    typeCheckVisitor.visitLogNeg = visitLogNeg;
    typeCheckVisitor.visitDecl = visitDecl;
    typeCheckVisitor.visitDeclAssign = visitDeclAssign;
    typeCheckVisitor.visitAssign = visitAssign;
    typeCheckVisitor.visitIncrement = visitIncrement;
    typeCheckVisitor.visitDoBegin = visitDoBegin;
    typeCheckVisitor.visitWhile = visitWhile;
    typeCheckVisitor.visitIfBegin = visitIfBegin;
    typeCheckVisitor.visitElseBegin = visitElseBegin;
    typeCheckVisitor.visitEndIf = visitEndIf;
    typeCheckVisitor.visitFuncCall = visitFuncCall;
    typeCheckVisitor.visitMethodCall = visitMethodCall;
    typeCheckVisitor.visitReturn = visitReturn;
    typeCheckVisitor.visitObjDecl = visitObjDecl;
    typeCheckVisitor.visitArgDecl = visitArgDecl;
    typeCheckVisitor.visitFuncDecl = visitFuncDecl;
    typeCheckVisitor.visitMethodDecl = visitMethodDecl;
    typeCheckVisitor.visitClassDecl = visitClassDecl;
    return &typeCheckVisitor;
}

int typeCheckErrorCount(void)
{
    return typeCheckContext.errorCount;
}
