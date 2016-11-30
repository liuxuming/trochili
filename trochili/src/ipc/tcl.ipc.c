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
    if ((pContext->Option) & OS_IPC_OPT_UARGENT)
    {
        if (property & OS_IPC_PROP_PREEMP_AUXIQ)
        {
            OsObjQueueAddPriorityNode(&(pQueue->AuxiliaryHandle), &(pContext->LinkNode));
        }
        else
        {
            OsObjQueueAddFifoNode(&(pQueue->AuxiliaryHandle), &(pContext->LinkNode), OsLinkTail);
        }
        property |= OS_IPC_PROP_AUXIQ_AVAIL;
    }
    else
    {
        if (property & OS_IPC_PROP_PREEMP_PRIMIQ)
        {
            OsObjQueueAddPriorityNode(&(pQueue->PrimaryHandle), &(pContext->LinkNode));
        }
        else
        {
            OsObjQueueAddFifoNode(&(pQueue->PrimaryHandle), &(pContext->LinkNode), OsLinkTail);
        }
        property |= OS_IPC_PROP_PRIMQ_AVAIL;
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
    if ((pContext->Option) & OS_IPC_OPT_UARGENT)
    {
        OsObjQueueRemoveNode(&(pQueue->AuxiliaryHandle), &(pContext->LinkNode));
        if (pQueue->AuxiliaryHandle == (TLinkNode*)0)
        {
            property &= ~OS_IPC_PROP_AUXIQ_AVAIL;
        }
    }
    else
    {
        OsObjQueueRemoveNode(&(pQueue->PrimaryHandle), &(pContext->LinkNode));
        if (pQueue->PrimaryHandle == (TLinkNode*)0)
        {
            property &= ~OS_IPC_PROP_PRIMQ_AVAIL;
        }
    }

    *(pQueue->Property) = property;

    /* 设置线程所属队列 */
    pContext->Queue = (TIpcQueue*)0;
}


/*************************************************************************************************
 *  功能：将线程放入资源阻塞队列                                                                 *
 *  参数：(1) pContext阻塞对象地址                                                               *
 *        (2) pQueue  线程队列结构地址                                                           *
 *  返回：无                                                                                     *
 *  说明：对于线程进出相关队列的策略根据队列策略特性来进行                                       *
 *************************************************************************************************/
void OsIpcBlockThread(TIpcContext* pContext, TIpcQueue* pQueue)
{
    TThread* pThread;

    OS_ASSERT((OsKernelVariable.State != OsExtremeState), "");

    /* 获得线程地址 */
    pThread = (TThread*)(pContext->Owner);

    /* 只有处于就绪状态的线程才可以被阻塞 */
    if (pThread->Status != OsThreadRunning)
    {
        OsKernelVariable.Diagnosis |= OS_KERNEL_DIAG_THREAD_ERROR;
        pThread->Diagnosis |= OS_THREAD_DIAG_INVALID_STATE;
        OsDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }

    /* 将线程放入内核线程辅助队列 */
    OsThreadLeaveQueue(OsKernelVariable.ThreadReadyQueue, pThread);
    OsThreadEnterQueue(OsKernelVariable.ThreadAuxiliaryQueue, pThread, OsLinkTail);
    pThread->Status = OsThreadBlocked;

    /* 将线程放入阻塞队列 */
    EnterBlockedQueue(pQueue, pContext);

    /* 如果需要就启动线程用于访问资源的时限定时器 */
    if ((pContext->Option & OS_IPC_OPT_TIMEO) && (pContext->Ticks > 0U))
    {
        pThread->Timer.RemainTicks = pContext->Ticks;
        OsObjListAddDiffNode(&(OsKernelVariable.ThreadTimerList),
                             &(pThread->Timer.LinkNode));
    }
}


