/*
File: parse.h
The parser inplementation for the C-minus compiler
 */

#include "util.h"
#include "scan.h"
#include "parse.h"

static TokenType token; /* current token */

static TokenType nextToken(void)
{
    TokenType next;

    do
    {
        next = getToken();
        if (next == ERROR)
        {
            reportScanError(listing);
            Error = TRUE;
        }
    } while (next == ERROR);

    return next;
}

/* helper to print token name */
static const char *tokenName(TokenType t)
{
    switch (t)
    {
    case INT:
        return "int";
    case VOID:
        return "void";
    case IF:
        return "if";
    case ELSE:
        return "else";
    case WHILE:
        return "while";
    case RETURN:
        return "return";
    case ID:
        return "identifier";
    case NUM:
        return "number";
    case PLUS:
        return "'+'";
    case MINUS:
        return "'-'";
    case TIMES:
        return "'*'";
    case OVER:
        return "'/'";
    case LT:
        return "'<'";
    case LTE:
        return "'<='";
    case GT:
        return "'>'";
    case GTE:
        return "'>='";
    case EQ:
        return "'=='";
    case NEQ:
        return "'!='";
    case ASSIGN:
        return "'='";
    case SEMI:
        return "';'";
    case COMMA:
        return "','";
    case LPAREN:
        return "'('";
    case RPAREN:
        return "')'";
    case LBRACKET:
        return "'['";
    case RBRACKET:
        return "']'";
    case LBRACE:
        return "'{'";
    case RBRACE:
        return "'}'";
    default:
        return "unknown";
    }
}

/* function prototypes ofr recursive calls */
static TreeNode *declaration_list(void);
static TreeNode *declaration(void);
static TreeNode *var_declaration(char *idname, TreeNode *typeNode);
static TreeNode *type_specifier(void);
static TreeNode *fun_declaration(char *idname, TreeNode *typeNode);
static TreeNode *params(void);
static TreeNode *param_list(void);
static TreeNode *param(void);
static TreeNode *finish_param(TreeNode *typeNode);
static TreeNode *compound_stmt(void);
static TreeNode *local_declarations(void);
static TreeNode *statement_list(void);
static TreeNode *statement(void);
static TreeNode *expression_stmt(void);
static TreeNode *selection_stmt(void);
static TreeNode *iteration_stmt(void);
static TreeNode *return_stmt(void);
static TreeNode *expression(void);
static TreeNode *simple_expression(void);
static TreeNode *additive_expression(void);
static TreeNode *term(void);
static TreeNode *factor(void);
static TreeNode *args(void);
static TreeNode *arg_list(void);

#define TOKEN_SET_SIZE(set) ((int)(sizeof(set) / sizeof((set)[0])))

static void error_expected(TokenType expected)
{
    fprintf(listing, "\n>>> Syntax error at line %d: expected %s but found %s\n",
            lineno, tokenName(expected), tokenName(token));
    Error = TRUE;
}

static void error_expected_str(const char *what)
{
    fprintf(listing, "\n>>> Syntax error at line %d: expected %s but found %s\n",
            lineno, what, tokenName(token));
    Error = TRUE;
}

static void error_invalid(const char *what)
{
    fprintf(listing, "\n>>> Syntax error at line %d: invalid %s\n",
            lineno, what);
    Error = TRUE;
}

static void error_unexpected(const char *context)
{
    fprintf(listing, "\n>>> Syntax error at line %d: unexpected token %s in %s\n",
            lineno, tokenName(token), context);
    Error = TRUE;
}

static int editDistance(const char *left, const char *right)
{
    int previous[MAXTOKENLEN + 1];
    int current[MAXTOKENLEN + 1];
    size_t leftLength = strlen(left);
    size_t rightLength = strlen(right);
    size_t i;
    size_t j;

    for (j = 0; j <= rightLength; j++)
        previous[j] = (int)j;

    for (i = 1; i <= leftLength; i++)
    {
        current[0] = (int)i;
        for (j = 1; j <= rightLength; j++)
        {
            int insertion = current[j - 1] + 1;
            int deletion = previous[j] + 1;
            int substitution = previous[j - 1] + (left[i - 1] != right[j - 1]);
            int best = insertion < deletion ? insertion : deletion;
            current[j] = best < substitution ? best : substitution;
        }
        for (j = 0; j <= rightLength; j++)
            previous[j] = current[j];
    }

    return previous[rightLength];
}

