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
static TTimerList TimerList;


/*************************************************************************************************
 *  功能：定时器执行处理函数                                                                     *
 *  参数：(1) pTimer 定时器                                                                      *
 *  返回：无                                                                                     *
 *  说明: (1)线程延时的时候，线程被放入内核线程辅助队列中                                        *
 *        (2)线程以时限方式访问资源的时候，如果得不到资源，则线程会被同时放入资源的线程阻塞队列  *
 *           和内核线程辅助队列中。                                                              *
 *        (3)用户周期性定时器期满后，会立刻进入下一轮计时。同时也会进入期满队列。所有用户定时器  *
 *           都将由系统定时器守护线程处理。也就是说，用户定时器的回调函数是在线程态执行的。      *
 *        (4)这里虽然有线程队列操作但是不进行调度，是因为这个函数是在中断中调用的，              *
 *           在最后一层中断返回后，会尝试进行一次线程切换，所以在这里做切换的话是白白浪费时间    *
 *************************************************************************************************/
static void DispatchExpiredTimer(TTimer* pTimer)
{
    TIndex spoke;

    /*
     * 将定时器放入内核定时器期满列表，
     * 最后由定时器守护线程处理。紧急的定时器优先处理;
     */
    if (!(pTimer->Property & TIMER_PROP_EXPIRED))
    {
        uObjListAddPriorityNode(&(TimerList.ExpiredHandle), &(pTimer->ExpiredNode));
        pTimer->ExpiredTicks = pTimer->MatchTicks;
        pTimer->Property |= TIMER_PROP_EXPIRED;
    }

    /* 将定时器从活动队列中移出 */
    uObjListRemoveNode(pTimer->LinkNode.Handle, &(pTimer->LinkNode));

    /* 将周期类型的用户定时器重新放回活动定时器队列里 */
    if (pTimer->Property & TIMER_PROP_PERIODIC)
    {
      	pTimer->ExpiredTimes++;
        pTimer->MatchTicks += pTimer->PeriodTicks;
        spoke = (TBase32)(pTimer->MatchTicks % TCLC_TIMER_WHEEL_SIZE);
        uObjListAddPriorityNode(&(TimerList.ActiveHandle[spoke]), &(pTimer->LinkNode));
        pTimer->Status = eTimerActive;
    }
    else
    {
        /* 将单次回调定时器放到休眠队列里 */
        uObjListAddNode(&(TimerList.DormantHandle), &(pTimer->LinkNode), eLinkPosHead);
        pTimer->Status = eTimerDormant;
    }
}


/*************************************************************************************************
 *  功能：内核定时器ISR处理函数                                                                  *
 *  参数：无                                                                                     *
 *  返回：无                                                                                     *
 *  说明:                                                                                        *
 *************************************************************************************************/
