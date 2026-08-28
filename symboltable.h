// This file is licensed under the MIT License. See LICENSE for details.

#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#define MAXSYMBOLS 100

typedef struct
{
    char entryType[20];     // "FUNC", "METHOD", "CLASS", "OBJ", "LVAR", "ARG", "ASSIGN"
    char symbolType[20];    // FUNC/METHOD: return type; CLASS: base class name; OBJ: class name
    char symbolName[20];    // identifier (mangled for METHOD: e.g. "Dog_speak")
    int insertOrder;        // monotonic insert-order counter — set by semantic pass
    int vtableSlot;         // METHOD: vtable slot index (override-aware); -1 for non-methods
    int codegenLocation;    // SSA slot (LLVM) or stack byte offset (asm) — set by codegen pass
    int size;
    int parentIndex;        // owning FUNC/METHOD/CLASS index. FUNC/METHOD self-reference
} SymbolEntry;

typedef struct
{
    int count;
    SymbolEntry entries[MAXSYMBOLS];
} SymbolTable;

void symInsert(SymbolTable *symbolTable, const char *entryType, const char *symbolType, const char *symbolName, int insertOrder, int vtableSlot, int size, int parentIndex);

void symPrint(SymbolTable *symbolTable);

// scoped lookups — return entry index or -1 when not found
int symFindByNameInFunc(SymbolTable *symbolTable, const char *name, int functionIndex);
int symFindClassByName(SymbolTable *symbolTable, const char *className);
int symFindObjInFunc(SymbolTable *symbolTable, const char *objectName, int functionIndex);

// vtable slot for (className, methodName); -1 if not found
int symFindMethodSlot(SymbolTable *symbolTable, const char *className, const char *methodName);
// next free vtable slot in chain rooted at classIndex
int symNextMethodSlot(SymbolTable *symbolTable, int classIndex);
// vtable size for className
int symVtableSize(SymbolTable *symbolTable, const char *className);
// class name of an OBJ within function scope, or "" if not found
const char *symObjClass(SymbolTable *symbolTable, const char *objectName, int functionIndex);

#endif
