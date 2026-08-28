// This file is licensed under the MIT License. See LICENSE for details.

#ifndef VISITOR_H
#define VISITOR_H

#include "ast.h"

typedef struct Visitor
{
    void *context;
    void (*visitInt)(struct Visitor *visitor, AstNode *node);
    void (*visitVarRef)(struct Visitor *visitor, AstNode *node);
    void (*visitAdd)(struct Visitor *visitor, AstNode *node);
    void (*visitSub)(struct Visitor *visitor, AstNode *node);
    void (*visitMul)(struct Visitor *visitor, AstNode *node);
    void (*visitLogNeg)(struct Visitor *visitor, AstNode *node);
    void (*visitDecl)(struct Visitor *visitor, AstNode *node);
    void (*visitDeclAssign)(struct Visitor *visitor, AstNode *node);
    void (*visitAssign)(struct Visitor *visitor, AstNode *node);
    void (*visitIncrement)(struct Visitor *visitor, AstNode *node);
    void (*visitDoBegin)(struct Visitor *visitor, AstNode *node);
    void (*visitWhile)(struct Visitor *visitor, AstNode *node);
    void (*visitIfBegin)(struct Visitor *visitor, AstNode *node);
    void (*visitElseBegin)(struct Visitor *visitor, AstNode *node);
    void (*visitEndIf)(struct Visitor *visitor, AstNode *node);
    void (*visitFuncCall)(struct Visitor *visitor, AstNode *node);
    void (*visitMethodCall)(struct Visitor *visitor, AstNode *node);
    void (*visitReturn)(struct Visitor *visitor, AstNode *node);
    void (*visitObjDecl)(struct Visitor *visitor, AstNode *node);
    void (*visitArgDecl)(struct Visitor *visitor, AstNode *node);
    void (*visitFuncDecl)(struct Visitor *visitor, AstNode *node);
    void (*visitMethodDecl)(struct Visitor *visitor, AstNode *node);
    void (*visitClassDecl)(struct Visitor *visitor, AstNode *node);
} Visitor;

#endif
