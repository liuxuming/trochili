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
#include "tcl.mailbox.h"

#if ((TCLC_IPC_ENABLE)&&(TCLC_IPC_MAILBOX_ENABLE))

/*************************************************************************************************
 *  功能: 尝试读取邮箱中的邮件                                                                   *
 *  参数: (1) pMailbox 邮箱结构地址                                                              *
 *        (2) pMail2   保存邮件结构地址的指针变量                                                *
 *        (3) pHiRP   是否在函数中唤醒过其它线程                                                 *
 *        (4) pError   详细调用结果                                                              *
 *  返回: (1) eFailure 操作失败                                                                  *
 *        (2) eSuccess 操作成功                                                                  *
 *  说明：在邮箱可以读取的时候,如果有线程处于邮箱的阻塞队列,说明是当前邮箱的线程阻塞队列是邮件   *
 *        发送队列, 这时需要从邮箱的阻塞队列中找到一个合适的线程,直接使得发送邮件成功。并且要    *
 *        保持邮箱的状态不变                                                                     *
 *************************************************************************************************/
static TState ReceiveMail(TMailBox* pMailbox, void** pMail2, TBool* pHiRP, TError* pError)
{
    TState state = eSuccess;
    TError error = IPC_ERR_NONE;
    TIpcContext* pContext = (TIpcContext*)0;

    if (pMailbox->Status == eMailBoxFull)
    {
        /* 从邮箱中读取邮件 */
        *pMail2 = pMailbox->Mail;

        /*
         * 如果此时在邮箱的线程阻塞队列中有线程存在,即该线程等待发送邮件,
         * 则从中将一个合适的线程解除阻塞。此时紧急邮件优先
         */
        if (pMailbox->Property &IPC_PROP_AUXIQ_AVAIL)
        {
            pContext = (TIpcContext*)(pMailbox->Queue.AuxiliaryHandle->Owner);
        }
        else
        {
            if (pMailbox->Property &IPC_PROP_PRIMQ_AVAIL)
            {
                pContext = (TIpcContext*)(pMailbox->Queue.PrimaryHandle->Owner);
            }
        }

        /* 如果有线程被解除阻塞,则将该线程待发送的邮件保存到邮箱中, 保持邮箱状态不变 */
        if (pContext != (TIpcContext*)0)
        {
            uIpcUnblockThread(pContext, eSuccess, IPC_ERR_NONE, pHiRP);
            pMailbox->Mail = *((TMail*)(pContext->Data.Addr2));
        }
        /* 否则读空邮箱,设置邮件为空,设置邮箱状态为空 */
        else
        {
            pMailbox->Mail = (void*)0;
            pMailbox->Status = eMailBoxEmpty;
        }
    }
    else
    {
        error = IPC_ERR_NORMAL;
        state = eFailure;
    }

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能: 尝试向邮箱发送邮件                                                                     *
 *  参数: (1) pMailbox 邮箱结构地址                                                              *
 *        (2) pMail2   保存邮件结构地址的指针变量                                                *
 *        (3) pHiRP    是否在函数中唤醒过其它线程                                                *
 *        (4) pError   详细调用结果                                                              *
 *  返回: (1) eFailure 操作失败                                                                  *
 *        (2) eSuccess 操作成功                                                                  *
 *  说明：                                                                                       *
 *************************************************************************************************/
static TState SendMail(TMailBox* pMailbox, void** pMail2, TBool* pHiRP, TError* pError)
{
    TState state = eSuccess;
    TError error = IPC_ERR_NONE;
    TIpcContext* pContext = (TIpcContext*)0;

    if (pMailbox->Status == eMailBoxEmpty)
    {
        /*
         * 当邮箱为空的时候,如果有线程处于邮箱的阻塞队列,说明是当前邮箱的的线程阻塞队列是
         * 邮件读取队列, 这时需要从邮箱的阻塞队列中找到一个合适的线程,并直接使得它读取邮件成功。
         * 同时邮箱的状态不变
         */
        if (pMailbox->Property &IPC_PROP_PRIMQ_AVAIL)
        {
            pContext = (TIpcContext*)(pMailbox->Queue.PrimaryHandle->Owner);
        }

        /* 如果找到了一个合适的线程,就将邮件发送给它 */
        if (pContext != (TIpcContext*)0)
        {
            uIpcUnblockThread(pContext, eSuccess, IPC_ERR_NONE, pHiRP);
            *(pContext->Data.Addr2) = * pMail2;
        }
        else
        {
            /* 否则将邮件写入邮箱,并置邮箱状态为满 */
            pMailbox->Mail = * pMail2;
            pMailbox->Status = eMailBoxFull;
        }
    }
    else
    {
        /* 邮箱内已经有邮件了，不能再放入其他邮件 */
        error = IPC_ERR_NORMAL;
        state = eFailure;
    }

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能: 线程/ISR从邮箱中读取邮件                                                               *
 *  参数: (1) pMailbox 邮箱结构地址                                                              *
 *        (2) pMail2   保存邮件结构地址的指针变量                                                *
 *        (3) option   访问邮箱的模式                                                            *
 *        (4) timeo    时限阻塞模式下访问邮箱的时限长度                                          *
 *        (5) pError   详细调用结果                                                              *
 *  返回: (1) eFailure 操作失败                                                                  *
 *        (2) eSuccess 操作成功                                                                  *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState xMailBoxReceive(TMailBox* pMailbox, TMail* pMail2, TOption option, TTimeTick timeo,
                       TError* pError)
{
    TState state = eFailure;
    TError error = IPC_ERR_UNREADY;
    TBool HiRP = eFalse;
    TIpcContext* pContext;
    TReg32 imask;

    CpuEnterCritical(&imask);
    if (pMailbox->Property &IPC_PROP_READY)
    {
        /*
         * 如果是中断程序调用本函数则只能以非阻塞方式从邮箱中读取邮件,
         * 并且暂时不考虑线程调度问题。
         * 在中断中,当前线程未必是最高就绪优先级线程,也未必处于内核就绪线程队列，
         * 所以在此处得到的HiRP标记无任何意义。
         */
        state = ReceiveMail(pMailbox, (void**)pMail2, &HiRP, &error);

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
                * 如果当前线程不能得到邮件,并且采用的是等待方式,并且内核没有关闭线程调度,
                * 那么当前线程必须阻塞在邮箱队列中,并且强制线程调度
                */
                if (option & IPC_OPT_WAIT)
                {
                    /* 如果当前线程不能被阻塞则函数直接返回 */
                    if (uKernelVariable.CurrentThread->ACAPI & THREAD_ACAPI_BLOCK)
                    {
                        /* 得到当前线程的IPC上下文结构地址 */
                        pContext = &(uKernelVariable.CurrentThread->IpcContext);

                        /* 保存线程挂起信息 */
                        uIpcSaveContext(pContext, (void*)pMailbox, (TBase32)pMail2,
                                        sizeof(TBase32), option | IPC_OPT_MAILBOX | IPC_OPT_READ_DATA,
                                        &state, &error);

                        /* 当前线程阻塞在该邮箱的阻塞队列，时限或者无限等待，由IPC_OPT_TIMED参数决定 */
                        uIpcBlockThread(pContext, &(pMailbox->Queue), timeo);

                        /* 当前线程被阻塞，其它线程得以执行 */
                        uThreadSchedule();

                        CpuLeaveCritical(imask);
                        /*
                         * 因为当前线程已经阻塞在邮箱的线程阻塞队列，所以处理器需要执行别的线程。
                         * 当处理器再次处理本线程时，从本处继续运行。
                         */
                        CpuEnterCritical(&imask);

                        /* 清除线程IPC阻塞信息 */
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
 *  功能: 线程/ISR向邮箱中发送邮件                                                               *
 *  参数: (1) pMailbox 邮箱结构地址                                                              *
 *        (2) pMail2   保存邮件结构地址的指针变量                                                *
 *        (3) option   访问邮箱的模式                                                            *
 *        (4) timeo    时限阻塞模式下访问邮箱的时限长度                                          *
 *        (5) pError   详细调用结果                                                              *
 *  返回: (1) eFailure 操作失败                                                                  *
 *        (2) eSuccess 操作成功                                                                  *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState xMailBoxSend(TMailBox* pMailbox, TMail* pMail2, TOption option, TTimeTick timeo,
                    TError* pError)
{
    TState state = eFailure;
    TError error = IPC_ERR_UNREADY;
    TBool HiRP = eFalse;
    TIpcContext* pContext;
    TReg32 imask;

    CpuEnterCritical(&imask);
    if (pMailbox->Property &IPC_PROP_READY)
    {
        /*
         * 如果是中断程序调用本函数则只能以非阻塞方式向邮箱中发送邮件,
         * 并且暂时不考虑线程调度问题。
         * 在中断中,当前线程未必是最高就绪优先级线程,也未必处于内核就绪线程队列，
         * 所以在此处得到的HiRP标记无任何意义。
         */
        state = SendMail(pMailbox, (void**)pMail2, &HiRP, &error);
		
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
                * 如果当前线程不能发送邮件,并且采用的是等待方式,并且内核没有关闭线程调度,
                * 那么当前线程必须阻塞在邮箱队列中,并且强制线程调度
                */
                if (option & IPC_OPT_WAIT)
                {
                    /* 如果当前线程不能被阻塞则函数直接返回 */
                    if (uKernelVariable.CurrentThread->ACAPI & THREAD_ACAPI_BLOCK)
                    {
                        /* 发送紧急程度不同的邮件的线程进入不同的阻塞队列 */
                        if (option &IPC_OPT_UARGENT)
                        {
                            option |= IPC_OPT_USE_AUXIQ;
                        }

                        /* 得到当前线程的IPC上下文结构地址 */
                        pContext = &(uKernelVariable.CurrentThread->IpcContext);

                        /* 保存线程挂起信息 */
                        uIpcSaveContext(pContext, (void*)pMailbox, (TBase32)pMail2, sizeof(TBase32),
                                        option | IPC_OPT_MAILBOX | IPC_OPT_WRITE_DATA, &state, &error);

                        /* 当前线程阻塞在该邮箱的阻塞队列，时限或者无限等待，由IPC_OPT_TIMED参数决定 */
                        uIpcBlockThread(pContext, &(pMailbox->Queue), timeo);

                        /* 当前线程被阻塞，其它线程得以执行 */
                        uThreadSchedule();

                        CpuLeaveCritical(imask);
                        /*
                         * 因为当前线程已经阻塞在邮箱的线程阻塞队列，所以处理器需要执行别的线程。
                         * 当处理器再次处理本线程时，从本处继续运行。
                         */
                        CpuEnterCritical(&imask);

                        /* 清除线程IPC阻塞信息 */
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
 *  功能：初始化邮箱                                                                             *
 *  参数：(1) pMailbox   邮箱的地址                                                              *
 *        (2) property   邮箱的初始属性                                                          *
 *        (3) pError     详细调用结果                                                            *
 *  返回: (1) eFailure   操作失败                                                                *
 *        (2) eSuccess   操作成功                                                                *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState xMailBoxCreate(TMailBox* pMailbox, TProperty property, TError* pError)
{
    TState state = eFailure;
    TError error = IPC_ERR_FAULT;
    TReg32 imask;

    CpuEnterCritical(&imask);

    if (!(pMailbox->Property &IPC_PROP_READY))
    {
        property |= IPC_PROP_READY;
        pMailbox->Property = property;
        pMailbox->Status = eMailBoxEmpty;
        pMailbox->Mail = (void*)0;

        pMailbox->Queue.PrimaryHandle   = (TObjNode*)0;
        pMailbox->Queue.AuxiliaryHandle = (TObjNode*)0;
        pMailbox->Queue.Property        = &(pMailbox->Property);

        error = IPC_ERR_NONE;
        state = eSuccess;
    }

    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能：重置邮箱                                                                               *
 *  参数：(1) pMailbox   邮箱的地址                                                              *
 *        (2) pError     详细调用结果                                                            *
 *  返回: (1) eFailure   操作失败                                                                *
 *        (2) eSuccess   操作成功                                                                *
 *  说明：注意线程的等待结果都是IPC_ERR_DELETE                                                   *
 *************************************************************************************************/
TState xMailBoxDelete(TMailBox* pMailbox, TError* pError)
{
    TState state = eFailure;
    TError error = IPC_ERR_UNREADY;
    TReg32 imask;
    TBool HiRP = eFalse;

    CpuEnterCritical(&imask);

    if (pMailbox->Property &IPC_PROP_READY)
    {
        /* 将邮箱阻塞队列上的所有等待线程都释放,所有线程的等待结果都是IPC_ERR_DELETE  */
        uIpcUnblockAll(&(pMailbox->Queue), eFailure, IPC_ERR_DELETE, (void**)0, &HiRP);

        /* 清除邮箱对象的全部数据 */
        memset(pMailbox, 0U, sizeof(TMailBox));

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
 *  功能: 清除邮箱阻塞队列                                                                       *
 *  参数: (1) pMailbox  邮箱结构地址                                                             *
 *        (2) pError    详细调用结果                                                             *
 *  返回: (1) eFailure  操作失败                                                                 *
 *        (2) eSuccess  操作成功                                                                 *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState xMailboxReset(TMailBox* pMailbox, TError* pError)
{
    TState state = eFailure;
    TError error = IPC_ERR_UNREADY;
    TReg32 imask;
    TBool HiRP = eFalse;

    CpuEnterCritical(&imask);
    if (pMailbox->Property &IPC_PROP_READY)
    {
        /* 将阻塞队列上的所有等待线程都释放,所有线程的等待结果都是IPC_ERR_RESET */
        uIpcUnblockAll(&(pMailbox->Queue), eFailure, IPC_ERR_RESET, (void**)0, &HiRP);

        /* 设置邮箱的状态为空,清空邮箱中的邮件 */
        pMailbox->Property &= IPC_RESET_MBOX_PROP;
        pMailbox->Status = eMailBoxEmpty;
        pMailbox->Mail = (TMail*)0;

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
 *  功能：邮箱阻塞终止函数,将指定的线程从邮箱的读阻塞队列中终止阻塞并唤醒                        *
 *  参数: (1) pMailbox 邮箱结构地址                                                              *
 *        (2) pThread  线程结构地址                                                              *
 *        (3) option   参数选项                                                                  *
 *        (4) pError   详细调用结果                                                              *
 *  返回: (1) eFailure 操作失败                                                                  *
 *        (2) eSuccess 操作成功                                                                  *
 *  说明?                                                                                        *
 *************************************************************************************************/
TState xMailBoxFlush(TMailBox* pMailbox, TError* pError)
{
    TState state = eFailure;
    TError error = IPC_ERR_UNREADY;
    TReg32 imask;
    TBool HiRP = eFalse;

    CpuEnterCritical(&imask);

    if (pMailbox->Property &IPC_PROP_READY)
    {
        /* 将邮箱阻塞队列上的所有等待线程都释放，所有线程的等待结果都是TCLE_IPC_FLUSH  */
        uIpcUnblockAll(&(pMailbox->Queue), eFailure, IPC_ERR_FLUSH, (void**)0, &HiRP);

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
 *  功能：邮箱广播函数,向所有读阻塞队列中的线程广播邮件                                          *
 *  参数: (1) pMailbox  邮箱结构地址                                                             *
 *        (2) pMail2    保存邮件结构地址的指针变量                                               *
 *        (3) pError    详细调用结果                                                             *
 *  返回: (1) eFailure  操作失败                                                                 *
 *        (2) eSuccess  操作成功                                                                 *
 *  说明：只有邮箱的线程阻塞队列中存在读邮箱的线程的时候,才能把邮件发送给队列中的线程            *
 *************************************************************************************************/
TState xMailBoxBroadcast(TMailBox* pMailbox, TMail* pMail2, TError* pError)
{
    TState state = eFailure;
    TError error = IPC_ERR_UNREADY;
    TReg32 imask;
    TBool HiRP = eFalse;

    CpuEnterCritical(&imask);

    if (pMailbox->Property &IPC_PROP_READY)
    {
        /* 只有邮箱空并且有线程等待读取邮件的时候才能进行广播 */
        if (pMailbox->Status == eMailBoxEmpty)
        {
            uIpcUnblockAll(&(pMailbox->Queue), eSuccess, IPC_ERR_NONE, (void**)pMail2, &HiRP);

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