/*************************************************************************************************
 *  功能：唤醒IPC阻塞队列中指定的线程                                                            *
 *  参数：(1) pContext阻塞对象地址                                                               *
 *        (2) state   线程资源访问返回结果                                                       *
 *        (3) error   详细调用结果                                                               *
 *        (4) pHiRP   是否因唤醒更高优先级而导致需要进行线程调度的标记                           *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void OsIpcUnblockThread(TIpcContext* pContext, TState state, TError error, TBool* pHiRP)
{
    TThread* pThread;
    pThread = (TThread*)(pContext->Owner);

    /* 只有处于阻塞状态的线程才可以被解除阻塞 */
    if (pThread->Status != OsThreadBlocked)
    {
        OsKernelVariable.Diagnosis |= OS_KERNEL_DIAG_THREAD_ERROR;
        pThread->Diagnosis |= OS_THREAD_DIAG_INVALID_STATE;
        OsDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }

    /*
     * 操作线程，完成线程队列和状态转换,注意只有中断处理时，
     * 当前线程才会处在内核线程辅助队列里(因为还没来得及线程切换)
     * 当前线程返回就绪队列时，一定要回到相应的队列头
     * 当线程进出就绪队列时，不需要处理线程的时钟节拍数
     */
    OsThreadLeaveQueue(OsKernelVariable.ThreadAuxiliaryQueue, pThread);
    if (pThread == OsKernelVariable.CurrentThread)
    {
        OsThreadEnterQueue(OsKernelVariable.ThreadReadyQueue, pThread, OsLinkHead);
        pThread->Status = OsThreadRunning;
    }
    else
    {
        OsThreadEnterQueue(OsKernelVariable.ThreadReadyQueue, pThread, OsLinkTail);
        pThread->Status = OsThreadReady;
    }

    /* 将线程从阻塞队列移出 */
    LeaveBlockedQueue(pContext->Queue, pContext);

    /* 设置线程访问资源的结果和错误代码 */
    *(pContext->State) = state;
    *(pContext->Error) = error;

    /* 如果线程是以时限方式访问资源则关闭该线程的时限定时器 */
    if (pContext->Option & OS_IPC_OPT_TIMEO)
    {
        OsObjListRemoveDiffNode(&(OsKernelVariable.ThreadTimerList),
                                &(pThread->Timer.LinkNode));
    }

    /* 设置线程调度请求标记,此标记只在线程环境下有效。
     * 在ISR里，当前线程可能在任何队列里，跟当前线程相比较优先级也是无意义的。
     * 在线程环境下，如果当前线程的优先级已经不再是线程就绪队列的最高优先级，
     * 并且内核此时并没有关闭线程调度，那么就需要进行一次线程抢占
     */
    if (pThread->Priority < OsKernelVariable.CurrentThread->Priority)
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
void OsIpcUnblockAll(TIpcQueue* pQueue, TState state, TError error, void** pData2, TBool* pHiRP)
{
    TIpcContext* pContext;

    /* 辅助队列中的线程首先被解除阻塞 */
    while (pQueue->AuxiliaryHandle != (TLinkNode*)0)
    {
        pContext = (TIpcContext*)(pQueue->AuxiliaryHandle->Owner);
        OsIpcUnblockThread(pContext, state, error, pHiRP);

        /* 如果被解除阻塞的线程正在等待读取数据 */
        if ((pContext->Option & OS_IPC_OPT_READ_DATA) &&
                (pContext->Data.Addr2 != (void**)0) &&
                (pData2 != (void**)0) )
        {
            *(pContext->Data.Addr2) = *pData2;
        }
    }

    /* 基本队列中的线程随后被解除阻塞 */
    while (pQueue->PrimaryHandle != (TLinkNode*)0)
    {
        pContext = (TIpcContext*)(pQueue->PrimaryHandle->Owner);
        OsIpcUnblockThread(pContext, state, error, pHiRP);

        /* 如果被解除阻塞的线程正在等待读取数据 */
        if ((pContext->Option & OS_IPC_OPT_READ_DATA) &&
                (pContext->Data.Addr2 != (void**)0) &&
                (pData2 != (void**)0) )
        {
            *(pContext->Data.Addr2) = *pData2;
        }
    }
}


