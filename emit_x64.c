// This file is licensed under the MIT License. See LICENSE for details.

#include <stdio.h>
#include <string.h>
#include "ast.h"
#include "visitor.h"
#include "symboltable.h"
#include "regalloc.h"
// TODO(Assignment 5): #include "liveness.h" so this file can call
// livenessAnalyze() / livenessIsVarLiveOut() / livenessFree().


#define MAX_LIVE_VAR_VIRTUALS 64

typedef struct
{
    const char *variableName;          // borrowed pointer into the AST
    int virtualId;                     // id returned by regAllocLastVirtualId()
} VariableToVirtualMapping;

typedef struct
{
    FILE *outputFile;
    SymbolTable *symbolTable;
    RegAlloc registerAllocator;        // Chaitin allocator over ebx, ecx, r10d, r11d
    int stackSlotIndex;                // next free 4-byte slot (byte offset = *4)
    int currentLoopLabelIndex;         // current do/while label number
    int loopLabelCounter;              // bumped per visitDoWhileBegin
    int currentIfLabelIndex;           // current if label number
    int ifLabelCounter;                // bumped per visitIfBegin
    int logicalNegationLabelIndex;     // counter for zero./done. labels
    int currentFunctionSymbolIndex;
    const char *currentFunctionName;
    // TODO(Assignment 5): Add fields to integrate liveness:
    //   LivenessResult *liveness;
    //   int currentProgramPoint;
    //   VariableToVirtualMapping variableToVirtualMap[MAX_LIVE_VAR_VIRTUALS];
    //   int variableToVirtualMapCount;
    // The map tracks which virtual currently holds each named variable's
    // most recently loaded value, so the per-statement sweep can kill
    // virtuals whose variable became dead.
} X64Context;


// The two Assignment 5 helpers are shown below, commented out, as a
// reference. Enabling them — and adding the matching X64Context fields — is
// part of the assignment; see the TODO above emitFunctionHeaderAndBody.
/*
// Record that the named variable was just loaded into the most recently
// allocated virtual register. The pair is consulted at statement boundaries
// to drop dead variables' virtuals.
static void recordVariableLoadedIntoLatestVirtual(X64Context *context, const char *variableName)
{
    if (!variableName || context->variableToVirtualMapCount >= MAX_LIVE_VAR_VIRTUALS)
    {
        return;
    }
    int latestVirtualId = regAllocLastVirtualId(&context->registerAllocator);
    if (latestVirtualId < 0)
    {
        return;
    }
    int slotIndex = context->variableToVirtualMapCount;
    context->variableToVirtualMap[slotIndex].variableName = variableName;
    context->variableToVirtualMap[slotIndex].virtualId    = latestVirtualId;
    context->variableToVirtualMapCount++;
}

// After a top-level statement finishes emitting, walk the map and kill the
// virtual for any named variable that is no longer live-out at this point.
// The killed virtual no longer contributes interference edges to future
// regAllocNew() calls, which is the entire payoff of liveness analysis.
static void killDeadVariableVirtuals(X64Context *context)
{
    int writeIndex = 0;
    for (int readIndex = 0; readIndex < context->variableToVirtualMapCount; readIndex++)
    {
        VariableToVirtualMapping mapping = context->variableToVirtualMap[readIndex];
        if (!livenessIsVarLiveOut(context->liveness, context->currentProgramPoint, mapping.variableName))
        {
            regAllocKill(&context->registerAllocator, mapping.virtualId);
            continue;
        }
        context->variableToVirtualMap[writeIndex] = mapping;
        writeIndex++;
    }
    context->variableToVirtualMapCount = writeIndex;
}
*/

__attribute__((unused)) static int findVariableStackOffset(SymbolTable *symbolTable, const char *variableName, int functionIndex)
{
    for (int entryIndex = 0; entryIndex < symbolTable->count; entryIndex++)
    {
        if (symbolTable->entries[entryIndex].parentIndex != functionIndex)
        {
            continue;
        }
        if (!strcmp(symbolTable->entries[entryIndex].entryType, "ASSIGN"))
        {
            continue;
        }
        if (!strcmp(symbolTable->entries[entryIndex].symbolName, variableName))
        {
            return symbolTable->entries[entryIndex].codegenLocation;
        }
    }
    return 0;
}

__attribute__((unused)) static int findObjectStackOffset(SymbolTable *symbolTable, const char *objectName, int functionIndex)
{
    int objectEntryIndex = symFindObjInFunc(symbolTable, objectName, functionIndex);
    if (objectEntryIndex < 0)
    {
        return 0;
    }
    return symbolTable->entries[objectEntryIndex].codegenLocation;
}