static TokenType reportUnknownType(const char *name)
{
    int intDistance = editDistance(name, "int");
    int voidDistance = editDistance(name, "void");
    const char *suggestion = NULL;
    TokenType recoveredType = INT;

    if (intDistance <= voidDistance && intDistance <= 2)
    {
        suggestion = "int";
        recoveredType = INT;
    }
    else if (voidDistance < intDistance && voidDistance <= 2)
    {
        suggestion = "void";
        recoveredType = VOID;
    }

    if (suggestion != NULL)
        fprintf(listing,
                "\n>>> Syntax error at line %d: unknown type name '%s'; did you mean '%s'?\n",
                tokenLine, name, suggestion);
    else
        fprintf(listing, "\n>>> Syntax error at line %d: unknown type name '%s'\n",
                tokenLine, name);
    Error = TRUE;
    return recoveredType;
}

static int tokenInSet(TokenType t, const TokenType set[], int count)
{
    int i;
    for (i = 0; i < count; i++)
    {
        if (set[i] == t)
            return TRUE;
    }
    return FALSE;
}

static void synchronize(TokenType expected, const TokenType syncSet[], int syncCount)
{
    while (token != expected && token != ENDFILE &&
           !tokenInSet(token, syncSet, syncCount))
    {
        token = nextToken();
    }
}

static int expect(TokenType expected, const TokenType syncSet[], int syncCount)
{
    if (token == expected)
    {
        token = nextToken();
        return TRUE;
    }

    error_expected(expected);
    synchronize(expected, syncSet, syncCount);

    if (token == expected)
    {
        token = nextToken();
        return TRUE;
    }

    return FALSE;
}

static int isExpressionFollow(TokenType t)
{
    return t == SEMI || t == COMMA || t == RPAREN || t == RBRACKET ||
           t == RBRACE || t == ELSE || t == ENDFILE;
}

static void match(TokenType expected)
{
    if (token == expected)
        token = nextToken();
    else
    {
        error_expected(expected);
    }
}

TreeNode *declaration_list(void)
{
    TreeNode *t = NULL; /* head pointer of declaration list */
    TreeNode *p = NULL; /* tail pointer of declaration list */

    while (token != ENDFILE)
    {
        TreeNode *q;

        if (token != INT && token != VOID && token != ID)
        {
            error_unexpected("declaration");
            token = nextToken();
            continue;
        }

        q = declaration(); /* parsed declaration subtree */
        if (q != NULL)
        {
            if (t == NULL)
                t = p = q;
            else
            {
                p->sibling = q;
                p = q;
            }
        }
    }
    return t;
}

TreeNode *declaration(void)
{
    TreeNode *t = NULL;
    TreeNode *typeNode = NULL;
    char *idname = NULL;

    /* parse type */
    typeNode = type_specifier(); /* consumes INT/VOID and returns a small type node */

    if (token != ID)
    {
        error_expected_str("identifier");
        /* try to recover */
        while (token != SEMI && token != LPAREN && token != ENDFILE)
            token = nextToken();
        return NULL;
    }

    /* save identifier name then comsume ID */
    idname = copyString(tokenString);
    match(ID);

    /* decide var vs fun by lookahead */
    if (token == LPAREN)
    {
        /* function declaration */
        t = fun_declaration(idname, typeNode);
    }
    else
    {
        /* variable declaration */
        t = var_declaration(idname, typeNode);
    }

    return t;
}

TreeNode *var_declaration(char *idname, TreeNode *typeNode)
{
    TreeNode *t = NULL;
    static const TokenType rbracketSync[] = {SEMI, COMMA, RPAREN, RBRACE};
    static const TokenType semiSync[] = {
        RBRACE, INT, VOID, IF, WHILE, RETURN, LBRACE, ID, NUM, LPAREN};

    t = newDeclNode(VarDeclK);
    t->attr.name = idname;
    t->child[0] = typeNode;

    if (token == LBRACKET)
    {
        match(LBRACKET);
        if (token == NUM)
        {
            t->size = atoi(tokenString);
            match(NUM);
        }
        else
        {
            error_expected_str("array size (number)");
        }
        expect(RBRACKET, rbracketSync, TOKEN_SET_SIZE(rbracketSync));
    }

    expect(SEMI, semiSync, TOKEN_SET_SIZE(semiSync));
    return t;
}

