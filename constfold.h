// This file is licensed under the MIT License. See LICENSE for details.

#ifndef CONSTFOLD_H
#define CONSTFOLD_H

#include <stdio.h>
#include "ast.h"

// Runs the optimization passes over `program` and writes a trace of every
// rewrite to `outputStream`. Three passes compose:
//
//   1. **Constant propagation**: per function, every `int x = <literal>;`
//      whose `x` is never reassigned (no AST_ASSIGN, no AST_INCREMENT) is
//      tracked, then every later AST_VAR_REF to `x` is replaced by the
//      literal AST_INT.
//   2. **Constant folding**: ADD/SUB/MUL of two AST_INT operands becomes
//      a single AST_INT carrying the computed value, and LOG_NEG of an
//      AST_INT becomes AST_INT 0 or 1. Cascades on top of propagation.
//   3. **Algebraic identities**: structural rewrites independent of the
//      operands' concrete values — `x + 0 -> x`, `0 + x -> x`,
//      `x - 0 -> x`, `x * 0 -> 0`, `0 * x -> 0`, `x * 1 -> x`,
//      `1 * x -> x`, `!!x -> x`. These run interleaved with folding.
void constFold(AstList *program, FILE *outputStream);

#endif