// Shared, allocate a fresh register, reload it from the same slot, inc stackSlotIndex.
static void emitSpillCurrentValueToStack(X64Context *context)
{
    fprintf(context->outputFile, "    mov [rbp-%d], edi\n\n", context->stackSlotIndex * 4);
    fprintf(context->outputFile, "    mov %s, [rbp-%d]\n\n", regAllocNew(&context->registerAllocator), context->stackSlotIndex * 4);
    context->stackSlotIndex++;
}

static void visitIntLiteral(Visitor *visitor, AstNode *node)
{
    X64Context *context = visitor->context;
    fprintf(context->outputFile, "    mov edi, %d\n", node->intValue);
    emitSpillCurrentValueToStack(context);
}

static void visitVariableReference(Visitor *visitor, AstNode *node)
{
    // TODO(Assignment 1): Reload a variable from its stack slot.
    // REMOVE the placeholder line(s) below when you implement this:
    (void)visitor;
    (void)node;
}

static void visitBinaryOperation(Visitor *visitor, AstNode *node, const char *x64Opcode)
{
    X64Context *context = visitor->context;
    astAccept(node->lOperand, visitor);
    astAccept(node->rOperand, visitor);

    // Snapshot the two operand virtuals *before* emitting any instruction
    // that would shift the "latest" pointer — once we call regAllocNew
    // later in this function we'd lose track of them.
    int rightOperandVirtual = regAllocLastVirtualId(&context->registerAllocator);

    fprintf(context->outputFile, "    %s %s, %s\n", x64Opcode, regAllocSecondLast(&context->registerAllocator), regAllocLast(&context->registerAllocator));
    fprintf(context->outputFile, "    mov [rbp-%d], %s\n", context->stackSlotIndex * 4 + 4, regAllocSecondLast(&context->registerAllocator));
    fprintf(context->outputFile, "    mov edi, %s\n\n", regAllocSecondLast(&context->registerAllocator));
    fprintf(context->outputFile, "    mov %s, %s \n\n", regAllocLast(&context->registerAllocator), regAllocSecondLast(&context->registerAllocator));

    // Liveness demonstration: the right-operand virtual is dead now —
    // its value was folded into the result that lives in the left
    // operand's register. Marking it dead lets the next regAllocNew()
    // skip adding an interference edge to it, which often avoids an
    // eviction. The left operand stays live because it now holds the
    // binop result the caller will consume.
    regAllocKill(&context->registerAllocator, rightOperandVirtual);
}

static void visitAdd(Visitor *visitor, AstNode *node)
{
    visitBinaryOperation(visitor, node, "add");
}

static void visitSub(Visitor *visitor, AstNode *node)
{
    // TODO(Assignment 1): Emit x86-64 integer subtraction.
    // REMOVE the placeholder line(s) below when you implement this:
    (void)visitor;
    (void)node;
}

static void visitMul(Visitor *visitor, AstNode *node)
{
    // TODO(Assignment 1): Emit x86-64 integer multiplication.
    // REMOVE the placeholder line(s) below when you implement this:
    (void)visitor;
    (void)node;
}

static void visitLogicalNeg(Visitor *visitor, AstNode *node)
{
    // TODO(Assignment 1): Emit `!x` as a compare/branch sequence.
    // REMOVE the placeholder line(s) below when you implement this:
    (void)visitor;
    (void)node;
}

static void visitDeclaration(Visitor *visitor, AstNode *node)
{
    // TODO(Assignment 1): Reserve a stack slot for an uninitialized local.
    // REMOVE the placeholder line(s) below when you implement this:
    (void)visitor;
    (void)node;
}

static void visitDeclarationWithAssignment(Visitor *visitor, AstNode *node)
{
    // TODO(Assignment 1): Reserve a slot AND store an initializer value.
    // REMOVE the placeholder line(s) below when you implement this:
    (void)visitor;
    (void)node;
}

static void visitAssignment(Visitor *visitor, AstNode *node)
{
    // TODO(Assignment 1): Emit a store for `x = expr;`.
    // REMOVE the placeholder line(s) below when you implement this:
    (void)visitor;
    (void)node;
}

static void visitIncrement(Visitor *visitor, AstNode *node)
{
    // TODO(Assignment 2): Emit `x++;`.
    // REMOVE the placeholder line(s) below when you implement this:
    (void)visitor;
    (void)node;
}

static void visitArgumentDeclaration(Visitor *visitor, AstNode *node)
{
    // TODO(Assignment 3): Spill the first integer argument to a stack slot.
    // REMOVE the placeholder line(s) below when you implement this:
    (void)visitor;
    (void)node;
}

static void visitObjectDeclaration(Visitor *visitor, AstNode *node)
{
    // TODO(Assignment 7): Reserve space for the object's vptr and store it.
    // REMOVE the placeholder line(s) below when you implement this:
    (void)visitor;
    (void)node;
}

