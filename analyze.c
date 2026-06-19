/*
File: analyze.c
Semantic analysis for the C-minus compiler
*/

#include <stdarg.h>

#include "analyze.h"
#include "util.h"

typedef enum
{
    SymVar,
    SymFunc
} SymbolKind;

typedef struct paramInfo
{
    ExpType type;
    int isArray;
} ParamInfo;

typedef struct scopeRec Scope;

typedef struct symbolRec
{
    char *name;
    SymbolKind kind;
    ExpType type;
    int isArray;
    int lineno;
    int isBuiltin;
    int paramCount;
    ParamInfo *params;
    Scope *scope;
    struct symbolRec *next;
} Symbol;

struct scopeRec
{
    char *name;
    TreeNode *owner;
    Scope *parent;
    Scope *firstChild;
    Scope *lastChild;
    Scope *nextSibling;
    Symbol *symbols;
};

static Scope *globalScope = NULL;
static Scope *currentScope = NULL;
static Symbol *currentFunction = NULL;

static void semanticError(int line, const char *format, ...)
{
    va_list args;

    fprintf(listing, ">>> Semantic error at line %d: ", line);
    va_start(args, format);
    vfprintf(listing, format, args);
    va_end(args);
    fprintf(listing, "\n");
    Error = TRUE;
}

static const char *typeName(ExpType type)
{
    switch (type)
    {
    case Void:
        return "void";
    case Integer:
        return "int";
    case IntegerArray:
        return "int[]";
    default:
        return "unknown";
    }
}

static Scope *newScope(const char *name, TreeNode *owner, Scope *parent)
{
    Scope *scope = (Scope *)malloc(sizeof(Scope));
    if (scope == NULL)
    {
        fprintf(listing, "Out of memory error at line %d\n", lineno);
        exit(1);
    }

    scope->name = copyString(name);
    scope->owner = owner;
    scope->parent = parent;
    scope->firstChild = NULL;
    scope->lastChild = NULL;
    scope->nextSibling = NULL;
    scope->symbols = NULL;

    if (parent != NULL)
    {
        if (parent->lastChild == NULL)
            parent->firstChild = scope;
        else
            parent->lastChild->nextSibling = scope;
        parent->lastChild = scope;
    }

    return scope;
}

static void enterScope(Scope *scope)
{
    currentScope = scope;
    if (TraceAnalyze && scope != NULL)
        fprintf(listing, "Entering scope: %s\n", scope->name);
}

static void leaveScope(void)
{
    if (TraceAnalyze && currentScope != NULL)
        fprintf(listing, "Leaving scope: %s\n", currentScope->name);
    if (currentScope != NULL)
        currentScope = currentScope->parent;
}

static Symbol *lookupInScope(Scope *scope, const char *name)
{
    Symbol *sym;

    if (scope == NULL || name == NULL)
        return NULL;

    for (sym = scope->symbols; sym != NULL; sym = sym->next)
    {
        if (strcmp(sym->name, name) == 0)
            return sym;
    }

    return NULL;
}

static Symbol *lookupSymbol(const char *name)
{
    Scope *scope;

    for (scope = currentScope; scope != NULL; scope = scope->parent)
    {
        Symbol *sym = lookupInScope(scope, name);
        if (sym != NULL)
        {
            if (TraceAnalyze)
                fprintf(listing, "Lookup: %s found in %s\n", name, scope->name);
            return sym;
        }
    }

    if (TraceAnalyze)
        fprintf(listing, "Lookup: %s not found\n", name);
    return NULL;
}

