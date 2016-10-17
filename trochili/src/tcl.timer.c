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

/* 内核定时器分为3种，分别用于线程延时、时限方式访问资源和用户定时器 */
static TTimerList TimerList;


/*************************************************************************************************
 *  功能：内核定时器初始化函数                                                                   *
 *  参数：(1) pTimer   定时器结构地址                                                            *
 *        (2) property 定时器属性                                                                *
 *        (3) type     定时器类型                                                                *
 *        (4) ticks    定时器时钟节拍数目                                                        *
 *        (3) pRoutine 定时器回调函数                                                            *
 *        (4) data     定时器回调函数参数                                                        *
 *        (5) pOwner   定时器所属线程                                                            *
 *  返回：无                                                                                     *
 *  说明                                                                                         *
 *************************************************************************************************/
void uTimerCreate(TTimer* pTimer, TProperty property, TTimerType type, TTimeTick ticks,
                  TTimerRoutine pRoutine, TArgument data, void* pOwner)
{
    /* 初始化定时器，设置定时器信息 */
    pTimer->Status         = eTimerDormant;
    pTimer->Property       = (property | TIMER_PROP_READY);
    pTimer->ID             = uKernelVariable.ObjID;
    uKernelVariable.ObjID++;

    /* uTimerConfig函数会动态调整Type\PeriodTicks这2个参数 */
    pTimer->Type           = type;
    pTimer->PeriodTicks    = ticks;
    pTimer->MatchTicks     = TCLM_MAX_VALUE64;
    pTimer->Routine        = pRoutine;
    pTimer->Argument       = data;
    pTimer->Owner          = pOwner;

    /* 设置定时器链表节点信息, 并将定时器加入休眠队列中 */
    pTimer->ObjNode.Next   = (TObjNode*)0;
    pTimer->ObjNode.Prev   = (TObjNode*)0;
    pTimer->ObjNode.Handle = (TObjNode**)0;
    pTimer->ObjNode.Data   = (TBase32*)(&(pTimer->MatchTicks));
    pTimer->ObjNode.Owner  = (void*)pTimer;
    uObjListAddNode(&(TimerList.DormantHandle), &(pTimer->ObjNode), eQuePosHead);
}


/*************************************************************************************************
 *  功能：内核定时器取消初始化                                                                   *
 *  参数：(1) pTimer  定时器结构地址                                                             *
 *  返回：无                                                                                     *
 *  说明                                                                                         *
 *************************************************************************************************/
void uTimerDelete(TTimer* pTimer)
{
    /* 将活动定时器从所属队列中移出 */
    uObjListRemoveNode(pTimer->ObjNode.Handle, &(pTimer->ObjNode));

    /* 清空定时器数据 */
    memset(pTimer, 0U, sizeof(TTimer));
}


/*************************************************************************************************
 *  功能：重置定时器类型和定时时间                                                               *
 *  参数：(1) pTimer 定时器结构地址                                                              *
 *        (2) type   定时器类型                                                                  *
 *        (2) ticks  定时器时钟节拍数目                                                          *
 *  返回：无                                                                                     *
 *  说明                                                                                         *
 *************************************************************************************************/
void uTimerConfig(TTimer* pTimer, TTimerType type, TTimeTick ticks)
{
    if (pTimer->Status == eTimerDormant)
    {
        pTimer->Type        = type;
        pTimer->PeriodTicks = ticks;
    }
}


/*************************************************************************************************
 *  功能：内核定时器启动函数                                                                     *
 *  参数：(1) pTimer     定时器结构地址                                                          *
 *        (2) lagTicks   定时器延缓开始计数时间                                                  *
 *  返回：无                                                                                     *
 *  说明  只有处于eTimerDormant状态的定时器才能够被启动                                          *
 *        只有用户线程才有定时器延缓开始计时的需求                                               *
 *************************************************************************************************/
void uTimerStart(TTimer* pTimer, TTimeTick lagticks)
{
    TIndex spoke;

    if (pTimer->Status == eTimerDormant)
    {
        /* 将定时器从休眠队列中移出 */
        uObjListRemoveNode(pTimer->ObjNode.Handle, &(pTimer->ObjNode));

        /* 将定时器加入活动队列里 */
        pTimer->MatchTicks  = uKernelVariable.Jiffies + pTimer->PeriodTicks + lagticks;
        spoke = (TBase32)(pTimer->MatchTicks % TCLC_TIMER_WHEEL_SIZE);
        uObjListAddPriorityNode(&(TimerList.ActiveHandle[spoke]), &(pTimer->ObjNode));
        pTimer->Status = eTimerActive;
    }
}