static void visitFunctionCall(Visitor *visitor, AstNode *node)
{
    // TODO(Assignment 4): Emit a direct call.
    // REMOVE the placeholder line(s) below when you implement this:
    (void)visitor;
    (void)node;
}

static void visitMethodCall(Visitor *visitor, AstNode *node)
{
    // TODO(Assignment 7): Emit a virtual call via the receiver's vtable.
    // REMOVE the placeholder line(s) below when you implement this:
    (void)visitor;
    (void)node;
}

static void visitReturn(Visitor *visitor, AstNode *node)
{
    X64Context *context = visitor->context;
    astAccept(node->rOperand, visitor);
    fprintf(context->outputFile, "    mov eax, edi\n");
    fprintf(context->outputFile, "    leave\n");
    fprintf(context->outputFile, "    ret\n");
}

static void visitDoWhileBegin(Visitor *visitor, AstNode *node)
{
    // TODO(Assignment 2): Emit the loop's entry label.
    // REMOVE the placeholder line(s) below when you implement this:
    (void)visitor;
    (void)node;
}

static void visitDoWhileEnd(Visitor *visitor, AstNode *node)
{
    // TODO(Assignment 2): Emit the loop's tail comparison + branch.
    // REMOVE the placeholder line(s) below when you implement this:
    (void)visitor;
    (void)node;
}

static void visitIfBegin(Visitor *visitor, AstNode *node)
{
    // TODO(Assignment 2): Bump ifLabelCounter, evaluate LHS/RHS of the condition,
    //   `cmp` the two operands, `jle else.N` (jump when !(LHS < RHS)), emit `then.N:`.
    //   Track the current label via currentIfLabelIndex. Mirror visitDoWhileBegin/End.
    // REMOVE the placeholder line(s) below when you implement this:
    (void)visitor;
    (void)node;
}

static void visitElseBegin(Visitor *visitor, AstNode *node)
{
    // TODO(Assignment 2): Emit `jmp endif.N` (terminates the then-block),
    //   then emit `else.N:`. Read the current label from currentIfLabelIndex.
    // REMOVE the placeholder line(s) below when you implement this:
    (void)visitor;
    (void)node;
}

static void visitEndIf(Visitor *visitor, AstNode *node)
{
    // TODO(Assignment 2): Emit `jmp endif.N` (terminates the else-block,
    //   or empty else-block when no else), then emit `endif.N:`.
    // REMOVE the placeholder line(s) below when you implement this:
    (void)visitor;
    (void)node;
}

static void visitClassDeclaration(Visitor *visitor, AstNode *node)
{
    (void)visitor;
    (void)node;
    // vtables live in .data; emitted up-front via x64EmitDataAndVtables()
}

// Per-function frame size: count children of this FUNC/METHOD. OBJ counts as
// 2 (8-byte vptr), the rest as 1, plus 1 for the FUNC/METHOD entry itself.
// The +1 preserves the legacy backend's per-function `location` counter which
// bumped at the end-of-rule for the FUNC entry — without it the frame ends
// up 16 bytes too small and runtime calls clobber red-zone slots holding
// caller-side locals.
static int countFunctionFrameSlots(SymbolTable *symbolTable, int functionIndex)
{
    int slotCount = 1; // the FUNC/METHOD entry itself
    for (int entryIndex = 0; entryIndex < symbolTable->count; entryIndex++)
    {
        if (symbolTable->entries[entryIndex].parentIndex != functionIndex)
        {
            continue;
        }
        if (entryIndex == functionIndex)
        {
            continue;
        }
        const char *entryType = symbolTable->entries[entryIndex].entryType;
        if (!strcmp(entryType, "OBJ"))
        {
            slotCount += 2;
        }
        else if (!strcmp(entryType, "ARG") || !strcmp(entryType, "LVAR") || !strcmp(entryType, "ASSIGN"))
        {
            slotCount += 1;
        }
    }
    return slotCount;
}

// TODO(Assignment 5): Add two helper functions above emitFunctionHeaderAndBody:
//
//   static void recordVariableLoadedIntoLatestVirtual(X64Context *context,
//                                                     const char *variableName);
//     - Look up the most recently allocated virtual via
//       regAllocLastVirtualId(&context->registerAllocator) and store the
//       (variableName, virtualId) pair into variableToVirtualMap.
//     - Call this right after each `mov %reg, [rbp-offset]` that loads a
//       named variable's value (inside visitVariableReference and
//       visitDeclarationWithAssignment).
//
//   static void killDeadVariableVirtuals(X64Context *context);
//     - Walk variableToVirtualMap; for each entry whose variableName is
//       NOT live-out at context->currentProgramPoint, call
//       regAllocKill(&context->registerAllocator, virtualId) and drop the
//       entry. Keep entries whose variable is still live.
//     - Call this from emitFunctionHeaderAndBody after each top-level
//       body statement finishes emitting.

