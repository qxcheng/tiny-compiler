#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#define MAXRESERVED 8  // 保留字的个数

typedef enum 
{   
    ENDFILE,ERROR,                             /* 内务记号 */
    IF,THEN,ELSE,END,REPEAT,UNTIL,READ,WRITE,  /* 保留字 */
    ID,NUM,                                    /* 多字符记号 */
    ASSIGN,EQ,LT,PLUS,MINUS,TIMES,OVER,LPAREN,RPAREN,SEMI  // 特殊符号 
} TokenType;

extern FILE *source;  /* 源代码 */
extern FILE *listing; /* 列举输出文件 */
extern FILE *code;    /* 运行在TM虚拟机上的代码 */

extern int lineno;    /* 源代码行号 */

/**************************************************/
/***********   Syntax tree for parsing ************/
/**************************************************/

typedef enum { StmtK, ExpK } NodeKind;
typedef enum { IfK, RepeatK, AssignK, ReadK, WriteK } StmtKind;
typedef enum { OpK, ConstK, IdK } ExpKind;

typedef enum { Void, Integer, Boolean } ExpType;  // 用于类型检查

#define MAXCHILDREN 3  // 最大子节点数

typedef struct treeNode
{
   struct treeNode *child[MAXCHILDREN];
   struct treeNode *sibling;  // 兄弟节点
   int lineno;
   NodeKind nodekind;  // 节点类型
   union
   {
      StmtKind stmt;  // 语句类型
      ExpKind exp;    // 表达式类型
   } kind;
   union
   {
      TokenType op;
      int val;
      char *name;
   } attr;
   ExpType type; /* for type checking of exps */
} TreeNode;

/**************************************************/
/***********   Flags for tracing       ************/
/**************************************************/

extern int EchoSource;
extern int TraceScan;
extern int TraceParse;
extern int TraceAnalyze;
extern int TraceCode;
extern int Error;

#endif