/*************************************************************************************************
 *  功能：内核定时器停止函数                                                                     *
 *  参数：(1) pTimer 定时器地址                                                                  *
 *  返回：无                                                                                     *
 *  说明  只有处于eTimerActive和eTimerExpired 状态的定时器才能够停止                             *
 *        定时器被停止后若返回eTimerDormant状态，仍可被再次启动                                  *
 *************************************************************************************************/
void uTimerStop(TTimer* pTimer)
{
    /* 将定时器从活动队列/期满队列中移出，放到休眠队列里 */
    if (pTimer->Status != eTimerDormant)
    {
        uObjListRemoveNode(pTimer->ObjNode.Handle, &(pTimer->ObjNode));
        uObjListAddNode(&(TimerList.DormantHandle), &(pTimer->ObjNode), eQuePosHead);
        pTimer->Status = eTimerDormant;
    }
}


/*************************************************************************************************
 *  功能：用户定时器重置函数                                                                     *
 *  参数：(1) pTimer  定时器结构地址                                                             *
 *  返回：无                                                                                     *
 *  说明                                                                                         *
 *************************************************************************************************/
static void ResetTimer(TTimer* pTimer)
{
    TIndex spoke;
    KNL_ASSERT((pTimer->Type == eUserTimer), "");

    /* 将定时器从期满队列中移出 */
    uObjListRemoveNode(pTimer->ObjNode.Handle, &(pTimer->ObjNode));

    /* 将周期类型的用户定时器重新放回活动定时器队列里 */
    if (pTimer->Property & TIMER_PROP_PERIODIC)
    {
        /* 需要重新恢复定时器计数值 */
        pTimer->MatchTicks = uKernelVariable.Jiffies + pTimer->PeriodTicks;
        spoke = (TBase32)(pTimer->MatchTicks % TCLC_TIMER_WHEEL_SIZE);
        uObjListAddPriorityNode(&(TimerList.ActiveHandle[spoke]), &(pTimer->ObjNode));
        pTimer->Status = eTimerActive;
    }
    else
    {
        /* 将单次回调定时器放到休眠队列里 */
        uObjListAddNode(&(TimerList.DormantHandle), &(pTimer->ObjNode), eQuePosHead);
        pTimer->Status = eTimerDormant;
    }
}


/*************************************************************************************************
 *  功能：内核定时器执行处理函数                                                                 *
 *  参数：(1) pTimer 定时器                                                                      *
 *  返回：无                                                                                     *
 *  说明: (1)线程延时的时候，线程被放入内核线程延时队列中                                        *
 *        (2)线程以时限方式访问资源的时候，如果得不到资源会被放入资源的线程阻塞队列              *
 *           这里虽然有线程队列操作但是不进行调度，是因为这个函数是在中断中调用的，              *
 *           在最后一层中断返回后，会尝试进行一次线程切换，所以在这里做切换的话是白白浪费时间    *
 *************************************************************************************************/
