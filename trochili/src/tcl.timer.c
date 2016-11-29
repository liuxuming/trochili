/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#include <string.h>

#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.object.h"
#include "tcl.debug.h"
#include "tcl.cpu.h"
#include "tcl.ipc.h"
#include "tcl.kernel.h"
#include "tcl.thread.h"
#include "tcl.timer.h"

#if (TCLC_TIMER_ENABLE)

/*
 * 内核定时器分为2种，分别用于线程延时、时限方式访问资源的线程定时器和用户定时器
 * 本模块只负责用户定时器。
 */
static TTimerList TimerActiveListA;
static TTimerList TimerActiveListB;
static TLinkNode* TimerDormantList;
static TLinkNode* TimerExpiredList;

/*************************************************************************************************
 *  功能：定时器执行处理函数                                                                     *
 *  参数：(1) pTimer 定时器                                                                      *
 *  返回：无                                                                                     *
 *  说明: (1)线程以时限方式访问资源的时候，如果得不到资源，则线程会被同时放入资源的线程阻塞队列  *
 *           和内核线程辅助队列中。                                                              *
 *        (2)用户周期性定时器期满后，会立刻进入下一轮计时。同时也会进入期满队列。所有用户定时器  *
 *           都将由系统定时器守护线程处理。也就是说，用户定时器的回调函数是在线程态执行的。      *
 *************************************************************************************************/
static void DispatchExpiredTimer(TTimer* pTimer)
{
    TIndex spoke;
    TTimerList *pList;

    /*
     * 将定时器放入内核定时器期满列表，
     * 最后由定时器守护线程处理。紧急的定时器优先处理;
     */
    if (!(pTimer->Property & OS_TIMER_PROP_EXPIRED))
    {
        OsObjListAddPriorityNode(&TimerExpiredList, &(pTimer->ExpiredNode));
        pTimer->ExpiredTicks = pTimer->MatchTicks;
        pTimer->ExpiredTicksCycles = OsKernelVariable.JiffyCycles;
        pTimer->Property |= OS_TIMER_PROP_EXPIRED;
    }

    /* 将定时器从活动队列中移出 */
    OsObjListRemoveNode(pTimer->LinkNode.Handle, &(pTimer->LinkNode));

    /* 将周期类型的用户定时器重新放回活动定时器队列里 */
    if (pTimer->Property & OS_TIMER_PROP_PERIODIC)
    {
        /* 定时器期满次数加1 */
        pTimer->ExpiredTimes++;

        /* 计算下次期满时间，如果定时器时间溢出，则将定时器放入另一个定时器队列里 */
        pTimer->MatchTicks += pTimer->PeriodTicks;
        if (pTimer->MatchTicks <= OsKernelVariable.Jiffies)
        {
            pList = (OsKernelVariable.TimerList == &TimerActiveListA)?\
                    (&TimerActiveListB): (&TimerActiveListA);
        }
        else
        {
            pList = OsKernelVariable.TimerList;
        }

        spoke = (TBase32)(pTimer->MatchTicks % TCLC_TIMER_WHEEL_SIZE);
        OsObjListAddPriorityNode(&(pList->Handle[spoke]), &(pTimer->LinkNode));
        pTimer->Status = OsTimerActive;
    }
    else
    {
        /* 将单次回调定时器放到休眠队列里 */
        OsObjListAddNode(&TimerDormantList, &(pTimer->LinkNode), OsLinkTail);
        pTimer->Status = OsTimerDormant;
    }
}


/*************************************************************************************************
 *  功能：内核定时器ISR处理函数                                                                  *
 *  参数：无                                                                                     *
 *  返回：无                                                                                     *
 *  说明:                                                                                        *
 *************************************************************************************************/