/*************************************************************************************************
 *  功能：改变处在IPC阻塞队列中的线程的优先级                                                    *
 *  参数：(1) pContext 阻塞对象地址                                                              *
 *        (2) priority 资源等待时限                                                              *
 *  返回：无                                                                                     *
 *  说明：如果线程所属阻塞队列采用优先级策略，则将线程从所属的阻塞队列中移出，然后修改它的优先级,*
 *        最后再放回原队列。如果是先入先出队列则不必处理。                                       *
 *************************************************************************************************/
void OsIpcSetPriority(TIpcContext* pContext, TPriority priority)
{
    TProperty property;
    TIpcQueue* pQueue;

    pQueue = pContext->Queue;

    /* 根据实际情况来重新安排线程在IPC阻塞队列里的位置 */
    property = *(pContext->Queue->Property);
    if (pContext->Option & OS_IPC_OPT_UARGENT)
    {
        if (property & OS_IPC_PROP_PREEMP_AUXIQ)
        {
            OsObjQueueRemoveNode(&(pQueue->AuxiliaryHandle), &(pContext->LinkNode));
            OsObjQueueAddPriorityNode(&(pQueue->AuxiliaryHandle), &(pContext->LinkNode));
        }
    }
    else
    {
        if (property & OS_IPC_PROP_PREEMP_PRIMIQ)
        {
            OsObjQueueRemoveNode(&(pQueue->PrimaryHandle), &(pContext->LinkNode));
            OsObjQueueAddPriorityNode(&(pQueue->PrimaryHandle), &(pContext->LinkNode));
        }
    }
}


/*************************************************************************************************
 *  功能：设定阻塞线程的IPC对象的信息                                                            *
 *  参数：(1) pContext阻塞对象地址                                                               *
 *        (2) pIpc    正在操作的IPC对象的地址                                                    *
 *        (3) data    指向数据目标对象指针的指针                                                 *
 *        (4) len     数据的长度                                                                 *
 *        (5) option  操作IPC对象时的各种参数                                                    *
 *        (6) state   IPC对象访问结果                                                            *
 *        (7) pError  详细调用结果                                                               *
 *  返回：无                                                                                     *
 *  说明：data指向的指针，就是需要通过IPC机制来传递的数据在线程空间的指针                        *
 *************************************************************************************************/
void OsIpcInitContext(TIpcContext* pContext, void* pIpc, TBase32 data, TBase32 len,
                      TOption option, TTimeTick ticks, TState* pState, TError* pError)
{
    TThread* pThread;

    pThread = OsKernelVariable.CurrentThread;
    pThread->IpcContext = pContext;

    pContext->Object     = pIpc;
    pContext->Queue      = (TIpcQueue*)0;
    pContext->Data.Value = data;
    pContext->Length     = len;
    pContext->Option     = option;
    pContext->Ticks      = ticks;
    pContext->State      = pState;
    pContext->Error      = pError;
    pContext->Owner      = (void*)pThread;
    pContext->LinkNode.Next   = (TLinkNode*)0;
    pContext->LinkNode.Prev   = (TLinkNode*)0;
    pContext->LinkNode.Handle = (TLinkNode**)0;
    pContext->LinkNode.Data   = (TBase32*)(&(pThread->Priority));
    pContext->LinkNode.Owner  = (void*)pContext;

    *pState = eError;
    *pError = OS_IPC_ERR_FAULT;
}


/*************************************************************************************************
 *  功能：清除阻塞线程的IPC对象的信息                                                            *
 *  参数：(1) pContext 阻塞对象地址                                                              *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void OsIpcCleanContext(TIpcContext* pContext)
{
    TThread* pThread;

    pThread = (TThread*)(pContext->Owner);
    pThread->IpcContext = (TIpcContext*)0;

    memset(pContext, 0U, sizeof(TIpcContext));
}

#endif

