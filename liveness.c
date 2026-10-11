// This file is licensed under the MIT License. See LICENSE for details.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "liveness.h"

// ---------------------------------------------------------------------------
// Variable table: a tiny set of names, looked up linearly. Keeps the bitmap
// columns in sync across use/def/in/out sets.
// ---------------------------------------------------------------------------

__attribute__((unused)) static int findOrAddVariable(LivenessResult *result, const char *name)
{
    if (!name)
    {
        return -1;
    }
    for (int variableIndex = 0; variableIndex < result->variableCount; variableIndex++)
    {
        if (!strcmp(result->variableNames[variableIndex], name))
        {
            return variableIndex;
        }
    }
    if (result->variableCount >= LIVE_MAX_VARIABLES)
    {
        return -1;
    }
    int newIndex = result->variableCount;
    result->variableNames[newIndex] = strdup(name);
    result->variableCount++;
    return newIndex;
}

static int findVariable(LivenessResult *result, const char *name)
{
    if (!name)
    {
        return -1;
    }
    for (int variableIndex = 0; variableIndex < result->variableCount; variableIndex++)
    {
        if (!strcmp(result->variableNames[variableIndex], name))
        {
            return variableIndex;
        }
    }
    return -1;
}

// ---------------------------------------------------------------------------
// USE/DEF collection.
// ---------------------------------------------------------------------------

// Walk an expression and mark every named variable it reads.
__attribute__((unused)) static void collectExpressionUses(AstNode *expressionNode, LivenessResult *result, int statementIndex)
{

    if (!expressionNode)
    {
        return;
    }
    switch (expressionNode->kind)
    {
        case AST_VAR_REF:
        {
            int variableIndex = findOrAddVariable(result, expressionNode->name);
            if (variableIndex >= 0)
            {
                result->useSet[statementIndex][variableIndex] = 1;
            }
            break;
        }
        case AST_ADD:
        case AST_SUB:
        case AST_MUL:
            collectExpressionUses(expressionNode->lOperand, result, statementIndex);
            collectExpressionUses(expressionNode->rOperand, result, statementIndex);
            break;
        case AST_LOG_NEG:
            collectExpressionUses(expressionNode->rOperand, result, statementIndex);
            break;
        case AST_FUNC_CALL:
        case AST_METHOD_CALL:
            if (expressionNode->hasArgument)
            {
                collectExpressionUses(expressionNode->rOperand, result, statementIndex);
            }
            break;
        default:
            break;
    }

}

// Fill use[s] and def[s] for one statement.
// TODO(Assignment 5):
//   AST_DECL / AST_ARG_DECL          -> def = {this name}
//   AST_DECL_ASSIGN / AST_ASSIGN     -> use = vars in rOperand; def = {this name}
//   AST_INCREMENT                    -> use = {this name}; def = {this name}
//   AST_RETURN                       -> use = vars in rOperand
//   AST_WHILE                        -> use = vars in lOperand + rOperand
//   AST_DO_BEGIN                     -> marker, no use, no def
static void computeUseAndDef(AstNode *statementNode, LivenessResult *result, int statementIndex)
{
    // REMOVE the placeholder line(s) below when you implement this:
    (void)statementNode;
    (void)result;
    (void)statementIndex;
}

// ---------------------------------------------------------------------------
// Successors. Without if/else, every statement falls through to s+1; the
// only exception is AST_WHILE, which has a back-edge to its matching
// AST_DO_BEGIN as well as the fallthrough.
// ---------------------------------------------------------------------------

// For every AST_WHILE at position s, set whileToDoBegin[s] to the position
// of the matching AST_DO_BEGIN. Use a tiny integer stack to handle nested
// loops. For non-AST_WHILE statements leave the entry at -1.
static void buildLoopPairs(AstList *body, int *whileToDoBegin)
{
    int stack[LIVE_MAX_STATEMENTS];
    int stackTop = 0;
    for (int statementIndex = 0; statementIndex < body->count; statementIndex++)
    {
        whileToDoBegin[statementIndex] = -1;
        AstNode *statementNode = body->items[statementIndex];
        if (!statementNode)
        {
            continue;
        }
        if (statementNode->kind == AST_DO_BEGIN)
        {
            stack[stackTop] = statementIndex;
            stackTop++;
        }
        else if (statementNode->kind == AST_WHILE)
        {
            if (stackTop > 0)
            {
                stackTop--;
                whileToDoBegin[statementIndex] = stack[stackTop];
            }
        }
    }
}

// Fill `outSuccessors` with the successor indices of statement s.
// Returns the number of successors (0, 1, or 2).
//
// TODO(Assignment 5):
//   If statementNode is AST_WHILE: successors = {matching DO_BEGIN,
//   statementIndex+1}. Skip either if out of range.
//   Otherwise: successor = {statementIndex+1} if in range, else none.
__attribute__((unused)) static int successorsOf(int statementIndex, int statementCount, AstNode *statementNode,
                        int *whileToDoBegin, int *outSuccessors)
{
    // REMOVE the placeholder line(s) below when you implement this:
    (void)statementIndex;
    (void)statementCount;
    (void)statementNode;
    (void)whileToDoBegin;
    (void)outSuccessors;
    return 0;
}

// ---------------------------------------------------------------------------
// Fixed-point iteration.
// ---------------------------------------------------------------------------

__attribute__((unused)) static int bitmapsEqual(int *leftBitmap, int *rightBitmap, int variableCount)
{
    for (int variableIndex = 0; variableIndex < variableCount; variableIndex++)
    {
        if (leftBitmap[variableIndex] != rightBitmap[variableIndex])
        {
            return 0;
        }
    }
    return 1;
}

