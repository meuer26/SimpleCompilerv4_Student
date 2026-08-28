// This file is licensed under the MIT License. See LICENSE for details.

#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "visitor.h"

void astListInit(AstList *list)
{
    list->items = 0;
    list->count = 0;
    list->capacity = 0;
}

void astListPush(AstList *list, AstNode *node)
{
    if (list->count == list->capacity)
    {
        if (list->capacity == 0)
        {
            list->capacity = 4;
        }
        else
        {
            list->capacity = list->capacity * 2;
        }
        list->items = realloc(list->items, list->capacity * sizeof(AstNode *));
    }
    list->items[list->count] = node;
    list->count++;
}

static AstNode *newNode(AstKind kind)
{
    AstNode *node = calloc(1, sizeof(AstNode));
    node->kind = kind;
    node->symbolTableIndex = -1;
    return node;
}

AstNode *intType(int value)
{
    AstNode *node = newNode(AST_INT);
    node->intValue = value;
    return node;
}

AstNode *stringType(char *identifierName)
{
    // TODO(Assignment 1): Construct an AST_VAR_REF node carrying identifierName.
    // REMOVE the placeholder line(s) below when you implement this:
    (void)identifierName;
    return 0;
}

AstNode *add(AstNode *leftOperand, AstNode *rightOperand)
{
    AstNode *node = newNode(AST_ADD);
    node->lOperand = leftOperand;
    node->rOperand = rightOperand;
    return node;
}

AstNode *subtract(AstNode *leftOperand, AstNode *rightOperand)
{
    // TODO(Assignment 1): Construct an AST_SUB node (mirror of add).
    // REMOVE the placeholder line(s) below when you implement this:
    (void)leftOperand;
    (void)rightOperand;
    return 0;
}

AstNode *multiply(AstNode *leftOperand, AstNode *rightOperand)
{
    // TODO(Assignment 1): Construct an AST_MUL node (mirror of add).
    // REMOVE the placeholder line(s) below when you implement this:
    (void)leftOperand;
    (void)rightOperand;
    return 0;
}

AstNode *logicalNegation(AstNode *rightOperand)
{
    // TODO(Assignment 1): Construct an AST_LOG_NEG node carrying rightOperand.
    // REMOVE the placeholder line(s) below when you implement this:
    (void)rightOperand;
    return 0;
}

AstNode *declaration(char *typeName, char *variableName)
{
    // TODO(Assignment 1): Construct an AST_DECL node carrying typeName and variableName.
    // REMOVE the placeholder line(s) below when you implement this:
    (void)typeName;
    (void)variableName;
    return 0;
}

AstNode *declarationWithAssign(char *typeName, char *variableName, AstNode *rightOperand)
{
    // TODO(Assignment 1): Construct an AST_DECL_ASSIGN node carrying typeName, variableName, and rightOperand.
    // REMOVE the placeholder line(s) below when you implement this:
    (void)typeName;
    (void)variableName;
    (void)rightOperand;
    return 0;
}

AstNode *assignment(char *variableName, AstNode *rightOperand)
{
    // TODO(Assignment 1): Construct an AST_ASSIGN node carrying variableName and rightOperand.
    // REMOVE the placeholder line(s) below when you implement this:
    (void)variableName;
    (void)rightOperand;
    return 0;
}

AstNode *increment(char *variableName)
{
    // TODO(Assignment 2): Construct an AST_INCREMENT node carrying variableName.
    // REMOVE the placeholder line(s) below when you implement this:
    (void)variableName;
    return 0;
}

AstNode *doBeginLoop(void)
{
    // TODO(Assignment 2): Construct an AST_DO_BEGIN marker node (no fields needed).
    // REMOVE the placeholder line(s) below when you implement this:
    return 0;
}

AstNode *whileLoop(AstNode *leftOperand, AstNode *rightOperand)
{
    // TODO(Assignment 2): Construct an AST_WHILE node carrying both compared operands.
    // REMOVE the placeholder line(s) below when you implement this:
    (void)leftOperand;
    (void)rightOperand;
    return 0;
}

AstNode *ifBegin(AstNode *leftOperand, AstNode *rightOperand)
{
    // TODO(Assignment 2): Construct an AST_IF_BEGIN node carrying both compared operands.
    // REMOVE the placeholder line(s) below when you implement this:
    (void)leftOperand;
    (void)rightOperand;
    return 0;
}

AstNode *elseBegin(void)
{
    // TODO(Assignment 2): Construct an AST_ELSE_BEGIN marker node (no fields needed).
    // REMOVE the placeholder line(s) below when you implement this:
    return 0;
}

AstNode *endIf(void)
{
    // TODO(Assignment 2): Construct an AST_END_IF marker node (no fields needed).
    // REMOVE the placeholder line(s) below when you implement this:
    return 0;
}

