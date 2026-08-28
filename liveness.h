// This file is licensed under the MIT License. See LICENSE for details.

#ifndef LIVENESS_H
#define LIVENESS_H

#include <stdio.h>
#include "ast.h"

// Per-function liveness analysis over named variables.
//
// Treat each top-level statement in the function body as one "program
// point." For each statement s:
//
//     use[s]  = names this statement reads (before any write)
//     def[s]  = names this statement writes
//
// Iterate to fixed point on the two equations:
//
//     live_in[s]  = use[s] ∪ (live_out[s] − def[s])
//     live_out[s] = ∪ live_in[succ]   for succ in successors(s)
//
// Successors: just s+1, except an AST_WHILE has TWO — s+1 (loop exit
// fallthrough) and matchingDoBegin (back-edge to the loop header).
// The back-edge is the only reason iteration is needed; without loops
// one backward pass would converge.

// Fixed upper bounds — kept small and static for pedagogy.
#define LIVE_MAX_STATEMENTS 128
#define LIVE_MAX_VARIABLES  32

typedef struct
{
    int statementCount;
    int variableCount;

    // Variable names, indexed 0..variableCount-1. Owned by the result.
    char *variableNames[LIVE_MAX_VARIABLES];

    // Bitmaps, indexed [statement][variable]. 1 = in the set.
    int useSet [LIVE_MAX_STATEMENTS][LIVE_MAX_VARIABLES];
    int defSet [LIVE_MAX_STATEMENTS][LIVE_MAX_VARIABLES];
    int liveIn [LIVE_MAX_STATEMENTS][LIVE_MAX_VARIABLES];
    int liveOut[LIVE_MAX_STATEMENTS][LIVE_MAX_VARIABLES];

    int iterationCount; // how many fixed-point passes ran
} LivenessResult;

// Build a fresh liveness result for one function/method. Caller frees.
LivenessResult *livenessAnalyze(AstNode *functionDeclNode);

// 1 if the named variable is in live_out[programPoint], 0 otherwise.
int livenessIsVarLiveOut(LivenessResult *result, int programPoint, const char *variableName);

// Human-readable dump of use/def/in/out per statement.
void livenessDump(LivenessResult *result, FILE *outputStream, const char *functionName);

void livenessFree(LivenessResult *result);

#endif
