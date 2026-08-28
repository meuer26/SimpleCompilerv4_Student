// This file is licensed under the MIT License. See LICENSE for details.

#include <stdio.h>
#include <string.h>
#include "ast.h"
#include "visitor.h"
#include "symboltable.h"

typedef struct
{
    FILE *outputFile;
    SymbolTable *symbolTable;
    int ssaRegisterIndex;             // next %N to allocate
    int loopLabelIndex;
    int ifLabelIndex;                 // bumped per visitIfBegin; read by Else/End
    int currentFunctionSymbolIndex;   // index of the FUNC/METHOD being visited
    char currentValueString[64];      // SSA name or literal of the just-visited expression
} LlvmContext;

__attribute__((unused)) static int findVariableSsaSlot(SymbolTable *symbolTable, const char *name, int funcIdx)
{
    for (int i = 0; i < symbolTable->count; i++)
    {
        if (symbolTable->entries[i].parentIndex != funcIdx)
        {
            continue;
        }
        if (!strcmp(symbolTable->entries[i].entryType, "ASSIGN"))
        {
            continue;
        }
        if (!strcmp(symbolTable->entries[i].symbolName, name))
        {
            return symbolTable->entries[i].codegenLocation;
        }
    }
    return -1;
}

static void visitIntLiteral(Visitor *visitor, AstNode *node)
{
    LlvmContext *context = visitor->context;
    snprintf(context->currentValueString, sizeof context->currentValueString, "%d", node->intValue);
}

static void visitVariableReference(Visitor *visitor, AstNode *node)
{
    // TODO(Assignment 1): Emit `load` IR for a variable reference.
    // REMOVE the placeholder line(s) below when you implement this:
    (void)visitor;
    (void)node;
}

static void visitBinaryOperation(Visitor *visitor, AstNode *node, const char *llvmOpcode)
{
    LlvmContext *context = visitor->context;
    char leftSideValueString[64];
    char rightSideValueString[64];
    astAccept(node->lOperand, visitor);
    strcpy(leftSideValueString, context->currentValueString);
    astAccept(node->rOperand, visitor);
    strcpy(rightSideValueString, context->currentValueString);
    int resultRegister = context->ssaRegisterIndex;
    context->ssaRegisterIndex++;
    fprintf(context->outputFile, "    %%%d = %s i32 %s, %s\n", resultRegister, llvmOpcode, leftSideValueString, rightSideValueString);
    snprintf(context->currentValueString, sizeof context->currentValueString, "%%%d", resultRegister);
}

static void visitAdd(Visitor *visitor, AstNode *node)
{
    visitBinaryOperation(visitor, node, "add");
}

static void visitSub(Visitor *visitor, AstNode *node)
{
    // TODO(Assignment 1): Emit LLVM IR for integer subtraction.
    // REMOVE the placeholder line(s) below when you implement this:
    (void)visitor;
    (void)node;
}

static void visitMul(Visitor *visitor, AstNode *node)
{
    // TODO(Assignment 1): Emit LLVM IR for integer multiplication.
    // REMOVE the placeholder line(s) below when you implement this:
    (void)visitor;
    (void)node;
}

static void visitLogicalNeg(Visitor *visitor, AstNode *node)
{
    // TODO(Assignment 1): Emit LLVM IR for logical negation `!x`.
    // REMOVE the placeholder line(s) below when you implement this:
    (void)visitor;
    (void)node;
}

static void visitDeclaration(Visitor *visitor, AstNode *node)
{
    // TODO(Assignment 1): Emit `alloca` for an uninitialized variable.
    // REMOVE the placeholder line(s) below when you implement this:
    (void)visitor;
    (void)node;
}

static void visitDeclarationWithAssignment(Visitor *visitor, AstNode *node)
{
    // TODO(Assignment 1): Emit `alloca` + `store` for `int x = expr;`.
    // REMOVE the placeholder line(s) below when you implement this:
    (void)visitor;
    (void)node;
}

static void visitAssignment(Visitor *visitor, AstNode *node)
{
    // TODO(Assignment 1): Emit a `store` for `x = expr;`.
    // REMOVE the placeholder line(s) below when you implement this:
    (void)visitor;
    (void)node;
}

static void visitIncrement(Visitor *visitor, AstNode *node)
{
    // TODO(Assignment 2): Emit load/add/store for `x++;`.
    // REMOVE the placeholder line(s) below when you implement this:
    (void)visitor;
    (void)node;
}

static void visitArgumentDeclaration(Visitor *visitor, AstNode *node)
{
    // TODO(Assignment 3): Materialize an incoming function argument.
    // REMOVE the placeholder line(s) below when you implement this:
    (void)visitor;
    (void)node;
}

static void visitObjectDeclaration(Visitor *visitor, AstNode *node)
{
    // TODO(Assignment 7): Allocate space for an object's vptr.
    // REMOVE the placeholder line(s) below when you implement this:
    (void)visitor;
    (void)node;
}

static void visitFunctionCall(Visitor *visitor, AstNode *node)
{
    // TODO(Assignment 4): Emit `call` IR for a function call.
    // REMOVE the placeholder line(s) below when you implement this:
    (void)visitor;
    (void)node;
}

static void visitMethodCall(Visitor *visitor, AstNode *node)
{
    // TODO(Assignment 7): Emit a virtual call through the receiver's vtable.
    // REMOVE the placeholder line(s) below when you implement this:
    (void)visitor;
    (void)node;
}

static void visitReturn(Visitor *visitor, AstNode *node)
{
    LlvmContext *context = visitor->context;
    astAccept(node->rOperand, visitor);
    fprintf(context->outputFile, "    ret i32 %s\n", context->currentValueString);
    fprintf(context->outputFile, "}\n\n");
}