static void emitFunctionHeaderAndBody(Visitor *visitor, AstNode *node)
{
    X64Context *context = visitor->context;
    context->currentFunctionSymbolIndex = node->symbolTableIndex;
    context->currentFunctionName = context->symbolTable->entries[node->symbolTableIndex].symbolName;
    context->stackSlotIndex = 1;
    context->loopLabelCounter = 0;
    context->ifLabelCounter = 0;
    context->logicalNegationLabelIndex = 1;
    regAllocReset(&context->registerAllocator);

    // TODO(Assignment 5): Call livenessAnalyze(node) and store the result
    // in context->liveness. Also reset context->variableToVirtualMapCount = 0
    // and context->currentProgramPoint = 0. Optionally call livenessDump for
    // tracing.

    // Assignments 1-2 emit the no-args prologue. From Assignment 3 onward
    // this routine becomes args-aware (it doesn't need to know the args by
    // name; visitArgumentDeclaration handles them). The frame is sized
    // by counting symbol-table children of this FUNC entry.
    int totalSlots = countFunctionFrameSlots(context->symbolTable, context->currentFunctionSymbolIndex);
    int frameBytes = totalSlots * 4 - 4;
    if (frameBytes < 0)
    {
        frameBytes = 0;
    }
    if (frameBytes > 0)
    {
        // System-V x86-64 requires rsp 16-byte aligned at every call boundary.
        frameBytes = ((frameBytes + 15) / 16) * 16;
    }

    fprintf(context->outputFile, "    global %sFinal:\n\n", context->currentFunctionName);
    fprintf(context->outputFile, "%sFinal:\n", context->currentFunctionName);
    fprintf(context->outputFile, "    push rbp\n");
    fprintf(context->outputFile, "    mov rbp, rsp\n");
    fprintf(context->outputFile, "    sub rsp, %d\n\n", frameBytes);

    for (int bodyIndex = 0; bodyIndex < node->body.count; bodyIndex++)
    {
        // TODO(Assignment 5): Set context->currentProgramPoint = bodyIndex
        // before emitting, and call killDeadVariableVirtuals(context) after
        // emitting. That is what makes the liveness analysis pay off.
        astAccept(node->body.items[bodyIndex], visitor);
    }

    regAllocPrintGraph(&context->registerAllocator, stderr, context->currentFunctionName);
    // TODO(Assignment 5): Call livenessFree(context->liveness) and null it out.
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

void x64EmitDataAndVtables(FILE *outputFile, SymbolTable *symbolTable)
{
    // TODO(Assignment 7): Emit a `section .data` block containing each
    // class's vtable (label + `dq <Method>Final` entries), then switch back
    // to `section .text`. Programs before Assignment 7 declare no classes,
    // so the placeholder no-op below is correct until then.
    // REMOVE the placeholder line(s) below when you implement this:
    (void)outputFile;
    (void)symbolTable;
}

Visitor *makeX64Visitor(FILE *outputFile, SymbolTable *symbolTable)
{
    static X64Context x64Context;
    static Visitor x64Visitor;

    x64Context.outputFile = outputFile;
    x64Context.symbolTable = symbolTable;
    regAllocInit(&x64Context.registerAllocator, "ebx", "ecx", "r10d", "r11d");

    x64Visitor.context = &x64Context;
    x64Visitor.visitInt = visitIntLiteral;
    x64Visitor.visitVarRef = visitVariableReference;
    x64Visitor.visitAdd = visitAdd;
    x64Visitor.visitSub = visitSub;
    x64Visitor.visitMul = visitMul;
    x64Visitor.visitLogNeg = visitLogicalNeg;
    x64Visitor.visitDecl = visitDeclaration;
    x64Visitor.visitDeclAssign = visitDeclarationWithAssignment;
    x64Visitor.visitAssign = visitAssignment;
    x64Visitor.visitIncrement = visitIncrement;
    x64Visitor.visitDoBegin = visitDoWhileBegin;
    x64Visitor.visitWhile = visitDoWhileEnd;
    x64Visitor.visitIfBegin = visitIfBegin;
    x64Visitor.visitElseBegin = visitElseBegin;
    x64Visitor.visitEndIf = visitEndIf;
    x64Visitor.visitFuncCall = visitFunctionCall;
    x64Visitor.visitMethodCall = visitMethodCall;
    x64Visitor.visitReturn = visitReturn;
    x64Visitor.visitObjDecl = visitObjectDeclaration;
    x64Visitor.visitArgDecl = visitArgumentDeclaration;
    x64Visitor.visitFuncDecl = visitFunctionDeclaration;
    x64Visitor.visitMethodDecl = visitMethodDeclaration;
    x64Visitor.visitClassDecl = visitClassDeclaration;
    return &x64Visitor;
}
