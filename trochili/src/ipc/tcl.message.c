/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#include <string.h>

#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.cpu.h"
#include "tcl.debug.h"
#include "tcl.thread.h"
#include "tcl.kernel.h"
#include "tcl.ipc.h"
#include "tcl.message.h"

#if ((TCLC_IPC_ENABLE)&&(TCLC_IPC_MQUE_ENABLE))


/*************************************************************************************************
 *  功能：将消息保存到消息队列                                                                   *
 *  参数：(1) pMsgQue 消息队列结构指针                                                           *
 *        (2) pMsg2   保存消息结构地址的指针变量                                                 *
 *        (3) type    消息的类型（紧急消息和普通消息）                                           *
 *  返回：无                                                                                     *
 *  说明：(1) 根据消息类型不同，将消息保存到消息队列队头或者队尾                                 *
 *        (2) 这个函数不能处理消息队列状态                                                       *
 *************************************************************************************************/
static void SaveMessage(TMsgQueue* pMsgQue, void** pMsg2, TMsgType type)
{
    /* 普通消息直接发送到消息队列头 */
    if (type == eNormalMessage)
    {
        *(pMsgQue->MsgPool + pMsgQue->Head) = *pMsg2;
        pMsgQue->Head++;
        if (pMsgQue->Head == pMsgQue->Capacity)
        {
            pMsgQue->Head = 0U;
        }
    }
    /* 紧急消息发送到消息队列尾 */
    else
    {
        if (pMsgQue->Tail == 0U)
        {
            pMsgQue->Tail = pMsgQue->Capacity - 1U;
        }
        else
        {
            pMsgQue->Tail--;
        }
        *(pMsgQue->MsgPool + pMsgQue->Tail) = *pMsg2;
    }

    /* 调整消息队列消息数目 */
    pMsgQue->MsgEntries++;
}


/*************************************************************************************************
 *  功能：将消息从消息队列中读出                                                                 *
 *  参数：(1) pMsgQue 消息队列结构指针                                                           *
 *        (2) pMsg2   保存消息结构地址的指针变量                                                 *
 *  返回：无                                                                                     *
 *  说明：(1) 从消息队列中读取消息的时候，只能从队尾读取                                         *
 *        (2) 这个函数不能处理消息队列状态                                                       *
 *************************************************************************************************/
static void ConsumeMessage(TMsgQueue* pMsgQue, void** pMsg2)
{
    /* 从消息队列中读取一个消息给当前线程 */
    *pMsg2 = *(pMsgQue->MsgPool + pMsgQue->Tail);

    /* 调整消息队列消息数目 */
    pMsgQue->MsgEntries--;

    /* 调整消息队列的消息读写游标 */
    pMsgQue->Tail++;
    if (pMsgQue->Tail == pMsgQue->Capacity)
    {
        pMsgQue->Tail = 0U;
    }
}


/*************************************************************************************************
 *  功能：线程/ISR尝试从消息队列中读取消息                                                       *
 *  参数：(1) pMsgQue 消息队列的地址                                                             *
 *        (2) pMsg2   保存消息结构地址的指针变量                                                 *
 *        (3) pHiRP   是否需要线程调度标记                                                       *
 *        (4) pError  详细调用结果                                                               *
 *  返回: (1) eFailure   操作失败                                                                *
 *        (2) eSuccess   操作成功                                                                *
 *  说明：                                                                                       *
 *************************************************************************************************/
/* 处理消息队列读数据的情况
   (1) 处理消息队列为满的情况：如果是容量是1的消息队列，读取操作会导致消息队列状态从满直接到空，
       否则消息队列进入 eMQPartial 状态
   (2) 处理消息队列普通的情况：如果消息队列中只有1个消息，对列读取操作可能导致消息队列进入空状态
       否则消息队列保持 eMQPartial 状态
   (3) 无论是消息队列满还是消息队列普通状态：最终状态只有普通和空。
 */
