// This file is licensed under the MIT License. See LICENSE for details.

#include <stdio.h>
#include <string.h>
#include "ast.h"
#include "visitor.h"
#include "constfold.h"

#define CONST_PROP_MAX_BINDINGS 32

typedef struct
{
    const char *variableName;          // borrowed pointer into the AST
    int constantValue;
    int isValid;                       // 0 if we've seen an assignment/increment that disqualifies it
} ConstantBinding;

typedef struct
{
    FILE *outputStream;
    int foldCount;
    int identityRewriteCount;
    int propagationRewriteCount;
} ConstFoldContext;

// ---------------------------------------------------------------------------
// Mutation helpers — keep the AST shape stable so other passes don't notice.
// ---------------------------------------------------------------------------

__attribute__((unused)) static void mutateToInt(AstNode *node, int newValue)
{
    node->kind = AST_INT;
    node->intValue = newValue;
    node->lOperand = 0;
    node->rOperand = 0;
}

// Replace `target` with a copy of `source` in place. Used by algebraic
// identities: when `x + 0` simplifies to `x`, the ADD node is overwritten
// with whatever subtree `x` is.
__attribute__((unused)) static void mutateToCopyOf(AstNode *target, AstNode *source)
{
    *target = *source;
}

__attribute__((unused)) static void reportBinaryFold(ConstFoldContext *context, const char *operatorLabel,
                             int leftValue, int rightValue, int resultValue)
{
    fprintf(context->outputStream, "  folded: %s(INT %d, INT %d) -> INT %d\n",
            operatorLabel, leftValue, rightValue, resultValue);
    context->foldCount++;
}

__attribute__((unused)) static void reportUnaryFold(ConstFoldContext *context, const char *operatorLabel,
                            int operandValue, int resultValue)
{
    fprintf(context->outputStream, "  folded: %s(INT %d) -> INT %d\n",
            operatorLabel, operandValue, resultValue);
    context->foldCount++;
}

__attribute__((unused)) static void reportIdentity(ConstFoldContext *context, const char *patternLabel)
{
    fprintf(context->outputStream, "  identity: %s\n", patternLabel);
    context->identityRewriteCount++;
}

__attribute__((unused)) static void reportPropagation(ConstFoldContext *context, const char *variableName, int constantValue)
{
    fprintf(context->outputStream, "  propagated: %s -> INT %d\n", variableName, constantValue);
    context->propagationRewriteCount++;
}

// ---------------------------------------------------------------------------
// Pass 1: constant propagation through unmodified locals.
//
// For each function we scan the body twice:
//   1. Collect tentative bindings: every `int x = <INT literal>;` is a
//      candidate. Any later AST_ASSIGN or AST_INCREMENT to x invalidates
//      its binding.
//   2. Rewrite: every AST_VAR_REF whose name has a still-valid binding is
//      replaced with the literal AST_INT.
// ---------------------------------------------------------------------------

static int findBinding(ConstantBinding *bindings, int bindingCount, const char *variableName)
{
    if (!variableName)
    {
        return -1;
    }
    for (int bindingIndex = 0; bindingIndex < bindingCount; bindingIndex++)
    {
        if (!strcmp(bindings[bindingIndex].variableName, variableName))
        {
            return bindingIndex;
        }
    }
    return -1;
}

__attribute__((unused)) static void invalidateBinding(ConstantBinding *bindings, int bindingCount, const char *variableName)
{
    int bindingIndex = findBinding(bindings, bindingCount, variableName);
    if (bindingIndex >= 0)
    {
        bindings[bindingIndex].isValid = 0;
    }
}

// Scan one statement, updating the bindings table.
//
// TODO(Assignment 6):
//   AST_DECL_ASSIGN where rOperand->kind == AST_INT
//       -> append a new binding {name, intValue, isValid=1}
//   AST_ASSIGN     -> invalidateBinding(name)
//   AST_INCREMENT  -> invalidateBinding(name)
//   Everything else -> do nothing
static void collectBindingsFromStatement(AstNode *statementNode,
                                         ConstantBinding *bindings, int *bindingCount)
{
    // REMOVE the placeholder line(s) below when you implement this:
    (void)statementNode;
    (void)bindings;
    (void)bindingCount;
}

// Walk the AST and replace AST_VAR_REF nodes whose name is in `bindings`
// with the bound literal value. Recurses through the expression shapes
// the language defines.
//
// TODO(Assignment 6):
//   AST_VAR_REF: if findBinding returns valid binding, report and mutateToInt.
//   AST_ADD/SUB/MUL/WHILE/IF_BEGIN: recurse into lOperand and rOperand.
//   AST_LOG_NEG/RETURN/DECL_ASSIGN/ASSIGN: recurse into rOperand.
//   AST_FUNC_CALL/METHOD_CALL: if hasArgument, recurse into rOperand.
static void rewriteWithBindings(AstNode *node, ConstantBinding *bindings, int bindingCount,
                                ConstFoldContext *context)
{
    // REMOVE the placeholder line(s) below when you implement this:
    (void)node;
    (void)bindings;
    (void)bindingCount;
    (void)context;
}

