#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"

static TokenType token; /* 当前token */

static TreeNode *stmt_sequence(void);
static TreeNode *statement(void);
static TreeNode *if_stmt(void);
static TreeNode *repeat_stmt(void);
static TreeNode *assign_stmt(void);
static TreeNode *read_stmt(void);
static TreeNode *write_stmt(void);
static TreeNode *exp(void);
static TreeNode *simple_exp(void);
static TreeNode *term(void);
static TreeNode *factor(void);


static void syntaxError(char *message)
{
    fprintf(listing, "\n>>> ");
    fprintf(listing, "Syntax error at line %d: %s", lineno, message);
    Error = TRUE;
}

static void match(TokenType expected)
{
    // 如果当前token与期望的匹配，则获取下一个token
    if (token == expected)
        token = getToken();
    else
    {
        syntaxError("unexpected token -> ");
        printToken(token, tokenString);
        fprintf(listing, "      ");
    }
}

// 递归：语句序列
TreeNode *stmt_sequence(void)
{
    TreeNode *t = statement();  // t指向第一个语句
    TreeNode *p = t;
    while ((token != ENDFILE) && (token != END) &&
           (token != ELSE) && (token != UNTIL))
    {
        TreeNode *q;
        match(SEMI);
        q = statement();
        if (q != NULL)
        {
            if (t == NULL)
                t = p = q;
            else /* 将语句组成一个链表 */
            {
                p->sibling = q;
                p = q;
            }
        }
    }
    return t;
}

// 递归：5种语句类型
TreeNode *statement(void)
{
    TreeNode *t = NULL;
    switch (token)
    {
    case IF:
        t = if_stmt();
        break;
    case REPEAT:
        t = repeat_stmt();
        break;
    case ID:
        t = assign_stmt();
        break;
    case READ:
        t = read_stmt();
        break;
    case WRITE:
        t = write_stmt();
        break;
    default:
        syntaxError("unexpected token -> ");
        printToken(token, tokenString);
        token = getToken();
        break;
    } 
    return t;
}

// 递归：条件语句
TreeNode *if_stmt(void)
{
    TreeNode *t = newStmtNode(IfK);
    match(IF);
    if (t != NULL)
        t->child[0] = exp();
    match(THEN);
    if (t != NULL)
        t->child[1] = stmt_sequence();
    if (token == ELSE)
    {
        match(ELSE);
        if (t != NULL)
            t->child[2] = stmt_sequence();
    }
    match(END);
    return t;
}

// 递归：循环语句
TreeNode *repeat_stmt(void)
{
    TreeNode *t = newStmtNode(RepeatK);
    match(REPEAT);
    if (t != NULL)
        t->child[0] = stmt_sequence();
    match(UNTIL);
    if (t != NULL)
        t->child[1] = exp();
    return t;
}

// 递归：赋值语句：读取变量，并将该标识符名存入本节点的属性中
TreeNode *assign_stmt(void)
{
    TreeNode *t = newStmtNode(AssignK);
    if ((t != NULL) && (token == ID))
        t->attr.name = copyString(tokenString);
    match(ID);
    match(ASSIGN);
    if (t != NULL)
        t->child[0] = exp();
    return t;
}

// 递归：输入语句：读取变量，并将该标识符名存入本节点的属性中
TreeNode *read_stmt(void)
{
    TreeNode *t = newStmtNode(ReadK);
    match(READ);
    if ((t != NULL) && (token == ID))
        t->attr.name = copyString(tokenString);
    match(ID);
    return t;
}

// 递归：输出语句
TreeNode *write_stmt(void)
{
    TreeNode *t = newStmtNode(WriteK);
    match(WRITE);
    if (t != NULL)
        t->child[0] = exp();
    return t;
}

// 递归：表达式
TreeNode *exp(void)
{
    TreeNode *t = simple_exp();
    if ((token == LT) || (token == EQ))
    {
        TreeNode *p = newExpNode(OpK);
        if (p != NULL)
        {
            p->child[0] = t;
            p->attr.op = token;
            t = p;
        }
        match(token);
        if (t != NULL)
            t->child[1] = simple_exp();
    }
    return t;
}

// 递归：简单表达式：+ -
TreeNode *simple_exp(void)
{
    TreeNode *t = term();
    while ((token == PLUS) || (token == MINUS))
    {
        TreeNode *p = newExpNode(OpK);
        if (p != NULL)
        {
            p->child[0] = t;
            p->attr.op = token;
            t = p;
            match(token);
            t->child[1] = term();
        }
    }
    return t;
}

// 递归：* /
TreeNode *term(void)
{
    TreeNode *t = factor();
    while ((token == TIMES) || (token == OVER))
    {
        TreeNode *p = newExpNode(OpK);
        if (p != NULL)
        {
            p->child[0] = t;
            p->attr.op = token;
            t = p;
            match(token);
            p->child[1] = factor();
        }
    }
    return t;
}

// 递归：因子：exp | 常量 | 变量
TreeNode *factor(void)
{
    TreeNode *t = NULL;
    switch (token)
    {
    case NUM:
        t = newExpNode(ConstK);
        if ((t != NULL) && (token == NUM))
            t->attr.val = atoi(tokenString);
        match(NUM);
        break;
    case ID:
        t = newExpNode(IdK);
        if ((t != NULL) && (token == ID))
            t->attr.name = copyString(tokenString);
        match(ID);
        break;
    case LPAREN:
        match(LPAREN);
        t = exp();
        match(RPAREN);
        break;
    default:
        syntaxError("unexpected token -> ");
        printToken(token, tokenString);
        token = getToken();
        break;
    }
    return t;
}

/* 分析主程序：返回新建的语法树 */
TreeNode *parse(void)
{
    TreeNode *t;
    token = getToken();
    t = stmt_sequence();
    if (token != ENDFILE)
        syntaxError("Code ends before file\n");
    return t;
}
