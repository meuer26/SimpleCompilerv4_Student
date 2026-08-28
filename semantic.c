// This file is licensed under the MIT License. See LICENSE for details.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "visitor.h"
#include "symboltable.h"
#include "semantic.h"

typedef struct
{
    SymbolTable *symbolTable;
    int currentFunctionOrMethodIndex;  // -1 outside any function/method body
    int insertOrderCounter;            // monotonic, written into each entry's insertOrder
} SemanticContext;

static void visitIntLiteral(Visitor *visitor, AstNode *node)
{
    (void)visitor;
    (void)node;
}

static void visitVariableReference(Visitor *visitor, AstNode *node)
{
    (void)visitor;
    (void)node;
}

static void visitAdd(Visitor *visitor, AstNode *node)
{
    astAccept(node->lOperand, visitor);
    astAccept(node->rOperand, visitor);
}

static void visitSub(Visitor *visitor, AstNode *node)
{
    astAccept(node->lOperand, visitor);
    astAccept(node->rOperand, visitor);
}

static void visitMul(Visitor *visitor, AstNode *node)
{
    astAccept(node->lOperand, visitor);
    astAccept(node->rOperand, visitor);
}

static void visitLogicalNeg(Visitor *visitor, AstNode *node)
{
    astAccept(node->rOperand, visitor);
}

static void visitDeclaration(Visitor *visitor, AstNode *node)
{
    SemanticContext *context = visitor->context;
    int newLvarEntryIndex = context->symbolTable->count;
    context->insertOrderCounter++;
    symInsert(context->symbolTable, "LVAR", node->typeName, node->name,
              context->insertOrderCounter, -1, 0, context->currentFunctionOrMethodIndex);
    node->symbolTableIndex = newLvarEntryIndex;
}

static void visitDeclarationWithAssignment(Visitor *visitor, AstNode *node)
{
    SemanticContext *context = visitor->context;
    int newLvarEntryIndex = context->symbolTable->count;
    context->insertOrderCounter++;
    symInsert(context->symbolTable, "LVAR", node->typeName, node->name,
              context->insertOrderCounter, -1, 0, context->currentFunctionOrMethodIndex);
    node->symbolTableIndex = newLvarEntryIndex;
    astAccept(node->rOperand, visitor);
}

static void visitAssignment(Visitor *visitor, AstNode *node)
{
    SemanticContext *context = visitor->context;
    int newAssignEntryIndex = context->symbolTable->count;
    context->insertOrderCounter++;
    symInsert(context->symbolTable, "ASSIGN", "N/A", node->name,
              context->insertOrderCounter, -1, 0, context->currentFunctionOrMethodIndex);
    node->symbolTableIndex = newAssignEntryIndex;
    astAccept(node->rOperand, visitor);
}

static void visitIncrement(Visitor *visitor, AstNode *node)
{
    (void)visitor;
    (void)node;
}

static void visitDoWhileBegin(Visitor *visitor, AstNode *node)
{
    (void)visitor;
    (void)node;
}

static void visitDoWhileEnd(Visitor *visitor, AstNode *node)
{
    astAccept(node->lOperand, visitor);
    astAccept(node->rOperand, visitor);
}

static void visitIfBegin(Visitor *visitor, AstNode *node)
{
    astAccept(node->lOperand, visitor);
    astAccept(node->rOperand, visitor);
}

static void visitElseBegin(Visitor *visitor, AstNode *node)
{
    (void)visitor;
    (void)node;
}

static void visitEndIf(Visitor *visitor, AstNode *node)
{
    (void)visitor;
    (void)node;
}

static void visitFunctionCall(Visitor *visitor, AstNode *node)
{
    if (node->hasArgument)
    {
        astAccept(node->rOperand, visitor);
    }
}

static void visitMethodCall(Visitor *visitor, AstNode *node)
{
    SemanticContext *context = visitor->context;
    // Receiver's class is recorded on its OBJ entry; copy into the AST node so codegen
    // can resolve the vtable without re-traversing the symbol table.
    const char *receiverClassName = symObjClass(context->symbolTable, node->receiverName, context->currentFunctionOrMethodIndex);
    node->className = strdup(receiverClassName);
    if (node->hasArgument)
    {
        astAccept(node->rOperand, visitor);
    }
}

static void visitReturn(Visitor *visitor, AstNode *node)
{
    astAccept(node->rOperand, visitor);
}

static void visitObjectDeclaration(Visitor *visitor, AstNode *node)
{
    SemanticContext *context = visitor->context;
    int newObjEntryIndex = context->symbolTable->count;
    context->insertOrderCounter++;
    symInsert(context->symbolTable, "OBJ", node->className, node->name,
              context->insertOrderCounter, -1, 0, context->currentFunctionOrMethodIndex);
    node->symbolTableIndex = newObjEntryIndex;
}

static void visitArgumentDeclaration(Visitor *visitor, AstNode *node)
{
    SemanticContext *context = visitor->context;
    int newArgEntryIndex = context->symbolTable->count;
    context->insertOrderCounter++;
    symInsert(context->symbolTable, "ARG", node->typeName, node->name,
              context->insertOrderCounter, -1, 0, context->currentFunctionOrMethodIndex);
    node->symbolTableIndex = newArgEntryIndex;
}