TreeNode *type_specifier(void)
{
    TreeNode *t = NULL;
    TokenType recoveredType;

    if (token == INT)
    {
        t = newDeclNode(ParamK);
        t->type = Integer;
        match(INT);
    }
    else if (token == VOID)
    {
        t = newDeclNode(ParamK);
        t->type = Void;
        match(VOID);
    }
    else if (token == ID)
    {
        recoveredType = reportUnknownType(tokenString);
        t = newDeclNode(ParamK);
        t->type = recoveredType == VOID ? Void : Integer;
        match(ID);
    }
    else
    {
        error_expected_str("type specifier (int or void)");
        t = newDeclNode(ParamK);
        t->type = Integer; /* default to int */
    }

    return t;
}

TreeNode *fun_declaration(char *idname, TreeNode *typeNode)
{
    TreeNode *t = NULL;
    static const TokenType rparenSync[] = {LBRACE, INT, VOID, ENDFILE};

    t = newDeclNode(FunDeclK);
    t->attr.name = idname;
    t->child[0] = typeNode;

    match(LPAREN);
    t->child[1] = params();
    expect(RPAREN, rparenSync, TOKEN_SET_SIZE(rparenSync));

    t->child[2] = compound_stmt();
    return t;
}

/* params -> param_list | VOID */
TreeNode *params(void)
{
    if (token == VOID)
    {
        TreeNode *typeNode = type_specifier();
        TreeNode *t;
        TreeNode *p;

        if (token == RPAREN)
            return typeNode;

        t = finish_param(typeNode);
        p = t;
        while (token == COMMA)
        {
            match(COMMA);
            TreeNode *q = param();
            if (q != NULL)
            {
                if (t == NULL)
                    t = p = q;
                else
                {
                    p->sibling = q;
                    p = q;
                }
            }
        }
        return t;
    }
    else
    {
        return param_list();
    }
}

/* param_list -> param { , param } */
TreeNode *param_list(void)
{
    TreeNode *t = param();
    TreeNode *p = t;
    while (token == COMMA)
    {
        match(COMMA);
        TreeNode *q = param();
        if (q != NULL)
        {
            if (t == NULL)
                t = p = q;
            else
            {
                p->sibling = q;
                p = q;
            }
        }
    }
    return t;
}

/* param -> type_specifier ID [ [] ] */
TreeNode *param(void)
{
    TreeNode *typeNode = type_specifier();
    return finish_param(typeNode);
}

TreeNode *finish_param(TreeNode *typeNode)
{
    TreeNode *t = NULL;
    static const TokenType rbracketSync[] = {COMMA, RPAREN, LBRACE};

    if (token != ID)
    {
        error_expected_str("identifier");
        while (token != COMMA && token != RPAREN && token != LBRACE && token != ENDFILE)
            token = nextToken();
        return NULL;
    }
    t = newDeclNode(ParamK);
    t->attr.name = copyString(tokenString);
    match(ID);
    t->child[0] = typeNode;
    if (token == LBRACKET)
    {
        match(LBRACKET);
        /* parameter arrays have unspecified size */
        t->isArray = TRUE;
        expect(RBRACKET, rbracketSync, TOKEN_SET_SIZE(rbracketSync));
    }
    return t;
}

/* compound_stmt -> { local_declarations statement_list } */
TreeNode *compound_stmt(void)
{
    TreeNode *t = newStmtNode(CompK);
    static const TokenType lbraceSync[] = {
        INT, VOID, IF, WHILE, RETURN, LBRACE, RBRACE, ID, NUM, LPAREN};
    static const TokenType rbraceSync[] = {
        INT, VOID, IF, WHILE, RETURN, LBRACE, ID, NUM, LPAREN};

    expect(LBRACE, lbraceSync, TOKEN_SET_SIZE(lbraceSync));
    t->child[0] = local_declarations();
    t->child[1] = statement_list();
    expect(RBRACE, rbraceSync, TOKEN_SET_SIZE(rbraceSync));
    return t;
}