static Symbol *insertSymbol(const char *name, SymbolKind kind, ExpType type,
                            int isArray, int line, int isBuiltin)
{
    Symbol *sym;

    if (name == NULL)
        return NULL;

    sym = lookupInScope(currentScope, name);
    if (sym != NULL)
    {
        semanticError(line, "duplicate declaration of '%s'", name);
        return sym;
    }

    sym = (Symbol *)malloc(sizeof(Symbol));
    if (sym == NULL)
    {
        fprintf(listing, "Out of memory error at line %d\n", lineno);
        exit(1);
    }

    sym->name = copyString(name);
    sym->kind = kind;
    sym->type = type;
    sym->isArray = isArray;
    sym->lineno = line;
    sym->isBuiltin = isBuiltin;
    sym->paramCount = 0;
    sym->params = NULL;
    sym->scope = currentScope;
    sym->next = currentScope->symbols;
    currentScope->symbols = sym;

    if (TraceAnalyze)
    {
        fprintf(listing, "Insert: %s as %s in %s\n",
                name, kind == SymFunc ? "function" : typeName(type),
                currentScope->name);
    }

    return sym;
}

static ExpType declaredType(TreeNode *decl)
{
    if (decl == NULL)
        return Void;
    if (decl->kind.decl == VarDeclK)
    {
        if (decl->size > 0 || decl->isArray)
            return IntegerArray;
        if (decl->child[0] != NULL)
            return decl->child[0]->type;
    }
    else if (decl->kind.decl == ParamK)
    {
        if (decl->isArray)
            return IntegerArray;
        if (decl->child[0] != NULL)
            return decl->child[0]->type;
        return decl->type;
    }
    else if (decl->kind.decl == FunDeclK)
    {
        if (decl->child[0] != NULL)
            return decl->child[0]->type;
    }
    return decl->type;
}

static int isVoidParamList(TreeNode *params)
{
    return params != NULL &&
           params->nodekind == DeclK &&
           params->kind.decl == ParamK &&
           params->attr.name == NULL &&
           params->type == Void &&
           params->sibling == NULL;
}

static int countParams(TreeNode *params)
{
    int count = 0;
    TreeNode *p;

    if (isVoidParamList(params))
        return 0;

    for (p = params; p != NULL; p = p->sibling)
    {
        if (p->attr.name != NULL)
            count++;
    }
    return count;
}

static ParamInfo *copyParamInfo(TreeNode *params, int count)
{
    ParamInfo *info;
    TreeNode *p;
    int i = 0;

    if (count == 0)
        return NULL;

    info = (ParamInfo *)malloc(sizeof(ParamInfo) * count);
    if (info == NULL)
    {
        fprintf(listing, "Out of memory error at line %d\n", lineno);
        exit(1);
    }

    for (p = params; p != NULL; p = p->sibling)
    {
        if (p->attr.name == NULL)
            continue;
        info[i].isArray = p->isArray;
        info[i].type = p->isArray ? IntegerArray : declaredType(p);
        i++;
    }

    return info;
}

static Scope *findScopeForOwnerRec(Scope *scope, TreeNode *owner)
{
    Scope *child;

    if (scope == NULL)
        return NULL;
    if (scope->owner == owner)
        return scope;

    for (child = scope->firstChild; child != NULL; child = child->nextSibling)
    {
        Scope *found = findScopeForOwnerRec(child, owner);
        if (found != NULL)
            return found;
    }

    return NULL;
}

static Scope *findScopeForOwner(TreeNode *owner)
{
    return findScopeForOwnerRec(globalScope, owner);
}

static void buildStatementScopes(TreeNode *tree);

static void buildVarDecl(TreeNode *tree)
{
    ExpType type;
    int isArray;

    if (tree == NULL)
        return;

    type = declaredType(tree);
    isArray = (tree->size > 0 || tree->isArray);

    if (tree->child[0] != NULL && tree->child[0]->type == Void)
        semanticError(tree->lineno, "variable '%s' cannot have type void", tree->attr.name);

    tree->isArray = isArray;
    tree->type = isArray ? IntegerArray : type;
    insertSymbol(tree->attr.name, SymVar, tree->type, isArray, tree->lineno, FALSE);
}

