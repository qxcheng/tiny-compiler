#include "globals.h"
#include "util.h"
#include "scan.h"

// DFA的状态
typedef enum { START,INASSIGN,INCOMMENT,INNUM,INID,DONE } StateType;

/* 标识符或保留字的特性：词法或是被识别的记号的串值 */
char tokenString[MAXTOKENLEN + 1];

#define BUFLEN 256  // 源文件行的缓存长度

static char lineBuf[BUFLEN];  /* 当前行 */
static int linepos = 0;       /* 行的当前位置 */
static int bufsize = 0;       /* 当前行的长度 */
static int EOF_flag = FALSE;  /* EOF时纠正ungetNextChar的行为 */


/* getNextChar 获取当前行的下一个非空字符，当前行无字符时读取新行 */
static int getNextChar(void)
{
    // 需要读取新的行
    if (!(linepos < bufsize))
    {
        lineno++;
        if (fgets(lineBuf, BUFLEN - 1, source))
        {
            if (EchoSource)
                fprintf(listing, "%4d: %s", lineno, lineBuf);
            bufsize = strlen(lineBuf);
            linepos = 0;
            return lineBuf[linepos++];
        }
        else  // 发生EOF 
        {
            EOF_flag = TRUE;
            return EOF;
        }
    }
    else  // 直接读取下一个字符
        return lineBuf[linepos++];
}

/* ungetNextChar 非EOF时回退当前行的一个字符 */
static void ungetNextChar(void)
{
    if (!EOF_flag)
        linepos--;
}

/* 保留字查询表 */
static struct
{
    char *str;
    TokenType tok;
} reservedWords[MAXRESERVED] = {{"if", IF}, {"then", THEN}, {"else", ELSE}, {"end", END}, {"repeat", REPEAT}, {"until", UNTIL}, {"read", READ}, {"write", WRITE}};

/* 查找一个标识符是否是保留字 */
static TokenType reservedLookup(char *s)
{
    int i;
    for (i = 0; i < MAXRESERVED; i++)
        if (!strcmp(s, reservedWords[i].str))
            return reservedWords[i].tok;
    return ID;
}

// 返回源文件的下一个记号
TokenType getToken(void)
{
    int tokenStringIndex = 0; // 保存tokenString的索引
    TokenType currentToken;   // 当前要返回的记号
    StateType state = START;  // 当前状态 - 永远从START开始
    int save;                 // 是否保存到tokenString

    while (state != DONE)
    {
        int c = getNextChar();
        save = TRUE;
        switch (state)
        {
        case START:
            if (isdigit(c))
                state = INNUM;
            else if (isalpha(c))
                state = INID;
            else if (c == ':')
                state = INASSIGN;
            else if ((c == ' ') || (c == '\t') || (c == '\n'))
                save = FALSE;  // 空白字符无属性保存
            else if (c == '{')
            {
                save = FALSE;  // 注释不保存
                state = INCOMMENT;
            }
            else
            {
                state = DONE;
                switch (c)
                {
                case EOF:
                    save = FALSE;  // EOF不保存
                    currentToken = ENDFILE;
                    break;
                case '=':
                    currentToken = EQ;
                    break;
                case '<':
                    currentToken = LT;
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
                case '/':
                    currentToken = OVER;
                    break;
                case '(':
                    currentToken = LPAREN;
                    break;
                case ')':
                    currentToken = RPAREN;
                    break;
                case ';':
                    currentToken = SEMI;
                    break;
                default:
                    currentToken = ERROR;
                    break;
                }
            }
            break;
        case INCOMMENT:
            save = FALSE;
            if (c == EOF)
            {
                state = DONE;
                currentToken = ENDFILE;
            }
            else if (c == '}')
                state = START;
            break;
        case INASSIGN:
            state = DONE;
            if (c == '=')
                currentToken = ASSIGN;
            else
            {   /* 回退一个字符 */
                ungetNextChar();
                save = FALSE;
                currentToken = ERROR;
            }
            break;
        case INNUM:
            if (!isdigit(c))
            {   /* 回退一个字符 */
                ungetNextChar();
                save = FALSE;
                state = DONE;
                currentToken = NUM;
            }
            break;
        case INID:
            if (!isalpha(c))
            {   /* 回退一个字符 */
                ungetNextChar();
                save = FALSE;
                state = DONE;
                currentToken = ID;
            }
            break;
        case DONE:
        default: /* 永远不应该发生 */
            fprintf(listing, "Scanner Bug: state= %d\n", state);
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
    if (TraceScan)
    {
        fprintf(listing, "\t%d: ", lineno);
        printToken(currentToken, tokenString);
    }
    return currentToken;
}