static void visitDoWhileBegin(Visitor *visitor, AstNode *node)
{
    // TODO(Assignment 2): Emit the loop entry edge + label.
    // REMOVE the placeholder line(s) below when you implement this:
    (void)visitor;
    (void)node;
}

static void visitDoWhileEnd(Visitor *visitor, AstNode *node)
{
    // TODO(Assignment 2): Emit the loop tail comparison + conditional branch.
    // REMOVE the placeholder line(s) below when you implement this:
    (void)visitor;
    (void)node;
}

static void visitIfBegin(Visitor *visitor, AstNode *node)
{
    // TODO(Assignment 2): Bump ifLabelIndex, evaluate LHS/RHS of the condition,
    //   `icmp slt`, `br i1 %cmp, label %then.N, label %else.N`, then emit `then.N:`.
    //   Mirror visitDoWhileBegin/End for the label-counter pattern; use a separate
    //   `ifLabelIndex` field on LlvmContext.
    // REMOVE the placeholder line(s) below when you implement this:
    (void)visitor;
    (void)node;
}

static void visitElseBegin(Visitor *visitor, AstNode *node)
{
    // TODO(Assignment 2): Emit `br label %endif.N` (terminates the then-block),
    //   then emit `else.N:`. Read the current label from ifLabelIndex.
    // REMOVE the placeholder line(s) below when you implement this:
    (void)visitor;
    (void)node;
}

static void visitEndIf(Visitor *visitor, AstNode *node)
{
    // TODO(Assignment 2): Emit `br label %endif.N` (terminates the else-block,
    //   or empty else-block when no else), then emit `endif.N:`.
    // REMOVE the placeholder line(s) below when you implement this:
    (void)visitor;
    (void)node;
}

static void visitClassDeclaration(Visitor *visitor, AstNode *node)
{
    (void)visitor;
    (void)node;
    // vtables are emitted up-front via llvmEmitVtables(); no per-class IR
}

static void emitFunctionHeaderAndBody(Visitor *visitor, AstNode *node)
{
    LlvmContext *context = visitor->context;
    context->currentFunctionSymbolIndex = node->symbolTableIndex;
    context->loopLabelIndex = 0;
    context->ifLabelIndex = 0;

    // TODO(Assignment 3): Functions can now take one integer argument.
    // Detect whether any child node is an AST_ARG_DECL. If so, start SSA
    // numbering at %2 (%0 = incoming arg, %1 = implicit entry block) and
    // emit `define i32 @<name>(i32 %0) {`. Otherwise start numbering at %1
    // and emit the no-arg `define i32 @<name>() {`.
    // REMOVE the placeholder line(s) below when you implement this:
    context->ssaRegisterIndex = 1;
    fprintf(context->outputFile, "define i32 @%s() {\n", node->name);

    for (int bodyIndex = 0; bodyIndex < node->body.count; bodyIndex++)
    {
        astAccept(node->body.items[bodyIndex], visitor);
    }
}

static void visitFunctionDeclaration(Visitor *visitor, AstNode *node)
{
    emitFunctionHeaderAndBody(visitor, node);
}

static void visitMethodDeclaration(Visitor *visitor, AstNode *node)
{
    // TODO(Assignment 7): Emit a method body.
    // REMOVE the placeholder line(s) below when you implement this:
    (void)visitor;
    (void)node;
}

void llvmEmitVtables(FILE *outputFile, SymbolTable *symbolTable)
{
    // TODO(Assignment 7): Emit an LLVM vtable constant for every class
    // (walk each method slot up the inheritance chain). Programs before
    // Assignment 7 declare no classes, so the placeholder no-op below is
    // correct until then.
    // REMOVE the placeholder line(s) below when you implement this:
    (void)outputFile;
    (void)symbolTable;
}

Visitor *makeLlvmVisitor(FILE *outputFile, SymbolTable *symbolTable)
{
    static LlvmContext llvmContext;
    static Visitor llvmVisitor;

    llvmContext.outputFile = outputFile;
    llvmContext.symbolTable = symbolTable;

    llvmVisitor.context = &llvmContext;
    llvmVisitor.visitInt = visitIntLiteral;
    llvmVisitor.visitVarRef = visitVariableReference;
    llvmVisitor.visitAdd = visitAdd;
    llvmVisitor.visitSub = visitSub;
    llvmVisitor.visitMul = visitMul;
    llvmVisitor.visitLogNeg = visitLogicalNeg;
    llvmVisitor.visitDecl = visitDeclaration;
    llvmVisitor.visitDeclAssign = visitDeclarationWithAssignment;
    llvmVisitor.visitAssign = visitAssignment;
    llvmVisitor.visitIncrement = visitIncrement;
    llvmVisitor.visitDoBegin = visitDoWhileBegin;
    llvmVisitor.visitWhile = visitDoWhileEnd;
    llvmVisitor.visitIfBegin = visitIfBegin;
    llvmVisitor.visitElseBegin = visitElseBegin;
    llvmVisitor.visitEndIf = visitEndIf;
    llvmVisitor.visitFuncCall = visitFunctionCall;
    llvmVisitor.visitMethodCall = visitMethodCall;
    llvmVisitor.visitReturn = visitReturn;
    llvmVisitor.visitObjDecl = visitObjectDeclaration;
    llvmVisitor.visitArgDecl = visitArgumentDeclaration;
    llvmVisitor.visitFuncDecl = visitFunctionDeclaration;
    llvmVisitor.visitMethodDecl = visitMethodDeclaration;
    llvmVisitor.visitClassDecl = visitClassDeclaration;
    return &llvmVisitor;
}
