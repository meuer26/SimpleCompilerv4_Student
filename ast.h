// This file is licensed under the MIT License. See LICENSE for details.

#ifndef AST_H
#define AST_H

typedef enum
{
    AST_INT,                    // integer literal
    AST_VAR_REF,                // identifier as a value
    AST_ADD,                    // binop with lOperand + rOperand
    AST_SUB,
    AST_MUL,
    AST_LOG_NEG,                // unary !, rOperand
    AST_DECL,                   // int x;
    AST_DECL_ASSIGN,            // int x = expr;
    AST_ASSIGN,                 // x = expr;
    AST_INCREMENT,              // x++;
    AST_DO_BEGIN,               // marker: do {
    AST_WHILE,                  // while (lOperand < rOperand);
    AST_IF_BEGIN,               // if (lOperand < rOperand) {  — opens then-block
    AST_ELSE_BEGIN,             // marker: } else {  — always emitted (empty when no else)
    AST_END_IF,                 // marker: }  — closes the if/else
    AST_FUNC_CALL,              // func() or func(arg)
    AST_METHOD_CALL,            // recv.method() or recv.method(arg)
    AST_RETURN,                 // return expr;
    AST_OBJ_DECL,               // Class obj;
    AST_ARG_DECL,               // function/method argument
    AST_FUNC_DECL,              // top-level function
    AST_METHOD_DECL,            // class method (mangled name in `name`)
    AST_CLASS_DECL,             // class declaration (marker)
} AstKind;

typedef struct AstNode AstNode;

typedef struct
{
    AstNode **items;
    int count;
    int capacity;
} AstList;

struct AstNode
{
    AstKind kind;
    int intValue;             // AST_INT
    char *name;               // VAR_REF/DECL/DECL_ASSIGN/ASSIGN/INCREMENT/ARG_DECL/FUNC_DECL/METHOD_DECL/CLASS_DECL/FUNC_CALL/OBJ_DECL
    char *typeName;           // "int"/"i32" — DECL/DECL_ASSIGN/ARG_DECL/FUNC_DECL/METHOD_DECL
    char *className;          // OBJ_DECL/METHOD_DECL/METHOD_CALL
    char *baseName;           // CLASS_DECL ("" = no base)
    char *receiverName;       // METHOD_CALL (e.g. "d")
    char *methodName;         // METHOD_CALL (e.g. "speak")
    AstNode *lOperand;        // ADD/SUB/MUL/WHILE
    AstNode *rOperand;        // ADD/SUB/MUL/LOG_NOT/WHILE/DECL_ASSIGN/ASSIGN/RETURN/FUNC_CALL/METHOD_CALL
    int hasArgument;          // FUNC_CALL/METHOD_CALL — whether rOperand is set
    int symbolTableIndex;     // DECL/DECL_ASSIGN/ARG_DECL/OBJ_DECL/ASSIGN/FUNC_DECL/METHOD_DECL
    AstList body;             // FUNC_DECL/METHOD_DECL — statement list (incl. ARG_DECLs first)
};

void astListInit(AstList *list);
void astListPush(AstList *list, AstNode *node);

AstNode *intType(int value);
AstNode *stringType(char *identifierName);
AstNode *add(AstNode *leftOperand, AstNode *rightOperand);
AstNode *subtract(AstNode *leftOperand, AstNode *rightOperand);
AstNode *multiply(AstNode *leftOperand, AstNode *rightOperand);
AstNode *logicalNegation(AstNode *rightOperand);
AstNode *declaration(char *typeName, char *variableName);
AstNode *declarationWithAssign(char *typeName, char *variableName, AstNode *rightOperand);
AstNode *assignment(char *variableName, AstNode *rightOperand);
AstNode *increment(char *variableName);
AstNode *doBeginLoop(void);
AstNode *whileLoop(AstNode *leftOperand, AstNode *rightOperand);
AstNode *ifBegin(AstNode *leftOperand, AstNode *rightOperand);
AstNode *elseBegin(void);
AstNode *endIf(void);
AstNode *funcCall(char *functionName);
AstNode *funcCallWithArg(char *functionName, AstNode *argumentExpression);
AstNode *methodCall(char *receiverName, char *className, char *methodName, AstNode *argumentExpression);
AstNode *returnStmt(AstNode *rightOperand);
AstNode *objDecl(char *objectName, char *className);
AstNode *argDecl(char *typeName, char *argumentName);
AstNode *funcDecl(char *typeName, char *functionName, AstList body);
AstNode *methodDecl(char *typeName, char *className, char *mangledName, AstList body);
AstNode *classDecl(char *className, char *baseClassName);

struct Visitor;
void astAccept(AstNode *node, struct Visitor *visitor);

#endif
