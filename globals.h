/*
File: globals.h
Global types and vars for C-Minus compiler
must come before other include files
 */

#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

/* MAXRESERVED = the number of reserved words in C-Minus */
#define MAXRESERVED 6

/* MAXCHILDREN = the maximum number of children for an AST node */
#define MAXCHILDREN 3

typedef enum
{
    /* book-keeping tokens */
    ENDFILE,
    ERROR,

    /* reserved words */
    IF,
    ELSE,
    INT,
    RETURN,
    VOID,
    WHILE,

    /* multicharacter tokens */
    ID,
    NUM,

    /* special symbols */
    PLUS,
    MINUS,
    TIMES,
    OVER,
    LT,
    LTE,
    GT,
    GTE,
    EQ,
    NEQ,
    ASSIGN,
    SEMI,
    COMMA,
    LPAREN,
    RPAREN,
    LBRACKET,
    RBRACKET,
    LBRACE,
    RBRACE
} TokenType;

extern FILE *source;  /* source code text file */
extern FILE *listing; /* listing output text file */
extern FILE *code;    /* code text file for TM simulator */

extern int lineno; /* source line number for listing */

/**************************************************/
/***********   Syntax tree for parsing ************/
/**************************************************/

typedef enum
{
    DeclK,
    StmtK,
    ExpK
} NodeKind;

typedef enum
{
    VarDeclK,
    FunDeclK,
    ParamK
} DeclKind;
typedef enum
{
    CompK,
    IfK,
    WhileK,
    ReturnK,
    ExprStmtK
} StmtKind;
typedef enum
{
    AssignK,
    OpK,
    ConstK,
    IdK,
    ArrIdK,
    CallK
} ExpKind;

/* ExpType is used for declarations and type checking */
typedef enum
{
    Void,
    Integer,
    Boolean,
    IntegerArray
} ExpType;

typedef struct treeNode
{
    struct treeNode *child[MAXCHILDREN];
    struct treeNode *sibling;
    int lineno;

    NodeKind nodekind;

    /* each node can be of only one kind */
    union
    {
        DeclKind decl; /* declaration kind */
        StmtKind stmt; /* statement kind */
        ExpKind exp;   /* expression kind */
    } kind;

    /* each attribute is of only one kind */
    union
    {
        TokenType op; /* operator: PLUS, MINUS, etc. */
        int val;      /* value for NUM tokens */
        char *name;   /* identifier name for ID tokens */
    } attr;

    ExpType type; /* for type checking of exps */
    int size;     /* array size for VarDeclK, otherwise 0 */
    int isArray;  /* TRUE for array declarations/parameters/usages */
} TreeNode;

/**************************************************/
/***********   Flags for tracing       ************/
/**************************************************/

/* EchoSource = TRUE causes the source program to be echoed to the listing
 * file with line numbers during parsing.
 */
extern int EchoSource;

/* TraceScan = TRUE causes token information to be printed to the listing
 * file as each token is recognized by the scanner.
 */
extern int TraceScan;

/* TraceParse = TRUE causes the syntax tree to be printed to the listing file
 * in linearized form (using indents for children).
 */
extern int TraceParse;

/* TraceAnalyze = TRUE causes symbol table inserts and lookups to be reported
 * to the listing file.
 */
extern int TraceAnalyze;

/* TraceCode = TRUE causes comments to be written to the TM code file as code
 * is generated.
 */
extern int TraceCode;

/* Error = TRUE prevents further passes if an error occurs. */
extern int Error;

#endif