/* local_declarations -> { var_declaration } */
TreeNode *local_declarations(void)
{
    TreeNode *t = NULL;
    TreeNode *p = NULL;
    while (token == INT)
    {
        TreeNode *typeNode = type_specifier();
        if (token != ID)
        {
            static const TokenType semiSync[] = {RBRACE, INT, IF, WHILE, RETURN, LBRACE, ID, NUM, LPAREN};
            error_expected_str("identifier");
            while (token != SEMI && token != RBRACE && token != ENDFILE)
                token = nextToken();
            expect(SEMI, semiSync, TOKEN_SET_SIZE(semiSync));
            continue;
        }
        char *idname = copyString(tokenString);
        match(ID);
        TreeNode *q = var_declaration(idname, typeNode);
        if (q != NULL)
        {
            if (t == NULL)
                t = p = q;
            else
            {
                p->sibling = q;
                p = q;
            }
        }
    }
    return t;
}

/* statement_list -> { statement } */
TreeNode *statement_list(void)
{
    TreeNode *t = NULL;
    TreeNode *p = NULL;
    while (token != RBRACE && token != ENDFILE)
    {
        TreeNode *q = statement();
        if (q != NULL)
        {
            if (t == NULL)
                t = p = q;
            else
            {
                p->sibling = q;
                p = q;
            }
        }
    }
    return t;
}

/* statement -> expression_stmt | compound_stmt | selection_stmt | iteration_stmt | return_stmt */
TreeNode *statement(void)
{
    switch (token)
    {
    case IF:
        return selection_stmt();
    case WHILE:
        return iteration_stmt();
    case RETURN:
        return return_stmt();
    case LBRACE:
        return compound_stmt();
    default:
        return expression_stmt();
    }
}

/* expression_stmt -> expression ; | ; */
TreeNode *expression_stmt(void)
{
    TreeNode *t = NULL;
    static const TokenType semiSync[] = {
        RBRACE, IF, WHILE, RETURN, LBRACE, ID, NUM, LPAREN, ELSE, INT, VOID};

    if (token == SEMI)
    {
        match(SEMI);
        return NULL;
    }
    t = newStmtNode(ExprStmtK);
    t->child[0] = expression();
    expect(SEMI, semiSync, TOKEN_SET_SIZE(semiSync));
    return t;
}

/* selection_stmt -> IF ( expression ) statement [ ELSE statement ] */
TreeNode *selection_stmt(void)
{
    TreeNode *t = newStmtNode(IfK);
    static const TokenType lparenSync[] = {ID, NUM, LPAREN, SEMI, RBRACE};
    static const TokenType rparenSync[] = {
        LBRACE, IF, WHILE, RETURN, ID, NUM, LPAREN, SEMI, RBRACE, ELSE};

    match(IF);
    expect(LPAREN, lparenSync, TOKEN_SET_SIZE(lparenSync));
    t->child[0] = expression();
    expect(RPAREN, rparenSync, TOKEN_SET_SIZE(rparenSync));
    t->child[1] = statement();
    if (token == ELSE)
    {
        match(ELSE);
        t->child[2] = statement();
    }
    return t;
}

/* iteration_stmt -> WHILE ( expression ) statement */
TreeNode *iteration_stmt(void)
{
    TreeNode *t = newStmtNode(WhileK);
    static const TokenType lparenSync[] = {ID, NUM, LPAREN, SEMI, RBRACE};
    static const TokenType rparenSync[] = {
        LBRACE, IF, WHILE, RETURN, ID, NUM, LPAREN, SEMI, RBRACE};

    match(WHILE);
    expect(LPAREN, lparenSync, TOKEN_SET_SIZE(lparenSync));
    t->child[0] = expression();
    expect(RPAREN, rparenSync, TOKEN_SET_SIZE(rparenSync));
    t->child[1] = statement();
    return t;
}

/* return_stmt -> RETURN expression? ; */
TreeNode *return_stmt(void)
{
    TreeNode *t = newStmtNode(ReturnK);
    static const TokenType semiSync[] = {
        RBRACE, IF, WHILE, RETURN, LBRACE, ID, NUM, LPAREN, ELSE, INT, VOID};

    match(RETURN);
    if (token != SEMI)
        t->child[0] = expression();
    expect(SEMI, semiSync, TOKEN_SET_SIZE(semiSync));
    return t;
}

