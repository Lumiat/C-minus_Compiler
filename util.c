/*
File: util.c
Utility function implementation for the C-minus compiler
*/

#include "util.h"

void printSourceLine(FILE *out, int lineNumber, const char *lineText)
{
    size_t len = strlen(lineText);
    while (len && (lineText[len - 1] == '\n' || lineText[len - 1] == '\r'))
        len--;
    fprintf(out, "%d: %.*s\n", lineNumber, (int)len, lineText);
}

void printToken(TokenType token, const char *tokenString)
{
    switch (token)
    {
    case IF:
    case ELSE:
    case INT:
    case RETURN:
    case VOID:
    case WHILE:
        fprintf(listing, "reserved word: %s\n", tokenString);
        break;
    case PLUS:
        fprintf(listing, "+\n");
        break;
    case MINUS:
        fprintf(listing, "-\n");
        break;
    case TIMES:
        fprintf(listing, "*\n");
        break;
    case OVER:
        fprintf(listing, "/\n");
        break;
    case LT:
        fprintf(listing, "<\n");
        break;
    case LTE:
        fprintf(listing, "<=\n");
        break;
    case GT:
        fprintf(listing, ">\n");
        break;
    case GTE:
        fprintf(listing, ">=\n");
        break;
    case EQ:
        fprintf(listing, "==\n");
        break;
    case NEQ:
        fprintf(listing, "!=\n");
        break;
    case ASSIGN:
        fprintf(listing, "=\n");
        break;
    case SEMI:
        fprintf(listing, ";\n");
        break;
    case COMMA:
        fprintf(listing, ",\n");
        break;
    case LPAREN:
        fprintf(listing, "(\n");
        break;
    case RPAREN:
        fprintf(listing, ")\n");
        break;
    case LBRACKET:
        fprintf(listing, "[\n");
        break;
    case RBRACKET:
        fprintf(listing, "]\n");
        break;
    case LBRACE:
        fprintf(listing, "{\n");
        break;
    case RBRACE:
        fprintf(listing, "}\n");
        break;
    case ENDFILE:
        fprintf(listing, "EOF\n");
        break;
    case NUM:
        fprintf(listing, "NUM, val= %s\n", tokenString);
        break;
    case ID:
        fprintf(listing, "ID, name= %s\n", tokenString);
        break;
    case ERROR:
        fprintf(listing, "ERROR: %s\n", tokenString);
        break;
    default:
        fprintf(listing, "Unknown token: %d\n", token);
    }
}

/* Create a new declaration node */
TreeNode *newDeclNode(DeclKind kind)
{
    TreeNode *t = (TreeNode *)malloc(sizeof(TreeNode));
    int i;
    if (t == NULL)
    {
        fprintf(listing, "Out of memory error at line %d\n", lineno);
        exit(1);
    }
    for (i = 0; i < MAXCHILDREN; i++)
        t->child[i] = NULL;
    t->sibling = NULL;
    t->lineno = lineno;
    t->nodekind = DeclK;
    t->kind.decl = kind;
    t->attr.name = NULL;
    t->type = Void;
    t->size = 0;
    t->isArray = FALSE;
    return t;
}

/* Create a new statement node */
TreeNode *newStmtNode(StmtKind kind)
{
    TreeNode *t = (TreeNode *)malloc(sizeof(TreeNode));
    int i;
    if (t == NULL)
    {
        fprintf(listing, "Out of memory error at line %d\n", lineno);
        exit(1);
    }
    for (i = 0; i < MAXCHILDREN; i++)
        t->child[i] = NULL;
    t->sibling = NULL;
    t->lineno = lineno;
    t->nodekind = StmtK;
    t->kind.stmt = kind;
    t->attr.name = NULL;
    t->type = Void;
    t->size = 0;
    t->isArray = FALSE;
    return t;
}