static void visitFunctionDeclaration(Visitor *visitor, AstNode *node)
{
    SemanticContext *context = visitor->context;
    int newFunctionEntryIndex = context->symbolTable->count;
    context->insertOrderCounter++;
    // FUNC self-references its own entry for parentIndex (matches legacy behavior).
    symInsert(context->symbolTable, "FUNC", node->typeName, node->name,
              context->insertOrderCounter, -1, 0, newFunctionEntryIndex);
    node->symbolTableIndex = newFunctionEntryIndex;

    int previousFunctionOrMethodIndex = context->currentFunctionOrMethodIndex;
    context->currentFunctionOrMethodIndex = newFunctionEntryIndex;
    for (int bodyIndex = 0; bodyIndex < node->body.count; bodyIndex++)
    {
        astAccept(node->body.items[bodyIndex], visitor);
    }
    context->currentFunctionOrMethodIndex = previousFunctionOrMethodIndex;
}

static void visitMethodDeclaration(Visitor *visitor, AstNode *node)
{
    SemanticContext *context = visitor->context;
    int owningClassEntryIndex = symFindClassByName(context->symbolTable, node->className);

    // Override semantics: if a method with this name already exists in the base
    // chain, reuse its vtable slot. Otherwise allocate the next free slot.
    const char *underscorePosition = strchr(node->name, '_');
    const char *unmangledMethodName = node->name;
    if (underscorePosition)
    {
        unmangledMethodName = underscorePosition + 1;
    }
    int methodVtableSlot = symFindMethodSlot(context->symbolTable, node->className, unmangledMethodName);
    if (methodVtableSlot < 0)
    {
        methodVtableSlot = symNextMethodSlot(context->symbolTable, owningClassEntryIndex);
    }

    int newMethodEntryIndex = context->symbolTable->count;
    context->insertOrderCounter++;
    symInsert(context->symbolTable, "METHOD", node->typeName, node->name,
              context->insertOrderCounter, methodVtableSlot, 0, owningClassEntryIndex);
    node->symbolTableIndex = newMethodEntryIndex;

    int previousFunctionOrMethodIndex = context->currentFunctionOrMethodIndex;
    context->currentFunctionOrMethodIndex = newMethodEntryIndex;
    for (int bodyIndex = 0; bodyIndex < node->body.count; bodyIndex++)
    {
        astAccept(node->body.items[bodyIndex], visitor);
    }
    context->currentFunctionOrMethodIndex = previousFunctionOrMethodIndex;
}

static void visitClassDeclaration(Visitor *visitor, AstNode *node)
{
    SemanticContext *context = visitor->context;
    int newClassEntryIndex = context->symbolTable->count;
    context->insertOrderCounter++;
    // CLASS self-references its own entry for parentIndex (matches legacy behavior).
    // symbolType holds the base class name ("" when none).
    symInsert(context->symbolTable, "CLASS", node->baseName, node->name,
              context->insertOrderCounter, -1, 0, newClassEntryIndex);
    node->symbolTableIndex = newClassEntryIndex;
}

Visitor *makeSemanticVisitor(SymbolTable *symbolTable)
{
    static SemanticContext semanticContext;
    static Visitor semanticVisitor;

    semanticContext.symbolTable = symbolTable;
    semanticContext.currentFunctionOrMethodIndex = -1;
    semanticContext.insertOrderCounter = 0;

    semanticVisitor.context = &semanticContext;
    semanticVisitor.visitInt = visitIntLiteral;
    semanticVisitor.visitVarRef = visitVariableReference;
    semanticVisitor.visitAdd = visitAdd;
    semanticVisitor.visitSub = visitSub;
    semanticVisitor.visitMul = visitMul;
    semanticVisitor.visitLogNeg = visitLogicalNeg;
    semanticVisitor.visitDecl = visitDeclaration;
    semanticVisitor.visitDeclAssign = visitDeclarationWithAssignment;
    semanticVisitor.visitAssign = visitAssignment;
    semanticVisitor.visitIncrement = visitIncrement;
    semanticVisitor.visitDoBegin = visitDoWhileBegin;
    semanticVisitor.visitWhile = visitDoWhileEnd;
    semanticVisitor.visitIfBegin = visitIfBegin;
    semanticVisitor.visitElseBegin = visitElseBegin;
    semanticVisitor.visitEndIf = visitEndIf;
    semanticVisitor.visitFuncCall = visitFunctionCall;
    semanticVisitor.visitMethodCall = visitMethodCall;
    semanticVisitor.visitReturn = visitReturn;
    semanticVisitor.visitObjDecl = visitObjectDeclaration;
    semanticVisitor.visitArgDecl = visitArgumentDeclaration;
    semanticVisitor.visitFuncDecl = visitFunctionDeclaration;
    semanticVisitor.visitMethodDecl = visitMethodDeclaration;
    semanticVisitor.visitClassDecl = visitClassDeclaration;
    return &semanticVisitor;
}
