/*
File: SCAN.C
The scanner implementation for the C-minus compiler
*/

#include "globals.h"
#include "util.h"
#include "scan.h"

/* states in scanner DFA */
typedef enum
{
    START,   /* start of token */
    INNUM,   /* check if number */
    INID,    /* check if identifier */
    INLT,    /* check if < */
    INGT,    /* check if > */
    INEQ,    /* check if == */
    INSTAR,  /* check if * */
    INNOT,   /* check if ! */
    INSLASH, /* check if / */
    INCMT,   /* check if comment */
    INEND,   /* check if comment ending */
    DONE     /* accecpt token */
} StateType;

char tokenString[MAXTOKENLEN + 1]; /* holds current token string */
/* tokenLine: line number associated with the most recently returned token (1-based) */
int tokenLine = 0;

#define BUFLEN 256 /* length of the input buffer for source code lines */

static char lineBuf[BUFLEN];           /* holds the current line */
static int linepos = 0;                /* current position in LineBuf */
static int bufsize = 0;                /* current size of buffer string */
static int EOF_flag = FALSE;           /* corrects ungetNextChar behavior on EOF */
static int lastLineHadNewline = FALSE; /* tracks whether the last buffered line ended with '\n' */
static int eofTokenLineOverride = -1;  /* preserves the comment line for the ENDFILE token after unterminated comments */
static int unterminatedCommentEOF = FALSE;

void resetScanner(void)
{
    lineBuf[0] = '\0';
    linepos = 0;
    bufsize = 0;
    EOF_flag = FALSE;
    lastLineHadNewline = FALSE;
    eofTokenLineOverride = -1;
    unterminatedCommentEOF = FALSE;
    tokenLine = 0;
    tokenString[0] = '\0';
}

/* getNextChar fetches the next non-blank character
   from lineBuf, reading in a new line if lineBuf is
   exhausted */
static int getNextChar(void)
{
    if (!(linepos < bufsize))
    {
        lineno++;
        if (fgets(lineBuf, BUFLEN - 1, source))
        {
            bufsize = strlen(lineBuf);
            lastLineHadNewline = (bufsize > 0 && lineBuf[bufsize - 1] == '\n');
            if (EchoSource)
                printSourceLine(listing, lineno, lineBuf);
            linepos = 0;
            return lineBuf[linepos++];
        }
        else
        {
            EOF_flag = TRUE;
            return EOF;
        }
    }
    else
        return lineBuf[linepos++];
}

/* ungetNextChar backtracks one character
   in lineBuf */
static void ungetNextChar(void)
{
    if (!EOF_flag)
        linepos--;
}

/* lookup table of reserved words */
static struct
{
    char *str;
    TokenType tok;
} reservedWords[MAXRESERVED] = {
    {"if", IF},
    {"else", ELSE},
    {"int", INT},
    {"return", RETURN},
    {"void", VOID},
    {"while", WHILE}};

/* lookup an identifier to see if it is a reserved word */
/* uses linear search */
static TokenType reservedLookup(char *s)
{
    int i;
    for (i = 0; i < MAXRESERVED; i++)
        if (!strcmp(s, reservedWords[i].str))
            return reservedWords[i].tok;
    return ID;
}

