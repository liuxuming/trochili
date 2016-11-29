/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TCLC_MEMORY_BUDDY_H
#define _TCLC_MEMORY_BUDDY_H

#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.memory.h"

#if ((TCLC_MEMORY_ENABLE) && (TCLC_MEMORY_BUDDY_ENABLE))

#define OS_MEM_BUDDY_PAGE_TAGS  ((TCLC_MEMORY_BUDDY_PAGES + 31U) >> 5U)
#define OS_MEM_BUDDY_NODE_TAGS (TCLC_MEMORY_BUDDY_PAGES * 2U - 1U)

typedef struct MemBuddyDef
{
    TProperty Property;                       /* 内存页池属性                      */
    TChar*    PageAddr;                       /* 被管理的内存的起始地址            */
    TBase32   PageSize;                       /* 内存页大小                        */
    TBase32   PageNbr;                        /* 内存页数目                        */
    TBase32   PageAvail;                      /* 可用内存页数目                    */
    TBitMask  PageTags[OS_MEM_BUDDY_PAGE_TAGS];  /* 内存页是否可用标记                */
    TBase32   NodeNbr;
    TByte     NodeTags[OS_MEM_BUDDY_NODE_TAGS];
} TMemBuddy;


extern TState TclCreateMemoryBuddy(TMemBuddy* pBuddy, TChar* pAddr, TBase32 pages, TBase32 pagesize, TError* pError);
extern TState TclDeleteMemoryBuddy(TMemBuddy* pBuddy, TError* pError);
extern TState TclMallocBuddyMemory(TMemBuddy* pBuddy, TBase32 length, void** pAddr2, TError* pError);
extern TState TclFreeBuddyMemory(TMemBuddy* pBuddy,  void* pAddr, TError* pError);

#endif

#endif /* _TCLC_MEMORY_BUDDY_H  */

