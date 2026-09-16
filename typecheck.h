// This file is licensed under the MIT License. See LICENSE for details.

#ifndef TYPECHECK_H
#define TYPECHECK_H

#include "ast.h"
#include "symboltable.h"

// Minimal type-check pass. Verifies that the operands of ADD/SUB/MUL share
// the same type (e.g. int+int = OK; int+ObjectInstance = error). Run after
// the semantic pass populates the symbol table, before codegen.
struct Visitor;
struct Visitor *makeTypeCheckVisitor(SymbolTable *symbolTable);

// Number of type errors reported so far. Cleared each time
// makeTypeCheckVisitor() is called.
int typeCheckErrorCount(void);

#endif