static void buildParam(TreeNode *tree)
{
    ExpType type;

    if (tree == NULL || tree->attr.name == NULL)
        return;

    type = declaredType(tree);
    if (tree->child[0] != NULL && tree->child[0]->type == Void)
        semanticError(tree->lineno, "parameter '%s' cannot have type void", tree->attr.name);

    tree->type = tree->isArray ? IntegerArray : type;
    insertSymbol(tree->attr.name, SymVar, tree->type, tree->isArray, tree->lineno, FALSE);
}

static void buildCompound(TreeNode *tree, int createScope)
{
    TreeNode *decl;
    Scope *savedScope = currentScope;

    if (tree == NULL)
        return;

    if (createScope)
        enterScope(newScope("block", tree, currentScope));

    for (decl = tree->child[0]; decl != NULL; decl = decl->sibling)
        buildVarDecl(decl);

    buildStatementScopes(tree->child[1]);

    if (createScope)
        leaveScope();
    else
        currentScope = savedScope;
}

static void buildStatementScopes(TreeNode *tree)
{
    TreeNode *t;

    for (t = tree; t != NULL; t = t->sibling)
    {
        if (t->nodekind == StmtK)
        {
            switch (t->kind.stmt)
            {
            case CompK:
                buildCompound(t, TRUE);
                break;
            case IfK:
                buildStatementScopes(t->child[1]);
                buildStatementScopes(t->child[2]);
                break;
            case WhileK:
                buildStatementScopes(t->child[1]);
                break;
            default:
                break;
            }
        }
    }
}

static void buildFunction(TreeNode *tree)
{
    Symbol *prior;
    Symbol *func;
    Scope *functionScope;
    TreeNode *param;
    int paramCount;
    ExpType returnType;

    returnType = declaredType(tree);
    tree->type = returnType;

    prior = lookupInScope(currentScope, tree->attr.name);
    func = insertSymbol(tree->attr.name, SymFunc, returnType, FALSE, tree->lineno, FALSE);
    paramCount = countParams(tree->child[1]);
    if (func != NULL && prior == NULL)
    {
        func->paramCount = paramCount;
        func->params = copyParamInfo(tree->child[1], paramCount);
    }

    functionScope = newScope(tree->attr.name, tree, currentScope);
    enterScope(functionScope);

    for (param = tree->child[1]; param != NULL; param = param->sibling)
        buildParam(param);

    buildCompound(tree->child[2], FALSE);
    leaveScope();
}

static void insertBuiltinFunction(const char *name, ExpType returnType,
                                  int paramCount, ParamInfo *params)
{
    Symbol *func = insertSymbol(name, SymFunc, returnType, FALSE, 0, TRUE);
    if (func != NULL)
    {
        func->paramCount = paramCount;
        func->params = params;
    }
}

static void insertBuiltins(void)
{
    ParamInfo *outputParams = (ParamInfo *)malloc(sizeof(ParamInfo));
    if (outputParams == NULL)
    {
        fprintf(listing, "Out of memory error at line %d\n", lineno);
        exit(1);
    }
    outputParams[0].type = Integer;
    outputParams[0].isArray = FALSE;

    insertBuiltinFunction("input", Integer, 0, NULL);
    insertBuiltinFunction("output", Void, 1, outputParams);
}

static void checkMain(void)
{
    Symbol *mainSym = lookupInScope(globalScope, "main");

    if (mainSym == NULL || mainSym->kind != SymFunc)
    {
        semanticError(0, "program must declare function 'void main(void)'");
        return;
    }

    if (mainSym->type != Void || mainSym->paramCount != 0)
        semanticError(mainSym->lineno, "function 'main' must be declared as void main(void)");
}

void buildSymtab(TreeNode *syntaxTree)
{
    TreeNode *t;

    globalScope = newScope("global", NULL, NULL);
    enterScope(globalScope);
    insertBuiltins();

    for (t = syntaxTree; t != NULL; t = t->sibling)
    {
        if (t->nodekind != DeclK)
            continue;

        if (t->kind.decl == VarDeclK)
            buildVarDecl(t);
        else if (t->kind.decl == FunDeclK)
            buildFunction(t);
    }

    checkMain();
    currentScope = globalScope;
}

