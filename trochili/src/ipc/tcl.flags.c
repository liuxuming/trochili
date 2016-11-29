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
#include "tcl.flags.h"

#if ((TCLC_IPC_ENABLE) && (TCLC_IPC_FLAGS_ENABLE))

/*************************************************************************************************
 *  功能：尝试接收事件标记                                                                       *
 *  参数：(1) pFlags   事件标记的地址                                                            *
 *        (2) pPattern 需要接收的标记的组合                                                      *
 *        (3) option   访问事件标记的参数                                                        *
 *        (4) pError   详细调用结果                                                              *
 *  返回: (1) eFailure 操作失败                                                                  *
 *        (2) eSuccess 操作成功                                                                  *
 *  说明：                                                                                       *
 *************************************************************************************************/
static TState ReceiveFlags(TFlags* pFlags, TBitMask* pPattern, TOption option, TError* pError)
{
    TState state = eFailure;
    TError error = OS_IPC_ERR_NORMAL;
    TBitMask match;
    TBitMask pattern;

    pattern = *pPattern;
    match = (pFlags->Value) & pattern;
    if (((option & OS_IPC_OPT_AND) && (match == pattern)) ||
            ((option & OS_IPC_OPT_OR) && (match != 0U)))
    {
        if (option & OS_IPC_OPT_CONSUME)
        {
            pFlags->Value &= (~match);
        }

        *pPattern = match;

        error = OS_IPC_ERR_NONE;
        state = eSuccess;
    }

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能：尝试发送事件标记                                                                       *
 *  参数：(1) pFlags   事件标记的地址                                                            *
 *        (2) pPattern 需要发送的标记的组合                                                      *
 *        (3) pHiRP    是否在函数中唤醒过其它线程                                                *
 *        (4) pError   详细调用结果                                                              *
 *  返回: (1) eFailure 操作失败                                                                  *
 *        (2) eSuccess 操作成功                                                                  *
 *  说明：                                                                                       *
 *************************************************************************************************/
static TState SendFlags(TFlags* pFlags, TBitMask pattern, TBool* pHiRP, TError* pError)
{
    TState state = eError;
    TError error = OS_IPC_ERR_NORMAL;
    TLinkNode* pHead;
    TLinkNode* pTail;
    TLinkNode* pCurrent;
    TOption   option;
    TBitMask  mask;
    TBitMask* pTemp;
    TIpcContext* pContext;

    /* 检查事件是否需要发送 */
    mask = pFlags->Value | pattern;
    if (mask != pFlags->Value)
    {
        error = OS_IPC_ERR_NONE;
        state = eSuccess;

        /* 把事件发送到事件标记中 */
        pFlags->Value |= pattern;

        /* 事件标记是否有线程在等待事件的发生 */
        if (pFlags->Property & OS_IPC_PROP_PRIMQ_AVAIL)
        {
            /* 开始遍历事件组阻塞队列 */
            pHead = pFlags->Queue.PrimaryHandle;
            pTail = pFlags->Queue.PrimaryHandle->Prev;
            do
            {
                pCurrent = pHead;
                pHead = pHead->Next;

                /* 获得等待事件标记的线程和相关的事件节点 */
                pContext =  (TIpcContext*)(pCurrent->Owner);
                option = pContext->Option;
                pTemp = (TBitMask*)(pContext->Data.Addr1);

                /* 得到满足要求的事件标记 */
                mask = pFlags->Value & (*pTemp);
                if (((option & OS_IPC_OPT_AND) && (mask == *pTemp)) ||
                        ((option & OS_IPC_OPT_OR) && (mask != 0U)))
                {
                    *pTemp = mask;
                    OsIpcUnblockThread(pContext, eSuccess, OS_IPC_ERR_NONE, pHiRP);

                    /* 消耗某些事件，如果事件全部被消耗殆尽，则退出 */
                    if (option & OS_IPC_OPT_CONSUME)
                    {
                        pFlags->Value &= (~mask);
                        if (pFlags->Value == 0U)
                        {
                            break;
                        }
                    }
                }
            }
            while(pCurrent != pTail);
        }
    }

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能：线程/ISR接收事件标记                                                                   *
 *  参数：(1) pFlags   事件标记的地址                                                            *
 *        (2) pPattern 需要接收的标记的组合                                                      *
 *        (3) timeo    时限阻塞模式下访问事件标记的时限长度                                      *
 *        (4) option   访问事件标记的参数                                                        *
 *        (5) pError   详细调用结果                                                              *
 *  返回: (1) eFailure 操作失败                                                                  *
 *        (2) eSuccess 操作成功                                                                  *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclReceiveFlags(TFlags* pFlags, TBitMask* pPattern, TOption option, TTimeTick timeo,
                       TError* pError)
{
    TState state = eFailure;
    TError error = OS_IPC_ERR_UNREADY;
    TIpcContext context;
    TReg32 imask;

    OS_ASSERT((pFlags != (TFlags*)0), "");
    OS_ASSERT((option & (OS_IPC_OPT_AND | OS_IPC_OPT_OR)) != 0U, "");
    OS_ASSERT((pError != (TError*)0), "");

    /* 调整操作选项，屏蔽不需要支持的选项 */
    option &= OS_USER_FLAG_OPTION;

    OsCpuEnterCritical(&imask);

    if (pFlags->Property & OS_IPC_PROP_READY)
    {
        /*
         * 如果是中断程序调用本函数则只能以非阻塞方式获得事件标记,
         * 并且暂时不考虑线程调度问题。
         * 在中断中,当前线程未必是最高就绪优先级线程,也未必处于内核就绪线程队列，
         * 所以在此处得到的HiRP标记无任何意义。
         */
        state = ReceiveFlags(pFlags, pPattern, option, &error);

        /*
         * 因为事件标记线程队列中不会存在事件发送队列，所以不需要判断是否有新线程要调度，
         * 但是要处理是否需要将事件消耗的问题
         */
        if ((OsKernelVariable.State == OsThreadState) &&
                (OsKernelVariable.SchedLockTimes == 0U))
        {
            /*
             * 如果当前线程不能得到事件，并且采用的是等待方式，
             * 那么当前线程必须阻塞在事件标记的等待队列中，并且强制线程调度
             */
            if (state == eFailure)
            {
                if (option & OS_IPC_OPT_WAIT)
                {
                    /* 如果当前线程不能被阻塞则函数直接返回 */
                    if (OsKernelVariable.CurrentThread->ACAPI & OS_THREAD_ACAPI_BLOCK)
                    {
                        /* 保存线程挂起信息 */
                        OsIpcInitContext(&context, (void*)pFlags, (TBase32)pPattern, sizeof(TBase32),
                                         option | OS_IPC_OPT_FLAGS, timeo, &state, &error);

                        /* 当前线程阻塞在该事件标记的阻塞队列，时限或者无限等待，由OS_IPC_OPT_TIMEO参数决定 */
                        OsIpcBlockThread(&context, &(pFlags->Queue));

                        /* 当前线程被阻塞，其它线程得以执行 */
                        OsThreadSchedule();

                        OsCpuLeaveCritical(imask);
                        /*
                         * 因为当前线程已经阻塞在IPC对象的线程阻塞队列，所以处理器需要执行别的线程。
                         * 当处理器再次处理本线程时，从本处继续运行。
                         */
                        OsCpuEnterCritical(&imask);

                        /* 清除线程IPC阻塞信息 */
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
 *  功能：线程/ISR向事件标记发送事件                                                             *
 *  参数：(1) pFlags   事件标记的地址                                                            *
 *        (2) pPattern 需要接收的标记的组合                                                      *
 *        (3) pError   详细调用结果                                                              *
 *  返回: (1) eFailure   操作失败                                                                *
 *        (2) eSuccess   操作成功                                                                *
 *  说明：本函数不会引起当前线程阻塞,所以不区分是线程还是ISR来调用                               *
 *************************************************************************************************/
TState TclSendFlags(TFlags* pFlags, TBitMask pattern, TError* pError)
{
    TState state = eFailure;
    TError error = OS_IPC_ERR_UNREADY;
    TBool HiRP = eFalse;
    TReg32 imask;

    OS_ASSERT((pFlags != (TFlags*)0), "");
    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    if (pFlags->Property & OS_IPC_PROP_READY)
    {
        /*
         * 如果是中断程序调用本函数则只能以非阻塞方式发送事件,
         * 并且暂时不考虑线程调度问题。
         * 在中断中,当前线程未必是最高就绪优先级线程,也未必处于内核就绪线程队列，
         * 所以在此处得到的HiRP标记无任何意义。
        */
        state = SendFlags(pFlags, pattern, &HiRP, &error);
        /*
         * 如果在ISR环境下则直接返回。
         * 只有是线程环境下并且允许线程调度才可继续操作
         */
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
        }
    }

    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能：初始化事件标记                                                                         *
 *  参数：(1) pFlags     事件标记的地址                                                          *
 *        (2) pName      事件标记的名称                                                          *
 *        (3) property   事件标记的初始属性                                                      *
 *        (4) pError     函数调用详细返回值                                                      *
 *  返回: (1) eFailure   操作失败                                                                *
 *        (2) eSuccess   操作成功                                                                *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclCreateFlags(TFlags* pFlags, TChar* pName, TProperty property, TError* pError)
{
    TState state = eFailure;
    TError error = OS_IPC_ERR_FAULT;
    TReg32 imask;

    OS_ASSERT((pFlags != (TFlags*)0), "");
    OS_ASSERT((pName != (TChar*)0), "");
    OS_ASSERT((pError != (TError*)0), "");

    property &= OS_USER_FLAG_PROP;

    OsCpuEnterCritical(&imask);

    if (!(pFlags->Property & OS_IPC_PROP_READY))
    {
        /* 初始化事件标记对象信息 */
        OsKernelAddObject(&(pFlags->Object), pName, OsFlagObject, (void*)pFlags);

        /* 初始化事件标记基本信息 */
        property |= OS_IPC_PROP_READY;
        pFlags->Property = property;
        pFlags->Value = 0U;

        pFlags->Queue.PrimaryHandle   = (TLinkNode*)0;
        pFlags->Queue.AuxiliaryHandle = (TLinkNode*)0;
        pFlags->Queue.Property        = &(pFlags->Property);

        state = eSuccess;
        error = OS_IPC_ERR_NONE;
    }

    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能：取消事件标记初始化                                                                     *
 *  参数：(1) pFlags   事件标记的地址                                                            *
 *        (2) pError   函数调用详细返回值                                                        *
 *  返回: (1) eFailure 操作失败                                                                  *
 *        (2) eSuccess 操作成功                                                                  *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclDeleteFlags(TFlags* pFlags, TError* pError)
{
    TState state = eFailure;
    TError error = OS_IPC_ERR_UNREADY;
    TReg32 imask;
    TBool HiRP = eFalse;

    OS_ASSERT((pFlags != (TFlags*)0), "");
    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    if (pFlags->Property & OS_IPC_PROP_READY)
    {
        /* 将阻塞队列上的所有等待线程都释放，所有线程的等待结果都是OS_IPC_ERR_DELETE  */
        OsIpcUnblockAll(&(pFlags->Queue), eFailure, OS_IPC_ERR_DELETE, (void**)0, &HiRP);

        /* 从内核中移除事件标记对象 */
        OsKernelRemoveObject(&(pFlags->Object));

        /* 清除事件标记对象的全部数据 */
        memset(pFlags, 0U, sizeof(TFlags));

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
 *  功能: 清除事件标记阻塞队列                                                                   *
 *  参数：(1) pFlags   事件标记的地址                                                            *
 *        (2) pError   详细调用结果                                                              *
 *  返回: (1) eFailure   操作失败                                                                *
 *        (2) eSuccess   操作成功                                                                *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclResetFlags(TFlags* pFlags, TError* pError)
{
    TState state = eFailure;
    TError error = OS_IPC_ERR_UNREADY;
    TReg32 imask;
    TBool HiRP = eFalse;

    OS_ASSERT((pFlags != (TFlags*)0), "");
    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    if (pFlags->Property & OS_IPC_PROP_READY)
    {
        /* 将阻塞队列上的所有等待线程都释放，所有线程的等待结果都是OS_IPC_ERR_RESET */
        OsIpcUnblockAll(&(pFlags->Queue), eFailure, OS_IPC_ERR_RESET, (void**)0, &HiRP);

        pFlags->Property &= OS_RESET_FLAG_PROP;
        pFlags->Value = 0U;

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
 *  功能：事件标记终止函数,将指定的线程从事件标记的线程阻塞队列中终止阻塞并唤醒                  *
 *  参数：(1) pFlags   事件标记结构地址                                                          *
 *        (2) option   参数选项                                                                  *
 *        (3) pThread  线程地址                                                                  *
 *        (4) pError   详细调用结果                                                              *
 *  返回：(1) eSuccess                                                                           *
 *        (2) eFailure                                                                           *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclFlushFlags(TFlags* pFlags, TError* pError)
{
    TState state = eFailure;
    TError error = OS_IPC_ERR_UNREADY;
    TReg32 imask;
    TBool HiRP = eFalse;

    OS_ASSERT((pFlags != (TFlags*)0), "");
    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    if (pFlags->Property & OS_IPC_PROP_READY)
    {
        /* 将事件标记阻塞队列上的所有等待线程都释放，所有线程的等待结果都是TCLE_IPC_FLUSH */
        OsIpcUnblockAll(&(pFlags->Queue), eFailure, OS_IPC_ERR_FLUSH, (void**)0, &HiRP);

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

#endif