/* Create a new expression node */
TreeNode *newExpNode(ExpKind kind)
{
    TreeNode *t = (TreeNode *)malloc(sizeof(TreeNode));
    int i;
    if (t == NULL)
    {
        fprintf(listing, "Out of memory error at line %d\n", lineno);
        exit(1);
    }
    for (i = 0; i < MAXCHILDREN; i++)
        t->child[i] = NULL;
    t->sibling = NULL;
    t->lineno = lineno;
    t->nodekind = ExpK;
    t->kind.exp = kind;
    t->attr.name = NULL;
    t->type = Integer;
    t->size = 0;
    t->isArray = FALSE;
    return t;
}

/* copyString: allocate and copy a C string */
char *copyString(const char *s)
{
    char *t;
    size_t n;
    if (s == NULL)
        return NULL;
    n = strlen(s) + 1;
    t = (char *)malloc(n);
    if (t == NULL)
    {
        fprintf(listing, "Out of memory error at line %d\n", lineno);
        exit(1);
    }
    strcpy(t, s);
    return t;
}

/* pretty-print the syntax tree to the listing file */
/* pretty-print uses explicit indent parameter; no global indent state needed */

/* recursive print with explicit indent parameter to avoid global state issues */
static void printTreeRec(TreeNode *tree, int indent)
{
    while (tree)
    {
        int i;
        /* do not print a blanket indent here; each branch prints its own indent lines */
        if (tree->nodekind == DeclK)
        {
            if (tree->kind.decl == FunDeclK)
            {
                for (i = 0; i < indent; i++)
                    fprintf(listing, " ");
                fprintf(listing, "FuncK\n");
                /* return type */
                if (tree->child[0])
                {
                    for (i = 0; i < indent + 2; i++)
                        fprintf(listing, " ");
                    if (tree->child[0]->type == Integer)
                        fprintf(listing, "IntK\n");
                    else
                        fprintf(listing, "VoidK\n");
                }
                /* function name */
                if (tree->attr.name)
                {
                    for (i = 0; i < indent + 2; i++)
                        fprintf(listing, " ");
                    fprintf(listing, "IdK: %s\n", tree->attr.name);
                }
                /* params */
                for (i = 0; i < indent + 2; i++)
                    fprintf(listing, " ");
                fprintf(listing, "ParamsK\n");
                if (tree->child[1])
                    printTreeRec(tree->child[1], indent + 4);
                if (tree->child[2])
                    printTreeRec(tree->child[2], indent + 2);
            }
            else if (tree->kind.decl == VarDeclK)
            {
                for (i = 0; i < indent; i++)
                    fprintf(listing, " ");
                fprintf(listing, "Var_DeclK\n");
                if (tree->child[0])
                {
                    for (i = 0; i < indent + 2; i++)
                        fprintf(listing, " ");
                    if (tree->child[0]->type == Integer)
                        fprintf(listing, "IntK\n");
                    else
                        fprintf(listing, "VoidK\n");
                }
                if (tree->attr.name)
                {
                    for (i = 0; i < indent + 2; i++)
                        fprintf(listing, " ");
                    fprintf(listing, "IdK: %s\n", tree->attr.name);
                }
            }
            else if (tree->kind.decl == ParamK)
            {
                if (tree->attr.name == NULL)
                {
                    if (tree->type == Integer)
                    {
                        for (i = 0; i < indent; i++)
                            fprintf(listing, " ");
                        fprintf(listing, "IntK\n");
                    }
                    else
                    {
                        for (i = 0; i < indent; i++)
                            fprintf(listing, " ");
                        fprintf(listing, "VoidK\n");
                    }
                }
                else
                {
                    for (i = 0; i < indent; i++)
                        fprintf(listing, " ");
                    fprintf(listing, "ParamK\n");
                    if (tree->child[0])
                    {
                        for (i = 0; i < indent + 2; i++)
                            fprintf(listing, " ");
                        if (tree->child[0]->type == Integer)
                            fprintf(listing, "IntK\n");
                        else
                            fprintf(listing, "VoidK\n");
                    }
                    for (i = 0; i < indent + 2; i++)
                        fprintf(listing, " ");
                    fprintf(listing, "IdK: %s\n", tree->attr.name);
                }
            }
            else
            {
                fprintf(listing, "Unknown Decl kind\n");
            }
        }
        else if (tree->nodekind == StmtK)
        {
            switch (tree->kind.stmt)
            {
            case CompK:
                for (i = 0; i < indent; i++)
                    fprintf(listing, " ");
                fprintf(listing, "CompK\n");
                if (tree->child[0])
                    printTreeRec(tree->child[0], indent + 2);
                if (tree->child[1])
                    printTreeRec(tree->child[1], indent + 2);
                break;
            case IfK:
                for (i = 0; i < indent; i++)
                    fprintf(listing, " ");
                fprintf(listing, "If\n");
                if (tree->child[0])
                    printTreeRec(tree->child[0], indent + 2);
                if (tree->child[1])
                    printTreeRec(tree->child[1], indent + 2);
                if (tree->child[2])
                    printTreeRec(tree->child[2], indent + 2);
                break;
            case WhileK:
                for (i = 0; i < indent; i++)
                    fprintf(listing, " ");
                fprintf(listing, "While\n");
                if (tree->child[0])
                    printTreeRec(tree->child[0], indent + 2);
                if (tree->child[1])
                    printTreeRec(tree->child[1], indent + 2);
                break;
            case ReturnK:
                for (i = 0; i < indent; i++)
                    fprintf(listing, " ");
                fprintf(listing, "Return\n");
                if (tree->child[0])
                    printTreeRec(tree->child[0], indent + 2);
                break;
            case ExprStmtK:
                /* expression statement: print the expression */
                if (tree->child[0])
                    printTreeRec(tree->child[0], indent);
                break;
            default:
                for (i = 0; i < indent; i++)
                    fprintf(listing, " ");
                fprintf(listing, "Unknown Stmt kind\n");
                break;
            }
        }
        else if (tree->nodekind == ExpK)
        {
            switch (tree->kind.exp)
            {
            case OpK:
                for (i = 0; i < indent; i++)
                    fprintf(listing, " ");
                fprintf(listing, "Op: ");
                printToken(tree->attr.op, "\0");
                if (tree->child[0])
                    printTreeRec(tree->child[0], indent + 2);
                if (tree->child[1])
                    printTreeRec(tree->child[1], indent + 2);
                break;
            case ConstK:
                for (i = 0; i < indent; i++)
                    fprintf(listing, " ");
                fprintf(listing, "ConstK: %d\n", tree->attr.val);
                break;
            case IdK:
                for (i = 0; i < indent; i++)
                    fprintf(listing, " ");
                fprintf(listing, "IdK: %s\n", tree->attr.name);
                break;
            case ArrIdK:
                for (i = 0; i < indent; i++)
                    fprintf(listing, " ");
                fprintf(listing, "IdK: %s\n", tree->attr.name);
                if (tree->child[0])
                    printTreeRec(tree->child[0], indent + 2);
                break;
            case CallK:
                for (i = 0; i < indent; i++)
                    fprintf(listing, " ");
                fprintf(listing, "CallK\n");
                if (tree->child[0])
                {
                    for (i = 0; i < indent + 2; i++)
                        fprintf(listing, " ");
                    fprintf(listing, "IdK: %s\n", tree->child[0]->attr.name);
                }
                if (tree->child[1])
                {
                    for (i = 0; i < indent + 2; i++)
                        fprintf(listing, " ");
                    fprintf(listing, "ArgsK\n");
                    printTreeRec(tree->child[1], indent + 4);
                }
                break;
            case AssignK:
                for (i = 0; i < indent; i++)
                    fprintf(listing, " ");
                fprintf(listing, "Assign\n");
                if (tree->child[0])
                    printTreeRec(tree->child[0], indent + 2);
                if (tree->child[1])
                    printTreeRec(tree->child[1], indent + 2);
                break;
            default:
                fprintf(listing, "Unknown Exp kind\n");
                break;
            }
        }
        else
        {
            fprintf(listing, "Unknown node kind\n");
        }

        tree = tree->sibling;
    }
}

void printTree(TreeNode *tree)
{
    /* initial blank line handled by caller; start with indent 2 to match expected */
    printTreeRec(tree, 2);
}