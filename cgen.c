#include "globals.h"
#include "symtab.h"
#include "code.h"
#include "cgen.h"


static int tmpOffset = 0;  // 临时变量栈的顶部指针

static void cGen(TreeNode *tree);


/* Procedure genStmt：在语句节点生成代码 */
static void genStmt(TreeNode *tree)
{
    TreeNode *p1, *p2, *p3;
    int savedLoc1, savedLoc2, currentLoc;
    int loc;
    switch (tree->kind.stmt)
    {

    case IfK:
        if (TraceCode)
            emitComment("-> if");
        p1 = tree->child[0];
        p2 = tree->child[1];
        p3 = tree->child[2];
        
        /* 生成测试表达式的代码 */
        cGen(p1);
        savedLoc1 = emitSkip(1);  // 预留false jump到else部分的位置
        emitComment("if: jump to else belongs here");
        
        /* 递归then部分 */
        cGen(p2);
        savedLoc2 = emitSkip(1);  // 预留无条件转移跳过else部分的位置
        emitComment("if: jump to end belongs here");
        currentLoc = emitSkip(0);
        emitBackup(savedLoc1);
        emitRM_Abs("JEQ", ac, currentLoc, "if: jmp to else");
        emitRestore();
        
        /* 递归else部分 */
        cGen(p3);
        currentLoc = emitSkip(0);
        emitBackup(savedLoc2);
        emitRM_Abs("LDA", pc, currentLoc, "jmp to end");
        emitRestore();
        if (TraceCode)
            emitComment("<- if");
        break; 

    case RepeatK:
        if (TraceCode)
            emitComment("-> repeat");
        p1 = tree->child[0];
        p2 = tree->child[1];
        savedLoc1 = emitSkip(0);
        emitComment("repeat: jump after body comes back here");
        /* 生成循环体的代码 */
        cGen(p1);
        /* 生成测试表达式的代码 */
        cGen(p2);
        emitRM_Abs("JEQ", ac, savedLoc1, "repeat: jmp back to body");
        if (TraceCode)
            emitComment("<- repeat");
        break; 

    case AssignK:
        if (TraceCode)
            emitComment("-> assign");
        cGen(tree->child[0]);
        loc = st_lookup(tree->attr.name);  // 变量地址并以gp寄存器为基准
        emitRM("ST", ac, loc, gp, "assign: store value");  // 存储变量到内存
        if (TraceCode)
            emitComment("<- assign");
        break; 

    case ReadK:
        emitRO("IN", ac, 0, 0, "read integer value");
        loc = st_lookup(tree->attr.name);
        emitRM("ST", ac, loc, gp, "read: store value");  // 存储变量到内存
        break;
    case WriteK:
        cGen(tree->child[0]);
        emitRO("OUT", ac, 0, 0, "write ac");
        break;
    default:
        break;
    }
} 

/* Procedure genExp：在表达式节点生成代码 */
static void genExp(TreeNode *tree)
{
    int loc;
    TreeNode *p1, *p2;
    switch (tree->kind.exp)
    {

    case ConstK:
        if (TraceCode)
            emitComment("-> Const");
        /* reg(ac)=tree->attr.val 常数的值装入ac寄存器 */
        emitRM("LDC", ac, tree->attr.val, 0, "load const");
        if (TraceCode)
            emitComment("<- Const");
        break; 

    case IdK:
        if (TraceCode)
            emitComment("-> Id");
        loc = st_lookup(tree->attr.name);
        /* reg(ac)=dMem[loc+reg(gp)] 变量的值装入ac寄存器 */
        emitRM("LD", ac, loc, gp, "load id value");
        if (TraceCode)
            emitComment("<- Id");
        break; 

    case OpK:
        if (TraceCode)
            emitComment("-> Op");
        p1 = tree->child[0];
        p2 = tree->child[1];
        /* ac = 左操作数 */
        cGen(p1);
        /* 将左操作数保存到内存dMem[mp+tmpoffset]处 */
        emitRM("ST", ac, tmpOffset--, mp, "op: push left");
        /* ac = 右操作数 */
        cGen(p2);
        /* 将之前内存中的的左操作数加载到ac1寄存器 */
        emitRM("LD", ac1, ++tmpOffset, mp, "op: load left");
        switch (tree->attr.op)
        {
        case PLUS:
            emitRO("ADD", ac, ac1, ac, "op +");
            break;
        case MINUS:
            emitRO("SUB", ac, ac1, ac, "op -");
            break;
        case TIMES:
            emitRO("MUL", ac, ac1, ac, "op *");
            break;
        case OVER:
            emitRO("DIV", ac, ac1, ac, "op /");
            break;
        case LT:
            // ac=ac1-ac
            emitRO("SUB", ac, ac1, ac, "op <");
            // 若ac<0,跳过两条命令到最后一条，装入ac=1，代表LT为真
            emitRM("JLT", ac, 2, pc, "br if true");
            // 否则装入ac=0并跳过最后一条指令，代表LT为假
            emitRM("LDC", ac, 0, ac, "false case");
            emitRM("LDA", pc, 1, pc, "unconditional jmp");
            emitRM("LDC", ac, 1, ac, "true case");
            break;
        case EQ:
            //同上
            emitRO("SUB", ac, ac1, ac, "op ==");
            emitRM("JEQ", ac, 2, pc, "br if true");
            emitRM("LDC", ac, 0, ac, "false case");
            emitRM("LDA", pc, 1, pc, "unconditional jmp");
            emitRM("LDC", ac, 1, ac, "true case");
            break;
        default:
            emitComment("BUG: Unknown operator");
            break;
        } 
        if (TraceCode)
            emitComment("<- Op");
        break;

    default:
        break;
    }
}

/* Procedure cGen：通过树的遍历生成代码 */
static void cGen(TreeNode *tree)
{
    if (tree != NULL)
    {
        switch (tree->nodekind)
        {
        case StmtK:
            genStmt(tree);
            break;
        case ExpK:
            genExp(tree);
            break;
        default:
            break;
        }
        cGen(tree->sibling);
    }
}

/**********************************************/
/* 代码生成器主程 */
/**********************************************/
void codeGen(TreeNode *syntaxTree, char *codefile)
{
    char *s = malloc(strlen(codefile) + 7);
    strcpy(s, "File: ");
    strcat(s, codefile);
    emitComment("TINY Compilation to TM Code");
    emitComment(s);

    /* 生成标准序言 */
    emitComment("Standard prelude:");
    emitRM("LD", mp, 0, ac, "load maxaddress from location 0");
    emitRM("ST", ac, 0, ac, "clear location 0");
    emitComment("End of standard prelude.");
    
    /* 生成代码 */
    cGen(syntaxTree);
    
    /* 结束 */
    emitComment("End of execution.");
    emitRO("HALT", 0, 0, 0, "");
}