void OsTimerTickUpdate(void)
{
    TTimer* pTimer;
    TIndex  spoke;
    TLinkNode* pNode;
    TLinkNode* pNext;
    TTimerList* pList;

    /* 如果系统Jiffes达到0说明Jiffes计数溢出，此时需要切换用户定时器队列 */
    if (OsKernelVariable.Jiffies == 0U)
    {
        OsKernelVariable.TimerList =
            (OsKernelVariable.TimerList == &TimerActiveListA)?\
            (&TimerActiveListB): (&TimerActiveListA);
    }
    pList = OsKernelVariable.TimerList;

    /* 根据当前系统时钟节拍计数计算出当前活动定时器队列 */
    spoke = (TIndex)(OsKernelVariable.Jiffies % TCLC_TIMER_WHEEL_SIZE);
    pNode = pList->Handle[spoke];

    /*
     * 检查当前活动定时器队列里的定时器。队列里的定时器按照期满节拍值由小到大排列。
     * 队列队首定时器计数一定不会小于当前系统时钟节拍计数。
     * 在本系统中，系统时钟节拍计数默认为64Bits。
     * 因为双定时器队列互为缓冲，所以即使定时器计数发生溢出，也不会丢失计数。
     */
    while (pNode != (TLinkNode*)0)
    {
        pNext = pNode->Next;
        pTimer = (TTimer*)(pNode->Owner);

        /*
         * 比较定时器的延时节拍数和此时系统时钟节拍数，
         * 如果等于则处理该定时器;
         * 如果大于则退出整个流程;
         */
        if (pTimer->MatchTicks == OsKernelVariable.Jiffies)
        {
            DispatchExpiredTimer(pTimer);
            pNode = pNext;
        }
        else
        {
            break;
        }
    }

    /* 如果需要则唤醒内核内置的用户定时器守护线程 */
    if (TimerExpiredList != (TLinkNode*)0)
    {
        OsThreadResumeFromISR(OsKernelVariable.TimerDaemon);
    }
}


/*************************************************************************************************
 *  功能：用户定时器初始化函数                                                                   *
 *  参数：(1) pTimer   定时器地址                                                                *
 *        (2) pName    定时器名称                                                                *
 *        (3) property 定时器属性                                                                *
 *        (4) ticks    定时器滴答数目                                                            *
 *        (5) pRoutine 用户定时器回调函数                                                        *
 *        (6) pData    用户定时器回调函数参数                                                    *
 *        (7) priority 定时器优先级                                                              *
 *        (8) pError   详细调用结果                                                              *
 *  返回: (1) eSuccess 操作成功                                                                  *
 *        (2) eFailure 操作失败                                                                  *
 *  说明                                                                                         *
 *************************************************************************************************/
