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
static TState ReceiveMail(TMailbox* pMailbox, void** pMail2, TBool* pHiRP, TError* pError)
{
    TState state = eSuccess;
    TError error = OS_IPC_ERR_NONE;
    TIpcContext* pContext = (TIpcContext*)0;

    if (pMailbox->Status == OsMailboxFull)
    {
        /* 从邮箱中读取邮件 */
        *pMail2 = pMailbox->Mail;

        /*
         * 如果此时在邮箱的线程阻塞队列中有线程存在,即该线程等待发送邮件,
         * 则从中将一个合适的线程解除阻塞。此时紧急邮件优先
         */
        if (pMailbox->Property & OS_IPC_PROP_AUXIQ_AVAIL)
        {
            pContext = (TIpcContext*)(pMailbox->Queue.AuxiliaryHandle->Owner);
        }
        else
        {
            if (pMailbox->Property & OS_IPC_PROP_PRIMQ_AVAIL)
            {
                pContext = (TIpcContext*)(pMailbox->Queue.PrimaryHandle->Owner);
            }
        }

        /* 如果有线程被解除阻塞,则将该线程待发送的邮件保存到邮箱中, 保持邮箱状态不变 */
        if (pContext != (TIpcContext*)0)
        {
            OsIpcUnblockThread(pContext, eSuccess, OS_IPC_ERR_NONE, pHiRP);
            pMailbox->Mail = *((TMail*)(pContext->Data.Addr2));
        }
        /* 否则读空邮箱,设置邮件为空,设置邮箱状态为空 */
        else
        {
            pMailbox->Mail = (void*)0;
            pMailbox->Status = OsMailboxEmpty;
        }
    }
    else
    {
        error = OS_IPC_ERR_NORMAL;
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
static TState SendMail(TMailbox* pMailbox, void** pMail2, TBool* pHiRP, TError* pError)
{
    TState state = eSuccess;
    TError error = OS_IPC_ERR_NONE;
    TIpcContext* pContext = (TIpcContext*)0;

    if (pMailbox->Status == OsMailboxEmpty)
    {
        /*
         * 当邮箱为空的时候,如果有线程处于邮箱的阻塞队列,说明是当前邮箱的的线程阻塞队列是
         * 邮件读取队列, 这时需要从邮箱的阻塞队列中找到一个合适的线程,并直接使得它读取邮件成功。
         * 同时邮箱的状态不变
         */
        if (pMailbox->Property & OS_IPC_PROP_PRIMQ_AVAIL)
        {
            pContext = (TIpcContext*)(pMailbox->Queue.PrimaryHandle->Owner);
        }

        /* 如果找到了一个合适的线程,就将邮件发送给它 */
        if (pContext != (TIpcContext*)0)
        {
            OsIpcUnblockThread(pContext, eSuccess, OS_IPC_ERR_NONE, pHiRP);
            *(pContext->Data.Addr2) = * pMail2;
        }
        else
        {
            /* 否则将邮件写入邮箱,并置邮箱状态为满 */
            pMailbox->Mail = * pMail2;
            pMailbox->Status = OsMailboxFull;
        }
    }
    else
    {
        /* 邮箱内已经有邮件了，不能再放入其他邮件 */
        error = OS_IPC_ERR_NORMAL;
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
TState TclReceiveMail(TMailbox* pMailbox, TMail* pMail2, TOption option, TTimeTick timeo,
                      TError* pError)
{
    TState state = eFailure;
    TError error = OS_IPC_ERR_UNREADY;
    TBool HiRP = eFalse;
    TIpcContext context;
    TReg32 imask;

    OS_ASSERT((pMailbox != (TMailbox*)0), "");
    OS_ASSERT((pMail2 != (TMail*)0), "");
    OS_ASSERT((pError != (TError*)0), "");

    /* 调整操作选项，屏蔽不需要支持的选项 */
    option &= OS_USER_MBOX_OPTION;

    OsCpuEnterCritical(&imask);
    if (pMailbox->Property & OS_IPC_PROP_READY)
    {
        /*
         * 如果是中断程序调用本函数则只能以非阻塞方式从邮箱中读取邮件,
         * 并且暂时不考虑线程调度问题。
         * 在中断中,当前线程未必是最高就绪优先级线程,也未必处于内核就绪线程队列，
         * 所以在此处得到的HiRP标记无任何意义。
         */
        state = ReceiveMail(pMailbox, (void**)pMail2, &HiRP, &error);

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
                * 如果当前线程不能得到邮件,并且采用的是等待方式,并且内核没有关闭线程调度,
                * 那么当前线程必须阻塞在邮箱队列中,并且强制线程调度
                */
                if (option & OS_IPC_OPT_WAIT)
                {
                    /* 如果当前线程不能被阻塞则函数直接返回 */
                    if (OsKernelVariable.CurrentThread->ACAPI & OS_THREAD_ACAPI_BLOCK)
                    {
                        /* 保存线程阻塞信息 */
                        OsIpcInitContext(&context, (void*)pMailbox, (TBase32)pMail2, sizeof(TBase32),                                         
                                         option | OS_IPC_OPT_MAILBOX | OS_IPC_OPT_READ_DATA, timeo,
                                         &state, &error);

                        /* 当前线程阻塞在该邮箱的阻塞队列，时限或者无限等待，由OS_IPC_OPT_TIMEO参数决定 */
                        OsIpcBlockThread(&context, &(pMailbox->Queue));

                        /* 当前线程被阻塞，其它线程得以执行 */
                        OsThreadSchedule();

                        OsCpuLeaveCritical(imask);
                        /*
                         * 因为当前线程已经阻塞在邮箱的线程阻塞队列，所以处理器需要执行别的线程。
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
TState TclSendMail(TMailbox* pMailbox, TMail* pMail2, TOption option, TTimeTick timeo,
                   TError* pError)
{
    TState state = eFailure;
    TError error = OS_IPC_ERR_UNREADY;
    TBool HiRP = eFalse;
    TIpcContext context;
    TReg32 imask;

    OS_ASSERT((pMailbox != (TMailbox*)0), "");
    OS_ASSERT((pMail2 != (TMail*)0), "");
    OS_ASSERT((pError != (TError*)0), "");

    /* 调整操作选项，屏蔽不需要支持的选项 */
    option &= OS_USER_MBOX_OPTION;

    OsCpuEnterCritical(&imask);
    if (pMailbox->Property & OS_IPC_PROP_READY)
    {
        /*
         * 如果是中断程序调用本函数则只能以非阻塞方式向邮箱中发送邮件,
         * 并且暂时不考虑线程调度问题。
         * 在中断中,当前线程未必是最高就绪优先级线程,也未必处于内核就绪线程队列，
         * 所以在此处得到的HiRP标记无任何意义。
         */
        state = SendMail(pMailbox, (void**)pMail2, &HiRP, &error);

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
                * 如果当前线程不能发送邮件,并且采用的是等待方式,并且内核没有关闭线程调度,
                * 那么当前线程必须阻塞在邮箱队列中,并且强制线程调度
                */
                if (option & OS_IPC_OPT_WAIT)
                {
                    /* 如果当前线程不能被阻塞则函数直接返回 */
                    if (OsKernelVariable.CurrentThread->ACAPI & OS_THREAD_ACAPI_BLOCK)
                    {
                        /* 保存线程挂起信息 */
                        OsIpcInitContext(&context, (void*)pMailbox,
                                         (TBase32)pMail2, sizeof(TBase32),
                                         option | OS_IPC_OPT_MAILBOX | OS_IPC_OPT_WRITE_DATA, timeo,
                                         &state, &error);

                       /* 
                         * 发送紧急邮件的线程进入邮箱阻塞队列的辅助队列中,
                         * 发送普通邮件的线程进入邮箱阻塞队列的基本队列中,
                         * 当前线程阻塞在该邮箱的阻塞队列时，具体采用时限还是无限等待的方式，
                         * 由OS_IPC_OPT_TIMEO参数决定
                         */
                        OsIpcBlockThread(&context, &(pMailbox->Queue));

                        /* 当前线程被阻塞，其它线程得以执行 */
                        OsThreadSchedule();

                        OsCpuLeaveCritical(imask);
                        /*
                         * 因为当前线程已经阻塞在邮箱的线程阻塞队列，所以处理器需要执行别的线程。
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
 *  功能：ISR向邮箱发送邮件                                                                      *
 *  参数: (1) pMailbox 邮箱结构地址                                                              *
 *        (2) pMail2   保存邮件结构地址的指针变量                                                *
 *        (3) option   访问邮箱的模式                                                            *
 *        (4) pError   详细调用结果                                                              *
 *  返回: (1) eFailure 操作失败                                                                  *
 *        (2) eSuccess 操作成功                                                                  *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclIsrSendMail(TMailbox* pMailbox, TMail* pMail2, TOption option, TError* pError)
{
    TState state = eFailure;
    TError error = OS_IPC_ERR_UNREADY;
    TBool HiRP = eFalse;
    TReg32 imask;

    OS_ASSERT((pMailbox != (TMailbox*)0), "");
    OS_ASSERT((pMail2 != (TMail*)0), "");
    OS_ASSERT((pError != (TError*)0), "");

    /* 调整操作选项，屏蔽不需要支持的选项 */
    option &= OS_ISR_MBOX_OPTION;

    OsCpuEnterCritical(&imask);
    if (pMailbox->Property & OS_IPC_PROP_READY)
    {
        /*
         * 如果是中断程序调用本函数则只能以非阻塞方式向邮箱中发送邮件,
         * 并且暂时不考虑线程调度问题。
         * 在中断中,当前线程未必是最高就绪优先级线程,也未必处于内核就绪线程队列，
         * 所以在此处得到的HiRP标记无任何意义。
         */
        state = SendMail(pMailbox, (void**)pMail2, &HiRP, &error);
    }
    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}

/*************************************************************************************************
 *  功能：初始化邮箱                                                                             *
 *  参数：(1) pMailbox   邮箱的地址                                                              *
 *        (2) pName      邮箱的名称                                                              *
 *        (3) property   邮箱的初始属性                                                          *
 *        (4) pError     详细调用结果                                                            *
 *  返回: (1) eFailure   操作失败                                                                *
 *        (2) eSuccess   操作成功                                                                *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclCreateMailbox(TMailbox* pMailbox, TChar* pName, TProperty property, TError* pError)
{
    TState state = eFailure;
    TError error = OS_IPC_ERR_FAULT;
    TReg32 imask;

    OS_ASSERT((pMailbox != (TMailbox*)0), "");
    OS_ASSERT((pName != (TChar*)0), "");
    OS_ASSERT((pError != (TError*)0), "");

    property &= OS_USER_MBOX_PROP;

    OsCpuEnterCritical(&imask);

    if (!(pMailbox->Property & OS_IPC_PROP_READY))
    {
        /* 初始化邮箱对象信息 */
        OsKernelAddObject(&(pMailbox->Object), pName, OsMailboxObject, (void*)pMailbox);

        /* 初始化邮箱基本信息 */
        property |= OS_IPC_PROP_READY;
        pMailbox->Property = property;
        pMailbox->Status = OsMailboxEmpty;
        pMailbox->Mail = (void*)0;

        pMailbox->Queue.PrimaryHandle   = (TLinkNode*)0;
        pMailbox->Queue.AuxiliaryHandle = (TLinkNode*)0;
        pMailbox->Queue.Property        = &(pMailbox->Property);

        error = OS_IPC_ERR_NONE;
        state = eSuccess;
    }

    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能：重置邮箱                                                                               *
 *  参数：(1) pMailbox   邮箱的地址                                                              *
 *        (2) pError     详细调用结果                                                            *
 *  返回: (1) eFailure   操作失败                                                                *
 *        (2) eSuccess   操作成功                                                                *
 *  说明：注意线程的等待结果都是OS_IPC_ERR_DELETE                                                   *
 *************************************************************************************************/
TState TclDeleteMailbox(TMailbox* pMailbox, TError* pError)
{
    TState state = eFailure;
    TError error = OS_IPC_ERR_UNREADY;
    TReg32 imask;
    TBool HiRP = eFalse;

    OS_ASSERT((pMailbox != (TMailbox*)0), "");
    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    if (pMailbox->Property & OS_IPC_PROP_READY)
    {
        /* 将邮箱阻塞队列上的所有等待线程都释放,所有线程的等待结果都是OS_IPC_ERR_DELETE  */
        OsIpcUnblockAll(&(pMailbox->Queue), eFailure, OS_IPC_ERR_DELETE, (void**)0, &HiRP);

        /* 从内核中移除邮箱对象 */
        OsKernelRemoveObject(&(pMailbox->Object));

        /* 清除邮箱对象的全部数据 */
        memset(pMailbox, 0U, sizeof(TMailbox));

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
 *  功能: 清除邮箱阻塞队列                                                                       *
 *  参数: (1) pMailbox  邮箱结构地址                                                             *
 *        (2) pError    详细调用结果                                                             *
 *  返回: (1) eFailure  操作失败                                                                 *
 *        (2) eSuccess  操作成功                                                                 *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclResetMailbox(TMailbox* pMailbox, TError* pError)
{
    TState state = eFailure;
    TError error = OS_IPC_ERR_UNREADY;
    TReg32 imask;
    TBool HiRP = eFalse;

    OS_ASSERT((pMailbox != (TMailbox*)0), "");
    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);
    if (pMailbox->Property & OS_IPC_PROP_READY)
    {
        /* 将阻塞队列上的所有等待线程都释放,所有线程的等待结果都是OS_IPC_ERR_RESET */
        OsIpcUnblockAll(&(pMailbox->Queue), eFailure, OS_IPC_ERR_RESET, (void**)0, &HiRP);

        /* 设置邮箱的状态为空,清空邮箱中的邮件 */
        pMailbox->Property &= OS_RESET_MBOX_PROP;
        pMailbox->Status = OsMailboxEmpty;
        pMailbox->Mail = (TMail*)0;

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
 *  功能：邮箱阻塞终止函数,将指定的线程从邮箱的读阻塞队列中终止阻塞并唤醒                        *
 *  参数: (1) pMailbox 邮箱结构地址                                                              *
 *        (2) pThread  线程结构地址                                                              *
 *        (3) option   参数选项                                                                  *
 *        (4) pError   详细调用结果                                                              *
 *  返回: (1) eFailure 操作失败                                                                  *
 *        (2) eSuccess 操作成功                                                                  *
 *  说明?                                                                                        *
 *************************************************************************************************/
TState TclFlushMailbox(TMailbox* pMailbox, TError* pError)
{
    TState state = eFailure;
    TError error = OS_IPC_ERR_UNREADY;
    TReg32 imask;
    TBool HiRP = eFalse;
    OS_ASSERT((pMailbox != (TMailbox*)0), "");
    OS_ASSERT((pError != (TError*)0), "");


    OsCpuEnterCritical(&imask);

    if (pMailbox->Property & OS_IPC_PROP_READY)
    {
        /* 将邮箱阻塞队列上的所有等待线程都释放，所有线程的等待结果都是OS_IPC_ERR_FLUSH  */
        OsIpcUnblockAll(&(pMailbox->Queue), eFailure, OS_IPC_ERR_FLUSH, (void**)0, &HiRP);

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
 *  功能：邮箱广播函数,向所有读阻塞队列中的线程广播邮件                                          *
 *  参数: (1) pMailbox  邮箱结构地址                                                             *
 *        (2) pMail2    保存邮件结构地址的指针变量                                               *
 *        (3) pError    详细调用结果                                                             *
 *  返回: (1) eFailure  操作失败                                                                 *
 *        (2) eSuccess  操作成功                                                                 *
 *  说明：只有邮箱的线程阻塞队列中存在读邮箱的线程的时候,才能把邮件发送给队列中的线程            *
 *************************************************************************************************/
TState TclBroadcastMail(TMailbox* pMailbox, TMail* pMail2, TError* pError)
{
    TState state = eFailure;
    TError error = OS_IPC_ERR_UNREADY;
    TReg32 imask;
    TBool HiRP = eFalse;
    OS_ASSERT((pMailbox != (TMailbox*)0), "");
    OS_ASSERT((pMail2 != (TMail*)0), "");
    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    if (pMailbox->Property & OS_IPC_PROP_READY)
    {
        /* 只有邮箱空并且有线程等待读取邮件的时候才能进行广播 */
        if (pMailbox->Status == OsMailboxEmpty)
        {
            OsIpcUnblockAll(&(pMailbox->Queue), eSuccess, OS_IPC_ERR_NONE, (void**)pMail2, &HiRP);

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