AstNode *funcCall(char *functionName)
{
    // TODO(Assignment 4): Construct an AST_FUNC_CALL node with no argument.
    // REMOVE the placeholder line(s) below when you implement this:
    (void)functionName;
    return 0;
}

AstNode *funcCallWithArg(char *functionName, AstNode *argumentExpression)
{
    // TODO(Assignment 4): Construct an AST_FUNC_CALL node with one argument expression.
    // REMOVE the placeholder line(s) below when you implement this:
    (void)functionName;
    (void)argumentExpression;
    return 0;
}

AstNode *methodCall(char *receiverName, char *className, char *methodName, AstNode *argumentExpression)
{
    // TODO(Assignment 7): Construct an AST_METHOD_CALL node carrying receiver, class, method, and optional argument.
    // REMOVE the placeholder line(s) below when you implement this:
    (void)receiverName;
    (void)className;
    (void)methodName;
    (void)argumentExpression;
    return 0;
}

AstNode *returnStmt(AstNode *rightOperand)
{
    AstNode *node = newNode(AST_RETURN);
    node->rOperand = rightOperand;
    return node;
}

AstNode *objDecl(char *objectName, char *className)
{
    // TODO(Assignment 7): Construct an AST_OBJ_DECL node carrying objectName and className.
    // REMOVE the placeholder line(s) below when you implement this:
    (void)objectName;
    (void)className;
    return 0;
}

AstNode *argDecl(char *typeName, char *argumentName)
{
    // TODO(Assignment 3): Construct an AST_ARG_DECL node carrying typeName and argumentName.
    // REMOVE the placeholder line(s) below when you implement this:
    (void)typeName;
    (void)argumentName;
    return 0;
}

AstNode *funcDecl(char *typeName, char *functionName, AstList body)
{
    AstNode *node = newNode(AST_FUNC_DECL);
    node->typeName = typeName;
    node->name = functionName;
    node->body = body;
    return node;
}

AstNode *methodDecl(char *typeName, char *className, char *mangledName, AstList body)
{
    // TODO(Assignment 7): Construct an AST_METHOD_DECL node carrying typeName, className, mangledName, and body.
    // REMOVE the placeholder line(s) below when you implement this:
    (void)typeName;
    (void)className;
    (void)mangledName;
    (void)body;
    return 0;
}

AstNode *classDecl(char *className, char *baseClassName)
{
    // TODO(Assignment 7): Construct an AST_CLASS_DECL node carrying className and baseClassName.
    // REMOVE the placeholder line(s) below when you implement this:
    (void)className;
    (void)baseClassName;
    return 0;
}

void astAccept(AstNode *node, Visitor *visitor)
{
    if (!node)
    {
        return;
    }
    switch (node->kind)
    {
        case AST_INT:
            visitor->visitInt(visitor, node);
            break;
        case AST_VAR_REF:
            visitor->visitVarRef(visitor, node);
            break;
        case AST_ADD:
            visitor->visitAdd(visitor, node);
            break;
        case AST_SUB:
            visitor->visitSub(visitor, node);
            break;
        case AST_MUL:
            visitor->visitMul(visitor, node);
            break;
        case AST_LOG_NEG:
            visitor->visitLogNeg(visitor, node);
            break;
        case AST_DECL:
            visitor->visitDecl(visitor, node);
            break;
        case AST_DECL_ASSIGN:
            visitor->visitDeclAssign(visitor, node);
            break;
        case AST_ASSIGN:
            visitor->visitAssign(visitor, node);
            break;
        case AST_INCREMENT:
            visitor->visitIncrement(visitor, node);
            break;
        case AST_DO_BEGIN:
            visitor->visitDoBegin(visitor, node);
            break;
        case AST_WHILE:
            visitor->visitWhile(visitor, node);
            break;
        case AST_IF_BEGIN:
            visitor->visitIfBegin(visitor, node);
            break;
        case AST_ELSE_BEGIN:
            visitor->visitElseBegin(visitor, node);
            break;
        case AST_END_IF:
            visitor->visitEndIf(visitor, node);
            break;
        case AST_FUNC_CALL:
            visitor->visitFuncCall(visitor, node);
            break;
        case AST_METHOD_CALL:
            visitor->visitMethodCall(visitor, node);
            break;
        case AST_RETURN:
            visitor->visitReturn(visitor, node);
            break;
        case AST_OBJ_DECL:
            visitor->visitObjDecl(visitor, node);
            break;
        case AST_ARG_DECL:
            visitor->visitArgDecl(visitor, node);
            break;
        case AST_FUNC_DECL:
            visitor->visitFuncDecl(visitor, node);
            break;
        case AST_METHOD_DECL:
            visitor->visitMethodDecl(visitor, node);
            break;
        case AST_CLASS_DECL:
            visitor->visitClassDecl(visitor, node);
            break;
    }
}
