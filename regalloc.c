// This file is licensed under the MIT License. See LICENSE for details.

#include <string.h>
#include "regalloc.h"

void regAllocInit(RegAlloc *allocator,
                  const char *register0,
                  const char *register1,
                  const char *register2,
                  const char *register3)
{
    allocator->physicalRegisters[0] = register0;
    allocator->physicalRegisters[1] = register1;
    allocator->physicalRegisters[2] = register2;
    allocator->physicalRegisters[3] = register3;
    regAllocReset(allocator);
}

void regAllocReset(RegAlloc *allocator)
{
    memset(allocator->interferenceMatrix, 0, sizeof(allocator->interferenceMatrix));
    for (int virtualIndex = 0; virtualIndex < REG_MAX_VIRTUAL; virtualIndex++)
    {
        allocator->colorOf[virtualIndex] = -1;
        allocator->isActive[virtualIndex] = 0;
        allocator->birthSequence[virtualIndex] = 0;
        allocator->deathSequence[virtualIndex] = 0;
        allocator->decisions[virtualIndex].chosenColor = -1;
        allocator->decisions[virtualIndex].evictedVirtual = -1;
        allocator->killEvents[virtualIndex].killedVirtual = -1;
        allocator->killEvents[virtualIndex].killSequence = 0;
    }
    allocator->killEventCount = 0;
    allocator->virtualCount = 0;
    allocator->latestSequence = 0;
    allocator->latestVirtual = -1;
    allocator->secondLatestVirtual = -1;
}

// Picks the active virtual with the smallest birthSequence — the eviction
// victim when greedy coloring runs out of colors. Returns -1 if nothing is
// active.
static int findOldestActiveVirtual(RegAlloc *allocator)
{
    int oldestVirtual = -1;
    int oldestSequenceSeen = 0;
    for (int virtualIndex = 0; virtualIndex < allocator->virtualCount; virtualIndex++)
    {
        if (!allocator->isActive[virtualIndex])
        {
            continue;
        }
        if (oldestVirtual < 0 || allocator->birthSequence[virtualIndex] < oldestSequenceSeen)
        {
            oldestVirtual = virtualIndex;
            oldestSequenceSeen = allocator->birthSequence[virtualIndex];
        }
    }
    return oldestVirtual;
}

// Classic Chaitin greedy coloring: walk candidate colors 0..3 in order; for
// each, scan the new virtual's neighbors in the adjacency matrix and check
// whether any already holds that color. Return the first color that no
// neighbor is using, or -1 when every color is blocked.
static int chooseLowestAvailableColor(RegAlloc *allocator, int virtualIndex)
{
    for (int candidateColor = 0; candidateColor < REG_NUM_PHYSICAL; candidateColor++)
    {
        int neighborConflict = 0;
        for (int neighborIndex = 0; neighborIndex < allocator->virtualCount; neighborIndex++)
        {
            if (neighborIndex == virtualIndex)
            {
                continue;
            }
            if (!allocator->interferenceMatrix[virtualIndex][neighborIndex])
            {
                continue;
            }
            if (allocator->colorOf[neighborIndex] == candidateColor)
            {
                neighborConflict = 1;
                break;
            }
        }
        if (!neighborConflict)
        {
            return candidateColor;
        }
    }
    return -1;
}

const char *regAllocNew(RegAlloc *allocator)
{
    // Create a fresh virtual register node and record when it was born.
    int newVirtual = allocator->virtualCount;
    allocator->virtualCount++;
    allocator->latestSequence++;
    allocator->birthSequence[newVirtual] = allocator->latestSequence;
    allocator->isActive[newVirtual] = 1;

    // Add interference edges between the new virtual and every currently-active
    // virtual. With no liveness pass, "currently active" stands in for
    // "still holds a value that the emitter might need."
    for (int existingVirtual = 0; existingVirtual < newVirtual; existingVirtual++)
    {
        if (!allocator->isActive[existingVirtual])
        {
            continue;
        }
        allocator->interferenceMatrix[newVirtual][existingVirtual] = 1;
        allocator->interferenceMatrix[existingVirtual][newVirtual] = 1;
    }

    AllocDecision *decision = &allocator->decisions[newVirtual];
    decision->evictedVirtual = -1;

    // Greedy-color the new virtual. If all four colors are blocked by
    // neighbors, evict the oldest active virtual and reuse its color.
    int chosenColor = chooseLowestAvailableColor(allocator, newVirtual);
    if (chosenColor < 0)
    {
        int evictedVirtual = findOldestActiveVirtual(allocator);
        if (evictedVirtual >= 0 && evictedVirtual != newVirtual)
        {
            chosenColor = allocator->colorOf[evictedVirtual];
            allocator->isActive[evictedVirtual] = 0;
            allocator->interferenceMatrix[newVirtual][evictedVirtual] = 0;
            allocator->interferenceMatrix[evictedVirtual][newVirtual] = 0;
            decision->evictedVirtual = evictedVirtual;
        }
        else
        {
            chosenColor = 0;
        }
    }
    allocator->colorOf[newVirtual] = chosenColor;
    decision->chosenColor = chosenColor;

    allocator->secondLatestVirtual = allocator->latestVirtual;
    allocator->latestVirtual = newVirtual;
    return allocator->physicalRegisters[chosenColor];
}

