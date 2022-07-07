#include "globals.h"
#include "code.h"

/* 当前指令的TM位置（PS：指向的位置没有指令） */
static int emitLoc = 0;

/* 目前为止的最高TM位置，用于emitSkip, emitBackup, and emitRestore */
static int highEmitLoc = 0;


/* Procedure emitComment：生成一行注释 */
void emitComment(char *c)
{
    if (TraceCode)
        fprintf(code, "* %s\n", c);
}

/* Procedure emitRO：生成RO类指令 
 * op = the opcode
 * r = target register
 * s = 1st source register
 * t = 2nd source register
 * c = a comment to be printed if TraceCode is TRUE
 */
void emitRO(char *op, int r, int s, int t, char *c)
{
    fprintf(code, "%3d:  %5s  %d,%d,%d ", emitLoc++, op, r, s, t);
    if (TraceCode)
        fprintf(code, "\t%s", c);
    fprintf(code, "\n");
    if (highEmitLoc < emitLoc)
        highEmitLoc = emitLoc;
} 

/* Procedure emitRM：生成RM类指令
 * op = the opcode
 * r = target register
 * d = the offset
 * s = the base register
 * c = a comment to be printed if TraceCode is TRUE
 */
void emitRM(char *op, int r, int d, int s, char *c)
{
    fprintf(code, "%3d:  %5s  %d,%d(%d) ", emitLoc++, op, r, d, s);
    if (TraceCode)
        fprintf(code, "\t%s", c);
    fprintf(code, "\n");
    if (highEmitLoc < emitLoc)
        highEmitLoc = emitLoc;
} 

/* Function emitSkip：用于跳过将来要反填的一些位置并返回当前指令位置
emitSkip(1)跳过一个位置，这个位置后来会填上转移指令；
emitSkip(0)不跳过位置，调用它只是为了得到当前位置以备后来的转移引用；
 */
int emitSkip(int howMany)
{
    int i = emitLoc;
    emitLoc += howMany;
    if (highEmitLoc < emitLoc)
        highEmitLoc = emitLoc;
    return i;
} 

/* Procedure emitBackup：设置当前指令位置到先前位置来反填 */
void emitBackup(int loc)
{
    if (loc > highEmitLoc)
        emitComment("BUG in emitBackup");
    emitLoc = loc;
} 

/* Procedure emitRestore：恢复当前指令位置到最高位置 */
void emitRestore(void)
{
    emitLoc = highEmitLoc;
}

/* Procedure emitRM_Abs：产生诸如反填转移或任何由调用emitSkip返回的代码
 * 位置的转移的代码。
 * a是当前位置, emitLoc是预留的位置（if对应的：41-(13+1)=27）
 * op = the opcode
 * r = target register
 * a = the absolute location in memory
 * c = a comment to be printed if TraceCode is TRUE
 */
void emitRM_Abs(char *op, int r, int a, char *c)
{
    fprintf(code, "%3d:  %5s  %d,%d(%d) ",
            emitLoc, op, r, a - (emitLoc + 1), pc);
    ++emitLoc;
    if (TraceCode)
        fprintf(code, "\t%s", c);
    fprintf(code, "\n");
    if (highEmitLoc < emitLoc)
        highEmitLoc = emitLoc;
} 