/****************************************/
/* the primary function of the scanner  */
/****************************************/
/* function getToken returns the next token in source file */
TokenType getToken(void)
{
    int tokenStringIndex = 0; /* index for storing into tokenString */
    TokenType currentToken;   /* holds current token to be returned */
    StateType state = START;  /* current state - always begins at START */
    int save;                 /* flag to indicate save to tokenString */

    while (state != DONE)
    {
        int c = getNextChar();
        save = TRUE;
        switch (state)
        {
        case START:
            if (c == EOF)
            {
                save = FALSE;
                state = DONE;
                currentToken = ENDFILE;
            }
            else if ((c == ' ') || (c == '\t') || (c == '\n'))
                save = FALSE;
            else if (isdigit(c))
            {
                state = INNUM;
            }
            else if (isalpha(c))
            {
                state = INID;
            }
            else if (c == '<')
            {
                state = INLT;
            }
            else if (c == '>')
            {
                state = INGT;
            }
            else if (c == '=')
            {
                state = INEQ;
            }
            else if (c == '!')
            {
                state = INNOT;
            }
            else if (c == '/')
            {
                state = INSLASH;
            }
            else if (c == '*')
            {
                state = INSTAR;
            }
            else
            {
                state = DONE;
                switch (c)
                {
                case EOF:
                    save = FALSE;
                    currentToken = ENDFILE;
                    break;
                case '+':
                    currentToken = PLUS;
                    break;
                case '-':
                    currentToken = MINUS;
                    break;
                case '*':
                    currentToken = TIMES;
                    break;
                case ',':
                    currentToken = COMMA;
                    break;
                case ';':
                    currentToken = SEMI;
                    break;
                case '(':
                    currentToken = LPAREN;
                    break;
                case ')':
                    currentToken = RPAREN;
                    break;
                case '[':
                    currentToken = LBRACKET;
                    break;
                case ']':
                    currentToken = RBRACKET;
                    break;
                case '{':
                    currentToken = LBRACE;
                    break;
                case '}':
                    currentToken = RBRACE;
                    break;
                default:
                    currentToken = ERROR;
                    break;
                }
            }
            break;
        case INNUM:
            if (!isdigit(c))
            {
                ungetNextChar();
                save = FALSE;
                state = DONE;
                currentToken = NUM;
            }
            break;
        case INID:
            if (!isalpha(c))
            {
                ungetNextChar();
                save = FALSE;
                state = DONE;
                currentToken = ID;
            }
            break;
        case INLT:
            if (c == '=')
            {
                currentToken = LTE;
                state = DONE;
            }
            else
            {
                ungetNextChar();
                save = FALSE;
                currentToken = LT;
                state = DONE;
            }
            break;
        case INGT:
            if (c == '=')
            {
                currentToken = GTE;
                state = DONE;
            }
            else
            {
                ungetNextChar();
                save = FALSE;
                currentToken = GT;
                state = DONE;
            }
            break;
        case INEQ:
            if (c == '=')
            {
                currentToken = EQ;
                state = DONE;
            }
            else
            {
                ungetNextChar();
                save = FALSE;
                currentToken = ASSIGN;
                state = DONE;
            }
            break;
        case INNOT:
            if (c == '=')
            {
                currentToken = NEQ;
                state = DONE;
            }
            else
            {
                ungetNextChar();
                save = FALSE;
                currentToken = ERROR;
                state = DONE;
            }
            break;
        case INSTAR:
            if (c == '/')
            {
                save = FALSE;
                state = DONE;
                currentToken = ERROR;
                tokenStringIndex = 0;
                tokenString[tokenStringIndex++] = '*';
                tokenString[tokenStringIndex++] = '/';
            }
            else
            {
                ungetNextChar();
                state = DONE;
                currentToken = TIMES;
            }
            break;
        case INSLASH:
            if (c == '*')
            {
                state = INCMT;
            }
            else
            {
                ungetNextChar();
                save = FALSE;
                currentToken = OVER;
                state = DONE;
            }
            break;
        case INCMT:
            if (c == '*')
                state = INEND;
            else if (c == EOF)
            {
                while (tokenStringIndex > 0 &&
                       (tokenString[tokenStringIndex - 1] == '\n' ||
                        tokenString[tokenStringIndex - 1] == '\r'))
                    tokenStringIndex--;
                save = FALSE;
                state = DONE;
                currentToken = ERROR;
                unterminatedCommentEOF = TRUE;
            }
            else
                state = INCMT;
            break;
        case INEND:
            if (c == '/')
            {
                save = FALSE;
                tokenStringIndex = 0;
                state = START;
            }
            else if (c == EOF)
            {
                while (tokenStringIndex > 0 &&
                       (tokenString[tokenStringIndex - 1] == '\n' ||
                        tokenString[tokenStringIndex - 1] == '\r'))
                    tokenStringIndex--;
                save = FALSE;
                state = DONE;
                currentToken = ERROR;
                unterminatedCommentEOF = TRUE;
            }
            else
                state = INCMT;
            break;
        case DONE:
        default:
            fprintf(listing, "Scanner Bug: state=%d\n", state);
            state = DONE;
            currentToken = ERROR;
            break;
        }
        if ((save) && (tokenStringIndex <= MAXTOKENLEN))
            tokenString[tokenStringIndex++] = (char)c;
        if (state == DONE)
        {
            tokenString[tokenStringIndex] = '\0';
            if (currentToken == ID)
                currentToken = reservedLookup(tokenString);
        }
    }
    tokenLine = lineno;
    if (EOF_flag && !lastLineHadNewline && tokenLine > 0)
        tokenLine--;
    if (currentToken == ENDFILE && eofTokenLineOverride >= 0)
    {
        tokenLine = eofTokenLineOverride;
        eofTokenLineOverride = -1;
    }
    if (currentToken == ERROR && unterminatedCommentEOF)
    {
        eofTokenLineOverride = tokenLine;
        unterminatedCommentEOF = FALSE;
    }
    if (TraceScan)
    {
        fprintf(listing, "    %d: ", tokenLine);
        printToken(currentToken, tokenString);
    }
    return currentToken;
} /* end getToken */