static int isRelOp(TokenType op)
{
    return op == LT || op == LTE || op == GT || op == GTE || op == EQ || op == NEQ;
}

static int isArithOp(TokenType op)
{
    return op == PLUS || op == MINUS || op == TIMES || op == OVER;
}

static ExpType typeExp(TreeNode *tree);

static int isIntExpression(TreeNode *tree, ExpType type, const char *context)
{
    int line = tree != NULL ? tree->lineno : lineno;

    if (type == Integer)
        return TRUE;

    if (type == IntegerArray)
        semanticError(line, "array expression cannot be used as %s", context);
    else if (type == Void)
        semanticError(line, "void expression cannot be used as %s", context);
    else
        semanticError(line, "invalid expression type for %s", context);

    return FALSE;
}

static ExpType typeCall(TreeNode *tree)
{
    const char *name = NULL;
    Symbol *sym;
    TreeNode *arg;
    int index = 0;

    if (tree->child[0] != NULL)
        name = tree->child[0]->attr.name;

    sym = lookupSymbol(name);
    if (sym == NULL)
    {
        semanticError(tree->lineno, "call to undeclared function '%s'", name);
        tree->type = Integer;
        return Integer;
    }

    if (sym->kind != SymFunc)
    {
        semanticError(tree->lineno, "'%s' is not a function", name);
        tree->type = Integer;
        return Integer;
    }

    arg = tree->child[1];
    while (arg != NULL && index < sym->paramCount)
    {
        ExpType argType = typeExp(arg);
        ParamInfo expected = sym->params[index];

        if (expected.isArray)
        {
            if (argType != IntegerArray)
                semanticError(arg->lineno, "argument %d of '%s' must be int[]", index + 1, name);
        }
        else
        {
            isIntExpression(arg, argType, "function argument");
        }

        arg = arg->sibling;
        index++;
    }

    while (arg != NULL)
    {
        typeExp(arg);
        arg = arg->sibling;
        index++;
    }

    if (index != sym->paramCount)
        semanticError(tree->lineno, "function '%s' expects %d argument(s) but got %d",
                      name, sym->paramCount, index);

    tree->type = sym->type;
    return tree->type;
}

static ExpType typeExp(TreeNode *tree)
{
    Symbol *sym;
    ExpType leftType;
    ExpType rightType;

    if (tree == NULL)
        return Void;

    if (tree->nodekind != ExpK)
        return Void;

    switch (tree->kind.exp)
    {
    case ConstK:
        tree->type = Integer;
        return Integer;

    case IdK:
        sym = lookupSymbol(tree->attr.name);
        if (sym == NULL)
        {
            semanticError(tree->lineno, "use of undeclared identifier '%s'", tree->attr.name);
            tree->type = Integer;
            return Integer;
        }
        if (sym->kind == SymFunc)
        {
            semanticError(tree->lineno, "function '%s' used as a variable", tree->attr.name);
            tree->type = Integer;
            return Integer;
        }
        tree->isArray = sym->isArray;
        tree->type = sym->isArray ? IntegerArray : Integer;
        return tree->type;

    case ArrIdK:
        sym = lookupSymbol(tree->attr.name);
        if (sym == NULL)
        {
            semanticError(tree->lineno, "use of undeclared array '%s'", tree->attr.name);
            tree->type = Integer;
            typeExp(tree->child[0]);
            return Integer;
        }
        if (sym->kind == SymFunc)
        {
            semanticError(tree->lineno, "function '%s' used as an array", tree->attr.name);
        }
        else if (!sym->isArray)
        {
            semanticError(tree->lineno, "'%s' is not an array", tree->attr.name);
        }

        leftType = typeExp(tree->child[0]);
        isIntExpression(tree->child[0], leftType, "array index");
        tree->type = Integer;
        tree->isArray = FALSE;
        return Integer;

    case CallK:
        return typeCall(tree);

    case AssignK:
        leftType = typeExp(tree->child[0]);
        rightType = typeExp(tree->child[1]);

        if (tree->child[0] == NULL ||
            tree->child[0]->nodekind != ExpK ||
            (tree->child[0]->kind.exp != IdK && tree->child[0]->kind.exp != ArrIdK))
        {
            semanticError(tree->lineno, "left side of assignment must be a variable");
        }
        isIntExpression(tree->child[0], leftType, "assignment target");
        isIntExpression(tree->child[1], rightType, "assignment value");
        tree->type = Integer;
        return Integer;

    case OpK:
        leftType = typeExp(tree->child[0]);
        rightType = typeExp(tree->child[1]);
        if (isArithOp(tree->attr.op) || isRelOp(tree->attr.op))
        {
            isIntExpression(tree->child[0], leftType, "operator operand");
            isIntExpression(tree->child[1], rightType, "operator operand");
        }
        tree->type = Integer;
        return Integer;

    default:
        return Void;
    }
}

