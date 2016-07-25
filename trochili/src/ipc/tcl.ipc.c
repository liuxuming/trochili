/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.object.h"
#include "tcl.debug.h"
#include "tcl.kernel.h"
#include "tcl.timer.h"
#include "tcl.thread.h"
#include "tcl.ipc.h"

#if (TCLC_IPC_ENABLE)

/*************************************************************************************************
 *  功能：将线程加入到指定的IPC线程阻塞队列中                                                    *
 *  参数：(1) pQueue   IPC队列地址                                                               *
 *        (2) pThread  线程结构地址                                                              *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
static void EnterBlockedQueue(TIpcQueue* pQueue, TIpcContext* pContext)
{
    TProperty property;

    property = *(pQueue->Property);
    if ((pContext->Option) & IPC_OPT_USE_AUXIQ)
    {
        if (property &IPC_PROP_PREEMP_AUXIQ)
        {
            uObjQueueAddPriorityNode(&(pQueue->AuxiliaryHandle), &(pContext->ObjNode));
        }
        else
        {
            uObjQueueAddFifoNode(&(pQueue->AuxiliaryHandle), &(pContext->ObjNode), eQuePosTail);
        }
        property |= IPC_PROP_AUXIQ_AVAIL;
    }
    else
    {
        if (property &IPC_PROP_PREEMP_PRIMIQ)
        {
            uObjQueueAddPriorityNode(&(pQueue->PrimaryHandle), &(pContext->ObjNode));
        }
        else
        {
            uObjQueueAddFifoNode(&(pQueue->PrimaryHandle), &(pContext->ObjNode), eQuePosTail);
        }
        property |= IPC_PROP_PRIMQ_AVAIL;
    }

    *(pQueue->Property) = property;

    /* 设置线程所属队列 */
    pContext->Queue = pQueue;
}


/*************************************************************************************************
 *  功能：将线程从指定的线程队列中移出                                                           *
 *  参数：(1) pQueue   IPC队列地址                                                               *
 *        (2) pThread  线程结构地址                                                              *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
static void LeaveBlockedQueue(TIpcQueue* pQueue, TIpcContext* pContext)
{
    TProperty property;

    property = *(pQueue->Property);

    /* 将线程从指定的分队列中取出 */
    if ((pContext->Option) & IPC_OPT_USE_AUXIQ)
    {
        uObjQueueRemoveNode(&(pQueue->AuxiliaryHandle), &(pContext->ObjNode));
        if (pQueue->AuxiliaryHandle == (TObjNode*)0)
        {
            property &= ~IPC_PROP_AUXIQ_AVAIL;
        }
    }
    else
    {
        uObjQueueRemoveNode(&(pQueue->PrimaryHandle), &(pContext->ObjNode));
        if (pQueue->PrimaryHandle == (TObjNode*)0)
        {
            property &= ~IPC_PROP_PRIMQ_AVAIL;
        }
    }

    *(pQueue->Property) = property;

    /* 设置线程所属队列 */
    pContext->Queue = (TIpcQueue*)0;
}


/*************************************************************************************************
 *  功能：将线程放入资源阻塞队列                                                                 *
 *  参数：(1) pQueue  线程队列结构地址                                                           *
 *        (2) pThread 线程结构地址                                                               *
 *        (3) ticks   资源等待时限                                                               *
 *  返回：无                                                                                     *
 *  说明：对于线程进出相关队列的策略根据队列策略特性来进行                                       *
 *************************************************************************************************/
void uIpcBlockThread(TIpcContext* pContext, TIpcQueue* pQueue, TTimeTick ticks)
{
    TThread* pThread;

    KNL_ASSERT((uKernelVariable.State != eIntrState), "");

    /* 将线程放入内核线程辅助队列 */
    pThread = (TThread*)(pContext->Owner);

    /* 只有处于就绪状态的线程才可以被阻塞 */
    if (pThread->Status != eThreadRunning)
    {
        uDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }

    uThreadLeaveQueue(uKernelVariable.ThreadReadyQueue, pThread);
    uThreadEnterQueue(uKernelVariable.ThreadAuxiliaryQueue, pThread, eQuePosTail);
    pThread->Status = eThreadBlocked;

    /* 将线程放入阻塞队列 */
    EnterBlockedQueue(pQueue, pContext);

    /* 如果需要就初始化并且打开线程用于访问资源的时限定时器 */
#if (TCLC_TIMER_ENABLE && TCLC_IPC_TIMER_ENABLE)
    if ((pContext->Option & IPC_OPT_TIMED) && (ticks > 0U))
    {
        /* 重新配置并启动线程定时器 */
        uTimerConfig(&(pThread->Timer), eIpcTimer, ticks);
        uTimerStart(&(pThread->Timer), 0U);
    }
#else
    ticks = ticks;
#endif
}


