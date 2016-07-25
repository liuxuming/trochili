/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TCLC_MEMORY_POOL_H
#define _TCLC_MEMORY_POOL_H

#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.object.h"
#include "tcl.memory.h"

#if ((TCLC_MEMORY_ENABLE) && (TCLC_MEMORY_POOL_ENABLE))

#define MEM_PAGE_TAGS  ((TCLC_MEMORY_POOL_PAGES + 31U) >> 5u)

/* 内存池控制块结构 */
struct MemPoolDef
{
    TProperty Property;                   /* 内存页池属性                      */
    TChar*    PageAddr;                   /* 被管理的内存的起始地址            */
    TBase32   PageSize;                   /* 内存页大小                        */
    TBase32   PageNbr;                    /* 内存页数目                        */
    TBase32   PageAvail;                  /* 可用内存页数目                    */
    TBase32   PageTags[MEM_PAGE_TAGS];    /* 内存页是否可用标记                */
    TObjNode* PageList;                   /* 可用内存页链表头指针              */
};
typedef struct MemPoolDef TMemPool;

extern TState xMemPoolCreate(TMemPool* pPool, void* pData, TBase32 pages, TBase32 pgsize, TError* pError);
extern TState xMemPoolDelete(TMemPool* pPool, TError* pError);
extern TState xPoolMemMalloc(TMemPool* pPool, void** pAddr2, TError* pError);
extern TState xPoolMemFree (TMemPool* pPool, void* pAddr, TError* pError);
#endif

#endif /* _TCLC_MEMORY_POOL_H  */

