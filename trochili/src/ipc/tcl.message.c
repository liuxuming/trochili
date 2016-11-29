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

/* 消息类型定义 */
enum MsgTypeDef
{
    OsNormalMessage,  /* 普通消息,放入消息队列头 */
    OsUrgentMessage   /* 紧急消息,放入消息队列尾 */
};
typedef enum MsgTypeDef TMsgType;


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
    if (type == OsNormalMessage)
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
       否则消息队列进入 OsMQPartial 状态
   (2) 处理消息队列普通的情况：如果消息队列中只有1个消息，对列读取操作可能导致消息队列进入空状态
       否则消息队列保持 OsMQPartial 状态
   (3) 无论是消息队列满还是消息队列普通状态：最终状态只有普通和空。
 */
static TState ReceiveMessage(TMsgQueue* pMsgQue, void** pMsg2, TBool* pHiRP, TError* pError)
{
    TState state = eSuccess;
    TError error = OS_IPC_ERR_NONE;
    TMsgType type;
    TIpcContext* pContext = (TIpcContext*)0;

    /* 检查消息队列状态 */
    if (pMsgQue->Status == OsMQEmpty)
    {
        error = OS_IPC_ERR_NORMAL;
        state = eFailure;
    }
    else if (pMsgQue->Status == OsMQFull)
    {
        /* 从消息队列中读取一个消息给当前线程 */
        ConsumeMessage(pMsgQue, pMsg2);

        /*
        	   * 在消息队列满并且有线程阻塞在写队列中的情况下，
         * 需要将合适的线程接触阻塞并且将该线程携带的消息写入队列，
         * 所以保持消息队列状态不变
        	   */
        if (pMsgQue->Property & OS_IPC_PROP_AUXIQ_AVAIL)
        {
            pContext = (TIpcContext*)(pMsgQue->Queue.AuxiliaryHandle->Owner);
        }
        else
        {
            if (pMsgQue->Property & OS_IPC_PROP_PRIMQ_AVAIL)
            {
                pContext = (TIpcContext*)(pMsgQue->Queue.PrimaryHandle->Owner);
            }
        }

        if (pContext !=  (TIpcContext*)0)
        {
            OsIpcUnblockThread(pContext, eSuccess, OS_IPC_ERR_NONE, pHiRP);

            /* 根据线程所处的分队列判断消息类型,并且将该线程发送的消息写入消息队列 */
            type = ((pContext->Option) & OS_IPC_OPT_UARGENT) ? OsUrgentMessage : OsNormalMessage;
            SaveMessage(pMsgQue, pContext->Data.Addr2, type);
        }
        else
        {
            pMsgQue->Status = (pMsgQue->Tail == pMsgQue->Head) ? OsMQEmpty : OsMQPartial;
        }
    }
    else
        /* if (mq->Status == OsMQPartial) */
    {
        /* 从消息队列中读取一个消息给当前线程 */
        ConsumeMessage(pMsgQue, pMsg2);
        pMsgQue->Status = (pMsgQue->Tail == pMsgQue->Head) ? OsMQEmpty : OsMQPartial;
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
    TError error = OS_IPC_ERR_NONE;
    TIpcContext* pContext = (TIpcContext*)0;

    /* 检查消息队列状态，如果消息队列满则返回失败 */
    if (pMsgQue->Status == OsMQFull)
    {
        error = OS_IPC_ERR_NORMAL;
        state = eFailure;
    }
    else if (pMsgQue->Status == OsMQEmpty)
    {
        /*
         * 在消息队列为空的情况下，如果队列中有线程等待，则说明是读阻塞队列，
         * 将其中一个线程解除阻塞，并将消息发送给该线程，同时保持消息队列状态不变
         */
        if (pMsgQue->Property & OS_IPC_PROP_PRIMQ_AVAIL)
        {
            pContext = (TIpcContext*)(pMsgQue->Queue.PrimaryHandle->Owner);
        }

        if (pContext != (TIpcContext*)0)
        {
            OsIpcUnblockThread(pContext, eSuccess, OS_IPC_ERR_NONE, pHiRP);
            *(pContext->Data.Addr2) = *pMsg2;
        }
        else
        {
            /* 将线程发送的消息写入消息队列 */
            SaveMessage(pMsgQue, pMsg2, type);
            pMsgQue->Status = (pMsgQue->Tail == pMsgQue->Head) ? OsMQFull : OsMQPartial;
        }
    }
    else
        /* if (mq->Status == OsMQPartial) */
    {
        /* 将线程发送的消息写入消息队列 */
        SaveMessage(pMsgQue, pMsg2, type);

        /*
         * 消息队列空的情况，如果是消息队列的容量是1，那么它的状态从空直接到 OsMQFull，
         * 否则消息队列进入 OsMQPartial 状态;
         * 消息队列普通的情况，消息对列写操作可能导致消息队列进入 OsMQFull
         * 状态或者保持 OsMQPartial 状态
         */
        pMsgQue->Status = (pMsgQue->Tail == pMsgQue->Head) ? OsMQFull : OsMQPartial;
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
extern TState TclReceiveMessage(TMsgQueue* pMsgQue, TMessage* pMsg2, TOption option,
                                TTimeTick timeo, TError* pError)
{
    TState state = eFailure;
    TError error = OS_IPC_ERR_UNREADY;
    TBool  HiRP = eFalse;
    TIpcContext context;
    TReg32 imask;

    OS_ASSERT((pMsgQue != (TMsgQueue*)0), "");
    OS_ASSERT((pMsg2 != (TMessage*)0), "");
    OS_ASSERT((pError != (TError*)0), "");

    /* 调整操作选项，屏蔽不需要支持的选项 */
    option &= OS_USER_MSGQ_OPTION;

    OsCpuEnterCritical(&imask);

    if (pMsgQue->Property & OS_IPC_PROP_READY)
    {
        /*
         * 如果是中断程序调用本函数则只能以非阻塞方式接收消息,
         * 并且暂时不考虑线程调度问题。
         * 在中断中,当前线程未必是最高就绪优先级线程,也未必处于内核就绪线程队列，
         * 所以在此处得到的HiRP标记无任何意义。
         */
        state = ReceiveMessage(pMsgQue, (void**)pMsg2, &HiRP, &error);

        if ((OsKernelVariable.State == OsThreadState) &&
                (OsKernelVariable.SchedLockTimes == 0U))
        {
            /* 如果当前线程解除了更高优先级线程的阻塞则进行调度。*/
            if (state == eSuccess)
            {
                if (HiRP == eTrue)
                {
                    OsThreadSchedule();
                }
            }
            else
            {
                /*
                * 如果当前线程不能接收消息，并且采用的是等待方式，
                * 那么当前线程必须阻塞在消息队列中
                */
                if (option & OS_IPC_OPT_WAIT)
                {
                    /* 如果当前线程不能被阻塞则函数直接返回 */
                    if (OsKernelVariable.CurrentThread->ACAPI & OS_THREAD_ACAPI_BLOCK)
                    {
                        /* 保存线程阻塞信息 */
                        OsIpcInitContext(&context, (void*)pMsgQue, (TBase32)pMsg2, sizeof(TBase32),
                                         option | OS_IPC_OPT_MSGQUEUE | OS_IPC_OPT_READ_DATA, timeo,
                                         &state, &error);

                        /* 当前线程阻塞在该消息队列的阻塞队列，时限或者无限等待，由OS_IPC_OPT_TIMEO参数决定 */
                        OsIpcBlockThread(&context, &(pMsgQue->Queue));

                        /* 当前线程被阻塞，其它线程得以执行 */
                        OsThreadSchedule();

                        OsCpuLeaveCritical(imask);
                        /*
                         * 因为当前线程已经阻塞在IPC对象的线程阻塞队列，所以处理器需要执行别的线程。
                         * 当处理器再次处理本线程时，从本处继续运行。
                         */
                        OsCpuEnterCritical(&imask);

                        /* 清除线程挂起信息 */
                        OsIpcCleanContext(&context);
                    }
                    else
                    {
                        error = OS_IPC_ERR_ACAPI;
                    }
                }
            }
        }
    }

    OsCpuLeaveCritical(imask);

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
TState TclSendMessage(TMsgQueue* pMsgQue, TMessage* pMsg2, TOption option, TTimeTick timeo,
                      TError* pError)
{
    TState state = eFailure;
    TError error = OS_IPC_ERR_UNREADY;
    TBool HiRP = eFalse;
    TIpcContext context;
    TMsgType type;
    TReg32 imask;

    OsCpuEnterCritical(&imask);
    if (pMsgQue->Property & OS_IPC_PROP_READY)
    {
        /*
         * 如果是中断程序调用本函数则只能以非阻塞方式发送消息,
         * 并且暂时不考虑线程调度问题。
         * 在中断中,当前线程未必是最高就绪优先级线程,也未必处于内核就绪线程队列，
         * 所以在此处得到的HiRP标记无任何意义。
         */
        type = (option & OS_IPC_OPT_UARGENT) ? OsUrgentMessage : OsNormalMessage;
        state = SendMessage(pMsgQue, (void**)pMsg2, type, &HiRP, &error);

        if ((OsKernelVariable.State == OsThreadState) &&
                (OsKernelVariable.SchedLockTimes == 0U))
        {
            /* 如果当前线程解除了更高优先级线程的阻塞则进行调度。*/
            if (state == eSuccess)
            {
                if (HiRP == eTrue)
                {
                    OsThreadSchedule();
                }
            }
            else
            {
                /*
                * 如果当前线程不能发送消息，并且采用的是等待方式，
                * 那么当前线程必须阻塞在消息队列中
                */
                if (option & OS_IPC_OPT_WAIT)
                {
                    /* 如果当前线程不能被阻塞则函数直接返回 */
                    if (OsKernelVariable.CurrentThread->ACAPI & OS_THREAD_ACAPI_BLOCK)
                    {
                        /* 保存线程挂起信息 */
                        OsIpcInitContext(&context, (void*)pMsgQue,
                                         (TBase32)pMsg2, sizeof(TBase32),
                                         option | OS_IPC_OPT_MSGQUEUE | OS_IPC_OPT_WRITE_DATA,
                                         timeo, &state, &error);
                        /*
                         * 发送紧急消息的线程进入消息队列阻塞队列的辅助队列中,
                         * 发送普通消息的线程进入消息队列阻塞队列的基本队列中,
                         * 当前线程阻塞在该消息队列的阻塞队列时，具体采用时限还是无限等待的方式，
                         * 由OS_IPC_OPT_TIMEO参数决定
                         */
                        OsIpcBlockThread(&context, &(pMsgQue->Queue));

                        /* 当前线程被阻塞，其它线程得以执行 */
                        OsThreadSchedule();

                        OsCpuLeaveCritical(imask);
                        /*
                         * 因为当前线程已经阻塞在IPC对象的线程阻塞队列，所以处理器需要执行别的线程。
                         * 当处理器再次处理本线程时，从本处继续运行。
                         */
                        OsCpuEnterCritical(&imask);

                        /* 清除线程挂起信息 */
                        OsIpcCleanContext(&context);
                    }
                    else
                    {
                        error = OS_IPC_ERR_ACAPI;
                    }
                }
            }
        }
    }
    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能: 用于ISR向消息队列中发送消息                                                            *
 *  参数: (1) pMsgQue  消息队列结构地址                                                          *
 *        (2) pMsg2    保存消息结构地址的指针变量                                                *
 *        (3) option   访问邮箱的模式                                                            *
 *        (4) pError   详细调用结果                                                              *
 *  返回: (1) eFailure 操作失败                                                                  *
 *        (2) eSuccess 操作成功                                                                  *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclIsrSendMessage(TMsgQueue* pMsgQue, TMessage* pMsg2, TOption option, TError* pError)
{
    TState state = eFailure;
    TError error = OS_IPC_ERR_UNREADY;
    TBool HiRP = eFalse;
    TMsgType type;
    TReg32 imask;

    OS_ASSERT((pMsgQue != (TMsgQueue*)0), "");
    OS_ASSERT((pMsg2 != (TMessage*)0), "");
    OS_ASSERT((pError != (TError*)0), "");

    /* 调整操作选项，屏蔽不需要支持的选项 */
    option &= OS_ISR_MSGQ_OPTION;

    OsCpuEnterCritical(&imask);
    if (pMsgQue->Property & OS_IPC_PROP_READY)
    {
        /*
         * 如果是中断程序调用本函数则只能以非阻塞方式发送消息,
         * 并且暂时不考虑线程调度问题。
         * 在中断中,当前线程未必是最高就绪优先级线程,也未必处于内核就绪线程队列，
         * 所以在此处得到的HiRP标记无任何意义。
         */
        type = (option & OS_IPC_OPT_UARGENT) ? OsUrgentMessage : OsNormalMessage;
        state = SendMessage(pMsgQue, (void**)pMsg2, type, &HiRP, &error);
    }
    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能：消息队列初始化函数                                                                     *
 *  输入：(1) pMsgQue   消息队列结构地址                                                         *
 *        (2) pName     消息队列名称                                                             *
 *        (3) pPool2    消息数组地址                                                             *
 *        (4) capacity  消息队列容量，即消息数组大小                                             *
 *        (5) policy    消息队列线程调度策略                                                     *
 *        (6) pError    详细调用结果                                                             *
 *  返回：(1) eSuccess  操作成功                                                                 *
 *        (2) eFailure  操作失败                                                                 *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclCreateMsgQueue(TMsgQueue* pMsgQue, TChar* pName, void** pPool2, TBase32 capacity,
                         TProperty property, TError* pError)
{
    TState state = eFailure;
    TError error = OS_IPC_ERR_FAULT;
    TReg32 imask;

    OS_ASSERT((pMsgQue != (TMsgQueue*)0), "");
    OS_ASSERT((pName != (TChar*)0), "");
    OS_ASSERT((pPool2 != (void*)0), "");
    OS_ASSERT((capacity != 0U), "");
    OS_ASSERT((pError != (TError*)0), "");

    property &= OS_USER_MQUE_PROP;

    OsCpuEnterCritical(&imask);

    if (!(pMsgQue->Property & OS_IPC_PROP_READY))
    {
        /* 初始化消息队列对象信息 */
        OsKernelAddObject(&(pMsgQue->Object), pName, OsMessageQueueObject, (void*)pMsgQue);

        /* 初始化消息队列基本信息 */
        property |= OS_IPC_PROP_READY;
        pMsgQue->Property = property;
        pMsgQue->Capacity = capacity;
        pMsgQue->MsgPool = pPool2;
        pMsgQue->MsgEntries = 0U;
        pMsgQue->Head = 0U;
        pMsgQue->Tail = 0U;
        pMsgQue->Status = OsMQEmpty;

        pMsgQue->Queue.PrimaryHandle   = (TLinkNode*)0;
        pMsgQue->Queue.AuxiliaryHandle = (TLinkNode*)0;
        pMsgQue->Queue.Property        = &(pMsgQue->Property);

        error = OS_IPC_ERR_NONE;
        state = eSuccess;
    }

    OsCpuLeaveCritical(imask);

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
TState TclDeleteMsgQueue(TMsgQueue* pMsgQue, TError* pError)
{
    TState state = eFailure;
    TError error = OS_IPC_ERR_UNREADY;
    TBool HiRP = eFalse;
    TReg32 imask;

    OS_ASSERT((pMsgQue != (TMsgQueue*)0), "");
    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    if (pMsgQue->Property & OS_IPC_PROP_READY)
    {
        /* 向阻塞队列中的线程分发消息 */
        OsIpcUnblockAll(&(pMsgQue->Queue), eFailure, OS_IPC_ERR_DELETE, (void**)0, &HiRP);

        /* 从内核中移除消息队列对象 */
        OsKernelRemoveObject(&(pMsgQue->Object));

        /* 清除消息队列对象的全部数据 */
        memset(pMsgQue, 0U, sizeof(TMsgQueue));

        /*
         * 在线程环境下，如果当前线程的优先级已经不再是线程就绪队列的最高优先级，
         * 并且内核此时并没有关闭线程调度，那么就需要进行一次线程抢占
         */
        if ((OsKernelVariable.State == OsThreadState) &&
                (OsKernelVariable.SchedLockTimes == 0U) &&
                (HiRP == eTrue))
        {
            OsThreadSchedule();
        }
        error = OS_IPC_ERR_NONE;
        state = eSuccess;
    }

    OsCpuLeaveCritical(imask);

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
TState TclResetMsgQueue(TMsgQueue* pMsgQue, TError* pError)
{
    TState state = eFailure;
    TError error = OS_IPC_ERR_UNREADY;
    TBool HiRP = eFalse;
    TReg32 imask;

    OS_ASSERT((pMsgQue != (TMsgQueue*)0), "");
    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    if (pMsgQue->Property & OS_IPC_PROP_READY)
    {
        /* 将阻塞队列上的所有等待线程都释放，所有线程的等待结果都是OS_IPC_ERR_RESET    */
        OsIpcUnblockAll(&(pMsgQue->Queue), eFailure, OS_IPC_ERR_RESET, (void**)0, &HiRP);

        /* 重新设置消息队列结构 */
        pMsgQue->Property &= OS_RESET_MQUE_PROP;
        pMsgQue->MsgEntries = 0U;
        pMsgQue->Head = 0U;
        pMsgQue->Tail = 0U;
        pMsgQue->Status = OsMQEmpty;

        /*
         * 在线程环境下，如果当前线程的优先级已经不再是线程就绪队列的最高优先级，
         * 并且内核此时并没有关闭线程调度，那么就需要进行一次线程抢占
         */
        if ((OsKernelVariable.State == OsThreadState) &&
                (OsKernelVariable.SchedLockTimes == 0U) &&
                (HiRP == eTrue))
        {
            OsThreadSchedule();
        }
        error = OS_IPC_ERR_NONE;
        state = eSuccess;
    }

    OsCpuLeaveCritical(imask);

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
TState TclFlushMsgQueue(TMsgQueue* pMsgQue, TError* pError)
{
    TState state = eFailure;
    TError error = OS_IPC_ERR_UNREADY;
    TReg32 imask;
    TBool HiRP = eFalse;

    OS_ASSERT((pMsgQue != (TMsgQueue*)0), "");
    OS_ASSERT((pError != (TError*)0), "");


    OsCpuEnterCritical(&imask);

    if (pMsgQue->Property & OS_IPC_PROP_READY)
    {
        /* 将消息队列阻塞队列上的所有等待线程都释放，所有线程的等待结果都是OS_IPC_ERR_FLUSH  */
        OsIpcUnblockAll(&(pMsgQue->Queue), eFailure, OS_IPC_ERR_FLUSH, (void**)0, &HiRP);

        /*
         * 在线程环境下，如果当前线程的优先级已经不再是线程就绪队列的最高优先级，
         * 并且内核此时并没有关闭线程调度，那么就需要进行一次线程抢占
         */
        if ((OsKernelVariable.State == OsThreadState) &&
                (OsKernelVariable.SchedLockTimes == 0U) &&
                (HiRP == eTrue))
        {
            OsThreadSchedule();
        }
        state = eSuccess;
        error = OS_IPC_ERR_NONE;
    }

    OsCpuLeaveCritical(imask);

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
TState TclBroadcastMessage(TMsgQueue* pMsgQue, TMessage* pMsg2, TError* pError)
{
    TState state = eFailure;
    TError error = OS_IPC_ERR_UNREADY;
    TReg32 imask;
    TBool HiRP = eFalse;

    OS_ASSERT((pMsgQue != (TMsgQueue*)0), "");
    OS_ASSERT((pMsg2 != (TMessage*)0), "");
    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    if (pMsgQue->Property & OS_IPC_PROP_READY)
    {
        /* 判断消息队列是否可用，只有消息队列空并且有线程等待读取消息的时候才能进行广播 */
        if (pMsgQue->Status == OsMQEmpty)
        {
            /* 向消息队列的读阻塞队列中的线程广播数据 */
            OsIpcUnblockAll(&(pMsgQue->Queue), eSuccess, OS_IPC_ERR_NONE, (void**)pMsg2, &HiRP);

            /*
             * 在线程环境下，如果当前线程的优先级已经不再是线程就绪队列的最高优先级，
             * 并且内核此时并没有关闭线程调度，那么就需要进行一次线程抢占
             */
            if ((OsKernelVariable.State == OsThreadState) &&
                    (OsKernelVariable.SchedLockTimes == 0U) &&
                    (HiRP == eTrue))
            {
                OsThreadSchedule();
            }
            error = OS_IPC_ERR_NONE;
            state = eSuccess;
        }
        else
        {
            error = OS_IPC_ERR_NORMAL;
        }
    }

    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}

#endif

