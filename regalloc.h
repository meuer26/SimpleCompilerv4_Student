// This file is licensed under the MIT License. See LICENSE for details.

#ifndef REGALLOC_H
#define REGALLOC_H

#include <stdio.h>

// Chaitin-style greedy graph-coloring allocator over four physical registers.
//
// Each regAllocNew() call creates a new "virtual register" node in the
// interference graph. The graph is stored as an adjacency matrix:
// interferenceMatrix[i][j] == 1 means virtuals i and j must end up in
// different physical registers.
//
// We deliberately avoid liveness analysis. Instead, the allocator keeps an
// "active set" of virtuals that have been allocated and not yet evicted. A
// newly created virtual gets an interference edge to every currently-active
// virtual. The new virtual is then greedy-colored: scan candidate colors
// 0..3 in order, pick the first one not used by any interfering neighbor.
// If all four colors are blocked, evict the oldest active virtual and
// reclaim its color — the emitter is expected to have already spilled
// that register's value to the stack frame (matching the legacy two-register
// rotation's spill behavior).

#define REG_NUM_PHYSICAL 4
#define REG_MAX_VIRTUAL  256

// One snapshot per regAllocNew(): the color we ultimately picked and the
// virtual that had to be evicted (if any) to free that color.
typedef struct
{
    int chosenColor;                              // 0..REG_NUM_PHYSICAL-1
    int evictedVirtual;                           // -1 unless we had to evict to free a color
} AllocDecision;

// One snapshot per regAllocKill(): which virtual was marked dead and when.
// Recorded so the trace can show "v3 died at sequence 7" alongside births.
typedef struct
{
    int killedVirtual;                            // -1 if this slot unused
    int killSequence;                             // monotonic, same clock as birthSequence
} KillEvent;

typedef struct
{
    const char *physicalRegisters[REG_NUM_PHYSICAL];

    // Adjacency matrix for the interference graph. Symmetric; diagonal unused.
    int interferenceMatrix[REG_MAX_VIRTUAL][REG_MAX_VIRTUAL];

    // Greedy-coloring result: which physical register index (0..3) got
    // assigned to each virtual; -1 means uncolored.
    int colorOf[REG_MAX_VIRTUAL];

    // No-liveness proxy for "still holds a value that's needed."
    int isActive[REG_MAX_VIRTUAL];

    // Allocation time of each virtual — used to pick the oldest victim when
    // the greedy coloring step runs out of colors.
    int birthSequence[REG_MAX_VIRTUAL];

    // Death time of each virtual — 0 when still live (or never killed).
    // Set by regAllocKill(); read by regAllocPrintGraph() to show live
    // ranges in the trace.
    int deathSequence[REG_MAX_VIRTUAL];

    // Per-virtual decision trace — written by regAllocNew(), printed by
    // regAllocPrintGraph() so a reader can replay the coloring choices.
    AllocDecision decisions[REG_MAX_VIRTUAL];

    // Chronological record of regAllocKill() calls — printed alongside the
    // birth trace so the reader sees explicit-liveness events in the order
    // they happened.
    KillEvent killEvents[REG_MAX_VIRTUAL];
    int killEventCount;

    int virtualCount;
    int latestSequence;
    int latestVirtual;        // newest regAllocNew()
    int secondLatestVirtual;  // the one before that
} RegAlloc;

void regAllocInit(RegAlloc *allocator,
                  const char *register0,
                  const char *register1,
                  const char *register2,
                  const char *register3);
void regAllocReset(RegAlloc *allocator);

const char *regAllocNew(RegAlloc *allocator);
const char *regAllocLast(RegAlloc *allocator);
const char *regAllocSecondLast(RegAlloc *allocator);

// Explicit-liveness extension: the emitter marks a virtual dead when its
// value is no longer needed. A killed virtual stops contributing edges to
// future regAllocNew() calls, shrinking the interference graph. This is
// the simplest possible introduction to liveness analysis — the *where*
// to call this is exactly what a real liveness pass would compute.
//
// Safe to call with -1, an out-of-range index, or an already-dead virtual
// — those become no-ops.
void regAllocKill(RegAlloc *allocator, int virtualIndex);

// Returns the integer id of the most recently allocated virtual, or -1 if
// none have been allocated since the last reset. The emitter uses this to
// remember which virtual to kill later.
int regAllocLastVirtualId(RegAlloc *allocator);
int regAllocSecondLastVirtualId(RegAlloc *allocator);

// Dumps the per-decision trace and the final interference matrix for the
// most recently emitted function. Intended to be called at function-emit
// boundaries (before the next regAllocReset wipes the state).
void regAllocPrintGraph(RegAlloc *allocator, FILE *outputStream, const char *functionName);

#endif
