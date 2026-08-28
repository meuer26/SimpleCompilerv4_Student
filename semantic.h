// This file is licensed under the MIT License. See LICENSE for details.

#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"
#include "symboltable.h"

struct Visitor;
struct Visitor *makeSemanticVisitor(SymbolTable *symbolTable);

#endif
