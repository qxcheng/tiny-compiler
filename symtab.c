#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"


#define SIZE 211  // 哈希表的尺寸
#define SHIFT 4   

/* 哈希函数 */
static int hash(char *key)
{
    int temp = 0;
    int i = 0;
    while (key[i] != '\0')
    {
        temp = ((temp << SHIFT) + key[i]) % SIZE;
        ++i;
    }
    return temp;
}

/* 一个变量在源文件中所在的所有行号链表 */
typedef struct LineListRec
{
    int lineno;
    struct LineListRec *next;
} * LineList;

/* 每个变量的记录：包括name, 分配的内存地址, 在源文件中出现的行号 */
typedef struct BucketListRec
{
    char *name;
    LineList lines;
    int memloc;
    struct BucketListRec *next;
} * BucketList;

/* 哈希表 */
static BucketList hashTable[SIZE];

/* Procedure st_insert：插入行号和内存地址（只插入一次）到符号表 */
void st_insert(char *name, int lineno, int loc)
{
    int h = hash(name);
    BucketList l = hashTable[h];
    while ((l != NULL) && (strcmp(name, l->name) != 0))
        l = l->next;
    if (l == NULL) /* 变量不在表中 */
    {
        l = (BucketList)malloc(sizeof(struct BucketListRec));
        l->name = name;
        l->lines = (LineList)malloc(sizeof(struct LineListRec));
        l->lines->lineno = lineno;
        l->memloc = loc;
        l->lines->next = NULL;
        l->next = hashTable[h];
        hashTable[h] = l;
    }
    else /* 变量已在表中，只插入行号 */
    {
        LineList t = l->lines;
        while (t->next != NULL)
            t = t->next;
        t->next = (LineList)malloc(sizeof(struct LineListRec));
        t->next->lineno = lineno;
        t->next->next = NULL;
    }
} 

/* Function st_lookup: 返回一个变量的内存地址, 不存在返回-1 */
int st_lookup(char *name)
{
    int h = hash(name);
    BucketList l = hashTable[h];
    while ((l != NULL) && (strcmp(name, l->name) != 0))
        l = l->next;
    if (l == NULL)
        return -1;
    else
        return l->memloc;
}

/* Procedure printSymTab：打印符号表 */
void printSymTab(FILE *listing)
{
    int i;
    fprintf(listing, "Variable Name  Location   Line Numbers\n");
    fprintf(listing, "-------------  --------   ------------\n");
    for (i = 0; i < SIZE; ++i)
    {
        if (hashTable[i] != NULL)
        {
            BucketList l = hashTable[i];
            while (l != NULL)
            {
                LineList t = l->lines;
                fprintf(listing, "%-14s ", l->name);
                fprintf(listing, "%-8d  ", l->memloc);
                while (t != NULL)
                {
                    fprintf(listing, "%4d ", t->lineno);
                    t = t->next;
                }
                fprintf(listing, "\n");
                l = l->next;
            }
        }
    }
} 
