/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#include <string.h>

#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.object.h"
#include "tcl.cpu.h"
#include "tcl.ipc.h"
#include "tcl.debug.h"
#include "tcl.kernel.h"
#include "tcl.timer.h"
#include "tcl.thread.h"

/* 内核进就绪队列定义,处于就绪和运行的线程都放在这个队列里 */
static TThreadQueue ThreadReadyQueue;

/* 内核线程辅助队列定义，处于延时、挂起、休眠的线程都放在这个队列里 */
static TThreadQueue ThreadAuxiliaryQueue;


/*************************************************************************************************
 *  功能：将线程从指定的状态转换到就绪态，使得线程能够参与内核调度                               *
 *  参数：(1) pThread   线程结构地址                                                             *
 *        (2) status    线程当前状态，用于检查                                                   *
 *        (3) pError    保存操作结果                                                             *
 *  返回：(1) eFailure                                                                           *
 *        (2) eSuccess                                                                           *
 *  说明：函数名的前缀'u'(communal)表示本函数是全局函数                                          *
 *************************************************************************************************/
static TState SetThreadReady(TThread* pThread, TThreadStatus status, TBool* pHiRP, TError* pError)
{
    TState state = eFailure;
    TError error = THREAD_ERR_STATUS;

    /* 线程状态校验,只有状态符合的线程才能被操作 */
    if (pThread->Status == status)
    {
        /*
         * 操作线程，完成线程队列和状态转换
         * 因为是在线程环境下，所以此时pThread一定不是当前线程
         */
        uThreadLeaveQueue(&ThreadAuxiliaryQueue, pThread);
        uThreadEnterQueue(&ThreadReadyQueue, pThread, eLinkPosTail);
        pThread->Status = eThreadReady;
        if (pThread->Priority < uKernelVariable.CurrentThread->Priority)
        {
            *pHiRP = eTrue;
        }

        state = eSuccess;
        error = THREAD_ERR_NONE;
    }

    /* 如果是取消定时操作则需要停止线程定时器 */
    if ((state == eSuccess) && (status == eThreadDelayed))
    {
        uObjListRemoveDiffNode(&(uKernelVariable.ThreadTimerList), &(pThread->Timer.LinkNode));
    }

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能：线程挂起函数                                                                           *
 *  参数：(1) pThread   线程结构地址                                                             *
 *        (2) status    线程当前状态，用于检查                                                   *
 *        (3) ticks     线程延时时间                                                             *
 *        (4) pError    保存操作结果                                                             *
 *  返回：(1) eFailure                                                                           *
 *        (2) eSuccess                                                                           *
 *  说明：                                                                                       *
 *************************************************************************************************/
static TState SetThreadUnready(TThread* pThread, TThreadStatus status, TTimeTick ticks,
                               TBool* pHiRP, TError* pError)
{
    TState state = eFailure;
    TError error = THREAD_ERR_STATUS;

    /* 如果操作的是当前线程，则需要首先检查内核是否允许调度 */
    if (pThread->Status == eThreadRunning)
    {
        /* 如果内核此时禁止线程调度，那么当前线程不能被操作 */
        if (uKernelVariable.SchedLockTimes == 0U)
        {
            uThreadLeaveQueue(&ThreadReadyQueue, pThread);
            uThreadEnterQueue(&ThreadAuxiliaryQueue, pThread, eLinkPosTail);
            pThread->Status = status;
            *pHiRP = eTrue;

            error = THREAD_ERR_NONE;
            state = eSuccess;
        }
        else
        {
            error = THREAD_ERR_FAULT;
        }
    }
    else if (pThread->Status == eThreadReady)
    {
        /* 如果被操作的线程不是当前线程，则不会引起线程调度，所以直接处理线程和队列 */
        uThreadLeaveQueue(&ThreadReadyQueue, pThread);
        uThreadEnterQueue(&ThreadAuxiliaryQueue, pThread, eLinkPosTail);
        pThread->Status = status;

        error = THREAD_ERR_NONE;
        state = eSuccess;
    }
    else
    {
        uDebugAlarm("");
    }

    /* 重置并启动线程定时器 */
    if ((state == eSuccess) && (status == eThreadDelayed))
    {
        pThread->Timer.RemainTicks = ticks;
        uObjListAddDiffNode(&(uKernelVariable.ThreadTimerList), &(pThread->Timer.LinkNode));
    }

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能：计算就绪线程队列中的最高优先级函数                                                     *
 *  参数：无                                                                                     *
 *  返回：HiRP (Highest Ready Priority)                                                          *
 *  说明：                                                                                       *
 *************************************************************************************************/
static void CalcThreadHiRP(TPriority* priority)
{
    /* 如果就绪优先级不存在则说明内核发生致命错误 */
    if (ThreadReadyQueue.PriorityMask == (TBitMask)0)
    {
        uDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }
    *priority = CpuCalcHiPRIO(ThreadReadyQueue.PriorityMask);
}


#if (TCLC_THREAD_STACK_CHECK_ENABLE)
/*************************************************************************************************
 *  功能：告警和检查线程栈溢出问题                                                               *
 *  参数：(1) pThread  线程地址                                                                  *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
static void CheckThreadStack(TThread* pThread)
{
    if ((pThread->StackTop < pThread->StackBarrier) ||
            (*(TBase32*)(pThread->StackBarrier) != TCLC_THREAD_STACK_BARRIER_VALUE))
    {
        uKernelVariable.Diagnosis |= KERNEL_DIAG_THREAD_ERROR;
        pThread->Diagnosis |= THREAD_DIAG_STACK_OVERFLOW;
        uDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }

    if (pThread->StackTop < pThread->StackAlarm)
    {
        pThread->Diagnosis |= THREAD_DIAG_STACK_ALARM;
    }
}

#endif


/*************************************************************************************************
 *  功能：线程运行监理函数，线程的运行都以它为基础                                               *
 *  参数：(1) pThread  线程地址                                                                  *
 *  返回：无                                                                                     *
 *  说明：函数名的前缀'x'(eXtreme)表示本函数需要处理临界区代码                                   *
 *************************************************************************************************/
static void xSuperviseThread(TThread* pThread)
{
    /* 普通线程需要注意用户不小心退出导致非法指令等死机的问题 */
    KNL_ASSERT((pThread == uKernelVariable.CurrentThread), "");
    pThread->Entry(pThread->Argument);

    uKernelVariable.Diagnosis |= KERNEL_DIAG_THREAD_ERROR;
    pThread->Diagnosis |= THREAD_DIAG_INVALID_EXIT;
    uDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
}


/*************************************************************************************************
 *  功能：初始化内核线程管理模块                                                                 *
 *  参数：无                                                                                     *
 *  返回：无                                                                                     *
 *  说明：内核中的线程队列主要有一下几种：                                                       *
 *        (1) 线程就绪队列,用于存储所有的就绪线和运行线程。内核中只有一个就绪队列。              *
 *        (2) 线程辅助队列, 所有挂起状态、延时状态和休眠状态的线程都存储在这个队列中。           *
 *            同样内核中只有一个休眠队列                                                         *
 *        (3) IPC对象的线程阻塞队列，数量不定。所有阻塞状态的线程都保存在相应的线程阻塞队列里。  *
 *************************************************************************************************/
void uThreadModuleInit(void)
{
    /* 检查内核是否处于初始状态 */
    if (uKernelVariable.State != eOriginState)
    {
        uDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }

    memset(&ThreadReadyQueue, 0, sizeof(ThreadReadyQueue));
    memset(&ThreadAuxiliaryQueue, 0, sizeof(ThreadAuxiliaryQueue));

    uKernelVariable.ThreadReadyQueue = &ThreadReadyQueue;
    uKernelVariable.ThreadAuxiliaryQueue = &ThreadAuxiliaryQueue;
}

/* RULE
 * 1 当前线程离开就绪队列后，再次加入就绪队列时，
 *   如果仍然是当前线程则一定放在相应的队列头部，而且不重新计算时间片。
 *   如果已经不是当前线程则一定放在相应的队列尾部，而且不重新计算时间片。
 * 2 当前线程在就绪队列内部调整优先级时，在新的队列里也一定要在队列头。
 */

/*************************************************************************************************
 *  功能：将线程加入到指定的线程队列中                                                           *
 *  参数：(1) pQueue  线程队列地址地址                                                           *
 *        (2) pThread 线程结构地址                                                               *
 *        (3) pos     线程在线程队列中的位置                                                     *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void uThreadEnterQueue(TThreadQueue* pQueue, TThread* pThread, TLinkPos pos)
{
    TPriority priority;
    TLinkNode** pHandle;

    /* 检查线程和线程队列 */
    KNL_ASSERT((pThread != (TThread*)0), "");
    KNL_ASSERT((pThread->Queue == (TThreadQueue*)0), "");

    /* 根据线程优先级得出线程实际所属分队列 */
    priority = pThread->Priority;
    pHandle = &(pQueue->Handle[priority]);

    /* 将线程加入指定的分队列 */
    uObjQueueAddFifoNode(pHandle, &(pThread->LinkNode), pos);

    /* 设置线程所属队列 */
    pThread->Queue = pQueue;

    /* 设定该线程优先级为就绪优先级 */
    pQueue->PriorityMask |= (0x1 << priority);
}


/*************************************************************************************************
 *  功能：将线程从指定的线程队列中移出                                                           *
 *  参数：(1) pQueue  线程队列地址地址                                                           *
 *        (2) pThread 线程结构地址                                                               *
 *  返回：无                                                                                     *
 *  说明：FIFO PRIO两种访问资源的方式                                                            *
 *************************************************************************************************/
void uThreadLeaveQueue(TThreadQueue* pQueue, TThread* pThread)
{
    TPriority priority;
    TLinkNode** pHandle;

    /* 检查线程是否属于本队列,如果不属于则内核发生致命错误 */
    KNL_ASSERT((pThread != (TThread*)0), "");
    KNL_ASSERT((pQueue == pThread->Queue), "");

    /* 根据线程优先级得出线程实际所属分队列 */
    priority = pThread->Priority;
    pHandle = &(pQueue->Handle[priority]);

    /* 将线程从指定的分队列中取出 */
    uObjQueueRemoveNode(pHandle, &(pThread->LinkNode));

    /* 设置线程所属队列 */
    pThread->Queue = (TThreadQueue*)0;

    /* 处理线程离开队列后对队列优先级就绪标记的影响 */
    if (pQueue->Handle[priority] == (TLinkNode*)0)
    {
        /* 设定该线程优先级未就绪 */
        pQueue->PriorityMask &= (~(0x1 << priority));
    }
}


/*************************************************************************************************
 *  功能：线程时间片处理函数，在时间片中断处理ISR中会调用本函数                                  *
 *  参数：无                                                                                     *
 *  返回：无                                                                                     *
 *  说明：本函数完成了当前线程的时间片处理，但并没有选择需要调度的后继线程和进行线程切换         *
 *************************************************************************************************/
/*
 * 当前线程可能处于3种位置
 * 1 就绪队列的头位置(任何优先级)
 * 2 就绪队列的其它位置(任何优先级)
 * 3 辅助队列或者阻塞队列里
 * 只有情况1才需要进行时间片轮转的处理，但此时不涉及线程切换,因为本函数只在ISR中调用。
 */
void uThreadTickUpdate(void)
{
    TThread* pThread;
    TLinkNode* pHandle;

    /* 将当前线程时间片减去1个节拍数,线程运行总节拍数加1 */
    pThread = uKernelVariable.CurrentThread;
    pThread->Ticks--;
    pThread->Jiffies++;

    /* 如果本轮时间片运行完毕 */
    if (pThread->Ticks == 0U)
    {
        /* 恢复线程的时钟节拍数 */
        pThread->Ticks = pThread->BaseTicks;

        /* 判断线程是不是处于内核就绪线程队列的某个优先级的队列头 */
        pHandle = ThreadReadyQueue.Handle[pThread->Priority];
        if ((TThread*)(pHandle->Owner) == pThread)
        {
            /* 如果内核此时允许线程调度 */
            if (uKernelVariable.SchedLockTimes == 0U)
            {
                /*
                 * 发起时间片调度，之后pThread处于线程队列尾部,
                 * 当前线程所在线程队列也可能只有当前线程唯一1个线程
                 */
                ThreadReadyQueue.Handle[pThread->Priority] =
                    (ThreadReadyQueue.Handle[pThread->Priority])->Next;

                /* 将线程状态置为就绪,准备线程切换 */
                pThread->Status = eThreadReady;
            }
        }
    }
}


/*************************************************************************************************
 *  功能：线程定时器处理函数                                                                     *
 *  参数：无                                                                                     *
 *  返回：无                                                                                     *
 *  说明:                                                                                        *
 *************************************************************************************************/
void uThreadTimerUpdate(void)
{
    TThread* pThread;
    TTickTimer* pTimer;
    TBool HiRP = eFalse;

    /* 得到处于队列头的线程定时器，将对应的定时计数减1 */
    if (uKernelVariable.ThreadTimerList != (TLinkNode*)0)
    {
        pTimer = (TTickTimer*)(uKernelVariable.ThreadTimerList->Owner);
        pTimer->RemainTicks--;

        /* 处理计数为0的线程定时器 */
        while (pTimer->RemainTicks == 0u)
        {
            /*
             * 操作线程，完成线程队列和状态转换,注意只有中断处理时，
             * 当前线程才会处在内核线程辅助队列里(因为还没来得及线程切换)
             * 当前线程返回就绪队列时，一定要回到相应的队列头
             * 当线程进出就绪队列时，不需要处理线程的时钟节拍数
             */
            pThread = (TThread*)(pTimer->Owner);
            if (pThread->Status == eThreadDelayed)
            {
                uThreadLeaveQueue(uKernelVariable.ThreadAuxiliaryQueue, pThread);
                if (pThread == uKernelVariable.CurrentThread)
                {
                    uThreadEnterQueue(uKernelVariable.ThreadReadyQueue,
                                      pThread, eLinkPosHead);
                    pThread->Status = eThreadRunning;
                }
                else
                {
                    uThreadEnterQueue(uKernelVariable.ThreadReadyQueue,
                                      pThread, eLinkPosTail);
                    pThread->Status = eThreadReady;
                }
                /* 将线程定时器从差分队列中移出 */
                uObjListRemoveDiffNode(&(uKernelVariable.ThreadTimerList),
                                       &(pTimer->LinkNode));
            }
#if (TCLC_IPC_ENABLE)
            /* 将线程从阻塞队列中解除阻塞 */
            else if (pThread->Status == eThreadBlocked)
            {
                uIpcUnblockThread(pThread->IpcContext, eFailure, IPC_ERR_TIMEO, &HiRP);
            }
#endif
            else
            {
                uDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
            }

            if (uKernelVariable.ThreadTimerList == (TLinkNode*)0)
            {
                break;
            }

            /* 获得下一个线程定时器 */
            pTimer = (TTickTimer*)(uKernelVariable.ThreadTimerList->Owner);
        }
    }
}


/*************************************************************************************************
 *  功能：用于请求线程调度                                                                       *
 *  参数：无                                                                                     *
 *  返回：无                                                                                     *
 *  说明：线程的调度请求可能被ISR最终取消                                                        *
 *************************************************************************************************/
/*
 * 1 当前线程离开队列即代表它放弃本轮运行，当前线程返回就绪队列时，一定要回到相应的队列头
     当线程进出就绪队列时，不需要处理线程的时钟节拍数
 * 2 导致当前线程不是最高就绪优先级的原因有
 *   1 别的优先级更高的线程进入就绪队列
 *   2 当前线程自己离开队列
 *   3 别的线程的优先级被提高
 *   4 当前线程的优先级被拉低
 *   5 当前线程Yiled
 *   6 时间片中断中，当前线程被轮转
 * 3 在cortex处理器上, 有这样一种可能:
 *   当前线程释放了处理器，但在PendSV中断得到响应之前，又有其它高优先级中断发生，
 *   在高级isr中又把当前线程置为运行，
 *   1 并且当前线程仍然是最高就绪优先级，
 *   2 并且当前线程仍然在最高就绪线程队列的队列头。
 *   此时需要考虑取消PENDSV的操作，避免当前线程和自己切换
 */
void uThreadSchedule(void)
{
    TPriority priority;

    /* 如果就绪优先级不存在则说明内核发生致命错误 */
    if (ThreadReadyQueue.PriorityMask == (TBitMask)0)
    {
        uDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }

    /* 查找最高就绪优先级，获得后继线程，如果后继线程指针为空则说明内核发生致命错误 */
    CalcThreadHiRP(&priority);
    uKernelVariable.NomineeThread = (TThread*)((ThreadReadyQueue.Handle[priority])->Owner);
    if (uKernelVariable.NomineeThread == (TThread*)0)
    {
        uDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }

    /* 完成线程的优先级抢占或者时间片轮转 */
    if (uKernelVariable.NomineeThread != uKernelVariable.CurrentThread)
    {
#if (TCLC_THREAD_STACK_CHECK_ENABLE)
        CheckThreadStack(uKernelVariable.NomineeThread);
#endif
        /*
         * 此时有两种可能，一是线程正常执行，然后有更高优先级的线程就绪。
         * 二是当前线程短暂不就绪但是很快又返回运行状态，然后有更高优先级的线程就绪。
         * 不论哪种情况，都需要将当前线程设置为就绪状态。
         */
        if (uKernelVariable.CurrentThread->Status == eThreadRunning)
        {
            uKernelVariable.CurrentThread->Status = eThreadReady;
        }
        CpuConfirmThreadSwitch();
    }
    else
    {
        CpuCancelThreadSwitch();
        /*
         * 在定时器、DAEMON、Deamon等相关操作时，有可能在当先线程尚未切换上下文的时候，
         * 重新放回就绪队列，此时在相关代码里已经将当前线程重新设置成运行状态。
         * 而在yeild、tick isr里，有可能将当前线程设置成就绪态，而此时当前线程所在
         * 队列又只有唯一一个线程就绪，所以这时需要将当前线程重新设置成运行状态。
         */
        uKernelVariable.CurrentThread->Status = eThreadRunning;
    }
}


/*************************************************************************************************
 *  功能：线程结构初始化函数                                                                     *
 *  参数：(1)  pThread  线程结构地址                                                             *
 *        (2)  status   线程的初始状态                                                           *
 *        (3)  property 线程属性                                                                 *
 *        (4)  acapi    对线程管理API的许可控制                                                  *
 *        (5)  pEntry   线程函数地址                                                             *
 *        (6)  TArgument线程函数参数                                                             *
 *        (7)  pStack   线程栈地址                                                               *
 *        (8)  bytes    线程栈大小，以字为单位                                                   *
 *        (9)  priority 线程优先级                                                               *
 *        (10) ticks    线程时间片长度                                                           *
 *  返回：(1)  eFailure                                                                          *
 *        (2)  eSuccess                                                                          *
 *  说明：注意栈起始地址、栈大小和栈告警地址的字节对齐问题                                       *
 *************************************************************************************************/
void uThreadCreate(TThread* pThread, TChar* pName, TThreadStatus status, TProperty property,
                   TBitMask acapi, TThreadEntry pEntry, TArgument argument,
                   void* pStack, TBase32 bytes, TPriority priority, TTimeTick ticks)
{
    TThreadQueue* pQueue;

    /* 初始化线程基本对象信息 */
    uKernelAddObject(&(pThread->Object), pName, eThread, (void*)pThread);

    /* 设置线程栈相关数据和构造线程初始栈栈帧 */
    KNL_ASSERT((bytes >= TCLC_CPU_MINIMAL_STACK), "");

    /* 栈大小向下4byte对齐 */
    bytes &= (~((TBase32)0x3));
    pThread->StackBase = (TBase32)pStack + bytes;

    /* 清空线程栈空间 */
    if (property &THREAD_PROP_CLEAN_STACK)
    {
        memset(pStack, 0U, bytes);
    }

    /* 构造(伪造)线程初始栈帧,这里将线程结构地址作为参数传给xSuperviseThread()函数 */
    CpuBuildThreadStack(&(pThread->StackTop), pStack, bytes, (void*)(&xSuperviseThread),
                        (TArgument)pThread);

    /* 计算线程栈告警地址 */
#if (TCLC_THREAD_STACK_CHECK_ENABLE)
    pThread->StackAlarm = (TBase32)pStack + bytes - (bytes* TCLC_THREAD_STACK_ALARM_RATIO) / 100;
    pThread->StackBarrier = (TBase32)pStack;
    (*(TAddr32*)pStack) = TCLC_THREAD_STACK_BARRIER_VALUE;
#endif

    /* 设置线程时间片相关参数 */
    pThread->Ticks = ticks;
    pThread->BaseTicks = ticks;
    pThread->Jiffies = 0U;

    /* 设置线程优先级 */
    pThread->Priority = priority;
    pThread->BasePriority = priority;

    /* 设置线程入口函数和线程参数 */
    pThread->Entry = pEntry;
    pThread->Argument = argument;

    /* 设置线程所属队列信息 */
    pThread->Queue = (TThreadQueue*)0;

    /* 设置线程定时器 */
    pThread->Timer.LinkNode.Owner = (void*)(&(pThread->Timer));
    pThread->Timer.LinkNode.Data = (TBase32*)(&(pThread->Timer.RemainTicks));
    pThread->Timer.LinkNode.Prev = (TLinkNode*)0;
    pThread->Timer.LinkNode.Next = (TLinkNode*)0;
    pThread->Timer.LinkNode.Handle = (TLinkNode**)0;
    pThread->Timer.Owner = (void*)pThread;
    pThread->Timer.RemainTicks = (TTimeTick)0;

    /*
     * 线程IPC阻塞上下文结构，没有直接定义在线程结构里，而是在需要阻塞的时候，
     * 临时在线程栈里安排的。好处是减少了线程结构占用的内存。
     */
#if (TCLC_IPC_ENABLE)
    pThread->IpcContext = (TIpcContext*)0;
#endif

    /* 清除线程占有的锁(MUTEX)队列 */
#if ((TCLC_IPC_ENABLE) && (TCLC_IPC_MUTEX_ENABLE))
    pThread->LockList = (TLinkNode*)0;
#endif

    /* 初始线程运行诊断信息 */
    pThread->Diagnosis = THREAD_DIAG_NORMAL;

    /* 设置线程能够支持的线程管理API */
    pThread->ACAPI = acapi;

    /* 设置线程链表节点信息，线程此时不属于任何线程队列 */
    pThread->LinkNode.Owner = (void*)pThread;
    pThread->LinkNode.Data = (TBase32*)(&(pThread->Priority));
    pThread->LinkNode.Prev = (TLinkNode*)0;
    pThread->LinkNode.Next = (TLinkNode*)0;
    pThread->LinkNode.Handle = (TLinkNode**)0;

    /* 将线程加入内核线程队列，设置线程状态 */
    pQueue = (status == eThreadReady) ? (&ThreadReadyQueue): (&ThreadAuxiliaryQueue);
    uThreadEnterQueue(pQueue, pThread, eLinkPosTail);
    pThread->Status = status;

    /* 标记线程已经完成初始化 */
    property |= THREAD_PROP_READY;
    pThread->Property = property;
}


/*************************************************************************************************
 *  功能：线程注销                                                                               *
 *  参数：(1) pThread 线程结构地址                                                               *
 *  返回：(1) eFailure                                                                           *
 *        (2) eSuccess                                                                           *
 *  说明：初始化线程和定时器线程不能被注销                                                       *
 *************************************************************************************************/
TState uThreadDelete(TThread* pThread, TError* pError)
{
    TState state = eFailure;
    TError error = THREAD_ERR_STATUS;

    if (pThread->Status == eThreadDormant)
    {
#if ((TCLC_IPC_ENABLE) && (TCLC_IPC_MUTEX_ENABLE))
        if (pThread->LockList)
        {
            error = THREAD_ERR_FAULT;
            state = eFailure;
        }
        else
#endif
        {
            uKernelRemoveObject(&(pThread->Object));
            uThreadLeaveQueue(pThread->Queue, pThread);
            memset(pThread, 0, sizeof(pThread));
            error = THREAD_ERR_NONE;
            state = eSuccess;
        }
    }

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能：更改线程优先级                                                                         *
 *  参数：(1) pThread  线程结构地址                                                              *
 *        (2) priority 线程优先级                                                                *
 *        (3) flag     是否被SetPriority API调用                                                 *
 *        (4) pError   保存操作结果                                                              *
 *  返回：(1) eFailure 更改线程优先级失败                                                        *
 *        (2) eSuccess 更改线程优先级成功                                                        *
 *  说明：如果是临时修改优先级，则不修改线程结构的基本优先级                                     *
 *************************************************************************************************/
TState uThreadSetPriority(TThread* pThread, TPriority priority, TBool flag, TBool* pHiRP, TError* pError)
{
    TState state = eFailure;
    TError error = THREAD_ERR_PRIORITY;
    TPriority newPrio;

    if (pThread->Priority != priority)
    {
        if (pThread->Status == eThreadBlocked)
        {
            /* 阻塞状态的线程都在辅助队列里，修改其优先级 */
            uThreadLeaveQueue(&ThreadAuxiliaryQueue, pThread);
            pThread->Priority = priority;
            uThreadEnterQueue(&ThreadAuxiliaryQueue, pThread, eLinkPosTail);

            uIpcSetPriority(pThread->IpcContext, priority);
            state = eSuccess;
            error = THREAD_ERR_NONE;
        }
        /*
         * 就绪线程调整优先级时，可以直接调整其在就绪线程队列中的分队列
         * 对于处于就绪线程队列中的当前线程，如果修改它的优先级，
         * 因为不会把它移出线程就绪队列，所以即使内核不允许调度也没问题
         */
        else if (pThread->Status == eThreadReady)
        {
            uThreadLeaveQueue(&ThreadReadyQueue, pThread);
            pThread->Priority = priority;
            uThreadEnterQueue(&ThreadReadyQueue, pThread, eLinkPosTail);

            /*
             * 得到当前就绪队列的最高就绪优先级，因为就绪线程(包括当前线程)
             * 在线程就绪队列内的折腾会导致当前线程可能不是最高优先级。
             */
            if (priority < uKernelVariable.CurrentThread->Priority)
            {
                *pHiRP = eTrue;
            }
            state = eSuccess;
            error = THREAD_ERR_NONE;
        }
        else if (pThread->Status == eThreadRunning)
        {
            /*
             * 假设当前线程优先级最高且唯一，假如调低它的优先级之后仍然是最高，
             * 但是在新的优先级里有多个就绪线程，那么最好把当前线程放在新的就绪队列
             * 的头部，这样不会引起隐式的时间片轮转；当前线程先后被多次调整优先级时，只有
             * 每次都把它放在队列头才能保证它最后一次调整优先级后还处在队列头。
             */
            uThreadLeaveQueue(&ThreadReadyQueue, pThread);
            pThread->Priority = priority;
            uThreadEnterQueue(&ThreadReadyQueue, pThread, eLinkPosHead);

            /*
             * 因为当前线程在线程就绪队列内的折腾会导致当前线程可能不是最高优先级，
             * 所以需要重新计算当前就绪队列的最高就绪优先级。
             */
            CalcThreadHiRP(&newPrio);
            if (newPrio < uKernelVariable.CurrentThread->Priority)
            {
                *pHiRP = eTrue;
            }

            state = eSuccess;
            error = THREAD_ERR_NONE;
        }
        else
        {
            /*其它状态的线程都在辅助队列里，修改其优先级 */
            uThreadLeaveQueue(&ThreadAuxiliaryQueue, pThread);
            pThread->Priority = priority;
            uThreadEnterQueue(&ThreadAuxiliaryQueue, pThread, eLinkPosTail);
            state = eSuccess;
            error = THREAD_ERR_NONE;
        }

        /* 如果需要则修改线程固定优先级 */
        if (flag == eTrue)
        {
            pThread->BasePriority = priority;
        }
    }

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能：将线程从挂起状态转换到就绪态，使得线程能够参与内核调度                                 *
 *  参数：(1) pThread   线程结构地址                                                             *
 *  返回：(1) eFailure                                                                           *
 *        (2) eSuccess                                                                           *
 *  说明：函数名的前缀'u'(communal)表示本函数是全局函数                                          *
 *************************************************************************************************/
void uThreadResumeFromISR(TThread* pThread)
{
    /*
     * 操作线程，完成线程队列和状态转换,注意只有中断处理时，
     * 当前线程才会处在内核线程辅助队列里(因为还没来得及线程切换)
     * 当前线程返回就绪队列时，一定要回到相应的队列头
     * 当线程进出就绪队列时，不需要处理线程的时钟节拍数
     */
    if (pThread->Status == eThreadSuspended)
    {
        uThreadLeaveQueue(&ThreadAuxiliaryQueue, pThread);
        if (pThread == uKernelVariable.CurrentThread)
        {
            uThreadEnterQueue(&ThreadReadyQueue, pThread, eLinkPosHead);
            pThread->Status = eThreadRunning;
        }
        else
        {
            uThreadEnterQueue(&ThreadReadyQueue, pThread, eLinkPosTail);
            pThread->Status = eThreadReady;
        }
    }
}


/*************************************************************************************************
 *  功能：将线程自己挂起                                                                         *
 *  参数：无                                                                                     *
 *  返回：无                                                                                     *
 *  说明：函数名的前缀'u'(communal)表示本函数是全局函数                                          *
 *************************************************************************************************/
void uThreadSuspendSelf(void)
{
    /* 操作目标是当前线程 */
    TThread* pThread = uKernelVariable.CurrentThread;

    /* 将当前线程挂起，如果内核此时禁止线程调度，那么当前线程不能被操作 */
    if (uKernelVariable.SchedLockTimes == 0U)
    {
        uThreadLeaveQueue(&ThreadReadyQueue, pThread);
        uThreadEnterQueue(&ThreadAuxiliaryQueue, pThread, eLinkPosTail);
        pThread->Status = eThreadSuspended;
        uThreadSchedule();
    }
    else
    {
        uKernelVariable.Diagnosis |= KERNEL_DIAG_SCHED_ERROR;
        uDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }
}


/*************************************************************************************************
 *  功能：线程结构初始化函数                                                                     *
 *  参数：(1)  pThread  线程结构地址                                                             *
 *        (2)  status   线程的初始状态                                                           *
 *        (3)  property 线程属性                                                                 *
 *        (4)  acapi    对线程管理API的许可控制                                                  *
 *        (5)  pEntry   线程函数地址                                                             *
 *        (6)  pArg     线程函数参数                                                             *
 *        (7)  pStack   线程栈地址                                                               *
 *        (8)  bytes    线程栈大小，以字为单位                                                   *
 *        (9)  priority 线程优先级                                                               *
 *        (10) ticks    线程时间片长度                                                           *
 *        (11) pError   详细调用结果                                                             *
 *  返回：(1)  eFailure                                                                          *
 *        (2)  eSuccess                                                                          *
 *  说明：函数名的前缀'x'(eXtreme)表示本函数需要处理临界区代码                                   *
 *************************************************************************************************/
TState xThreadCreate(TThread* pThread, TChar* pName, TThreadStatus status, TProperty property,
                     TBitMask acapi, TThreadEntry pEntry, TArgument argument,
                     void* pStack, TBase32 bytes, TPriority priority, TTimeTick ticks,
                     TError* pError)
{
    TState state = eFailure;
    TError error = THREAD_ERR_FAULT;
    TReg32 imask;

    CpuEnterCritical(&imask);

    /* 只允许在线程代码里调用本函数 */
    if (uKernelVariable.State == eThreadState)
    {
        /* 检查线程是否已经被初始化 */
        if (!(pThread->Property &THREAD_PROP_READY))
        {
            uThreadCreate(pThread, pName, status, property,
                          acapi, pEntry, argument,
                          pStack, bytes,
                          priority, ticks);
            error = THREAD_ERR_NONE;
            state = eSuccess;
        }
    }

    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能：线程注销                                                                               *
 *  参数：(1) pThread 线程结构地址                                                               *
 *        (2) pError  详细调用结果                                                               *
 *  返回：(1) eFailure                                                                           *
 *        (2) eSuccess                                                                           *
 *  说明：IDLE线程、中断处理线程和定时器线程不能被注销                                           *
 *************************************************************************************************/
TState xThreadDelete(TThread* pThread, TError* pError)
{
    TState state = eFailure;
    TError error = THREAD_ERR_FAULT;
    TReg32 imask;

    CpuEnterCritical(&imask);

    /* 只允许在线程代码里调用本函数 */
    if (uKernelVariable.State == eThreadState)
    {
        /* 如果没有给出被操作的线程地址，则强制使用当前线程 */
        if (pThread == (TThread*)0)
        {
            pThread = uKernelVariable.CurrentThread;
        }

        /* 检查线程是否已经被初始化 */
        if (pThread->Property &THREAD_PROP_READY)
        {
            /* 检查线程是否接收相关API调用 */
            if (pThread->ACAPI &THREAD_ACAPI_DELETE)
            {
                state = uThreadDelete(pThread, &error);
            }
            else
            {
                error = THREAD_ERR_ACAPI;
            }
        }
        else
        {
            error = THREAD_ERR_UNREADY;
        }
    }

    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能：更改线程优先级                                                                         *
 *  参数：(1) pThread  线程结构地址                                                              *
 *        (2) priority 线程优先级                                                                *
 *        (3) pError   详细调用结果                                                              *
 *  返回：(1) eFailure 更改线程优先级失败                                                        *
 *        (2) eSuccess 更改线程优先级成功                                                        *
 *  说明：(1) 如果是临时修改优先级，则不修改线程结构的基本优先级数据                             *
 *        (2) 互斥量实施优先级继承协议的时候不受AUTHORITY控制                                    *
 *************************************************************************************************/
TState xThreadSetPriority(TThread* pThread, TPriority priority, TError* pError)
{
    TState state = eFailure;
    TError error = THREAD_ERR_FAULT;
    TBool HiRP = eFalse;
    TReg32 imask;

    CpuEnterCritical(&imask);

    /* 只允许在线程代码里调用本函数 */
    if (uKernelVariable.State == eThreadState)
    {
        /* 如果没有给出被操作的线程地址，则强制使用当前线程 */
        if (pThread == (TThread*)0)
        {
            pThread = uKernelVariable.CurrentThread;
        }

        /* 检查线程是否已经被初始化 */
        if (pThread->Property &THREAD_PROP_READY)
        {
            /* 检查线程是否接收相关API调用 */
            if (pThread->ACAPI &THREAD_ACAPI_PRIORITY)
            {
                if ((!(pThread->Property & THREAD_PROP_PRIORITY_FIXED)) &&
                        (pThread->Property & THREAD_PROP_PRIORITY_SAFE))
                {
                    state = uThreadSetPriority(pThread, priority, eTrue, &HiRP, &error);
                    if ((uKernelVariable.SchedLockTimes == 0U) && (HiRP == eTrue))
                    {
                        uThreadSchedule();
                    }
                }
                else
                {
                    error = THREAD_ERR_PRIORITY;
                }
            }
            else
            {
                error = THREAD_ERR_ACAPI;
            }
        }
        else
        {
            error = THREAD_ERR_UNREADY;
        }
    }

    CpuLeaveCritical(imask);
    *pError = error;
    return state;

}


/*************************************************************************************************
 *  功能：修改线程时间片长度                                                                     *
 *  参数：(1) pThread 线程结构地址                                                               *
 *        (2) slice   线程时间片长度                                                             *
 *        (3) pError  详细调用结果                                                               *
 *  返回：(1) eSuccess                                                                           *
 *        (2) eFailure                                                                           *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState xThreadSetTimeSlice(TThread* pThread, TTimeTick ticks, TError* pError)
{
    TState state = eFailure;
    TError error = THREAD_ERR_FAULT;
    TReg32 imask;

    CpuEnterCritical(&imask);

    /* 只允许在线程代码里调用本函数 */
    if (uKernelVariable.State == eThreadState)
    {
        /* 如果没有给出被操作的线程地址，则强制使用当前线程 */
        if (pThread == (TThread*)0)
        {
            pThread = uKernelVariable.CurrentThread;
        }

        /* 检查线程是否已经被初始化 */
        if (pThread->Property &THREAD_PROP_READY)
        {
            /* 检查线程是否接收相关API调用 */
            if (pThread->ACAPI &THREAD_ACAPI_SLICE)
            {
                /* 调整线程时间片长度 */
                if (pThread->BaseTicks > ticks)
                {
                    pThread->Ticks = (pThread->Ticks < ticks) ? (pThread->Ticks): ticks;
                }
                else
                {
                    pThread->Ticks += (ticks - pThread->BaseTicks);
                }
                pThread->BaseTicks = ticks;

                error = THREAD_ERR_NONE;
                state = eSuccess;
            }
            else
            {
                error = THREAD_ERR_ACAPI;
            }
        }
        else
        {
            error = THREAD_ERR_UNREADY;
        }
    }

    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}



/*************************************************************************************************
 *  功能：线程级线程调度函数，当前线程主动让出处理器(保持就绪状态)                               *
 *  参数：(1) pError    详细调用结果                                                             *
 *  返回：(1) eFailure                                                                           *
 *        (2) eSuccess                                                                           *
 *  说明：因为不能破坏最高就绪优先级占用处理器的原则，                                           *
 *        所以Yield操作只能在拥有最高就绪优先级的线程之间操作                                    *
 *************************************************************************************************/
TState xThreadYield(TError* pError)
{
    TState state = eFailure;
    TError error = THREAD_ERR_FAULT;
    TReg32 imask;
    TThread* pThread;

    CpuEnterCritical(&imask);

    /* 只能在线程环境下才能调用本函数 */
    if (uKernelVariable.State == eThreadState)
    {
        /* 操作目标是当前线程 */
        pThread = uKernelVariable.CurrentThread;

        /* 检查线程是否已经被初始化 */
        if (pThread->Property &THREAD_PROP_READY)
        {
            /* 检查线程是否接收相关API调用 */
            if (pThread->ACAPI &THREAD_ACAPI_YIELD)
            {
                /* 只能在内核允许线程调度的条件下才能调用本函数 */
                if (uKernelVariable.SchedLockTimes == 0U)
                {
                    /*
                     * 调整当前线程所在队列的头指针
                     * 当前线程所在线程队列也可能只有当前线程唯一1个线程
                     */
                    ThreadReadyQueue.Handle[pThread->Priority] =
                        (ThreadReadyQueue.Handle[pThread->Priority])->Next;
                    pThread->Status = eThreadReady;
                    uThreadSchedule();
                    error = THREAD_ERR_NONE;
                    state = eSuccess;
                }
            }
            else
            {
                error = THREAD_ERR_ACAPI;
            }
        }
        else
        {
            error = THREAD_ERR_UNREADY;
        }
    }
    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能：线程终止，使得线程不再参与内核调度                                                     *
 *  参数：(1) pThread 线程结构地址                                                               *
 *        (2) pError  详细调用结果                                                               *
 *  返回：(1) eFailure                                                                           *
 *        (2) eSuccess                                                                           *
 *  说明：(1) 初始化线程和定时器线程不能被休眠                                                   *
 *************************************************************************************************/
TState xThreadDeactivate(TThread* pThread, TError* pError)
{
    TState state = eFailure;
    TError error = THREAD_ERR_FAULT;
    TBool HiRP = eFalse;
    TReg32 imask;

    CpuEnterCritical(&imask);

    /* 只允许在线程代码里调用本函数 */
    if (uKernelVariable.State == eThreadState)
    {
        /* 如果没有给出被操作的线程地址，则强制使用当前线程 */
        if (pThread == (TThread*)0)
        {
            pThread = uKernelVariable.CurrentThread;
        }

        /* 检查线程是否已经被初始化 */
        if (pThread->Property &THREAD_PROP_READY)
        {
            /* 检查线程是否接收相关API调用 */
            if (pThread->ACAPI &THREAD_ACAPI_DEACTIVATE)
            {
                state = SetThreadUnready(pThread, eThreadDormant, 0U, &HiRP, &error);
                if (HiRP == eTrue)
                {
                    uThreadSchedule();
                }
            }
            else
            {
                error = THREAD_ERR_ACAPI;
            }
        }
        else
        {
            error = THREAD_ERR_UNREADY;
        }
    }

    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能：激活线程，使得线程能够参与内核调度                                                     *
 *  参数：(1) pThread  线程结构地址                                                              *
 *        (2) pError   详细调用结果                                                              *
 *  返回：(1) eFailure                                                                           *
 *        (2) eSuccess                                                                           *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState xThreadActivate(TThread* pThread, TError* pError)
{
    TState state = eFailure;
    TError error = THREAD_ERR_FAULT;
    TBool HiRP = eFalse;
    TReg32 imask;

    CpuEnterCritical(&imask);

    /* 只允许在线程代码里调用本函数 */
    if (uKernelVariable.State == eThreadState)
    {
        /* 检查线程是否已经被初始化 */
        if (pThread->Property &THREAD_PROP_READY)
        {
            /* 检查线程是否接收相关API调用 */
            if (pThread->ACAPI &THREAD_ACAPI_ACTIVATE)
            {
                state = SetThreadReady(pThread, eThreadDormant, &HiRP, &error);
                if ((uKernelVariable.SchedLockTimes == 0U) && (HiRP == eTrue))
                {
                    uThreadSchedule();
                }
            }
            else
            {
                error = THREAD_ERR_ACAPI;
            }
        }
        else
        {
            error = THREAD_ERR_UNREADY;
        }
    }

    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能：线程挂起函数                                                                           *
 *  参数：(1) pThread 线程结构地址                                                               *
 *        (2) pError  详细调用结果                                                               *
 *  返回：(1) eFailure                                                                           *
 *        (2) eSuccess                                                                           *
 *  说明：(1) 内核初始化线程不能被挂起                                                           *
 *************************************************************************************************/
TState xThreadSuspend(TThread* pThread, TError* pError)
{
    TState state = eFailure;
    TError error = THREAD_ERR_FAULT;
    TBool HiRP = eFalse;
    TReg32 imask;

    CpuEnterCritical(&imask);

    /* 只允许在线程代码里调用本函数 */
    if (uKernelVariable.State == eThreadState)
    {
        /* 如果没有给出被操作的线程地址，则强制使用当前线程 */
        if (pThread == (TThread*)0)
        {
            pThread = uKernelVariable.CurrentThread;
        }

        /* 检查线程是否已经被初始化 */
        if (pThread->Property &THREAD_PROP_READY)
        {
            /* 检查线程是否接收相关API调用 */
            if (pThread->ACAPI &THREAD_ACAPI_SUSPEND)
            {
                state = SetThreadUnready(pThread, eThreadSuspended, 0U, &HiRP, &error);
                if (HiRP == eTrue)
                {
                    uThreadSchedule();
                }
            }
            else
            {
                error = THREAD_ERR_ACAPI;
            }
        }
        else
        {
            error = THREAD_ERR_UNREADY;
        }
    }

    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能：线程解挂函数                                                                           *
 *  参数：(1) pThread 线程结构地址                                                               *
 *        (2) pError  详细调用结果                                                               *
 *  返回：(1) eFailure                                                                           *
 *        (2) eSuccess                                                                           *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState xThreadResume(TThread* pThread, TError* pError)
{
    TState state = eFailure;
    TError error = THREAD_ERR_FAULT;
    TBool HiRP = eFalse;
    TReg32 imask;

    CpuEnterCritical(&imask);

    /* 只允许在线程代码里调用本函数 */
    if (uKernelVariable.State == eThreadState)
    {
        /* 检查线程是否已经被初始化 */
        if (pThread->Property &THREAD_PROP_READY)
        {
            /* 检查线程是否接收相关API调用 */
            if (pThread->ACAPI &THREAD_ACAPI_RESUME)
            {
                state = SetThreadReady(pThread, eThreadSuspended, &HiRP, &error);
                if ((uKernelVariable.SchedLockTimes == 0U) && (HiRP == eTrue))
                {
                    uThreadSchedule();
                }
            }
            else
            {
                error = THREAD_ERR_ACAPI;
            }
        }
        else
        {
            error = THREAD_ERR_UNREADY;
        }
    }

    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能：线程延时模块接口函数                                                                   *
 *  参数：(1) pThread 线程结构地址                                                               *
 *        (2) ticks   需要延时的滴答数目                                                         *
 *        (3) pError  详细调用结果                                                               *
 *  返回：(1) eSuccess                                                                           *
 *        (2) eFailure                                                                           *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState xThreadDelay(TTimeTick ticks, TError* pError)
{
    TState state = eFailure;
    TError error = THREAD_ERR_FAULT;
    TBool HiRP = eFalse;
    TReg32 imask;
    TThread* pThread;

    CpuEnterCritical(&imask);

    /* 只允许在线程代码里调用本函数 */
    if (uKernelVariable.State == eThreadState)
    {
        /* 强制使用当前线程 */
        pThread = uKernelVariable.CurrentThread;

        /* 检查线程是否已经被初始化 */
        if (pThread->Property &THREAD_PROP_READY)
        {
            /* 检查线程是否接收相关API调用 */
            if (pThread->ACAPI &THREAD_ACAPI_DELAY)
            {
                state = SetThreadUnready(pThread, eThreadDelayed, ticks, &HiRP, &error);
                if (HiRP == eTrue)
                {
                    uThreadSchedule();
                }
            }
            else
            {
                error = THREAD_ERR_ACAPI;
            }
        }
        else
        {
            error = THREAD_ERR_UNREADY;
        }
    }
    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能：线程延时取消函数                                                                       *
 *  参数：(1) pThread 线程结构地址                                                               *
 *        (2) pError  详细调用结果                                                               *
 *  返回：(1) eSuccess                                                                           *
 *        (2) eFailure                                                                           *
 *  说明：(1) 这个函数对以时限等待方式阻塞在IPC线程阻塞队列上的线程无效                          *
 *************************************************************************************************/
TState xThreadUndelay(TThread* pThread, TError* pError)
{
    TState state = eFailure;
    TError error = THREAD_ERR_FAULT;
    TBool HiRP = eFalse;
    TReg32 imask;

    CpuEnterCritical(&imask);

    /* 只允许在线程代码里调用本函数 */
    if (uKernelVariable.State == eThreadState)
    {
        /* 检查线程是否已经被初始化 */
        if (pThread->Property &THREAD_PROP_READY)
        {
            /* 检查线程是否接收相关API调用 */
            if (pThread->ACAPI &THREAD_ACAPI_UNDELAY)
            {
                state = SetThreadReady(pThread, eThreadDelayed, &HiRP, &error);
                if ((uKernelVariable.SchedLockTimes == 0U) && (HiRP == eTrue))
                {
                    uThreadSchedule();
                }
            }
            else
            {
                error = THREAD_ERR_ACAPI;
            }
        }
        else
        {
            error = THREAD_ERR_UNREADY;
        }
    }
    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}

#if (TCLC_IPC_ENABLE)
/*************************************************************************************************
 *  功能：解除线程阻塞函数                                                                       *
 *  参数：(1) pThread 线程结构地址                                                               *
 *        (2) pError  详细调用结果                                                               *
 *  返回：(1) eFailure                                                                           *
 *        (2) eSuccess                                                                           *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState xThreadUnblock(TThread* pThread, TError* pError)
{
    TState state = eFailure;
    TError error = THREAD_ERR_FAULT;
    TBool HiRP = eFalse;
    TReg32 imask;

    CpuEnterCritical(&imask);

    /* 只允许在线程代码里调用本函数 */
    if (uKernelVariable.State == eThreadState)
    {
        /* 检查线程是否已经被初始化 */
        if (pThread->Property &THREAD_PROP_READY)
        {
            /* 检查线程是否接收相关API调用 */
            if (pThread->ACAPI &THREAD_ACAPI_UNBLOCK)
            {
                if (pThread->Status == eThreadBlocked)
                {
                    /*
                     * 将阻塞队列上的指定阻塞线程释放
                     * 在线程环境下，如果当前线程的优先级已经不再是线程就绪队列的最高优先级，
                     * 并且内核此时并没有关闭线程调度，那么就需要进行一次线程抢占
                     */
                    uIpcUnblockThread(pThread->IpcContext, eFailure, IPC_ERR_ABORT, &HiRP);
                    if ((uKernelVariable.SchedLockTimes == 0U) && (HiRP == eTrue))
                    {
                        uThreadSchedule();
                    }
                    error = THREAD_ERR_NONE;
                    state = eSuccess;
                }
                else
                {
                    error = THREAD_ERR_STATUS;
                }
            }
            else
            {
                error = THREAD_ERR_ACAPI;
            }
        }
    }
    CpuLeaveCritical(imask);

    *pError = error;
    return state;
}
#endif

