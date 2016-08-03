/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#include <string.h>

#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.cpu.h"
#include "tcl.thread.h"
#include "tcl.debug.h"
#include "tcl.kernel.h"
#include "tcl.ipc.h"
#include "tcl.semaphore.h"

#if ((TCLC_IPC_ENABLE)&&(TCLC_IPC_SEMAPHORE_ENABLE))

/*************************************************************************************************
 *  功能: 释放计数信号量                                                                         *
 *  参数: (1) pSemaphore 计数信号量结构地址                                                      *
 *        (2) pHiRP      是否存在高就绪优先级标记                                                *
 *        (3) pError     详细调用结果                                                            *
 *  返回: (1) eSuccess   操作成功                                                                *
 *        (2) eFailure   操作失败                                                                *
 *  说明：当信号量可以释放的时候，如果在信号量的阻塞队列中存在线程，那么说明信号量的阻塞队列是   *
 *        Obtain队列,需要从信号量的阻塞队列中找到一个合适的线程，并且直接使得它成功获得信号量,   *
 *        同时保持信号量的计数不变                                                               *
 *************************************************************************************************/
static TState ReleaseSemaphore(TSemaphore* pSemaphore, TBool* pHiRP, TError* pError)
{
    TState state = eSuccess;
    TError error = IPC_ERR_NONE;
    TIpcContext* pContext;

    if (pSemaphore->Value == pSemaphore->LimitedValue)
    {
        state = eFailure;
        error = IPC_ERR_INVALID_VALUE;
    }
    else if (pSemaphore->Value == 0U)
    {
        /*
        * 尝试从信号量的阻塞队列中找到一个合适的线程并唤醒,保持信号量计数不变，
        * 如果被唤醒的线程的优先级高于当前线程优先级则设置线程调度请求标记
        */
        if (pSemaphore->Property & IPC_PROP_PRIMQ_AVAIL)
        {
            pContext = (TIpcContext*)(pSemaphore->Queue.PrimaryHandle->Owner);
            uIpcUnblockThread(pContext, eSuccess, IPC_ERR_NONE, pHiRP);
        }
        else
        {
            /* 如果没有找到合适的线程，则信号量计数加1 */
            pSemaphore->Value++;
        }
    }
    else
    {
        /* 信号量计数直接加1 */
        pSemaphore->Value++;
    }

    *pError = error;
    return state;
}

/*************************************************************************************************
 *  功能: 尝试获得计数信号量                                                                     *
 *  参数: (1) pSemaphore 计数信号量结构地址                                                      *
 *        (2) pHiRP     是否因唤醒更高优先级而导致需要进行线程调度的标记                        *
 *        (3) pError     详细调用结果                                                            *
 *  返回: (1) eSuccess   操作成功                                                                *
 *        (2) eFailure   操作失败                                                                *
 *  说明：当信号量可以申请的时候，如果在信号量的阻塞队列中存在线程，那么说明信号量的阻塞队列是   *
 *        Release队列,需要从信号量的阻塞队列中找到一个合适的线程，并且直接使得它释放信号量成功,  *
 *        同时保持信号量的计数不变                                                               *
 *************************************************************************************************/