TState TclCreateTimer(TTimer* pTimer, TChar* pName, TProperty property, TTimeTick ticks,
                      TTimerRoutine pRoutine, TArgument data, TPriority priority, TError* pError)
{
    TState state = eFailure;
    TError error = OS_TIMER_ERR_FAULT;
    TReg32 imask;

    OS_ASSERT((pTimer != (TTimer*)0), "");
    OS_ASSERT((pName != (TChar*)0), "");
    OS_ASSERT((pRoutine != (TTimerRoutine)0), "");
    OS_ASSERT((ticks > 0U), "");
    OS_ASSERT((pError != (TError*)0), "");

    property &= OS_TIMER_USER_PROPERTY;

    OsCpuEnterCritical(&imask);

    /* 检查定时器就绪属性 */
    if (!(pTimer->Property & OS_TIMER_PROP_READY))
    {
        /* 将定时器加入到内核对象队列中 */
        OsKernelAddObject(&(pTimer->Object), pName, OsTimerObject, (void*)pTimer);

        /* 初始化定时器，设置定时器信息 */
        pTimer->Status       = OsTimerDormant;
        pTimer->Property     = (property | OS_TIMER_PROP_READY);
        pTimer->PeriodTicks  = ticks;
        pTimer->MatchTicks   = (TTimeTick)0;
        pTimer->Routine      = pRoutine;
        pTimer->Argument     = data;
        pTimer->Priority     = priority;
        pTimer->ExpiredTicks = (TTimeTick)0;
        pTimer->ExpiredTicksCycles  = 0U;
        pTimer->ExpiredTimes = 0U;

        /* 设置定时器期满链表节点信息 */
        pTimer->ExpiredNode.Next   = (TLinkNode*)0;
        pTimer->ExpiredNode.Prev   = (TLinkNode*)0;
        pTimer->ExpiredNode.Handle = (TLinkNode**)0;
        pTimer->ExpiredNode.Data   = (TBase32*)(&(pTimer->Priority));
        pTimer->ExpiredNode.Owner  = (void*)pTimer;

        /* 设置定时器链表节点信息, 并将定时器加入休眠队列中 */
        pTimer->LinkNode.Next   = (TLinkNode*)0;
        pTimer->LinkNode.Prev   = (TLinkNode*)0;
        pTimer->LinkNode.Handle = (TLinkNode**)0;
        pTimer->LinkNode.Data   = (TBase32*)(&(pTimer->MatchTicks));
        pTimer->LinkNode.Owner  = (void*)pTimer;
        OsObjListAddNode(&TimerDormantList, &(pTimer->LinkNode), OsLinkTail);

        error = OS_TIMER_ERR_NONE;
        state = eSuccess;
    }

    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能：内核定时器取消初始化                                                                   *
 *  参数：(1) pTimer   定时器结构地址                                                            *
 *        (2) pError   详细调用结果                                                              *
 *  返回: (1) eSuccess 操作成功                                                                  *
 *        (2) eFailure 操作失败                                                                  *
 *  说明                                                                                         *
 *************************************************************************************************/
TState TclDeleteTimer(TTimer* pTimer, TError* pError)
{
    TState state = eFailure;
    TError error = OS_TIMER_ERR_UNREADY;
    TReg32 imask;

    OS_ASSERT((pTimer != (TTimer*)0), "");
    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    /* 检查定时器就绪属性 */
    if (pTimer->Property & OS_TIMER_PROP_READY)
    {
        if (pTimer->Status == OsTimerDormant)
        {
            /* 将定时器从内核对象列表中移出 */
            OsKernelRemoveObject(&(pTimer->Object));

            /* 将定时器从所属定时器队列中移出 */
            OsObjListRemoveNode(pTimer->LinkNode.Handle, &(pTimer->LinkNode));

            /* 清空定时器数据 */
            memset(pTimer, 0U, sizeof(TTimer));
            error = OS_TIMER_ERR_NONE;
            state = eSuccess;
        }
        else
        {
            error = OS_TIMER_ERR_STATUS;
        }
    }

    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能：定时器启动函数                                                                         *
 *  参数：(1) pTimer     定时器结构地址                                                          *
 *        (2) lagticks   定时器延缓开始计数时间                                                  *
 *        (3) pError     详细调用结果                                                            *
 *  返回: (1) eSuccess   操作成功                                                                *
 *        (2) eFailure   操作失败                                                                *
 *  说明：无                                                                                     *
 *************************************************************************************************/
TState TclStartTimer(TTimer* pTimer,TTimeTick lagticks, TError* pError)
{
    TState state = eFailure;
    TError error = OS_TIMER_ERR_UNREADY;
    TReg32 imask;
    TIndex spoke;
    TTimerList* pList;

    OS_ASSERT((pTimer != (TTimer*)0), "");
    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    /* 检查定时器就绪属性 */
    if (pTimer->Property & OS_TIMER_PROP_READY)
    {
        if (pTimer->Status == OsTimerDormant)
        {
            /* 将定时器从休眠队列中移出 */
            OsObjListRemoveNode(pTimer->LinkNode.Handle, &(pTimer->LinkNode));

            /* 将定时器加入活动队列里 */
            pTimer->MatchTicks  = OsKernelVariable.Jiffies + pTimer->PeriodTicks + lagticks;
            if (pTimer->MatchTicks <= OsKernelVariable.Jiffies)
            {
                pList = (OsKernelVariable.TimerList == &TimerActiveListA)?\
                        (&TimerActiveListB): (&TimerActiveListA);
            }
            else
            {
                pList = OsKernelVariable.TimerList;
            }

            spoke = (TBase32)(pTimer->MatchTicks % TCLC_TIMER_WHEEL_SIZE);
            OsObjListAddPriorityNode(&(pList->Handle[spoke]), &(pTimer->LinkNode));
            pTimer->Status = OsTimerActive;
            error = OS_TIMER_ERR_NONE;
            state = eSuccess;
        }
        else
        {
            error = OS_TIMER_ERR_STATUS;
        }
    }

    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能：停止用户定时器函数                                                                     *
 *  参数：(1) pTimer   定时器地址                                                                *
 *        (2) pError   详细调用结果                                                              *
 *  返回: (1) eSuccess 操作成功                                                                  *
 *        (2) eFailure 操作失败                                                                  *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclStopTimer(TTimer* pTimer, TError* pError)
{
    TState state = eFailure;
    TError error = OS_TIMER_ERR_UNREADY;
    TReg32 imask;

    OS_ASSERT((pTimer != (TTimer*)0), "");
    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    /* 检查定时器就绪属性 */
    if (pTimer->Property & OS_TIMER_PROP_READY)
    {
        /* 将定时器从活动队列/期满队列中移出，放到休眠队列里 */
        if (pTimer->Status == OsTimerActive)
        {
            if (pTimer->Property & OS_TIMER_PROP_EXPIRED)
            {
                OsObjListRemoveNode(pTimer->ExpiredNode.Handle, &(pTimer->ExpiredNode));
                pTimer->Property &= ~OS_TIMER_PROP_EXPIRED;
            }

            OsObjListRemoveNode(pTimer->LinkNode.Handle, &(pTimer->LinkNode));
            OsObjListAddNode(&TimerDormantList, &(pTimer->LinkNode), OsLinkTail);
            pTimer->Status = OsTimerDormant;

            error = OS_TIMER_ERR_NONE;
            state = eSuccess;
        }
        else
        {
            error = OS_TIMER_ERR_STATUS;
        }
    }

    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能：重置定时器类型、定时时间和优先级                                                       *
 *  参数：(1) pTimer   定时器结构地址                                                            *
 *        (2) ticks    定时器时钟节拍数目                                                        *
 *        (3) priority 定时器优先级                                                              *
 *        (4) pError   详细调用结果                                                              *
 *  返回: (1) eSuccess 操作成功                                                                  *
 *        (2) eFailure 操作失败                                                                  *
 *  说明                                                                                         *
 *************************************************************************************************/
TState TclConfigTimer(TTimer* pTimer, TTimeTick ticks, TPriority priority, TError* pError)
{
    TState state = eFailure;
    TError error = OS_TIMER_ERR_UNREADY;
    TReg32 imask;

    OS_ASSERT((pTimer != (TTimer*)0), "");
    OS_ASSERT((ticks > 0U), "");
    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    /* 检查定时器就绪属性 */
    if (pTimer->Property & OS_TIMER_PROP_READY)
    {
        if (pTimer->Status == OsTimerDormant)
        {
            pTimer->PeriodTicks = ticks;
            pTimer->Priority    = priority;
            error = OS_TIMER_ERR_NONE;
            state = eSuccess;
        }
        else
        {
            error = OS_TIMER_ERR_STATUS;
        }
    }

    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/* 内核定时器守护线程定义和栈定义 */
static TBase32 TimerDaemonStack[TCLC_TIMER_DAEMON_STACK_BYTES >> 2];
static TThread TimerDaemonThread;

/* 内核定时器守护线程不接受任何线程管理API操作 */
#define TIMER_DAEMON_ACAPI (OS_THREAD_ACAPI_NONE)

/*************************************************************************************************
 *  功能：内核中的定时器守护线程函数                                                             *
 *  参数：(1) argument 定时器线程的用户参数                                                      *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
static void TimerDaemonEntry(TArgument argument)
{
    TBase32       imask;
    TTimer*       pTimer;
    TTimerRoutine pRoutine;
    TArgument     data;
    TTimeTick     ticks;
    TBase32       cycles;

    /*
     * 逐个处理用户定时器，在线程环境下处理定时器回调事务
     * 如果期满定时器队列为空则将定时器守护线程挂起
     */
    while(eTrue)
    {
        OsCpuEnterCritical(&imask);

        if (TimerExpiredList == (TLinkNode*)0)
        {
            OsThreadSuspendSelf();
            OsCpuLeaveCritical(imask);
        }
        else
        {
            /* 从期满队列中取得一个定时器 */
            pTimer = (TTimer*)(TimerExpiredList->Owner);

            /* 将定时器从期满队列中移出 */
            OsObjListRemoveNode(pTimer->ExpiredNode.Handle, &(pTimer->ExpiredNode));
            pTimer->Property &= ~OS_TIMER_PROP_EXPIRED;

            /* 复制定时器函数和函数参数 */
            pRoutine = pTimer->Routine;
            data     = pTimer->Argument;
            ticks    = 0U;
            cycles   = 0U;
					
            /* 如果是精准定时器则计算定时器的漂移时间 */
            if (pTimer->Property & OS_TIMER_PROP_ACCURATE)
            {
                if (OsKernelVariable.Jiffies >= pTimer->ExpiredTicks)
                {
                    ticks = OsKernelVariable.Jiffies - pTimer->ExpiredTicks;
                    cycles = OsKernelVariable.JiffyCycles - pTimer->ExpiredTicksCycles;
                }
                else
                {
                    ticks = (~(TTimeTick)(0U)) - pTimer->ExpiredTicks + OsKernelVariable.Jiffies;
                    cycles = OsKernelVariable.JiffyCycles - pTimer->ExpiredTicksCycles - 1U;
                }
            }

            OsCpuLeaveCritical(imask);

            /* 在线程环境下执行定时器函数 */
            pRoutine(data, cycles, ticks);
        }
    }
}


/*************************************************************************************************
 *  功能：定时器模块初始化                                                                       *
 *  参数：无                                                                                     *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void OsTimerModuleInit(void)
{
    /* 检查内核是否处于初始状态 */
    if(OsKernelVariable.State != OsOriginState)
    {
        OsDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }

    memset(&TimerActiveListA, 0U, sizeof(TimerActiveListA));
    memset(&TimerActiveListB, 0U, sizeof(TimerActiveListB));

    /* 初始化相关的内核变量 */
    OsKernelVariable.TimerList = &TimerActiveListA;

    /* 初始化内核定时器服务线程 */
    OsThreadCreate(&TimerDaemonThread,
                   "kernel timer daemon",
                   OsThreadSuspended,
                   OS_THREAD_PROP_PRIORITY_FIXED|\
                   OS_THREAD_PROP_CLEAN_STACK|\
                   OS_THREAD_PROP_KERNEL_DAEMON,
                   TIMER_DAEMON_ACAPI,
                   TimerDaemonEntry,
                   (TArgument)(0U),
                   (void*)TimerDaemonStack,
                   (TBase32)TCLC_TIMER_DAEMON_STACK_BYTES,
                   (TPriority)TCLC_TIMER_DAEMON_PRIORITY,
                   (TTimeTick)TCLC_TIMER_DAEMON_SLICE);

    /* 初始化相关的内核变量 */
    OsKernelVariable.TimerDaemon = &TimerDaemonThread;
}
#endif

