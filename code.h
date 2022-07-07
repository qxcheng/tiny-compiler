#ifndef _CODE_H_
#define _CODE_H_


#define  pc 7  // 程序计数器
#define  mp 6  // 指向内存顶端，用于存储临时变量（相当于栈）
#define  gp 5  // 指向内存低端，用于存储变量

#define  ac 0   // 累加器
#define  ac1 1  // 另一个累加器


void emitComment( char * c );
void emitRO( char *op, int r, int s, int t, char *c);
void emitRM( char * op, int r, int d, int s, char *c);
int emitSkip( int howMany);
void emitBackup( int loc);
void emitRestore(void);
void emitRM_Abs( char *op, int r, int a, char * c);

#endif