/*************************************************************************************************
 *  功能：唤醒IPC阻塞队列中指定的线程                                                            *
 *  参数：(1) pThread 线程地址                                                                   *
 *        (2) state   线程资源访问返回结果                                                       *
 *        (3) error   详细调用结果                                                               *
 *        (4) pHiRP   是否因唤醒更高优先级而导致需要进行线程调度的标记                           *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void uIpcUnblockThread(TIpcContext* pContext, TState state, TError error, TBool* pHiRP)
{
    TThread* pThread;
    TQueuePos pos = eQuePosTail;
    TThreadStatus newStatus = eThreadReady;
	
    /* 
     * 将线程从IPC资源的阻塞队列中移出，加入到内核线程就绪队列,
     * 如果当前线程刚刚被阻塞到阻塞队列中，但还未发生线程切换，
     * 而在此时被ISR打断并且ISR又将当前线程唤醒，则当前线程也不必返回就绪队列头 
     */
    pThread = (TThread*)(pContext->Owner);

    /* 只有处于阻塞状态的线程才可以被解除阻塞 */
    if (pThread->Status != eThreadBlocked)
    {
        uDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }

    /*
     * 操作线程，完成线程队列和状态转换,注意只有中断处理时，
     * 当前线程才会处在内核线程辅助队列里(因为还没来得及线程切换)
     * 当前线程返回就绪队列时，一定要回到相应的队列头
     * 当线程进出就绪队列时，不需要处理线程的时钟节拍数
     */
    uThreadLeaveQueue(uKernelVariable.ThreadAuxiliaryQueue, pThread);
    if (pThread == uKernelVariable.CurrentThread)
    {
        pos = eQuePosHead;
        newStatus = eThreadRunning;
    }
    uThreadEnterQueue(uKernelVariable.ThreadReadyQueue, pThread, pos);
    pThread->Status = newStatus;

    /* 将线程从阻塞队列移出 */
    LeaveBlockedQueue(pContext->Queue, pContext);

    /* 设置线程访问资源的结果和错误代码 */
    *(pContext->State) = state;
    *(pContext->Error) = error;

    /* 如果线程是以时限方式访问资源则取消该线程的时限定时器 */
#if ((TCLC_IPC_TIMER_ENABLE) && (TCLC_TIMER_ENABLE))
    if ((pContext->Option & IPC_OPT_TIMED) && (error != IPC_ERR_TIMEO))
    {
        KNL_ASSERT((pThread->Timer.Type == eIpcTimer), "");
        uTimerStop(&(pThread->Timer));
    }
#endif

    /* 设置线程调度请求标记,此标记只在线程环境下有效。
       在ISR里，当前线程可能在任何队列里，
       跟当前线程相比较优先级也是无意义的 */
    /*
    * 在线程环境下，如果当前线程的优先级已经不再是线程就绪队列的最高优先级，
    * 并且内核此时并没有关闭线程调度，那么就需要进行一次线程抢占
    */
    if (pThread->Priority < uKernelVariable.CurrentThread->Priority)
    {
        *pHiRP = eTrue;
    }
}


/*************************************************************************************************
 *  功能：选择唤醒阻塞队列中的全部线程                                                           *
 *  参数：(1) pQueue  线程队列结构地址                                                           *
 *        (2) state   线程资源访问返回结果                                                       *
 *        (3) error   详细调用结果                                                               *
 *        (4) pData   线程访问IPC得到的数据                                                      *
 *        (5) pHiRP  线程是否需要调度的标记                                                      *
 *  返回：                                                                                       *
 *  说明：只有邮箱和消息队列广播时才会传递pData2参数                                             *
 *************************************************************************************************/
void uIpcUnblockAll(TIpcQueue* pQueue, TState state, TError error, void** pData2, TBool* pHiRP)
{
    TIpcContext* pContext;

    /* 辅助队列中的线程首先被解除阻塞 */
    while (pQueue->AuxiliaryHandle != (TObjNode*)0)
    {
        pContext = (TIpcContext*)(pQueue->AuxiliaryHandle->Owner);
        uIpcUnblockThread(pContext, state, error, pHiRP);

        if ((pData2 != (void**)0) && (pContext->Data.Addr2 != (void**)0))
        {
            *(pContext->Data.Addr2) = *pData2;
        }
    }

    /* 基本队列中的线程随后被解除阻塞 */
    while (pQueue->PrimaryHandle != (TObjNode*)0)
    {
        pContext = (TIpcContext*)(pQueue->PrimaryHandle->Owner);
        uIpcUnblockThread(pContext, state, error, pHiRP);

        if ((pData2 != (void**)0) && (pContext->Data.Addr2 != (void**)0))
        {
            *(pContext->Data.Addr2) = *pData2;
        }
    }
}


