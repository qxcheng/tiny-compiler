#include "globals.h"
#include "symtab.h"
#include "analyze.h"


static int location = 0;  /* 变量内存地址计数器 */


/* Procedure traverse: preProc前序遍历t postProc后序遍历t */
static void traverse(TreeNode *t,
                     void (*preProc)(TreeNode *),
                     void (*postProc)(TreeNode *))
{
    if (t != NULL)
    {
        preProc(t);
        {
            int i;
            for (i = 0; i < MAXCHILDREN; i++)
                traverse(t->child[i], preProc, postProc);
        }
        postProc(t);
        traverse(t->sibling, preProc, postProc);
    }
}

/* nullProc: 不做任何事 */
static void nullProc(TreeNode *t)
{
    if (t == NULL)
        return;
    else
        return;
}

/* Procedure insertNode：插入t中的标识符到符号表 */
static void insertNode(TreeNode *t)
{
    switch (t->nodekind)
    {
    case StmtK:
        switch (t->kind.stmt)
        {
        case AssignK:
        case ReadK:
            if (st_lookup(t->attr.name) == -1)
                /* 不在表中，插入新的记录 */
                st_insert(t->attr.name, t->lineno, location++);
            else
                /* 在表中，忽略地址，只插入行号 */
                st_insert(t->attr.name, t->lineno, 0);
            break;
        default:
            break;
        }
        break;
    case ExpK:
        switch (t->kind.exp)
        {
        case IdK:
            if (st_lookup(t->attr.name) == -1)
                /* 不在表中，插入新的记录 */
                st_insert(t->attr.name, t->lineno, location++);
            else
                /* 在表中，忽略地址，只插入行号 */
                st_insert(t->attr.name, t->lineno, 0);
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

static void typeError(TreeNode *t, char *message)
{
    fprintf(listing, "Type error at line %d: %s\n", t->lineno, message);
    Error = TRUE;
}

/* Procedure checkNode：在一个节点上进行类型检查 */
static void checkNode(TreeNode *t)
{
    switch (t->nodekind)
    {
    case ExpK:
        switch (t->kind.exp)
        {
        case OpK:
            if ((t->child[0]->type != Integer) ||
                (t->child[1]->type != Integer))
                typeError(t, "Op applied to non-integer");
            if ((t->attr.op == EQ) || (t->attr.op == LT))
                t->type = Boolean;
            else
                t->type = Integer;
            break;
        case ConstK:
        case IdK:
            t->type = Integer;
            break;
        default:
            break;
        }
        break;
    case StmtK:
        switch (t->kind.stmt)
        {
        case IfK:
            if (t->child[0]->type == Integer)
                typeError(t->child[0], "if test is not Boolean");
            break;
        case AssignK:
            if (t->child[0]->type != Integer)
                typeError(t->child[0], "assignment of non-integer value");
            break;
        case WriteK:
            if (t->child[0]->type != Integer)
                typeError(t->child[0], "write of non-integer value");
            break;
        case RepeatK:
            if (t->child[1]->type == Integer)
                typeError(t->child[1], "repeat test is not Boolean");
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

/* Function buildSymtab：通过前序遍历语法树构建符号表 */
void buildSymtab(TreeNode *syntaxTree)
{
    traverse(syntaxTree, insertNode, nullProc);
    if (TraceAnalyze)
    {
        fprintf(listing, "\nSymbol table:\n\n");
        printSymTab(listing);
    }
}

/* Procedure typeCheck：通过后序遍历语法树进行类型检查 */
void typeCheck(TreeNode *syntaxTree)
{
    traverse(syntaxTree, nullProc, checkNode);
}
