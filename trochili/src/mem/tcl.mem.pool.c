/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#include "string.h"

#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.debug.h"
#include "tcl.cpu.h"
#include "tcl.mem.pool.h"

#if ((TCLC_MEMORY_ENABLE) && (TCLC_MEMORY_POOL_ENABLE))
/*************************************************************************************************
 *  功能: 初始化内存页池                                                                         *
 *  参数: (1) pPool      内存页池结构地址                                                        *
 *        (2) pAddr      内存池数据区地址                                                        *
 *        (3) pages      内存池中内存页数目                                                      *
 *        (4) pgsize     内存页大小                                                              *
 *        (5) pError     详细调用结果                                                            *
 *  返回: (1) eSuccess   操作成功                                                                *
 *        (2) eFailure   操作失败                                                                *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState xMemPoolCreate(TMemPool* pPool, void* pAddr, TBase32 pages, TBase32 pgsize, TError* pError)
{
    TState state = eFailure;
    TError error = MEM_ERR_FAULT;
    TReg32 imask;
    TIndex index;
    TChar* pTemp;

    CpuEnterCritical(&imask);

    if (!(pPool->Property & MEM_PROP_READY))
    {
        /* 清空被管理的内存空间 */
        memset(pAddr, 0U, pages * pgsize);

        /* 建立可用内存页链表 */
        pTemp = (TChar*)pAddr;
        for (index = 0; index < pages; index++)
        {
            uObjListAddNode(&(pPool->PageList), (TObjNode*)pTemp, eQuePosTail);
            pTemp += pgsize;
        }

        /* 设置所有内存都处于可分配状态 */
        for (index = 0; index < MEM_PAGE_TAGS; index++)
        {
            pPool->PageTags[index] = ~0U;
        }
        pPool->PageAddr  = pAddr;
        pPool->PageAvail = pages;
        pPool->PageNbr   = pages;
        pPool->PageSize  = pgsize;
        pPool->Property  = MEM_PROP_READY;

        error = MEM_ERR_NONE;
        state = eSuccess;
    }

    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能: 销毁内存池                                                                             *
 *  参数: (1) pPool      内存池结构地址                                                          *
 *        (2) pError     详细调用结果                                                            *
 *  返回: (1) eSuccess   操作成功                                                                *
 *        (2) eFailure   操作失败                                                                *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState xMemPoolDelete(TMemPool* pPool, TError* pError)
{
    TReg32 imask;
    TState state = eFailure;
    TError error = MEM_ERR_UNREADY;

    CpuEnterCritical(&imask);
    if (pPool->Property & MEM_PROP_READY)
    {
        memset(pPool->PageAddr, 0, pPool->PageNbr * pPool->PageSize);
        memset(pPool, 0, sizeof(TMemPool));
        error = MEM_ERR_NONE;
        state = eSuccess;
    }
    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能: 从内存池中申请分配内存                                                                 *
 *  参数: (1) pPool      内存池结构地址                                                          *
 *        (2) pAddr2     保存申请到的内存的指针变量                                              *
 *        (3) pError     详细调用结果                                                            *
 *  返回: (1) eSuccess   操作成功                                                                *
 *        (2) eFailure   操作失败                                                                *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState xPoolMemMalloc(TMemPool* pPool, void** pAddr2, TError* pError)
{
    TState state = eFailure;
    TError error = MEM_ERR_UNREADY;
    TReg32 imask;
    TIndex x;
    TIndex y;
    TIndex index;
    TChar* pTemp;

    CpuEnterCritical(&imask);

    if (pPool->Property & MEM_PROP_READY)
    {
        /* 如果内存池存在可以内存页 */
        if (pPool->PageAvail > 0U)
        {
            /* 将首内存页分配出去 */
            pTemp = (TChar*)(pPool->PageList);
            uObjListRemoveNode(&(pPool->PageList), (TObjNode*)pTemp);
            pPool->PageAvail--;
            *pAddr2 = (void*)pTemp;

            /* 标记该内存页已经被分配 */
            index = (pTemp - pPool->PageAddr)/(pPool->PageSize);
            y = (index >> 5);
            x = (index & 0x1f);
            pPool->PageTags[y]  &= ~(0x1 << x);

            /* 清空该内存页内容 */
            memset((void*)pTemp, 0U, pPool->PageSize);

            error = MEM_ERR_NONE;
            state = eSuccess;
        }
        else
        {
            error = MEM_ERR_NO_MEM;
        }
    }

    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能: 向内存池中释放内存                                                                     *
 *  参数: (1) pPool      内存池结构地址                                                          *
 *        (2) pAddr      待释放内存的地址                                                        *
 *        (3) pError     详细调用结果                                                            *
 *  返回: (1) eSuccess   操作成功                                                                *
 *        (2) eFailure   操作失败                                                                *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState xPoolMemFree (TMemPool* pPool, void* pAddr, TError* pError)
{
    TState state = eFailure;
    TError error = MEM_ERR_UNREADY;
    TReg32 imask;
    TIndex index;
    TChar* pTemp;
    TBase32 x;
    TBase32 y;
	TBase32 tag;

    CpuEnterCritical(&imask);

    if (pPool->Property & MEM_PROP_READY)
    {
        /* 如果内存池确实有内存页被分配出去尚未回收 */
        if (pPool->PageAvail < pPool->PageNbr)
        {
            /* 检查释放的内存地址是否真的处在合适的块起始地址上。
               此处代码要求被管理的内存空间必须是连续的 */
            index = ((TChar*)pAddr - pPool->PageAddr) / (pPool->PageSize);
          //  index = (index < pPool->PageNbr) ? index : pPool->PageNbr;
            pTemp = pPool->PageAddr + index * pPool->PageSize;

            /* 如果该地址满足条件，确实是处在某个内存页的首地址 */
            if (pTemp == (TChar*)pAddr)
            {
                /* 检查内存页管理标记，避免再次释放已经释放过的内存页地址 */
                y = (index >> 5);
                x = (index & 0x1f);
				tag = pPool->PageTags[y] & (0x1 << x);
                if (tag == 0)
                {
                    /* 清空该内存页 */
                    memset(pAddr, 0U, pPool->PageSize);

                    /* 收回该地址的内存页 */
                    uObjListAddNode(&(pPool->PageList), (TObjNode*)pAddr, eQuePosTail);
                    pPool->PageAvail++;

                    /* 标记该内存页可以被分配 */
                    pPool->PageTags[y] |= (0x1 << x);

                    error = MEM_ERR_NONE;
                    state = eSuccess;
                }
                else
                {
                    error = MEM_ERR_DBL_FREE;
                }
            }
            else
            {
                error = MEM_ERR_BAD_ADDR;
            }
        }
        else
        {
            error = MEM_ERR_POOL_FULL;
        }
    }

    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}

#endif

