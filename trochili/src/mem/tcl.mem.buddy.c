/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#include <string.h>

#include "tcl.config.h"
#include "tcl.types.h"
#include "tcl.cpu.h"
#include "tcl.debug.h"
#include "tcl.mem.buddy.h"

#if ((TCLC_MEMORY_ENABLE) && (TCLC_MEMORY_BUDDY_ENABLE))

/* BUDDY属性标记定义 */
#define BUDDY_PROP_NONE           (0x0)                  /* BUDDY空属性标记                     */
#define BUDDY_PROP_READY          (0x1<<0U)              /* BUDDY就绪标记                       */

#define PAGES_AVAIL (0x1<<7u)

#define PARENT_NODE(x) (((x) - 1u) / 2u)
#define LEFT_NODE(x) ((x) * 2u + 1u)
#define RIGHT_NODE(x) ((x) * 2u + 2u)

/* 调整x到和它最相近且不小于它的2的幂数 */
static TBase32 clp2(TBase32 x)
{
    x = x - 1U;
    x = x | (x >> 1U);
    x = x | (x >> 2U);
    x = x | (x >> 4U);
    x = x | (x >> 8U);
    x = x | (x >> 16U);
    return (x + 1U);
}

/* 调整x到和它最相近且不大于它的2的幂数 */
static TBase32 flp2(TBase32 x)
{
    x = x | (x >> 1U);
    x = x | (x >> 2U);
    x = x | (x >> 4U);
    x = x | (x >> 8U);
    x = x | (x >> 16U);
    return (x - (x >> 1U));
}

/* 计算x以2为底的对数 */
static TBase32 log2(TBase32 x)
{
    TBase32 i = 0;
    while (!(x &0x1))
    {
        i++;
        x = x >> 1;
    }

    return i;
}

/* 计算2的x次幂 */
static TBase32 power2(TBase32 x)
{
    TBase32 i = 1;
    while (x)
    {
        i = i << 1;
        x--;
    }

    return i;
}


/*************************************************************************************************
 *  功能：分配每个二叉树节点管理的内存页数                                                       *
 *  参数：(1) pBuddy  伙伴系统分配器地址                                                         *
 *  返回: 无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
static void BuildPageTree(TMemBuddy* pBuddy)
{
    TIndex node;
    TBase32 logn;
    TBase32 x;
    TBase32 y;

    /* 计算树的节点总数 */
    pBuddy->NodeNbr  = pBuddy->PageNbr * 2u  - 1u;

    /* 计算每个节点管理的页数(采用以2为底的对数来表示) */
    logn = log2(pBuddy->PageNbr) & 0x3f;
    node = 0U;
    for (y = 0; y <= logn; y++)
    {
        x = (pBuddy->PageNbr >> (logn - y));
        while (x--)
        {
            pBuddy->NodeTags[node] = ((logn - y) | PAGES_AVAIL);
            node++;
        }
    }
}


static int GetAvailPages(TMemBuddy* pBuddy)
{
    TByte tag;
    tag = pBuddy->NodeTags[0];
    if (tag & PAGES_AVAIL)
    {
        return power2(tag & 0x3f);
    }
    return 0;
}

/*************************************************************************************************
 *  功能：从伙伴管理器中分配一定数目的页                                                         *
 *  参数：(1) pBuddy    伙伴系统分配器对象地址                                                   *
 *        (2) pages     需要分配的内存页数                                                       *
 *        (3) index     分配得到的内存页号                                                       *
 *  返回: (1) 分配到的页面的编号                                                                 *
 *  说明：                                                                                       *
 *************************************************************************************************/
static TBase32 MallocPages(TMemBuddy* pBuddy, TBase32 pages)
{
    TBase32 index;
    TBase32 lvl;
    TBase32 node;
    TByte tag;
    TByte logn;

    TByte ltag;
    TByte rtag;
    TByte llogn;
    TByte rlogn;

    /* 计算和pages对应的以2为底的对数 */
    logn = log2(pages);

    /* 计算需要遍历的(从root算起)层数 */
    lvl = log2(pBuddy->PageNbr) - logn;

    /* 从根节点开始遍历lvl次，找到可供分配的节点 */
    node = 0U;
    while (lvl--)
    {
        tag = (pBuddy->NodeTags[LEFT_NODE(node)]);
        if ((tag & PAGES_AVAIL) && ((tag & 0x3f) >= logn))
        {
            node = LEFT_NODE(node);
        }
        else
        {
            node = RIGHT_NODE(node);
        }
    }

    /* 取消分配给该节点的内存页 */
    pBuddy->NodeTags[node] = 0U;

    /* 通过节点编号计算内存页编号 */
    index = (node + 1u) * pages - pBuddy->PageNbr;

    /* 回溯调整到根节点路径上的所有节点的可分配内存页数(注意是路径上的全部节点) */
    while (node)
    {
        node  = PARENT_NODE(node);
        ltag = pBuddy->NodeTags[LEFT_NODE(node)];
        rtag = pBuddy->NodeTags[RIGHT_NODE(node)];
        if ((ltag & PAGES_AVAIL) && (rtag & PAGES_AVAIL))
        {
            llogn = (ltag & 0x3f);
            rlogn = (rtag & 0x3f);
            tag   = (llogn > rlogn) ? llogn : rlogn;
            tag   |= PAGES_AVAIL;
        }
        else if (ltag & PAGES_AVAIL)
        {
            tag = ltag;
        }
        else if (rtag & PAGES_AVAIL)
        {
            tag = rtag;
        }
        else
        {
            tag = 0U;
        }

        pBuddy->NodeTags[node] = tag;
    }

    return index;
}