void regAllocPrintGraph(RegAlloc *allocator, FILE *outputStream, const char *functionName)
{
    if (allocator->virtualCount == 0)
    {
        return;
    }

    int evictionCount = 0;
    for (int virtualIndex = 0; virtualIndex < allocator->virtualCount; virtualIndex++)
    {
        if (allocator->decisions[virtualIndex].evictedVirtual >= 0)
        {
            evictionCount++;
        }
    }

    fprintf(outputStream, "\n***** Register Allocation: %s *****\n", functionName);
    fprintf(outputStream, "%d virtual registers, %d physical colors, %d evictions, %d explicit kills\n",
            allocator->virtualCount, REG_NUM_PHYSICAL, evictionCount, allocator->killEventCount);

    // ---- Decision trace: one row per regAllocNew(), showing what color the
    // allocator chose, when applicable which virtual it had to evict to free
    // that color, and (if regAllocKill was later called) the virtual's death
    // point so the live range is visible.
    fprintf(outputStream, "\nDecision trace (one row per regAllocNew call):\n");
    fprintf(outputStream, "  %-5s %-7s %-9s %s\n", "ID", "BORN_AT", "DIED_AT", "CHOSE");
    fprintf(outputStream, "  %-5s %-7s %-9s %s\n", "-----", "-------", "---------", "------------------");
    for (int virtualIndex = 0; virtualIndex < allocator->virtualCount; virtualIndex++)
    {
        const AllocDecision *decision = &allocator->decisions[virtualIndex];
        char diedAtCell[16];
        if (allocator->deathSequence[virtualIndex] > 0)
        {
            // Explicit kill via regAllocKill — record the sequence number
            // so the live range [born_at, died_at] is readable directly.
            snprintf(diedAtCell, sizeof diedAtCell, "%d",
                     allocator->deathSequence[virtualIndex]);
        }
        else if (!allocator->isActive[virtualIndex])
        {
            // Inactive without a kill timestamp means a later regAllocNew()
            // evicted this virtual to reclaim its color. The evicting
            // virtual's CHOSE column says "evict v<this>, reuse color N"
            // so the reader can correlate.
            snprintf(diedAtCell, sizeof diedAtCell, "evict");
        }
        else
        {
            // Still active when the function finished emitting — the
            // virtual's value was never freed and never displaced.
            snprintf(diedAtCell, sizeof diedAtCell, "survived");
        }
        fprintf(outputStream, "  v%-4d %-7d %-9s ",
                virtualIndex,
                allocator->birthSequence[virtualIndex],
                diedAtCell);
        if (decision->evictedVirtual >= 0)
        {
            fprintf(outputStream, "evict v%d, reuse color %d (%s)\n",
                    decision->evictedVirtual,
                    decision->chosenColor,
                    allocator->physicalRegisters[decision->chosenColor]);
        }
        else
        {
            fprintf(outputStream, "color %d (%s)\n",
                    decision->chosenColor,
                    allocator->physicalRegisters[decision->chosenColor]);
        }
    }

    // ---- Kill log: chronological list of regAllocKill() calls. Only
    // printed when at least one kill happened so the trace stays quiet for
    // backends that don't use explicit liveness yet.
    if (allocator->killEventCount > 0)
    {
        fprintf(outputStream, "\nExplicit-liveness kill log (one row per regAllocKill call):\n");
        fprintf(outputStream, "  %-5s %-9s %s\n", "ORDER", "AT_SEQ", "KILLED");
        fprintf(outputStream, "  %-5s %-9s %s\n", "-----", "---------", "------");
        for (int killIndex = 0; killIndex < allocator->killEventCount; killIndex++)
        {
            const KillEvent *event = &allocator->killEvents[killIndex];
            fprintf(outputStream, "  %-5d %-9d v%d (born at %d, lifetime = %d ticks)\n",
                    killIndex,
                    event->killSequence,
                    event->killedVirtual,
                    allocator->birthSequence[event->killedVirtual],
                    event->killSequence - allocator->birthSequence[event->killedVirtual]);
        }
    }

    fprintf(outputStream, "\n");
}

const char *regAllocLast(RegAlloc *allocator)
{
    if (allocator->latestVirtual < 0)
    {
        return allocator->physicalRegisters[0];
    }
    return allocator->physicalRegisters[allocator->colorOf[allocator->latestVirtual]];
}

const char *regAllocSecondLast(RegAlloc *allocator)
{
    if (allocator->secondLatestVirtual < 0)
    {
        return allocator->physicalRegisters[0];
    }
    return allocator->physicalRegisters[allocator->colorOf[allocator->secondLatestVirtual]];
}

int regAllocLastVirtualId(RegAlloc *allocator)
{
    return allocator->latestVirtual;
}

int regAllocSecondLastVirtualId(RegAlloc *allocator)
{
    return allocator->secondLatestVirtual;
}

void regAllocKill(RegAlloc *allocator, int virtualIndex)
{
    if (virtualIndex < 0 || virtualIndex >= allocator->virtualCount)
    {
        return;
    }
    if (!allocator->isActive[virtualIndex])
    {
        return;
    }

    // Mark dead. The interference matrix retains its historical edges
    // (a graph dump of past decisions should still show them), but
    // because future regAllocNew() calls only add edges to *active*
    // virtuals, this is enough to take the killed virtual out of the
    // interference picture going forward.
    allocator->isActive[virtualIndex] = 0;
    allocator->latestSequence++;
    allocator->deathSequence[virtualIndex] = allocator->latestSequence;

    KillEvent *event = &allocator->killEvents[allocator->killEventCount];
    event->killedVirtual = virtualIndex;
    event->killSequence = allocator->latestSequence;
    allocator->killEventCount++;
}