static void typeStmt(TreeNode *tree);

static void typeStmtList(TreeNode *tree)
{
    TreeNode *t;

    for (t = tree; t != NULL; t = t->sibling)
        typeStmt(t);
}

static void typeCompound(TreeNode *tree, int enterExistingScope)
{
    if (tree == NULL)
        return;

    if (enterExistingScope)
    {
        Scope *scope = findScopeForOwner(tree);
        if (scope != NULL)
            enterScope(scope);
    }

    typeStmtList(tree->child[1]);

    if (enterExistingScope)
        leaveScope();
}

static void typeStmt(TreeNode *tree)
{
    ExpType conditionType;
    ExpType returnType;

    if (tree == NULL)
        return;

    if (tree->nodekind == ExpK)
    {
        typeExp(tree);
        return;
    }

    if (tree->nodekind != StmtK)
        return;

    switch (tree->kind.stmt)
    {
    case CompK:
        typeCompound(tree, TRUE);
        break;

    case IfK:
        conditionType = typeExp(tree->child[0]);
        isIntExpression(tree->child[0], conditionType, "if condition");
        typeStmt(tree->child[1]);
        typeStmt(tree->child[2]);
        break;

    case WhileK:
        conditionType = typeExp(tree->child[0]);
        isIntExpression(tree->child[0], conditionType, "while condition");
        typeStmt(tree->child[1]);
        break;

    case ReturnK:
        if (currentFunction == NULL)
            break;
        if (currentFunction->type == Void)
        {
            if (tree->child[0] != NULL)
                semanticError(tree->lineno, "void function '%s' should not return a value",
                              currentFunction->name);
        }
        else
        {
            if (tree->child[0] == NULL)
            {
                semanticError(tree->lineno, "int function '%s' must return a value",
                              currentFunction->name);
            }
            else
            {
                returnType = typeExp(tree->child[0]);
                isIntExpression(tree->child[0], returnType, "return value");
            }
        }
        break;

    case ExprStmtK:
        if (tree->child[0] != NULL)
        {
            ExpType exprType = typeExp(tree->child[0]);
            if (exprType == IntegerArray)
                semanticError(tree->child[0]->lineno, "array expression cannot be used as a statement");
        }
        break;

    default:
        break;
    }
}

void typeCheck(TreeNode *syntaxTree)
{
    TreeNode *t;

    currentScope = globalScope;
    for (t = syntaxTree; t != NULL; t = t->sibling)
    {
        if (t->nodekind != DeclK || t->kind.decl != FunDeclK)
            continue;

        currentFunction = lookupInScope(globalScope, t->attr.name);
        enterScope(findScopeForOwner(t));
        typeCompound(t->child[2], FALSE);
        leaveScope();
        currentFunction = NULL;
    }
}

void analyze(TreeNode *syntaxTree)
{
    buildSymtab(syntaxTree);
    typeCheck(syntaxTree);
}