/*************************************************************************************************
 *  功能：线程通用阻塞终止函数,将指定的线程从阻塞队列中终止阻塞                                  *
 *  参数：(1) pBuddy    伙伴系统分配器对象地址                                                   *
 *        (2) index     待释放的多个内存页的起始页号                                             *
 *  返回: 无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
static TBase32 FreePages(TMemBuddy* pBuddy, TBase32 index)
{
    TBase32 pages;
    TBase32 node;
    TByte tag;
    TByte logn;
    TBase32 lvl;

    TByte ltag;
    TByte rtag;
    TByte llogn;
    TByte rlogn;

    /* 计算根节点以2为底的对数 */
    lvl = log2(pBuddy->PageNbr);

    /* 通过内存页编号获得叶子节点编号 */
    node = index + pBuddy->PageNbr - 1u;

    /* 通过该叶子节点向上回溯查找分配该起始内存页的节点, 比对次数最多为树的深度 */
    for (logn = 0; logn <= lvl; logn++)
    {
        /* 如果找到分配该内存(n)页的那个节点 */
        if (!(pBuddy->NodeTags[node] & PAGES_AVAIL))
        {
            break;
        }
        node = PARENT_NODE(node);
    }

    /* 回收该相关内存(n)页,重新计算它可以管理的内存页数 */
    pBuddy->NodeTags[node] = ((logn & 0x3f) | PAGES_AVAIL);
    pages = power2(logn);

    /* 尝试进行进行伙伴节点合并,需要一直遍历到root节点。
       如果是根节点则不需要以下操作 */
    while (node)
    {
        node = PARENT_NODE(node);
        logn++;
        ltag = pBuddy->NodeTags[LEFT_NODE(node)];
        rtag = pBuddy->NodeTags[RIGHT_NODE(node)];

        if ((ltag &PAGES_AVAIL) && (rtag &PAGES_AVAIL))
        {
            llogn = (ltag &0x3f);
            rlogn = (rtag &0x3f);
            if (power2(llogn) + power2(rlogn) == power2(logn))
            {
                tag = ((logn &0x3f) | PAGES_AVAIL);
            }
            else
            {
                tag = (llogn > rlogn) ? llogn : rlogn;
                tag |= PAGES_AVAIL;
            }
        }
        else if (ltag &PAGES_AVAIL)
        {
            tag = ltag;
        }
        else //if (rtag &PAGES_AVAIL)
        {
            tag = rtag;
        }
        pBuddy->NodeTags[node] = tag;
    }

    return pages;
}