static void runConstantPropagationOnFunction(AstNode *functionDeclNode, ConstFoldContext *context)
{
    if (!functionDeclNode)
    {
        return;
    }
    ConstantBinding bindings[CONST_PROP_MAX_BINDINGS];
    int bindingCount = 0;
    AstList *body = &functionDeclNode->body;

    for (int statementIndex = 0; statementIndex < body->count; statementIndex++)
    {
        collectBindingsFromStatement(body->items[statementIndex], bindings, &bindingCount);
    }
    for (int statementIndex = 0; statementIndex < body->count; statementIndex++)
    {
        rewriteWithBindings(body->items[statementIndex], bindings, bindingCount, context);
    }
}

// ---------------------------------------------------------------------------
// Pass 2: constant folding + algebraic identities.
// ---------------------------------------------------------------------------

static void foldIntLiteral(Visitor *visitor, AstNode *node)
{
    (void)visitor;
    (void)node;
}

static void foldVariableReference(Visitor *visitor, AstNode *node)
{
    (void)visitor;
    (void)node;
}

static void foldAdd(Visitor *visitor, AstNode *node)
{
    // TODO(Assignment 6): Fold ADD when both operands are AST_INT
    //                     (reportBinaryFold + mutateToInt).
    // TODO(Assignment 6): Algebraic identity "x + 0 -> x":
    //                     when rOperand is AST_INT 0,
    //                     reportIdentity + mutateToCopyOf(node, node->lOperand).
    // TODO(Assignment 6): Algebraic identity "0 + x -> x" (mirror case).
    astAccept(node->lOperand, visitor);
    astAccept(node->rOperand, visitor);
}

static void foldSub(Visitor *visitor, AstNode *node)
{
    // TODO(Assignment 6): Fold SUB when both operands are AST_INT.
    // TODO(Assignment 6): Algebraic identity "x - 0 -> x".
    astAccept(node->lOperand, visitor);
    astAccept(node->rOperand, visitor);
}

static void foldMul(Visitor *visitor, AstNode *node)
{
    // TODO(Assignment 6): Fold MUL when both operands are AST_INT.
    // TODO(Assignment 6): Algebraic identity "x * 0 -> 0" (either side zero).
    // TODO(Assignment 6): Algebraic identity "x * 1 -> x" and "1 * x -> x".
    astAccept(node->lOperand, visitor);
    astAccept(node->rOperand, visitor);
}

static void foldLogicalNeg(Visitor *visitor, AstNode *node)
{
    // TODO(Assignment 6): Fold LOG_NEG when its operand is AST_INT
    //                     (result is 1 if value is 0, else 0).
    // TODO(Assignment 6): Algebraic identity "!!x -> x":
    //                     when rOperand is AST_LOG_NEG, mutateToCopyOf
    //                     the inner operand.
    astAccept(node->rOperand, visitor);
}

static void foldDeclaration(Visitor *visitor, AstNode *node)
{
    (void)visitor;
    (void)node;
}

static void foldDeclarationWithAssignment(Visitor *visitor, AstNode *node)
{
    astAccept(node->rOperand, visitor);
}

static void foldAssignment(Visitor *visitor, AstNode *node)
{
    astAccept(node->rOperand, visitor);
}

static void foldIncrement(Visitor *visitor, AstNode *node)
{
    (void)visitor;
    (void)node;
}

static void foldDoBegin(Visitor *visitor, AstNode *node)
{
    (void)visitor;
    (void)node;
}

static void foldWhile(Visitor *visitor, AstNode *node)
{
    astAccept(node->lOperand, visitor);
    astAccept(node->rOperand, visitor);
}

static void foldIfBegin(Visitor *visitor, AstNode *node)
{
    astAccept(node->lOperand, visitor);
    astAccept(node->rOperand, visitor);
}

static void foldElseBegin(Visitor *visitor, AstNode *node)
{
    (void)visitor;
    (void)node;
}

static void foldEndIf(Visitor *visitor, AstNode *node)
{
    (void)visitor;
    (void)node;
}

static void foldFunctionCall(Visitor *visitor, AstNode *node)
{
    if (node->hasArgument)
    {
        astAccept(node->rOperand, visitor);
    }
}

static void foldMethodCall(Visitor *visitor, AstNode *node)
{
    if (node->hasArgument)
    {
        astAccept(node->rOperand, visitor);
    }
}