/* expression -> var = expression | simple_expression */
TreeNode *expression(void)
{
    TreeNode *t = simple_expression();
    if (token == ASSIGN)
    {
        if (t != NULL && t->nodekind == ExpK && (t->kind.exp == IdK || t->kind.exp == ArrIdK))
        {
            TreeNode *p = newExpNode(AssignK);
            p->child[0] = t;
            match(ASSIGN);
            p->child[1] = expression();
            t = p;
        }
        else
        {
            error_invalid("assignment target (can only assign to variable or array element)");
        }
    }
    return t;
}

/* simple_expression -> additive_expression [ relop additive_expression ] */
TreeNode *simple_expression(void)
{
    TreeNode *t = additive_expression();
    if (token == LT || token == LTE || token == GT || token == GTE || token == EQ || token == NEQ)
    {
        TreeNode *p = newExpNode(OpK);
        p->child[0] = t;
        p->attr.op = token;
        match(token);
        p->child[1] = additive_expression();
        t = p;
    }
    return t;
}

/* additive_expression -> term { addop term } */
TreeNode *additive_expression(void)
{
    TreeNode *t = term();
    while (token == PLUS || token == MINUS)
    {
        TreeNode *p = newExpNode(OpK);
        p->child[0] = t;
        p->attr.op = token;
        match(token);
        p->child[1] = term();
        t = p;
    }
    return t;
}

/* term -> factor { mulop factor } */
TreeNode *term(void)
{
    TreeNode *t = factor();
    while (token == TIMES || token == OVER)
    {
        TreeNode *p = newExpNode(OpK);
        p->child[0] = t;
        p->attr.op = token;
        match(token);
        p->child[1] = factor();
        t = p;
    }
    return t;
}

/* factor -> ( expression ) | var | call | NUM */
TreeNode *factor(void)
{
    TreeNode *t = NULL;
    static const TokenType exprFollowSync[] = {
        SEMI, COMMA, RPAREN, RBRACKET, RBRACE, ELSE};

    switch (token)
    {
    case LPAREN:
        match(LPAREN);
        t = expression();
        expect(RPAREN, exprFollowSync, TOKEN_SET_SIZE(exprFollowSync));
        break;
    case ID:
        /* need to decide between call and var */
        {
            char savedString[MAXTOKENLEN + 1];
            strncpy(savedString, tokenString, MAXTOKENLEN);
            savedString[MAXTOKENLEN] = '\0';
            match(ID);
            if (token == LPAREN)
            {
                /* it's a call: create Call node with Id child */
                TreeNode *idnode = newExpNode(IdK);
                idnode->attr.name = copyString(savedString);
                t = newExpNode(CallK);
                t->child[0] = idnode;
                match(LPAREN);
                t->child[1] = args();
                expect(RPAREN, exprFollowSync, TOKEN_SET_SIZE(exprFollowSync));
            }
            else if (token == LBRACKET)
            {
                /* array access */
                TreeNode *index = NULL;
                match(LBRACKET);
                index = expression();
                expect(RBRACKET, exprFollowSync, TOKEN_SET_SIZE(exprFollowSync));
                t = newExpNode(ArrIdK);
                t->attr.name = copyString(savedString);
                t->child[0] = index;
            }
            else
            {
                t = newExpNode(IdK);
                t->attr.name = copyString(savedString);
            }
        }
        break;
    case NUM:
        t = newExpNode(ConstK);
        t->attr.val = atoi(tokenString);
        match(NUM);
        break;
    default:
        error_unexpected("expression");
        if (!isExpressionFollow(token))
            token = nextToken();
        break;
    }
    return t;
}

/* args -> arg_list | empty */
TreeNode *args(void)
{
    if (token == RPAREN)
        return NULL;
    return arg_list();
}

/* arg_list -> expression { , expression } */
TreeNode *arg_list(void)
{
    TreeNode *t = expression();
    TreeNode *p = t;
    while (token == COMMA)
    {
        match(COMMA);
        TreeNode *q = expression();
        if (q != NULL)
        {
            if (t == NULL)
                t = p = q;
            else
            {
                p->sibling = q;
                p = q;
            }
        }
    }
    return t;
}

/* primary parse entry */
TreeNode *parse(void)
{
    TreeNode *t;
    token = nextToken();
    t = declaration_list();
    return t;
}
