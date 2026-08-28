// This file is licensed under the MIT License. See LICENSE for details.

#include <stdio.h>
#include <string.h>
#include "symboltable.h"

void symInsert(SymbolTable *symbolTable, const char *entryType, const char *symbolType, const char *symbolName, int insertOrder, int vtableSlot, int size, int parentIndex)
{
    SymbolEntry *entry = &symbolTable->entries[symbolTable->count];
    strncpy(entry->entryType, entryType, sizeof(entry->entryType) - 1);
    entry->entryType[sizeof(entry->entryType) - 1] = '\0';
    strncpy(entry->symbolType, symbolType, sizeof(entry->symbolType) - 1);
    entry->symbolType[sizeof(entry->symbolType) - 1] = '\0';
    strncpy(entry->symbolName, symbolName, sizeof(entry->symbolName) - 1);
    entry->symbolName[sizeof(entry->symbolName) - 1] = '\0';
    entry->insertOrder = insertOrder;
    entry->vtableSlot = vtableSlot;
    entry->codegenLocation = 0;
    entry->size = size;
    entry->parentIndex = parentIndex;
    symbolTable->count++;
}

void symPrint(SymbolTable *symbolTable)
{
    fprintf(stderr, "\n\n***** Symbol Table Dump *****\n");
    fprintf(stderr, "%-3s  %-7s  %-20s  %-10s  %-12s  %-11s  %-15s  %-4s  %-6s\n",
            "ID", "ENTRY", "NAME", "TYPE", "INSERT_ORDER", "VTABLE_SLOT", "CODEGEN_LOC", "SIZE", "PARENT");
    fprintf(stderr, "%-3s  %-7s  %-20s  %-10s  %-12s  %-11s  %-15s  %-4s  %-6s\n",
            "---", "-------", "--------------------", "----------", "------------", "-----------", "---------------", "----", "------");
    for (int entryIndex = 0; entryIndex < symbolTable->count; entryIndex++)
    {
        SymbolEntry *entry = &symbolTable->entries[entryIndex];
        fprintf(stderr, "%-3d  %-7s  %-20s  %-10s  %-12d  %-11d  %-15d  %-4d  %-6d\n",
                entryIndex,
                entry->entryType,
                entry->symbolName,
                entry->symbolType,
                entry->insertOrder,
                entry->vtableSlot,
                entry->codegenLocation,
                entry->size,
                entry->parentIndex);
    }
    fprintf(stderr, "\n\n");
}

int symFindByNameInFunc(SymbolTable *symbolTable, const char *name, int functionIndex)
{
    for (int entryIndex = 0; entryIndex < symbolTable->count; entryIndex++)
    {
        if (symbolTable->entries[entryIndex].parentIndex != functionIndex)
        {
            continue;
        }
        if (!strcmp(symbolTable->entries[entryIndex].symbolName, name))
        {
            return entryIndex;
        }
    }
    return -1;
}

int symFindClassByName(SymbolTable *symbolTable, const char *className)
{
    for (int entryIndex = 0; entryIndex < symbolTable->count; entryIndex++)
    {
        if (strcmp(symbolTable->entries[entryIndex].entryType, "CLASS"))
        {
            continue;
        }
        if (!strcmp(symbolTable->entries[entryIndex].symbolName, className))
        {
            return entryIndex;
        }
    }
    return -1;
}

int symFindObjInFunc(SymbolTable *symbolTable, const char *objectName, int functionIndex)
{
    for (int entryIndex = 0; entryIndex < symbolTable->count; entryIndex++)
    {
        if (symbolTable->entries[entryIndex].parentIndex != functionIndex)
        {
            continue;
        }
        if (strcmp(symbolTable->entries[entryIndex].entryType, "OBJ"))
        {
            continue;
        }
        if (!strcmp(symbolTable->entries[entryIndex].symbolName, objectName))
        {
            return entryIndex;
        }
    }
    return -1;
}

int symFindMethodSlot(SymbolTable *symbolTable, const char *className, const char *methodName)
{
    int classIndex = symFindClassByName(symbolTable, className);
    while (classIndex >= 0)
    {
        for (int methodIndex = 0; methodIndex < symbolTable->count; methodIndex++)
        {
            if (strcmp(symbolTable->entries[methodIndex].entryType, "METHOD"))
            {
                continue;
            }
            if (symbolTable->entries[methodIndex].parentIndex != classIndex)
            {
                continue;
            }
            // mangled symbol name is "Class_method" — match the suffix part
            char *underscorePosition = strchr(symbolTable->entries[methodIndex].symbolName, '_');
            if (underscorePosition && !strcmp(underscorePosition + 1, methodName))
            {
                return symbolTable->entries[methodIndex].vtableSlot;
            }
        }
        const char *baseClassName = symbolTable->entries[classIndex].symbolType;
        if (!baseClassName || baseClassName[0] == '\0')
        {
            break;
        }
        classIndex = symFindClassByName(symbolTable, baseClassName);
    }
    return -1;
}

int symNextMethodSlot(SymbolTable *symbolTable, int classIndex)
{
    int highestSlotSeen = -1;
    int currentClassIndex = classIndex;
    while (currentClassIndex >= 0)
    {
        for (int methodIndex = 0; methodIndex < symbolTable->count; methodIndex++)
        {
            if (strcmp(symbolTable->entries[methodIndex].entryType, "METHOD"))
            {
                continue;
            }
            if (symbolTable->entries[methodIndex].parentIndex != currentClassIndex)
            {
                continue;
            }
            if (symbolTable->entries[methodIndex].vtableSlot > highestSlotSeen)
            {
                highestSlotSeen = symbolTable->entries[methodIndex].vtableSlot;
            }
        }
        const char *baseClassName = symbolTable->entries[currentClassIndex].symbolType;
        if (!baseClassName || baseClassName[0] == '\0')
        {
            break;
        }
        currentClassIndex = symFindClassByName(symbolTable, baseClassName);
    }
    return highestSlotSeen + 1;
}

int symVtableSize(SymbolTable *symbolTable, const char *className)
{
    int classIndex = symFindClassByName(symbolTable, className);
    if (classIndex < 0)
    {
        return 0;
    }
    return symNextMethodSlot(symbolTable, classIndex);
}

const char *symObjClass(SymbolTable *symbolTable, const char *objectName, int functionIndex)
{
    int objectIndex = symFindObjInFunc(symbolTable, objectName, functionIndex);
    if (objectIndex < 0)
    {
        return "";
    }
    return symbolTable->entries[objectIndex].symbolType;
}