/*************************************************************************************************
 *  功能：改变处在IPC阻塞队列中的线程的优先级                                                    *
 *  参数：(1) pThread  线程结构地址                                                              *
 *        (2) priority 资源等待时限                                                              *
 *  返回：无                                                                                     *
 *  说明：如果线程所属阻塞队列采用优先级策略，则将线程从所属的阻塞队列中移出，然后修改它的优先级,*
 *        最后再放回原队列。如果是先入先出队列则不必处理。                                       *
 *************************************************************************************************/
void uIpcSetPriority(TIpcContext* pContext, TPriority priority)
{
    TProperty property;
    TIpcQueue* pQueue;
    TThread* pThread;

    pQueue = pContext->Queue;
    pThread = (TThread*)(pContext->Owner);

    /* 因为线程现在为阻塞状态，所以可以直接在内核线程辅助队列中修改线程的优先级 */
    pThread->Priority = priority;

    /* 根据实际情况来重新安排线程在IPC阻塞队列里的位置 */
    property = *(pContext->Queue->Property);
    if (pContext->Option & IPC_OPT_USE_AUXIQ)
    {
        if (property & IPC_PROP_PREEMP_AUXIQ)
        {
            uObjQueueRemoveNode(&(pQueue->AuxiliaryHandle), &(pContext->ObjNode));
            uObjQueueAddPriorityNode(&(pQueue->AuxiliaryHandle), &(pContext->ObjNode));
        }
    }
    else
    {
        if (property & IPC_PROP_PREEMP_PRIMIQ)
        {
            uObjQueueRemoveNode(&(pQueue->PrimaryHandle), &(pContext->ObjNode));
            uObjQueueAddPriorityNode(&(pQueue->PrimaryHandle), &(pContext->ObjNode));
        }
    }
}


/*************************************************************************************************
 *  功能：设定阻塞线程的IPC对象的信息                                                            *
 *  参数：(1) pThread 线程结构地址                                                               *
 *        (2) pIpc    正在操作的IPC对象的地址                                                    *
 *        (3) data    指向数据目标对象指针的指针                                                 *
 *        (4) len     数据的长度                                                                 *
 *        (5) option  操作IPC对象时的各种参数                                                    *
 *        (6) state   IPC对象访问结果                                                            *
 *        (7) pError  详细调用结果                                                               *
 *  返回：无                                                                                     *
 *  说明：data指向的指针，就是需要通过IPC机制来传递的数据在线程空间的指针                        *
 *************************************************************************************************/
void uIpcSaveContext(TIpcContext* pContext, void* pIpc, TBase32 data, TBase32 len,
                     TOption option, TState* pState, TError* pError)
{
    pContext->Object     = pIpc;
    pContext->Queue      = (TIpcQueue*)0;
    pContext->Data.Value = data;
    pContext->Length     = len;
    pContext->Option     = option;
    pContext->State      = pState;
    pContext->Error      = pError;
    *pState              = eError;
    *pError              = IPC_ERR_FAULT;
}


/*************************************************************************************************
 *  功能：清除阻塞线程的IPC对象的信息                                                            *
 *  参数：(1) pThread  线程结构地址                                                              *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void uIpcCleanContext(TIpcContext* pContext)
{
    pContext->Object     = (void*)0;
    pContext->Queue      = (TIpcQueue*)0;
    pContext->Data.Value = 0U;
    pContext->Length     = 0U;
    pContext->Option     = IPC_OPTION;
    pContext->State      = (TState*)0;
    pContext->Error      = (TError*)0;
}


/*************************************************************************************************
 *  功能：初始化IPC队列                                                                          *
 *  参数：(1) pQueue   IPC队列结构地址                                                           *
 *        (2) property IPC队列的行为参数                                                         *
 *  返回：无                                                                                     *
 *  说明：pContext->Owner 成员在此初始化之后就永远不要被清除                                     *
 *************************************************************************************************/
void uIpcInitContext(TIpcContext* pContext, void* pOwner)
{
    TThread* pThread;

    pThread = (TThread*)pOwner;
    pContext->Owner      = pOwner;
    pContext->Object     = (void*)0;
    pContext->Queue      = (TIpcQueue*)0;
    pContext->Data.Value = 0U;
    pContext->Length     = 0U;
    pContext->Option     = IPC_OPTION;
    pContext->State      = (TState*)0;
    pContext->Error      = (TError*)0;

    pContext->ObjNode.Next   = (TObjNode*)0;
    pContext->ObjNode.Prev   = (TObjNode*)0;
    pContext->ObjNode.Handle = (TObjNode**)0;
    pContext->ObjNode.Data   = (TBase32*)(&(pThread->Priority));
    pContext->ObjNode.Owner  = (void*)pContext;
}

#endif