static TState ReceiveMessage(TMsgQueue* pMsgQue, void** pMsg2, TBool* pHiRP, TError* pError)
{
    TState state = eSuccess;
    TError error = IPC_ERR_NONE;
    TMsgType type;
    TIpcContext* pContext = (TIpcContext*)0;

    /* 检查消息队列状态 */
    if (pMsgQue->Status == eMQEmpty)
    {
        error = IPC_ERR_NORMAL;
        state = eFailure;
    }
    else if (pMsgQue->Status == eMQFull)
    {
        /* 从消息队列中读取一个消息给当前线程 */
        ConsumeMessage(pMsgQue, pMsg2);

        /*
        	   * 在消息队列满并且有线程阻塞在写队列中的情况下，
         * 需要将合适的线程接触阻塞并且将该线程携带的消息写入队列，
         * 所以保持消息队列状态不变
        	   */
        if (pMsgQue->Property & IPC_PROP_AUXIQ_AVAIL)
        {
            pContext = (TIpcContext*)(pMsgQue->Queue.AuxiliaryHandle->Owner);
        }
        else
        {
            if (pMsgQue->Property & IPC_PROP_PRIMQ_AVAIL)
            {
                pContext = (TIpcContext*)(pMsgQue->Queue.PrimaryHandle->Owner);
            }
        }

        if (pContext !=  (TIpcContext*)0)
        {
            uIpcUnblockThread(pContext, eSuccess, IPC_ERR_NONE, pHiRP);

            /* 根据线程所处的分队列判断消息类型,并且将该线程发送的消息写入消息队列 */
            type = ((pContext->Option) & IPC_OPT_UARGENT) ? eUrgentMessage : eNormalMessage;
            SaveMessage(pMsgQue, pContext->Data.Addr2, type);
        }
        else
        {
            pMsgQue->Status = (pMsgQue->Tail == pMsgQue->Head) ? eMQEmpty : eMQPartial;
        }
    }
    else
        /* if (mq->Status == eMQPartial) */
    {
        /* 从消息队列中读取一个消息给当前线程 */
        ConsumeMessage(pMsgQue, pMsg2);
        pMsgQue->Status = (pMsgQue->Tail == pMsgQue->Head) ? eMQEmpty : eMQPartial;
    }

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能：线程/ISR尝试向消息队列中发送消息                                                       *
 *  参数：(1) pMsgQue 消息队列的地址                                                             *
 *        (2) pMsg2   保存消息结构地址的指针变量                                                 *
 *        (3) type    消息类型                                                                   *
 *        (4) pHiRP   是否需要线程调度标记                                                       *
 *        (5) pError  详细调用结果                                                               *
 *  返回: (1) eFailure   操作失败                                                                *
 *        (2) eSuccess   操作成功                                                                *
 *  说明：                                                                                       *
 *************************************************************************************************/
static TState SendMessage(TMsgQueue* pMsgQue, void** pMsg2, TMsgType type, TBool* pHiRP,
                          TError* pError)
{
    TState state = eSuccess;
    TError error = IPC_ERR_NONE;
    TIpcContext* pContext = (TIpcContext*)0;

    /* 检查消息队列状态，如果消息队列满则返回失败 */
    if (pMsgQue->Status == eMQFull)
    {
        error = IPC_ERR_NORMAL;
        state = eFailure;
    }
    else if (pMsgQue->Status == eMQEmpty)
    {
        /*
         * 在消息队列为空的情况下，如果队列中有线程等待，则说明是读阻塞队列，
         * 将其中一个线程解除阻塞，并将消息发送给该线程，同时保持消息队列状态不变
         */
        if (pMsgQue->Property & IPC_PROP_PRIMQ_AVAIL)
        {
            pContext = (TIpcContext*)(pMsgQue->Queue.PrimaryHandle->Owner);
        }
        if (pContext != (TIpcContext*)0)
        {
            uIpcUnblockThread(pContext, eSuccess, IPC_ERR_NONE, pHiRP);
            *(pContext->Data.Addr2) = *pMsg2;
        }
        else
        {
            /* 将线程发送的消息写入消息队列 */
            SaveMessage(pMsgQue, pMsg2, type);
            pMsgQue->Status = (pMsgQue->Tail == pMsgQue->Head) ? eMQFull : eMQPartial;
        }
    }
    else
        /* if (mq->Status == eMQPartial) */
    {
        /* 将线程发送的消息写入消息队列 */
        SaveMessage(pMsgQue, pMsg2, type);

        /* 消息队列空的情况，如果是消息队列的容量是1，那么它的状态从空直接到 eMQFull，
           否则消息队列进入 eMQPartial 状态 */
        /* 消息队列普通的情况，消息对列写操作可能导致消息队列进入 eMQFull
           状态或者保持 eMQPartial 状态 */
        pMsgQue->Status = (pMsgQue->Tail == pMsgQue->Head) ? eMQFull : eMQPartial;
    }

    *pError = error;
    return state;
}

/*************************************************************************************************
 *  功能: 用于线程/ISR接收消息队列中的消息                                                       *
 *  参数: (1) pMsgQue  消息队列结构地址                                                          *
 *        (2) pMsg2    保存消息结构地址的指针变量                                                *
 *        (3) option   访问消息队列的模式                                                        *
 *        (4) timeo    时限阻塞模式下访问邮箱的时限长度                                          *
 *        (5) pError   详细调用结果                                                              *
 *  返回: (1) eFailure   操作失败                                                                *
 *        (2) eSuccess   操作成功                                                                *
 *  说明：                                                                                       *
 *************************************************************************************************/
extern TState xMQReceive(TMsgQueue* pMsgQue, TMessage* pMsg2, TOption option,
                         TTimeTick timeo, TError* pError)
{
    TState state = eFailure;
    TError error = IPC_ERR_UNREADY;
    TBool  HiRP = eFalse;
    TIpcContext* pContext = (TIpcContext*)0;
    TReg32 imask;

    CpuEnterCritical(&imask);

    if (pMsgQue->Property & IPC_PROP_READY)
    {
        /*
         * 如果是中断程序调用本函数则只能以非阻塞方式接收消息,
         * 并且暂时不考虑线程调度问题。
         * 在中断中,当前线程未必是最高就绪优先级线程,也未必处于内核就绪线程队列，
         * 所以在此处得到的HiRP标记无任何意义。
         */
        state = ReceiveMessage(pMsgQue, (void**)pMsg2, &HiRP, &error);

            if ((uKernelVariable.State == eThreadState) &&
                    (uKernelVariable.SchedLockTimes == 0U))
            {
                /* 如果当前线程解除了更高优先级线程的阻塞则进行调度。*/
                if (state == eSuccess)
                {
                    if (HiRP == eTrue)
                    {
                        uThreadSchedule();
                    }
                }
                else
                {
                    /*
                    * 如果当前线程不能接收消息，并且采用的是等待方式，
                    * 那么当前线程必须阻塞在消息队列中
                    */
                    if (option & IPC_OPT_WAIT)
                    {
                        /* 如果当前线程不能被阻塞则函数直接返回 */
                        if (uKernelVariable.CurrentThread->ACAPI & THREAD_ACAPI_BLOCK)
                        {
                            /* 得到当前线程的IPC上下文结构地址 */
                            pContext = &(uKernelVariable.CurrentThread->IpcContext);

                            /* 保存线程挂起信息 */
                            option |= IPC_OPT_MSGQUEUE | IPC_OPT_READ_DATA;
                            uIpcSaveContext(pContext, (void*)pMsgQue, (TBase32)pMsg2, sizeof(TBase32), option,
                                            &state, &error);

                            /* 当前线程阻塞在该消息队列的阻塞队列，时限或者无限等待，由IPC_OPT_TIMED参数决定 */
                            uIpcBlockThread(pContext, &(pMsgQue->Queue), timeo);

                            /* 当前线程被阻塞，其它线程得以执行 */
                            uThreadSchedule();

                            CpuLeaveCritical(imask);
                            /*
                                * 因为当前线程已经阻塞在IPC对象的线程阻塞队列，所以处理器需要执行别的线程。
                                * 当处理器再次处理本线程时，从本处继续运行。
                                */
                            CpuEnterCritical(&imask);

                            /* 清除线程挂起信息 */
                            uIpcCleanContext(pContext);
                        }
                        else
                        {
                            error = IPC_ERR_ACAPI;
                        }
                    }
                }
        }
    }

    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能: 用于线程/ISR向消息队列中发送消息                                                       *
 *  参数: (1) pMsgQue  消息队列结构地址                                                          *
 *        (2) pMsg2    保存消息结构地址的指针变量                                                *
 *        (3) option   访问消息队列的模式                                                        *
 *        (4) timeo    时限阻塞模式下访问邮箱的时限长度                                          *
 *        (5) pError   详细调用结果                                                              *
 *  返回: (1) eFailure 操作失败                                                                  *
 *        (2) eSuccess 操作成功                                                                  *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState xMQSend(TMsgQueue* pMsgQue, TMessage* pMsg2, TOption option, TTimeTick timeo,
               TError* pError)
{
    TState state = eFailure;
    TError error = IPC_ERR_UNREADY;
    TBool HiRP = eFalse;
    TIpcContext* pContext;
    TMsgType type;
    TReg32 imask;

    CpuEnterCritical(&imask);
    if (pMsgQue->Property & IPC_PROP_READY)
    {
        /*
         * 如果是中断程序调用本函数则只能以非阻塞方式发送消息,
         * 并且暂时不考虑线程调度问题。
         * 在中断中,当前线程未必是最高就绪优先级线程,也未必处于内核就绪线程队列，
         * 所以在此处得到的HiRP标记无任何意义。
         */
        type = (option & IPC_OPT_UARGENT) ? eUrgentMessage : eNormalMessage;
        state = SendMessage(pMsgQue, (void**)pMsg2, type, &HiRP, &error);

        if ((uKernelVariable.State == eThreadState) &&
                (uKernelVariable.SchedLockTimes == 0U))
        {
            /* 如果当前线程解除了更高优先级线程的阻塞则进行调度。*/
            if (state == eSuccess)
            {
                if (HiRP == eTrue)
                {
                    uThreadSchedule();
                }
            }
            else
            {
                /*
                * 如果当前线程不能发送消息，并且采用的是等待方式，
                * 那么当前线程必须阻塞在消息队列中
                */
                if (option & IPC_OPT_WAIT)
                {
                    /* 如果当前线程不能被阻塞则函数直接返回 */
                    if (uKernelVariable.CurrentThread->ACAPI & THREAD_ACAPI_BLOCK)
                    {
                        if (option & IPC_OPT_UARGENT)
                        {
                            option |= IPC_OPT_USE_AUXIQ;
                        }

                        /* 得到当前线程的IPC上下文结构地址 */
                        pContext = &(uKernelVariable.CurrentThread->IpcContext);

                        /* 保存线程挂起信息 */
                        option |= IPC_OPT_MSGQUEUE | IPC_OPT_WRITE_DATA;
                        uIpcSaveContext(pContext, (void*)pMsgQue, (TBase32)pMsg2,  sizeof(TBase32), option,
                                        &state, &error);

                        /* 当前线程阻塞在该消息队列的阻塞队列，时限或者无限等待，由IPC_OPT_TIMED参数决定 */
                        uIpcBlockThread(pContext, &(pMsgQue->Queue), timeo);

                        /* 当前线程被阻塞，其它线程得以执行 */
                        uThreadSchedule();

                        CpuLeaveCritical(imask);
                        /*
                         * 因为当前线程已经阻塞在IPC对象的线程阻塞队列，所以处理器需要执行别的线程。
                         * 当处理器再次处理本线程时，从本处继续运行。
                         */
                        CpuEnterCritical(&imask);

                        /* 清除线程挂起信息 */
                        uIpcCleanContext(pContext);
                    }
                    else
                    {
                        error = IPC_ERR_ACAPI;
                    }
                }
            }
        }
    }
    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能：消息队列初始化函数                                                                     *
 *  输入：(1) pMsgQue   消息队列结构地址                                                         *
 *        (2) pPool2    消息数组地址                                                             *
 *        (3) capacity  消息队列容量，即消息数组大小                                             *
 *        (4) policy    消息队列线程调度策略                                                     *
 *        (5) pError    详细调用结果                                                             *
 *  返回：(1) eSuccess  操作成功                                                                 *
 *        (2) eFailure  操作失败                                                                 *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState xMQCreate(TMsgQueue* pMsgQue, void** pPool2, TBase32 capacity, TProperty property,
                 TError* pError)
{
    TState state = eFailure;
    TError error = IPC_ERR_FAULT;
    TReg32 imask;

    CpuEnterCritical(&imask);

    if (!(pMsgQue->Property & IPC_PROP_READY))
    {
        property |= IPC_PROP_READY;
        pMsgQue->Property = property;
        pMsgQue->Capacity = capacity;
        pMsgQue->MsgPool = pPool2;
        pMsgQue->MsgEntries = 0U;
        pMsgQue->Head = 0U;
        pMsgQue->Tail = 0U;
        pMsgQue->Status = eMQEmpty;

        pMsgQue->Queue.PrimaryHandle   = (TObjNode*)0;
        pMsgQue->Queue.AuxiliaryHandle = (TObjNode*)0;
        pMsgQue->Queue.Property        = &(pMsgQue->Property);

        error = IPC_ERR_NONE;
        state = eSuccess;
    }

    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能：消息队列重置函数                                                                       *
 *  输入：(1) pMsgQue   消息队列结构地址                                                         *
 *        (2) pError    详细调用结果                                                             *
 *  返回：(1) eSuccess  操作成功                                                                 *
 *        (2) eFailure  操作失败                                                                 *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState xMQDelete(TMsgQueue* pMsgQue, TError* pError)
{
    TState state = eFailure;
    TError error = IPC_ERR_UNREADY;
    TBool HiRP = eFalse;
    TReg32 imask;

    CpuEnterCritical(&imask);

    if (pMsgQue->Property & IPC_PROP_READY)
    {
        /* 向阻塞队列中的线程分发消息 */
        uIpcUnblockAll(&(pMsgQue->Queue), eFailure, IPC_ERR_DELETE, (void**)0, &HiRP);

        /* 清除消息队列对象的全部数据 */
        memset(pMsgQue, 0U, sizeof(TMsgQueue));

        /*
         * 在线程环境下，如果当前线程的优先级已经不再是线程就绪队列的最高优先级，
         * 并且内核此时并没有关闭线程调度，那么就需要进行一次线程抢占
         */
        if ((uKernelVariable.State == eThreadState) &&
                (uKernelVariable.SchedLockTimes == 0U) &&
                (HiRP == eTrue))
        {
            uThreadSchedule();
        }
        error = IPC_ERR_NONE;
        state = eSuccess;
    }

    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能: 清除消息队列阻塞队列                                                                   *
 *  参数: (1) pMsgQue   消息队列结构地址                                                         *
 *        (2) pError    详细调用结果                                                             *
 *  返回：(1) eSuccess  操作成功                                                                 *
 *        (2) eFailure  操作失败                                                                 *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState xMQReset(TMsgQueue* pMsgQue, TError* pError)
{
    TState state = eFailure;
    TError error = IPC_ERR_UNREADY;
    TBool HiRP = eFalse;
    TReg32 imask;

    CpuEnterCritical(&imask);

    if (pMsgQue->Property & IPC_PROP_READY)
    {
        /* 将阻塞队列上的所有等待线程都释放，所有线程的等待结果都是TCLE_IPC_RESET    */
        uIpcUnblockAll(&(pMsgQue->Queue), eFailure, IPC_ERR_RESET, (void**)0, &HiRP);

        /* 重新设置消息队列结构 */
        pMsgQue->Property &= IPC_RESET_MQUE_PROP;
        pMsgQue->MsgEntries = 0U;
        pMsgQue->Head = 0U;
        pMsgQue->Tail = 0U;
        pMsgQue->Status = eMQEmpty;

        /*
         * 在线程环境下，如果当前线程的优先级已经不再是线程就绪队列的最高优先级，
         * 并且内核此时并没有关闭线程调度，那么就需要进行一次线程抢占
         */
        if ((uKernelVariable.State == eThreadState) &&
                (uKernelVariable.SchedLockTimes == 0U) &&
                (HiRP == eTrue))
        {
            uThreadSchedule();
        }
        error = IPC_ERR_NONE;
        state = eSuccess;
    }

    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能：消息队列阻塞终止函数,将指定的线程从消息队列的阻塞队列中终止阻塞并唤醒                  *
 *  参数：(1) pMsgQue  消息队列结构地址                                                          *
 *        (2) option   参数选项                                                                  *
 *        (3) pThread  线程地址                                                                  *
 *        (4) pError   详细调用结果                                                              *
 *  返回：(1) eSuccess 成功                                                                      *
 *        (2) eFailure 失败                                                                      *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState xMQFlush(TMsgQueue* pMsgQue, TError* pError)
{
    TState state = eFailure;
    TError error = IPC_ERR_UNREADY;
    TReg32 imask;
    TBool HiRP = eFalse;

    CpuEnterCritical(&imask);

    if (pMsgQue->Property & IPC_PROP_READY)
    {
        /* 将消息队列阻塞队列上的所有等待线程都释放，所有线程的等待结果都是TCLE_IPC_FLUSH  */
        uIpcUnblockAll(&(pMsgQue->Queue), eFailure, IPC_ERR_FLUSH, (void**)0, &HiRP);

        /*
         * 在线程环境下，如果当前线程的优先级已经不再是线程就绪队列的最高优先级，
         * 并且内核此时并没有关闭线程调度，那么就需要进行一次线程抢占
         */
        if ((uKernelVariable.State == eThreadState) &&
                (uKernelVariable.SchedLockTimes == 0U) &&
                (HiRP == eTrue))
        {
            uThreadSchedule();
        }
        state = eSuccess;
        error = IPC_ERR_NONE;
    }

    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能：消息队列广播函数,向所有读阻塞队列中的线程广播消息                                      *
 *  参数：(1) pMsgQue    消息队列结构地址                                                        *
 *        (2) pMsg2      保存消息结构地址的指针变量                                              *
 *        (3) pError     详细调用结果                                                            *
 *  返回：(1) eSuccess   成功广播发送消息                                                        *
 *        (2) eFailure   广播发送消息失败                                                        *
 *  说明：只有队列有读消息队列的时候，才能把消息发送给队列中的线程                               *
 *************************************************************************************************/
TState xMQBroadcast(TMsgQueue* pMsgQue, TMessage* pMsg2, TError* pError)
{
    TState state = eFailure;
    TError error = IPC_ERR_UNREADY;
    TReg32 imask;
    TBool HiRP = eFalse;

    CpuEnterCritical(&imask);

    if (pMsgQue->Property & IPC_PROP_READY)
    {
        /* 判断消息队列是否可用，只有消息队列空并且有线程等待读取消息的时候才能进行广播 */
        if (pMsgQue->Status == eMQEmpty)
        {
            /* 向消息队列的读阻塞队列中的线程广播数据 */
            uIpcUnblockAll(&(pMsgQue->Queue), eSuccess, IPC_ERR_NONE, (void**)pMsg2, &HiRP);

            /*
             * 在线程环境下，如果当前线程的优先级已经不再是线程就绪队列的最高优先级，
             * 并且内核此时并没有关闭线程调度，那么就需要进行一次线程抢占
             */
            if ((uKernelVariable.State == eThreadState) &&
                    (uKernelVariable.SchedLockTimes == 0U) &&
                    (HiRP == eTrue))
            {
                uThreadSchedule();
            }
            error = IPC_ERR_NONE;
            state = eSuccess;
        }
        else
        {
            error = IPC_ERR_NORMAL;
        }
    }

    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}


#endif