static void DispatchTimer(TTimer* pTimer)
{
    TThread* pThread;
#if ((TCLC_IPC_ENABLE) && (TCLC_IPC_TIMER_ENABLE))
    TBool HiRP = eFalse;
#endif

    /* 如果定时器是线程延时类型的定时器 */
    if (pTimer->Type == eThreadTimer)
    {
        /* 将定时器从活动队列中移出,将定时器放到休眠队列里 */
        uObjListRemoveNode(pTimer->ObjNode.Handle, &(pTimer->ObjNode));
        uObjListAddNode(&(TimerList.DormantHandle), &(pTimer->ObjNode), eQuePosHead);
        pTimer->Status = eTimerDormant;

        /* 如果定时器所在的线程处于延时状态则把线程放入内核线程活动队列 */
        pThread = (TThread*)(pTimer->Owner);
        KNL_ASSERT((pThread->Status == eThreadDelayed), "");
        /*
         * 操作线程，完成线程队列和状态转换,注意只有中断处理时，
         * 当前线程才会处在内核线程辅助队列里(因为还没来得及线程切换)
         * 当前线程返回就绪队列时，一定要回到相应的队列头
         * 当线程进出就绪队列时，不需要处理线程的时钟节拍数
         */
        uThreadLeaveQueue(uKernelVariable.ThreadAuxiliaryQueue, pThread);
        if (pThread == uKernelVariable.CurrentThread)
        {
            uThreadEnterQueue(uKernelVariable.ThreadReadyQueue, pThread, eQuePosHead);
            pThread->Status = eThreadRunning;
        }
        else
        {
            uThreadEnterQueue(uKernelVariable.ThreadReadyQueue, pThread, eQuePosTail);
            pThread->Status = eThreadReady;
        }
    }

    /* 如果定时器是用户定时器类型 */
    else if (pTimer->Type == eUserTimer)
    {
#if (TCLC_TIMER_DAEMON_ENABLE)
        /*
         * 紧急的定时器直接在ISR里处理回调事务;
         * 否则将定时器放入内核定时器期满列表，最后由定时器守护线程处理
         */
        if (pTimer->Property & TIMER_PROP_URGENT)
        {
            pTimer->Routine(pTimer->Argument);
            ResetTimer(pTimer);
        }
        else
        {
            uObjListRemoveNode(pTimer->ObjNode.Handle, &(pTimer->ObjNode));
            uObjListAddNode(&(TimerList.ExpiredHandle), &(pTimer->ObjNode), eQuePosTail);
            pTimer->Status = eTimerExpired;
        }
#else
        /* 在ISR里直接处理定时器回调事务 */
        pTimer->Routine(pTimer->Argument);
        ResetTimer(pTimer);
#endif
    }

#if ((TCLC_IPC_ENABLE) && (TCLC_IPC_TIMER_ENABLE))
    /* 如果定时器是线程阻塞类型的定时器（时限方式访问资源）则将线程从阻塞队列中唤醒 */
    else if (pTimer->Type == eIpcTimer)
    {
        /* 将定时器从活动队列中移出,将定时器放到休眠队列里 */
        uObjListRemoveNode(pTimer->ObjNode.Handle, &(pTimer->ObjNode));
        uObjListAddNode(&(TimerList.DormantHandle), &(pTimer->ObjNode), eQuePosHead);
        pTimer->Status = eTimerDormant;

        /* IPC代码确保不会再次停止timer */
        pThread = (TThread*)(pTimer->Owner);
        KNL_ASSERT((pThread->Status == eThreadBlocked), "");
        uIpcUnblockThread(&(pThread->IpcContext), eFailure, IPC_ERR_TIMEO, &HiRP);
    }
#endif

    else
    {
        uDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }
}


/*************************************************************************************************
 *  功能：内核定时器ISR处理函数                                                                  *
 *  参数：无                                                                                     *
 *  返回：无                                                                                     *
 *  说明:                                                                                        *
 *************************************************************************************************/
void uTimerTickISR(void)
{
    TTimer*   pTimer;
    TIndex    spoke;
    TObjNode* pNode;
    TObjNode* pNext;

    /* 获得当前活动定时器队列 */
    spoke = (TIndex)(uKernelVariable.Jiffies % TCLC_TIMER_WHEEL_SIZE);
    pNode = TimerList.ActiveHandle[spoke];

    /* 检查当前活动定时器队列里的每个定时器 */
    while (pNode != (TObjNode*)0)
    {
        pNext = pNode->Next;
        pTimer = (TTimer*)(pNode->Owner);

        /* 如果当前定时器的定时时钟节拍数和此时的系统内核时钟节拍计数不相等则退出循环 */
        if (pTimer->MatchTicks != uKernelVariable.Jiffies)
        {
            break;
        }

        /* 否则分析处理该定时器 */
        DispatchTimer(pTimer);

        pNode = pNext;
    }

    /* 如果需要则唤醒内核内置的用户定时器守护线程 */
#if (TCLC_TIMER_DAEMON_ENABLE)
    if (TimerList.ExpiredHandle != (TObjNode*)0)
    {
        uThreadResumeFromISR(uKernelVariable.TimerDaemon);
    }
#endif
}