/*************************************************************************************************
 *  功能：初始化伙伴内存管理控制结构                                                             *
 *  参数：(1) pBuddy    伙伴系统分配器内存地址                                                   *
 *        (2) pAddr     可供分配的内存地址                                                       *
 *        (3) pagesize  内存页大小                                                               *
 *        (4) pages     可供分配的内存页总数                                                     *
 *        (5) pError    详细调用结果                                                             *
 *  返回: (1) eSuccess  操作成功                                                                 *
 *        (2) eFailure  操作失败                                                                 *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState xBuddyCreate(TMemBuddy* pBuddy, TChar* pAddr, TBase32 pages, TBase32 pagesize, TError* pError)
{
    TState state = eFailure;
    TError error = MEM_ERR_FAULT;
    TIndex index;

    TReg32 imask;

    CpuEnterCritical(&imask);
    if (!(pBuddy->Property & BUDDY_PROP_READY))
    {
        /* 调整pages到和它最相近且不大于它的2的幂数 */
        pages = flp2(pages);
        if (pages)
        {
            pBuddy->Property  = BUDDY_PROP_READY;
            pBuddy->PageAddr  = pAddr;
            pBuddy->PageSize  = pagesize;
            pBuddy->PageNbr   = pages;
            pBuddy->PageAvail = pages;

            /* 设置所有内存都处于可分配状态 */
            for (index = 0; index < MEM_BUDDY_PAGE_TAGS; index++)
            {
                pBuddy->PageTags[index] = ~0U;
            }

            /* 创建二叉树控制结构 */
            BuildPageTree(pBuddy);

            error = MEM_ERR_NONE;
            state = eSuccess;
        }
    }
    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能：销毁伙伴内存管理控制结构                                                               *
 *  参数：(1) pBuddy     伙伴系统分配器内存地址                                                  *
 *        (2) pError     详细调用结果                                                            *
 *  返回: (1) eSuccess   操作成功                                                                *
 *        (2) eFailure   操作失败                                                                *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState xBuddyDelete(TMemBuddy* pBuddy, TError* pError)
{
    TReg32 imask;
    TState state = eFailure;
    TError error = MEM_ERR_UNREADY;

    CpuEnterCritical(&imask);
    if (pBuddy->Property & MEM_PROP_READY)
    {
        memset(pBuddy->PageAddr, 0U, pBuddy->PageSize * pBuddy->PageNbr);
        memset(pBuddy, 0U, sizeof(TMemBuddy));
        error = MEM_ERR_NONE;
        state = eSuccess;
    }
    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能：从伙伴系统中申请内存                                                                   *
 *  参数：(1) pBuddy    伙伴系统分配器对象地址                                                   *
 *        (2) len       需要分配的内存长度                                                       *
 *        (3) pAddr2    分配得到的内存地址指针                                                   *
 *        (4) pError    详细调用结果                                                             *
 *  返回: (1) eSuccess  操作成功                                                                 *
 *        (2) eFailure  操作失败                                                                 *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState xBuddyMemMalloc(TMemBuddy* pBuddy, TBase32 length, void** pAddr2, TError* pError)
{
    TState state = eFailure;
    TError error = MEM_ERR_UNREADY;
    TReg32 imask;
    TBase32 pages;
    TBase32 index;
    TBase32 avail;
    TIndex x;
    TIndex y;
    TIndex i;

    CpuEnterCritical(&imask);

    if (pBuddy->Property &BUDDY_PROP_READY)
    {
        /* 如果申请的内存长度没有超过BUDDY的范围 */
        if (length <= (pBuddy->PageNbr * pBuddy->PageSize))
        {
            /* 计算需要分配多少内存页 */
            pages = (length + pBuddy->PageSize - 1u) / (pBuddy->PageSize);

            /* 调整pages到和它最相近且不小于它的2的幂数 */
            pages = clp2(pages);

            avail = GetAvailPages(pBuddy);
            if (avail >= pages)
            {
                /* 获得分配的内存页编号 */
                index = MallocPages(pBuddy, pages);

                /* 标记该部分内存页已经被分配 */
                for (i = 0; i < pages; i++)
                {
                    y = ((index + i) >> 5);
                    x = ((index + i) & 0x1f);
                    pBuddy->PageTags[y]  &= ~(0x1 << x);
                }
                pBuddy->PageAvail -= pages;

                /* 通过内存页编号获得内存地址 */
                *pAddr2 = (void*)(pBuddy->PageAddr + index * pBuddy->PageSize);
                
                error = MEM_ERR_NONE;
                state = eSuccess;
            }
            else
            {
                *pAddr2 = (void*)0;
                error = MEM_ERR_NO_MEM;
            }
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
 *  功能：向伙伴系统中释放内存                                                                   *
 *  参数：(1) pBuddy    伙伴系统分配器对象地址                                                   *
 *        (2) pAddr     待释放的内存地址                                                         *
 *        (3) pError    详细调用结果                                                             *
 *  返回: (1) eSuccess  操作成功                                                                 *
 *        (2) eFailure  操作失败                                                                 *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState xBuddyMemFree(TMemBuddy* pBuddy, void* pAddr, TError* pError)
{
    TState state = eFailure;
    TError error = MEM_ERR_UNREADY;
    TReg32 imask;
    TBase32 index;
    TIndex x;
    TIndex y;
    TBitMask tag;
    TBase32 pages;
    TIndex i;

    CpuEnterCritical(&imask);
    if ((pBuddy->Property &BUDDY_PROP_READY))
    {
        /* 检查被释放的地址是否在伙伴系统管理的内存范围内 */
        if (((char*)pAddr >= (char*)(pBuddy->PageAddr)) &&
            ((char*)pAddr < ((char*)(pBuddy->PageAddr) + pBuddy->PageSize* pBuddy->PageNbr)))
        {
            /* 通过内存地址计算起始页编号 */
            index = ((char*)pAddr - (char*)(pBuddy->PageAddr)) / pBuddy->PageSize;

            /* 检查内存页管理标记，避免再次释放已经释放过的内存页地址 */
            y = (index >> 5);
            x = (index & 0x1f);
            tag = pBuddy->PageTags[y] & (0x1 << x);
            if (tag == 0)
            {
                /* 释放该页起始的内存 */
                pages = FreePages(pBuddy, index);

                /* 标记该部分内存可以被分配 */
                for (i = 0; i < pages; i++)
                {
                    y = (index + i) >> 5;
                    x = (index + i) & 0x1f;
                    pBuddy->PageTags[y] |= (0x1 << x);
                }

                pBuddy->PageAvail += pages;
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
    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}

#endif