static TState ObtainSemaphore(TSemaphore* pSemaphore, TBool* pHiRP, TError* pError)
{
    TState state = eSuccess;
    TError error = IPC_ERR_NONE;
    TIpcContext* pContext;

    if (pSemaphore->Value == 0U)
    {
        error = IPC_ERR_INVALID_VALUE;
        state = eFailure;
    }
    else if (pSemaphore->Value == pSemaphore->LimitedValue)
    {
        /*
         * 尝试从信号量的阻塞队列中找到一个合适的线程并唤醒,保持信号量计数不变，
         * 如果被唤醒的线程的优先级高于当前线程优先级则设置线程调度请求标记
         */
        if (pSemaphore->Property & IPC_PROP_PRIMQ_AVAIL)
        {
            pContext = (TIpcContext*)(pSemaphore->Queue.PrimaryHandle->Owner);
            uIpcUnblockThread(pContext, eSuccess, IPC_ERR_NONE, pHiRP);
        }
        else
        {
            /* 如果没有找到合适的线程，则信号量计数减1 */
            pSemaphore->Value--;
        }
    }
    else
    {
        /* 信号量计数直接减1 */
        pSemaphore->Value--;
    }

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能: 线程/ISR 获得信号量获得互斥信号量                                                      *
 *  参数: (1) pSemaphore 信号量结构地址                                                          *
 *        (2) option     访问信号量的的模式                                                      *
 *        (3) timeo      时限阻塞模式下访问信号量的时限长度                                      *
 *        (5) pError     详细调用结果                                                            *
 *  返回: (1) eSuccess   操作成功                                                                *
 *        (2) eFailure   操作失败                                                                *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState xSemaphoreObtain(TSemaphore* pSemaphore, TOption option, TTimeTick timeo, TError* pError)
{
    TState state = eFailure;
    TError error = IPC_ERR_UNREADY;
    TBool HiRP = eFalse;
    TIpcContext* pContext;
    TReg32 imask;

    CpuEnterCritical(&imask);

    if (pSemaphore->Property & IPC_PROP_READY)
    {
        state = ObtainSemaphore(pSemaphore, &HiRP, &error);

        /* 如果没有声明不需要调度则进入线程调度处理流程 */
        if (!(option & IPC_OPT_NO_SCHED))
        {
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
                     * 如果当前线程不能得到信号量，并且采用的是等待方式，
                     * 那么当前线程必须阻塞在信号量队列中
                     */
                    if (option & IPC_OPT_WAIT)
                    {
                        /* 得到当前线程的IPC上下文结构地址 */
                        pContext = &(uKernelVariable.CurrentThread->IpcContext);

                        /* 设定线程正在等待的资源的信息 */
                        uIpcSaveContext(pContext, (void*)pSemaphore, 0U, 0U,
                                        option | IPC_OPT_SEMAPHORE, &state, &error);

                        /* 当前线程阻塞在该信号量的阻塞队列，时限或者无限等待，由IPC_OPT_TIMED参数决定 */
                        uIpcBlockThread(pContext, &(pSemaphore->Queue), timeo);

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
                }
            }
        }
    }
    CpuLeaveCritical(imask);

    * pError = error;
    return state;
}


/*************************************************************************************************
 *  功能: 线程/ISR尝试释放信号量                                                                 *
 *  参数: (1) pSemaphore 信号量结构地址                                                          *
 *        (2) option     线程释放信号量的方式                                                    *
 *        (3) timeo      线程释放信号量的时限                                                    *
 *        (4) pError     详细调用结果                                                            *
 *  返回: (1) eSuccess   操作成功                                                                *
 *        (2) eFailure   操作失败                                                                *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState xSemaphoreRelease(TSemaphore* pSemaphore, TOption option, TTimeTick timeo, TError* pError)
{
    TState state = eFailure;
    TError error = IPC_ERR_UNREADY;
    TBool HiRP = eFalse;
    TIpcContext* pContext;

    TReg32 imask;

    CpuEnterCritical(&imask);
    if (pSemaphore->Property & IPC_PROP_READY)
    {
        /*
         * 如果是中断程序调用本函数则只能以非阻塞方式释放信号量,
         * 并且暂时不考虑线程调度问题。
         * 在中断中,当前线程未必是最高就绪优先级线程,也未必处于内核就绪线程队列，
         * 所以在此处得到的HiRP标记无任何意义。
         */
        state = ReleaseSemaphore(pSemaphore, &HiRP, &error);

        /* 如果没有声明不需要调度则进入线程调度处理流程 */
        if (!(option & IPC_OPT_NO_SCHED))
        {
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
                     * 如果当前线程不能释放信号量，并且采用的是等待方式，
                     * 那么当前线程必须阻塞在信号量队列中
                     */
                    if (option & IPC_OPT_WAIT)
                    {
                        /* 得到当前线程的IPC上下文结构地址 */
                        pContext = &(uKernelVariable.CurrentThread->IpcContext);

                        /* 设定线程正在等待的资源的信息 */
                        uIpcSaveContext(pContext, (void*)pSemaphore, 0U, 0U,
                                        option | IPC_OPT_SEMAPHORE, &state, &error);

                        /* 当前线程阻塞在该信号量的阻塞队列，时限或者无限等待，由IPC_OPT_TIMED参数决定 */
                        uIpcBlockThread(pContext, &(pSemaphore->Queue), timeo);

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
                }
            }
        }
    }
    CpuLeaveCritical(imask);

    * pError = error;
    return state;
}


/*************************************************************************************************
 *  功能: 初始化计数信号量                                                                       *
 *  参数: (1) pSemaphore 计数信号量结构地址                                                      *
 *        (2) value      计数信号量初始值                                                        *
 *        (3) mvalue     计数信号量最大计数值                                                    *
 *        (4) property   信号量的初始属性                                                        *
 *        (5) pError     详细调用结果                                                            *
 *  返回: (1) eSuccess   操作成功                                                                *
 *        (2) eFailure   操作失败                                                                *
 *  说明：信号量只使用基本IPC队列                                                                *
 *************************************************************************************************/