void uTimerTickUpdate(void)
{
    TTimer*   pTimer;
    TIndex    spoke;
    TLinkNode* pNode;
    TLinkNode* pNext;

    /* 根据当前系统时钟节拍计数计算出当前活动定时器队列 */
    spoke = (TIndex)(uKernelVariable.Jiffies % TCLC_TIMER_WHEEL_SIZE);
    pNode = TimerList.ActiveHandle[spoke];

    /*
     * 检查当前活动定时器队列里的定时器。队列里的定时器按照期满节拍值由小到大排列。
     * 如果某个队列队首定时器计数小于当前系统时钟节拍计数，这说明有定时器计数发生溢出，
     * 不过在本系统中，系统时钟节拍计数为64Bits,同时强制要求定时器延时计数必须小于63Bits，
     * 这样即使定时器计数发生溢出，也不会丢失计数。
     */
    while (pNode != (TLinkNode*)0)
    {
        pNext = pNode->Next;
        pTimer = (TTimer*)(pNode->Owner);

        /*
         * 比较定时器的延时节拍数和此时系统时钟节拍数，
         * 如果小于则跳过该定时器;
         * 如果等于则处理该定时器;
         * 如果大于则退出整个流程;
         */
        if (pTimer->MatchTicks < uKernelVariable.Jiffies)
        {
            pNode = pNext;
        }
        else if (pTimer->MatchTicks == uKernelVariable.Jiffies)
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
    if (TimerList.ExpiredHandle != (TLinkNode*)0)
    {
        uThreadResumeFromISR(uKernelVariable.TimerDaemon);
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
TState xTimerCreate(TTimer* pTimer, TChar* pName, TProperty property, TTimeTick ticks,
                    TTimerRoutine pRoutine, TArgument data, TPriority priority, TError* pError)
{
    TState state = eFailure;
    TError error = TIMER_ERR_FAULT;
    TReg32 imask;

    CpuEnterCritical(&imask);

    /* 检查定时器就绪属性 */
    if (!(pTimer->Property & TIMER_PROP_READY))
    {
        /* 将定时器加入到内核对象队列中 */
        uKernelAddObject(&(pTimer->Object), pName, eTimer, (void*)pTimer);

        /* 初始化定时器，设置定时器信息 */
        pTimer->Status       = eTimerDormant;
        pTimer->Property     = (property | TIMER_PROP_READY);
        pTimer->PeriodTicks  = ticks;
        pTimer->MatchTicks   = (TTimeTick)0;
        pTimer->Routine      = pRoutine;
        pTimer->Argument     = data;
        pTimer->Priority     = priority;
        pTimer->ExpiredTicks = (TTimeTick)0;
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
        uObjListAddNode(&(TimerList.DormantHandle), &(pTimer->LinkNode), eLinkPosHead);

        error = TIMER_ERR_NONE;
        state = eSuccess;

    }

    CpuLeaveCritical(imask);

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
TState xTimerDelete(TTimer* pTimer, TError* pError)
{
    TState state = eFailure;
    TError error = TIMER_ERR_UNREADY;
    TReg32 imask;

    CpuEnterCritical(&imask);

    /* 检查定时器就绪属性 */
    if (pTimer->Property & TIMER_PROP_READY)
    {
        if (pTimer->Status == eTimerDormant)
        {
            /* 将定时器从内核对象列表中移出 */
            uKernelRemoveObject(&(pTimer->Object));

            /* 将定时器从所属定时器队列中移出 */
            uObjListRemoveNode(pTimer->LinkNode.Handle, &(pTimer->LinkNode));

            /* 清空定时器数据 */
            memset(pTimer, 0U, sizeof(TTimer));
            error = TIMER_ERR_NONE;
            state = eSuccess;
        }
        else
        {
            error = TIMER_ERR_STATUS;
        }
    }

    CpuLeaveCritical(imask);

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
TState xTimerStart(TTimer* pTimer,TTimeTick lagticks, TError* pError)
{
    TState state = eFailure;
    TError error = TIMER_ERR_UNREADY;
    TReg32 imask;
    TIndex spoke;

    CpuEnterCritical(&imask);

    /* 检查定时器就绪属性 */
    if (pTimer->Property & TIMER_PROP_READY)
    {
        if (pTimer->Status == eTimerDormant)
        {
            /* 将定时器从休眠队列中移出 */
            uObjListRemoveNode(pTimer->LinkNode.Handle, &(pTimer->LinkNode));

            /* 将定时器加入活动队列里 */
            pTimer->MatchTicks  = uKernelVariable.Jiffies + pTimer->PeriodTicks + lagticks;
            spoke = (TBase32)(pTimer->MatchTicks % TCLC_TIMER_WHEEL_SIZE);
            uObjListAddPriorityNode(&(TimerList.ActiveHandle[spoke]), &(pTimer->LinkNode));
            pTimer->Status = eTimerActive;
            error = TIMER_ERR_NONE;
            state = eSuccess;
        }
        else
        {
            error = TIMER_ERR_STATUS;
        }

    }

    CpuLeaveCritical(imask);

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
TState xTimerStop(TTimer* pTimer, TError* pError)
{
    TState state = eFailure;
    TError error = TIMER_ERR_UNREADY;
    TReg32 imask;

    CpuEnterCritical(&imask);

    /* 检查定时器就绪属性 */
    if (pTimer->Property & TIMER_PROP_READY)
    {
        /* 将定时器从活动队列/期满队列中移出，放到休眠队列里 */
        if (pTimer->Status == eTimerActive)
        {
            if (pTimer->Property & TIMER_PROP_EXPIRED)
            {
                uObjListRemoveNode(pTimer->ExpiredNode.Handle, &(pTimer->ExpiredNode));
                pTimer->Property &= ~TIMER_PROP_EXPIRED;
            }

            uObjListRemoveNode(pTimer->LinkNode.Handle, &(pTimer->LinkNode));
            uObjListAddNode(&(TimerList.DormantHandle), &(pTimer->LinkNode), eLinkPosHead);
            pTimer->Status = eTimerDormant;

            error = TIMER_ERR_NONE;
            state = eSuccess;
        }
        else
        {
            error = TIMER_ERR_STATUS;
        }

    }

    CpuLeaveCritical(imask);

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
TState xTimerConfig(TTimer* pTimer, TTimeTick ticks, TPriority priority, TError* pError)
{
    TState state = eFailure;
    TError error = TIMER_ERR_UNREADY;
    TReg32 imask;

    CpuEnterCritical(&imask);

    /* 检查定时器就绪属性 */
    if (pTimer->Property & TIMER_PROP_READY)
    {
        if (pTimer->Status == eTimerDormant)
        {
            pTimer->PeriodTicks = ticks;
            pTimer->Priority    = priority;
            error = TIMER_ERR_NONE;
            state = eSuccess;
        }
        else
        {
            error = TIMER_ERR_STATUS;
        }
    }

    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/* 内核定时器守护线程定义和栈定义 */
static TBase32 TimerDaemonStack[TCLC_TIMER_DAEMON_STACK_BYTES >> 2];
static TThread TimerDaemonThread;

/* 内核定时器守护线程不接受任何线程管理API操作 */
#define TIMER_DAEMON_ACAPI (THREAD_ACAPI_NONE)

/*************************************************************************************************
 *  功能：内核中的定时器守护线程函数                                                             *
 *  参数：(1) argument 定时器线程的用户参数                                                      *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
static void xTimerDaemonEntry(TArgument argument)
{
    TBase32       imask;
    TTimer*       pTimer;
    TTimerRoutine pRoutine;
    TArgument     data;
    TTimeTick     ticks;

    /*
     * 逐个处理用户定时器，在线程环境下处理定时器回调事务
     * 如果期满定时器队列为空则将定时器守护线程挂起
     */
    while(eTrue)
    {
        CpuEnterCritical(&imask);

        if (TimerList.ExpiredHandle == (TLinkNode*)0)
        {
            uThreadSuspendSelf();
            CpuLeaveCritical(imask);
        }
        else
        {
            /* 从期满队列中取得一个定时器 */
            pTimer = (TTimer*)(TimerList.ExpiredHandle->Owner);

            /*
             * 计算定时器的漂移时间,如果精准定时器的漂移时间大于等于定时周期，
             * 说明定时器被耽搁的实在太长了~,一定是哪里有问题。
             */
            if (uKernelVariable.Jiffies == pTimer->ExpiredTicks)
            {
                ticks = 0U;
            }
            else if (uKernelVariable.Jiffies > pTimer->ExpiredTicks)
            {
                ticks = uKernelVariable.Jiffies - pTimer->ExpiredTicks;
            }
            else
            {
                ticks = TCLM_MAX_VALUE64 - pTimer->ExpiredTicks + uKernelVariable.Jiffies;
            }

            if (pTimer->Property & TIMER_PROP_ACCURATE)
            {
                if (ticks >= pTimer->PeriodTicks)
                {
                    uKernelVariable.Diagnosis |= KERNEL_DIAG_TIMER_ERROR;
                    pTimer->Diagnosis |= TIMER_DIAG_OVERFLOW;
                    uDebugAlarm("");
                }
            }

            /* 将定时器从期满队列中移出 */
            uObjListRemoveNode(pTimer->ExpiredNode.Handle, &(pTimer->ExpiredNode));
            pTimer->Property &= ~TIMER_PROP_EXPIRED;

            /* 复制定时器函数和函数参数 */
            pRoutine = pTimer->Routine;
            data = pTimer->Argument;

            CpuLeaveCritical(imask);

            /* 在线程环境下执行定时器函数 */
            pRoutine(data, ticks);
        }
    }
}


/*************************************************************************************************
 *  功能：初始化用户定时器守护线程                                                               *
 *  参数：无                                                                                     *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void uTimerCreateDaemon(void)
{
    /* 检查内核是否处于初始状态 */
    if(uKernelVariable.State != eOriginState)
    {
        uDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }

    /* 初始化内核定时器服务线程 */
    uThreadCreate(&TimerDaemonThread,
                  "timer daemon",
                  eThreadSuspended,
                  THREAD_PROP_PRIORITY_FIXED|\
                  THREAD_PROP_CLEAN_STACK|\
                  THREAD_PROP_KERNEL_DAEMON,
                  TIMER_DAEMON_ACAPI,
                  xTimerDaemonEntry,
                  (TArgument)(0U),
                  (void*)TimerDaemonStack,
                  (TBase32)TCLC_TIMER_DAEMON_STACK_BYTES,
                  (TPriority)TCLC_TIMER_DAEMON_PRIORITY,
                  (TTimeTick)TCLC_TIMER_DAEMON_SLICE);

    /* 初始化相关的内核变量 */
    uKernelVariable.TimerDaemon = &TimerDaemonThread;
}


/*************************************************************************************************
 *  功能：定时器模块初始化                                                                       *
 *  参数：无                                                                                     *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void uTimerModuleInit(void)
{
    /* 检查内核是否处于初始状态 */
    if(uKernelVariable.State != eOriginState)
    {
        uDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }

    memset(&TimerList, 0, sizeof(TimerList));

    /* 初始化相关的内核变量 */
    uKernelVariable.TimerList = &TimerList;
}
#endif