static void foldReturn(Visitor *visitor, AstNode *node)
{
    astAccept(node->rOperand, visitor);
}

static void foldObjectDeclaration(Visitor *visitor, AstNode *node)
{
    (void)visitor;
    (void)node;
}

static void foldArgumentDeclaration(Visitor *visitor, AstNode *node)
{
    (void)visitor;
    (void)node;
}

static void foldFunctionDeclaration(Visitor *visitor, AstNode *node)
{
    for (int bodyIndex = 0; bodyIndex < node->body.count; bodyIndex++)
    {
        astAccept(node->body.items[bodyIndex], visitor);
    }
}

static void foldMethodDeclaration(Visitor *visitor, AstNode *node)
{
    for (int bodyIndex = 0; bodyIndex < node->body.count; bodyIndex++)
    {
        astAccept(node->body.items[bodyIndex], visitor);
    }
}

static void foldClassDeclaration(Visitor *visitor, AstNode *node)
{
    (void)visitor;
    (void)node;
}

// ---------------------------------------------------------------------------
// Driver: run propagation per function, then run the fold visitor across
// the program. Folding after propagation lets the cascaded literal nodes
// collapse — e.g. `int x = 5; return x + 1;` becomes `return 6;`.
// ---------------------------------------------------------------------------

void constFold(AstList *program, FILE *outputStream)
{
    static ConstFoldContext constFoldContext;
    static Visitor foldVisitor;

    constFoldContext.outputStream = outputStream;
    constFoldContext.foldCount = 0;
    constFoldContext.identityRewriteCount = 0;
    constFoldContext.propagationRewriteCount = 0;

    fprintf(outputStream, "\n***** Constant Propagation *****\n");
    for (int topLevelDeclIndex = 0; topLevelDeclIndex < program->count; topLevelDeclIndex++)
    {
        AstNode *topLevelDecl = program->items[topLevelDeclIndex];
        if (!topLevelDecl)
        {
            continue;
        }
        if (topLevelDecl->kind == AST_FUNC_DECL || topLevelDecl->kind == AST_METHOD_DECL)
        {
            runConstantPropagationOnFunction(topLevelDecl, &constFoldContext);
        }
    }
    if (constFoldContext.propagationRewriteCount == 0)
    {
        fprintf(outputStream, "No constant propagation applied\n");
    }
    else
    {
        fprintf(outputStream, "(%d variable reference%s propagated)\n",
                constFoldContext.propagationRewriteCount,
                constFoldContext.propagationRewriteCount == 1 ? "" : "s");
    }

    foldVisitor.context = &constFoldContext;
    foldVisitor.visitInt = foldIntLiteral;
    foldVisitor.visitVarRef = foldVariableReference;
    foldVisitor.visitAdd = foldAdd;
    foldVisitor.visitSub = foldSub;
    foldVisitor.visitMul = foldMul;
    foldVisitor.visitLogNeg = foldLogicalNeg;
    foldVisitor.visitDecl = foldDeclaration;
    foldVisitor.visitDeclAssign = foldDeclarationWithAssignment;
    foldVisitor.visitAssign = foldAssignment;
    foldVisitor.visitIncrement = foldIncrement;
    foldVisitor.visitDoBegin = foldDoBegin;
    foldVisitor.visitWhile = foldWhile;
    foldVisitor.visitIfBegin = foldIfBegin;
    foldVisitor.visitElseBegin = foldElseBegin;
    foldVisitor.visitEndIf = foldEndIf;
    foldVisitor.visitFuncCall = foldFunctionCall;
    foldVisitor.visitMethodCall = foldMethodCall;
    foldVisitor.visitReturn = foldReturn;
    foldVisitor.visitObjDecl = foldObjectDeclaration;
    foldVisitor.visitArgDecl = foldArgumentDeclaration;
    foldVisitor.visitFuncDecl = foldFunctionDeclaration;
    foldVisitor.visitMethodDecl = foldMethodDeclaration;
    foldVisitor.visitClassDecl = foldClassDeclaration;

    fprintf(outputStream, "\n***** Constant Folding + Algebraic Identities *****\n");
    for (int topLevelDeclIndex = 0; topLevelDeclIndex < program->count; topLevelDeclIndex++)
    {
        astAccept(program->items[topLevelDeclIndex], &foldVisitor);
    }

    if (constFoldContext.foldCount == 0 && constFoldContext.identityRewriteCount == 0)
    {
        fprintf(outputStream, "No folding or identity rewrites applied\n");
    }
    else
    {
        fprintf(outputStream, "(%d literal fold%s, %d identity rewrite%s)\n",
                constFoldContext.foldCount,
                constFoldContext.foldCount == 1 ? "" : "s",
                constFoldContext.identityRewriteCount,
                constFoldContext.identityRewriteCount == 1 ? "" : "s");
    }
}