// Iterate backward over the statements, recomputing live_in and live_out
// from the equations in the header, until nothing changes. Bump
// iterationCount once per outer pass so the dump can show convergence.
//
// TODO(Assignment 5): Implement the outer "while (changed)" loop.
//   For each statement (backward):
//     1. Snapshot previous liveIn/liveOut for change detection.
//     2. live_out[s] = union over successors of their live_in[succ].
//     3. live_in[s]  = use[s] OR (live_out[s] AND NOT def[s]), per variable.
//     4. If either bitmap differs from snapshot, mark changed=1.
static void runFixedPoint(LivenessResult *result, AstList *body, int *whileToDoBegin)
{
    // REMOVE the placeholder line(s) below when you implement this:
    (void)result;
    (void)body;
    (void)whileToDoBegin;
}

// ---------------------------------------------------------------------------
// Public entry points.
// ---------------------------------------------------------------------------

LivenessResult *livenessAnalyze(AstNode *functionDeclNode)
{
    LivenessResult *result = calloc(1, sizeof(LivenessResult));
    if (!functionDeclNode)
    {
        return result;
    }

    AstList *body = &functionDeclNode->body;
    int statementCount = body->count;
    if (statementCount > LIVE_MAX_STATEMENTS)
    {
        statementCount = LIVE_MAX_STATEMENTS;
    }
    result->statementCount = statementCount;

    // Pass 1: discover variables, fill use[s] / def[s].
    for (int statementIndex = 0; statementIndex < statementCount; statementIndex++)
    {
        computeUseAndDef(body->items[statementIndex], result, statementIndex);
    }

    // Pass 2: pair every AST_WHILE with its matching AST_DO_BEGIN.
    int whileToDoBegin[LIVE_MAX_STATEMENTS];
    buildLoopPairs(body, whileToDoBegin);

    // Pass 3: iterate the dataflow equations to fixed point.
    runFixedPoint(result, body, whileToDoBegin);

    return result;
}

int livenessIsVarLiveOut(LivenessResult *result, int programPoint, const char *variableName)
{
    if (!result || programPoint < 0 || programPoint >= result->statementCount)
    {
        return 0;
    }
    int variableIndex = findVariable(result, variableName);
    if (variableIndex < 0)
    {
        return 0;
    }
    return result->liveOut[programPoint][variableIndex];
}

void livenessDump(LivenessResult *result, FILE *outputStream, const char *functionName)
{
    if (!result || result->statementCount == 0)
    {
        return;
    }
    fprintf(outputStream, "\n***** Liveness Analysis: %s *****\n", functionName);
    fprintf(outputStream, "%d statements, %d variables, %d fixed-point iterations\n",
            result->statementCount, result->variableCount, result->iterationCount);

    fprintf(outputStream, "Variables:");
    for (int variableIndex = 0; variableIndex < result->variableCount; variableIndex++)
    {
        fprintf(outputStream, " %s", result->variableNames[variableIndex]);
    }
    fprintf(outputStream, "\n\n");

    fprintf(outputStream, "  %-4s %-12s %-12s %-12s %-12s\n",
            "STMT", "USE", "DEF", "LIVE_IN", "LIVE_OUT");
    fprintf(outputStream, "  %-4s %-12s %-12s %-12s %-12s\n",
            "----", "------------", "------------", "------------", "------------");
    for (int statementIndex = 0; statementIndex < result->statementCount; statementIndex++)
    {
        char useBuffer [64] = "";
        char defBuffer [64] = "";
        char liveInBuffer [64] = "";
        char liveOutBuffer[64] = "";
        for (int variableIndex = 0; variableIndex < result->variableCount; variableIndex++)
        {
            if (result->useSet[statementIndex][variableIndex])
            {
                strncat(useBuffer,  result->variableNames[variableIndex], sizeof useBuffer  - strlen(useBuffer)  - 2);
                strncat(useBuffer,  " ", sizeof useBuffer  - strlen(useBuffer)  - 1);
            }
            if (result->defSet[statementIndex][variableIndex])
            {
                strncat(defBuffer,  result->variableNames[variableIndex], sizeof defBuffer  - strlen(defBuffer)  - 2);
                strncat(defBuffer,  " ", sizeof defBuffer  - strlen(defBuffer)  - 1);
            }
            if (result->liveIn[statementIndex][variableIndex])
            {
                strncat(liveInBuffer,  result->variableNames[variableIndex], sizeof liveInBuffer  - strlen(liveInBuffer)  - 2);
                strncat(liveInBuffer,  " ", sizeof liveInBuffer  - strlen(liveInBuffer)  - 1);
            }
            if (result->liveOut[statementIndex][variableIndex])
            {
                strncat(liveOutBuffer, result->variableNames[variableIndex], sizeof liveOutBuffer - strlen(liveOutBuffer) - 2);
                strncat(liveOutBuffer, " ", sizeof liveOutBuffer - strlen(liveOutBuffer) - 1);
            }
        }
        fprintf(outputStream, "  %-4d %-12s %-12s %-12s %-12s\n",
                statementIndex,
                useBuffer[0]    ? useBuffer    : "-",
                defBuffer[0]    ? defBuffer    : "-",
                liveInBuffer[0] ? liveInBuffer : "-",
                liveOutBuffer[0]? liveOutBuffer: "-");
    }
    fprintf(outputStream, "\n");
}

void livenessFree(LivenessResult *result)
{
    if (!result)
    {
        return;
    }
    for (int variableIndex = 0; variableIndex < result->variableCount; variableIndex++)
    {
        free(result->variableNames[variableIndex]);
    }
    free(result);
}