/*************************************************************************************************
 *  功能：用户定时器初始化函数                                                                   *
 *  参数：(1) pTimer   定时器地址                                                                *
 *        (2) property 定时器属性                                                                *
 *        (3) ticks    定时器滴答数目                                                            *
 *        (4) pRoutine 用户定时器回调函数                                                        *
 *        (5) data     用户定时器回调函数参数                                                    *
 *        (6) pError   详细调用结果                                                              *
 *  返回: (1) eSuccess 操作成功                                                                  *
 *        (2) eFailure 操作失败                                                                  *
 *  说明                                                                                         *
 *************************************************************************************************/
TState xTimerCreate(TTimer* pTimer, TProperty property, TTimeTick ticks,
                    TTimerRoutine pRoutine, TArgument data, TError* pError)
{
    TState state = eFailure;
    TError error = TIMER_ERR_FAULT;
    TReg32 imask;

    CpuEnterCritical(&imask);

    /* 检查定时器就绪属性 */
    if (!(pTimer->Property & TIMER_PROP_READY))
    {
        if (ticks > 0U)
        {
            uTimerCreate(pTimer, property, eUserTimer, ticks, pRoutine, data, (void*)0);
            error = TIMER_ERR_NONE;
            state = eSuccess;
        }
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
        if (pTimer->Type == eUserTimer)
        {
            uTimerDelete(pTimer);
            error = TIMER_ERR_NONE;
            state = eSuccess;
        }
        else
        {
            error = TIMER_ERR_FAULT;
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

    CpuEnterCritical(&imask);

    /* 检查定时器就绪属性 */
    if (pTimer->Property & TIMER_PROP_READY)
    {
        if (pTimer->Type == eUserTimer)
        {
            uTimerStart(pTimer, lagticks);
            error = TIMER_ERR_NONE;
            state = eSuccess;
        }
        else
        {
            error = TIMER_ERR_FAULT;
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
 *        (3) pError   详细调用结果                                                              *
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
        if (pTimer->Type == eUserTimer)
        {
            uTimerStop(pTimer);
            error = TIMER_ERR_NONE;
            state = eSuccess;
        }
        else
        {
            error = TIMER_ERR_FAULT;
        }
    }
    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能：重置定时器类型和定时时间                                                               *
 *  参数：(1) pTimer 定时器结构地址                                                              *
 *        (2) ticks  定时器时钟节拍数目                                                          *
 *        (3) pError 详细调用结果                                                                *
 *  返回：无                                                                                     *
 *  说明                                                                                         *
 *************************************************************************************************/
TState xTimerConfig(TTimer* pTimer, TTimeTick ticks, TError* pError)
{
    TState state = eFailure;
    TError error = TIMER_ERR_UNREADY;
    TReg32 imask;

    CpuEnterCritical(&imask);

    /* 检查定时器就绪属性 */
    if (pTimer->Property & TIMER_PROP_READY)
    {
        /* 定时器类型检查，然后保持定时器类型不变，稍有啰嗦 */
        if (pTimer->Type == eUserTimer)
        {
            uTimerConfig(pTimer, eUserTimer, ticks);
            error = TIMER_ERR_NONE;
            state = eSuccess;
        }
        else
        {
            error = TIMER_ERR_FAULT;
        }
    }
    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}


#if (TCLC_TIMER_DAEMON_ENABLE)

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

    /*
     * 逐个处理用户定时器，在线程环境下处理定时器回调事务
     * 如果期满定时器队列为空则将定时器守护线程挂起
     */
    while(eTrue)
    {
        CpuEnterCritical(&imask);

        if (TimerList.ExpiredHandle == (TObjNode*)0)
        {
            uThreadSuspendSelf();
            CpuLeaveCritical(imask);
        }
        else
        {
            pTimer = (TTimer*)(TimerList.ExpiredHandle->Owner);
            pRoutine = pTimer->Routine;
            data = pTimer->Argument;
            ResetTimer(pTimer);
            CpuLeaveCritical(imask);
            pRoutine(data);
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
                  eThreadSuspended,
                  THREAD_PROP_PRIORITY_FIXED|\
                  THREAD_PROP_CLEAN_STACK|\
                  THREAD_PROP_DAEMON,
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
#endif


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