TState xSemaphoreCreate(TSemaphore* pSemaphore, TBase32 value, TBase32 mvalue,
                        TProperty property, TError* pError)
{
    TState state = eFailure;
    TError error = IPC_ERR_FAULT;
    TReg32 imask;

    CpuEnterCritical(&imask);

    if (!(pSemaphore->Property & IPC_PROP_READY))
    {
        property |= IPC_PROP_READY;
        pSemaphore->Property     = property;
        pSemaphore->Value        = value;
        pSemaphore->LimitedValue = mvalue;
        pSemaphore->InitialValue = value;
        pSemaphore->Queue.PrimaryHandle   = (TObjNode*)0;
        pSemaphore->Queue.AuxiliaryHandle = (TObjNode*)0;
        pSemaphore->Queue.Property        = &(pSemaphore->Property);

        error = IPC_ERR_NONE;
        state = eSuccess;
    }

    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能: 信号量删除                                                                             *
 *  参数: (1) pSemaphore 信号量结构地址                                                          *
 *        (2) pError     详细调用结果                                                            *
 *  返回: (1) eSuccess   操作成功                                                                *
 *        (2) eFailure   操作失败                                                                *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState xSemaphoreDelete(TSemaphore* pSemaphore, TError* pError)
{
    TState state = eFailure;
    TError error = IPC_ERR_UNREADY;
    TReg32 imask;
    TBool HiRP = eFalse;

    CpuEnterCritical(&imask);

    if (pSemaphore->Property & IPC_PROP_READY)
    {
        /*
         * 调用下面的函数时会记录是否有优先级更高的线程就绪，但不记录具体是哪个线程
         * 将信号量阻塞队列上的所有等待线程都释放，所有线程的等待结果都是TCLE_IPC_DELETE
         */
        uIpcUnblockAll(&(pSemaphore->Queue), eFailure, IPC_ERR_DELETE, (void**)0, &HiRP);

        /* 清除信号量对象的全部数据 */
        memset(pSemaphore, 0U, sizeof(TSemaphore));

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
 *  功能: 重置计数信号量                                                                         *
 *  参数: (1) pSemaphore 信号量结构地址                                                          *
 *        (2) pError     详细调用结果                                                            *
 *  返回: (1) eSuccess   操作成功                                                                *
 *        (2) eFailure   操作失败                                                                *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState xSemaphoreReset(TSemaphore* pSemaphore, TError* pError)
{
    TState state = eFailure;
    TError error = IPC_ERR_UNREADY;
    TReg32 imask;
    TBool HiRP = eFalse;

    CpuEnterCritical(&imask);

    if (pSemaphore->Property & IPC_PROP_READY)
    {
        /*
         * 调用下面的函数时会记录是否有优先级更高的线程就绪，但不记录具体是哪个线程
         * 将信号量阻塞队列上的所有等待线程都释放，所有线程的等待结果都是TCLE_IPC_RESET
         */
        uIpcUnblockAll(&(pSemaphore->Queue), eFailure, IPC_ERR_RESET, (void**)0, &HiRP);

        /* 重置信号量计数和属性 */
        pSemaphore->Property &= IPC_RESET_SEMAPHORE_PROP;
        pSemaphore->Value = pSemaphore->InitialValue;

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
 *  功能：信号量刷新函数,将阻塞在信号量上的线程从信号量的线程阻塞队列中解除阻塞                  *
 *  参数：(1) pSemaphore 信号量结构地址                                                          *
 *        (2) pError     详细调用结果                                                            *
 *  返回: (1) eSuccess   操作成功                                                                *
 *        (2) eFailure   操作失败                                                                *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState xSemaphoreFlush(TSemaphore* pSemaphore, TError* pError)
{
    TState state = eFailure;
    TError error = IPC_ERR_UNREADY;
    TReg32 imask;
    TBool HiRP = eFalse;

    CpuEnterCritical(&imask);

    if (pSemaphore->Property & IPC_PROP_READY)
    {
        /*
         * 调用下面的函数时会记录是否有优先级更高的线程就绪，但不记录具体是哪个线程
         * 将信号量阻塞队列上的所有等待线程都释放，所有线程的等待结果都是TCLE_IPC_FLUSH
         */
        uIpcUnblockAll(&(pSemaphore->Queue), eFailure, IPC_ERR_FLUSH, (void**)0, &HiRP);

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

#endif

