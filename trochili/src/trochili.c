/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#include "trochili.h"


/*************************************************************************************************
 *  功能：内核启动API                                                                            *
 *  参数：(1) pUserEntry  应用初始化函数                                                         *
 *        (2) pCpuEntry   处理器初始化函数                                                       *
 *        (3) pBoardEntry 板级设备初始化函数                                                     *
 *        (4) pTraceEntry 调试输出函数                                                           *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void TclStartKernel(TUserEntry pUserEntry,
                    TCpuSetupEntry pCpuEntry,
                    TBoardSetupEntry pBoardEntry,
                    TTraceEntry pTraceEntry)
{
    KNL_ASSERT((pUserEntry  != (TUserEntry)0), "");
    KNL_ASSERT((pCpuEntry   != (TCpuSetupEntry)0), "");
    KNL_ASSERT((pBoardEntry != (TBoardSetupEntry)0), "");
    KNL_ASSERT((pTraceEntry != (TTraceEntry)0), "");

    xKernelStart(pUserEntry, pCpuEntry, pBoardEntry, pTraceEntry);
}


/*************************************************************************************************
 *  功能：设置系统Idle函数供IDLE线程调用                                                         *
 *  参数：(1) pEntry 系统Idle函数                                                                *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void TclSetSysIdleEntry(TSysIdleEntry pEntry)
{
    KNL_ASSERT((pEntry != (TSysIdleEntry)0), "");
    xKernelSetIdleEntry(pEntry);
}


/*************************************************************************************************
 *  功能：设置系统Fault函数                                                                      *
 *  参数：(1) pEntry 系统Fault函数                                                               *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void TclSetSysFaultEntry(TSysFaultEntry pEntry)
{
    KNL_ASSERT((pEntry != (TSysFaultEntry)0), "");
    xKernelSetFaultEntry(pEntry);
}


/*************************************************************************************************
 *  功能：获得系统当前线程指针                                                                   *
 *  参数：(1) pThread2 返回当前线程指针                                                          *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void TclGetCurrentThread(TThread** pThread2)
{
    KNL_ASSERT((pThread2 != (TThread**)0), "");
    xKernelGetCurrentThread(pThread2);
}


/*************************************************************************************************
 *  功能：获得系统已运行时钟节拍数                                                               *
 *  参数：(1) pJiffies 保存系统已运行的时钟节拍数                                                *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void TclGetTimeJiffies(TTimeTick* pJiffies)
{
    KNL_ASSERT((pJiffies != (TTimeTick*)0), "");
    xKernelGetJiffies(pJiffies);
}


/*************************************************************************************************
 *  功能：获得系统已运行时间戳                                                                   *
 *  参数：(1) pStamp 以微秒为单位的时间戳数值                                                    *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void TclGetTimeStamp(TTimeStamp* pStamp)
{
    TTimeTick jiffies;
    KNL_ASSERT((pStamp != (TTimeStamp*)0), "");

    xKernelGetJiffies(&jiffies);
    *pStamp = (jiffies * 1000U * 1000U) / TCLC_TIME_TICK_RATE;
}


#if (TCLC_IRQ_ENABLE)
/*************************************************************************************************
 *  功能：设置中断向量函数                                                                       *
 *  参数：(1) irqn     中断号                                                                    *
 *        (2) pISR     ISR处理函数                                                               *
 *        (3) pDAEMON     中断处理线程                                                              *
 *        (4) data     应用提供的回调数据                                                        *
 *        (5) pError   详细调用结果                                                              *
 *  返回: (1) eFailure 操作失败                                                                  *
 *        (2) eSuccess 操作成功                                                                  *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclSetIrqVector(TIndex irqn, TISR pISR, TArgument data, TError* pError)
{
    TState state;
    KNL_ASSERT((irqn < TCLC_CPU_IRQ_NUM), "");
    KNL_ASSERT((pISR != (TISR)0), "");

    state = xIrqSetVector(irqn, pISR,  data, pError);
    return state;
}


/*************************************************************************************************
 *  功能：清空中断向量API函数                                                                    *
 *  参数：(1) irqn 中断编号                                                                      *
 *  返回: (1) eFailure 操作失败                                                                  *
 *        (2) eSuccess 操作成功                                                                  *
 *  说明：内核启动时，会调用本函数                                                               *
 *************************************************************************************************/
TState TclCleanIrqVector(TIndex irqn, TError* pError)
{
    TState state;
    KNL_ASSERT((irqn < TCLC_CPU_IRQ_NUM), "");

    state = xIrqCleanVector(irqn, pError);
    return state;
}


#endif

/*************************************************************************************************
 *  功能：使能线程调度API                                                                        *
 *  参数：无                                                                                     *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclUnlockScheduler(void)
{
    TState state;
    state = xKernelUnlockSched();
    return state;
}


/*************************************************************************************************
 *  功能：禁止线程调度API                                                                        *
 *  参数：无                                                                                     *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclLockScheduler(void)
{
    TState state;
    state = xKernelLockSched();
    return state;
}

/*************************************************************************************************
 *  功能：板级字符串打印函数                                                                     *
 *  参数：(1) pStr 待打印的字符串                                                                *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void TclTrace(const char* pNote)
{
    /* 必要的参数检查 */
    KNL_ASSERT((pNote != (char*)0), "");

    xKernelTrace(pNote);
}

/*************************************************************************************************
 *  功能：线程结构初始化API                                                                      *
 *  参数：(1) pThread  线程结构地址                                                              *
 *        (2) pEntry   线程函数地址                                                              *
 *        (3) argument 线程参数地址(整形数表示)                                                  *
 *        (4) pStack   线程栈地址                                                                *
 *        (5) bytes    线程栈大小，以字节为单位                                                  *
 *        (6) priority 线程优先级                                                                *
 *        (7) ticks    线程时间片长度                                                            *
 *        (8) pError   详细调用结果                                                              *
 *  返回：(1) eFailure                                                                           *
 *        (2) eSuccess                                                                           *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclCreateThread(TThread*     pThread,
                       TChar*       pName,
                       TThreadEntry pEntry,
                       TArgument    data,
                       void*        pStack,
                       TBase32      bytes,
                       TPriority    priority,
                       TTimeTick    ticks,
                       TError*      pError)
{
    TState state;

    /* 必要的参数检查 */
    KNL_ASSERT((pThread != (TThread*)0), "");
    KNL_ASSERT((pName != (TChar*)0), "");
    KNL_ASSERT((pEntry != (void*)0), "");
    KNL_ASSERT((pStack != (void*)0), "");
    KNL_ASSERT((bytes > 0U), "");
    KNL_ASSERT((priority <= TCLC_USER_PRIORITY_LOW), "");
    KNL_ASSERT((priority >= TCLC_USER_PRIORITY_HIGH), "");
    KNL_ASSERT((ticks > 0U), "");

    state = xThreadCreate(pThread,
                          pName,
                          eThreadDormant,
                          THREAD_PROP_PRIORITY_SAFE,
                          THREAD_ACAPI_ALL,
                          pEntry,
                          data,
                          pStack,
                          bytes,
                          priority,
                          ticks,
                          pError);

    return state;
}


/*************************************************************************************************
 *  功能：当前线程注销                                                                           *
 *  参数：(1) pThread 线程结构地址                                                               *
 *  返回：(1) eFailure                                                                           *
 *        (2) eSuccess                                                                           *
 *  说明：初始化线程和定时器线程不能被休眠                                                       *
 *************************************************************************************************/
TState TclDeleteThread(TThread* pThread, TError* pError)
{
    TState state;

    /* 必要的参数检查 */
    KNL_ASSERT((pError != (TError*)0), "");

    state = xThreadDelete(pThread, pError);
    return state;
}


/*************************************************************************************************
 *  功能：线程激活API                                                                            *
 *  参数：(1) pThread 线程结构地址                                                               *
 *  返回：参考线程相关返回值定义                                                                 *
 *  说明：激活线程，使得线程能够参与内核调度                                                     *
 *************************************************************************************************/
TState TclActivateThread(TThread* pThread, TError* pError)
{
    TState state;
    KNL_ASSERT((pThread != (TThread*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xThreadActivate(pThread, pError);
    return state;
}


/*************************************************************************************************
 *  功能：线程休眠API                                                                            *
 *  参数：(1) thread 线程结构地址                                                                *
 *  返回：参考线程相关返回值定义                                                                 *
 *  说明：初始化线程和定时器线程不能被休眠                                                       *
 *************************************************************************************************/
TState TclDeactivateThread(TThread* pThread, TError* pError)
{
    TState state;
    KNL_ASSERT((pError != (TError*)0), "");

    state = xThreadDeactivate(pThread, pError);
    return state;
}


/*************************************************************************************************
 *  功能：线程挂起API                                                                            *
 *  参数：(1) pThread 线程结构地址                                                               *
 *  返回：参考线程相关返回值定义                                                                 *
 *  说明：内核初始化线程不能被挂起                                                               *
 *************************************************************************************************/
TState TclSuspendThread(TThread* pThread, TError* pError)
{
    TState state;
    KNL_ASSERT((pError != (TError*)0), "");

    state = xThreadSuspend(pThread, pError);
    return state;
}


/*************************************************************************************************
 *  功能：线程恢复API                                                                            *
 *  参数：(1) pThread 线程结构地址                                                               *
 *  返回：参考线程相关返回值定义                                                                 *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclResumeThread(TThread* pThread, TError* pError)
{
    TState state;
    KNL_ASSERT((pThread != (TThread*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xThreadResume(pThread, pError);
    return state;
}


/*************************************************************************************************
 *  功能：线程级线程调度API                                                                      *
 *  参数：无                                                                                     *
 *  返回：参考线程相关返回值定义                                                                 *
 *  说明：如果内核关闭时间调度，只使用本函数来调度的话，就可以实现协作式任务调度内核             *
 *************************************************************************************************/
TState TclYieldThread(TError* pError)
{
    TState state = eFailure;
    KNL_ASSERT((pError != (TError*)0), "");

    state = xThreadYield(pError);
    return state;
}


/*************************************************************************************************
 *  功能：更改线程优先级API                                                                      *
 *  参数：(1) pThread 线程结构地址                                                               *
 *        (2) priority   线程优先级                                                              *
 *  返回：参考线程相关返回值定义                                                                 *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclSetThreadPriority(TThread* pThread, TPriority priority, TError* pError)
{
    TState state;
    KNL_ASSERT((priority < TCLC_PRIORITY_NUM), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xThreadSetPriority(pThread, priority, pError);
    return state;
}


/*************************************************************************************************
 *  功能：修改线程时间片长度API                                                                  *
 *  参数：(1) pThread 线程结构地址                                                               *
 *        (2) slice   线程时间片长度                                                             *
 *  返回：参考线程相关返回值定义                                                                 *
 *  说明：下次调度时才会生效                                                                     *
 *************************************************************************************************/
TState TclSetThreadSlice(TThread* pThread, TTimeTick ticks, TError* pError)
{
    TState state;
    KNL_ASSERT((ticks > 0U), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xThreadSetTimeSlice(pThread, ticks, pError);
    return state;
}


/*************************************************************************************************
 *  功能：线程延时模块接口函数                                                                   *
 *  参数：(1) pThread 线程结构地址                                                               *
 *        (2) ticks   需要延时的滴答数目                                                         *
 *  返回: (1) eSuccess   操作成功                                                                *
 *        (2) eFailure   操作失败                                                                *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclDelayThread(TTimeTick ticks, TError* pError)
{
    TState state;
    KNL_ASSERT((ticks > 0U), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xThreadDelay(ticks, pError);
    return state;
}


/*************************************************************************************************
 *  功能：线程延时取消API                                                                        *
 *  参数：(1) pThread 线程结构地址                                                               *
 *  返回：参考线程相关返回值定义                                                                 *
 *  说明：这个函数对以时限等待方式阻塞在IPC线程阻塞队列上的线程无效                              *
 *************************************************************************************************/
TState TclUnDelayThread(TThread* pThread, TError* pError)
{
    TState state;
    KNL_ASSERT((pThread != (TThread*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xThreadUndelay(pThread, pError);
    return state;
}


#if (TCLC_IPC_ENABLE)
/*************************************************************************************************
 *  功能：强制解除线程阻塞API                                                                    *
 *  参数：(1) pThread 线程结构地址                                                               *
 *  返回：参考线程相关返回值定义                                                                 *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclUnblockThread(TThread* pThread, TError* pError)
{
    TState state;
    KNL_ASSERT((pThread != (TThread*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xThreadUnblock(pThread, pError);
    return state;
}
#endif

#if ((TCLC_IPC_ENABLE)&&(TCLC_IPC_SEMAPHORE_ENABLE))
/*************************************************************************************************
 *  功能: 初始化计数信号量                                                                       *
 *  参数: (1) pSemaphore 计数信号量结构地址                                                      *
 *        (2) pName      计数信号量名称                                                          *
 *        (3) value      计数信号量初始值                                                        *
 *        (4) mvalue     计数信号量最大计数值                                                    *
 *        (5) property   信号量的初始属性                                                        *
 *        (6) pError     详细调用结果                                                            *
 *  返回: (1) eSuccess   操作成功                                                                *
 *        (2) eFailure   操作失败                                                                *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclCreateSemaphore(TSemaphore* pSemaphore, TChar* pName, TBase32 value, TBase32 mvalue,
                          TProperty property, TError* pError)
{
    TState state;
    KNL_ASSERT((pSemaphore != (TSemaphore*)0), "");
    KNL_ASSERT((pName != (TChar*)0), "");
    KNL_ASSERT((mvalue >= 1U), "");
    KNL_ASSERT((mvalue >= value), "");
    KNL_ASSERT((pError != (TError*)0), "");

    property &= IPC_USER_SEMAPHORE_PROP;
    state = xSemaphoreCreate(pSemaphore, pName, value, mvalue, property, pError);
    return state;
}


/*************************************************************************************************
 *  功能: 线程/ISR 获得信号量获得互斥信号量                                                      *
 *  参数: (1) pSemaphore 信号量结构地址                                                          *
 *        (2) option     访问信号量的的选项                                                      *
 *        (3) timeo      时限阻塞模式下访问信号量的时限长度                                      *
 *        (5) pError     详细调用结果                                                            *
 *  返回: (1) eSuccess   操作成功                                                                *
 *        (2) eFailure   操作失败                                                                *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclObtainSemaphore(TSemaphore* pSemaphore, TOption option, TTimeTick timeo,
                          TError* pError)
{
    TState state;
    KNL_ASSERT((pSemaphore != (TSemaphore*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");
    KNL_ASSERT((timeo < TCLM_MAX_VALUE32), "");

    /* 调整操作选项，屏蔽不需要支持的选项 */
    option &= IPC_USER_SEMAPHORE_OPTION;
    state = xSemaphoreObtain(pSemaphore, option, timeo, pError);

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
TState TclReleaseSemaphore(TSemaphore* pSemaphore, TOption option, TTimeTick timeo, TError* pError)
{
    TState state;
    KNL_ASSERT((pSemaphore != (TSemaphore*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");
    KNL_ASSERT((timeo < TCLM_MAX_VALUE32), "");

    /* 调整操作选项，屏蔽不需要支持的选项 */
    option &= IPC_USER_SEMAPHORE_OPTION;
    state = xSemaphoreRelease(pSemaphore, option, timeo, pError);
    return state;
}


/*************************************************************************************************
 *  功能: ISR释放信号量                                                                          *
 *  参数: (1) pSemaphore 信号量结构地址                                                          *
 *        (2) pError     详细调用结果                                                            *
 *  返回: (1) eSuccess   操作成功                                                                *
 *        (2) eFailure   操作失败                                                                *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclIsrReleaseSemaphore(TSemaphore* pSemaphore, TError* pError)
{
    TState state;
    KNL_ASSERT((pSemaphore != (TSemaphore*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xSemaphoreRelease(pSemaphore, (TOption)0, (TTimeTick)0, pError);
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
TState TclDeleteSemaphore(TSemaphore* pSemaphore, TError* pError)
{
    TState state;
    KNL_ASSERT((pSemaphore != (TSemaphore*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xSemaphoreDelete(pSemaphore, pError);
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
TState TclResetSemaphore(TSemaphore* pSemaphore, TError* pError)
{
    TState state;
    KNL_ASSERT((pSemaphore != (TSemaphore*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xSemaphoreReset(pSemaphore, pError);
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
TState TclFlushSemaphore(TSemaphore* pSemaphore, TError* pError)
{
    TState state;
    KNL_ASSERT((pSemaphore != (TSemaphore*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xSemaphoreFlush(pSemaphore, pError);
    return state;
}
#endif


#if ((TCLC_IPC_ENABLE)&&(TCLC_IPC_MUTEX_ENABLE))
/*************************************************************************************************
 *  功能: 初始化互斥量                                                                           *
 *  参数: (1) pMute    互斥量结构地址                                                            *
 *        (2) pName    互斥量的名称                                                              *
 *        (3) priority 互斥量的优先级天花板                                                      *
 *        (4) property 互斥量的初始属性                                                          *
 *        (5) pError   详细调用结果                                                              *
 *  返回: (1) eSuccess 操作成功                                                                  *
 *        (2) eFailure 操作失败                                                                  *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclCreateMutex(TMutex* pMutex, TChar* pName, TPriority priority, TProperty property, TError* pError)
{
    TState state;
    KNL_ASSERT((pMutex != (TMutex*)0), "");
    KNL_ASSERT((pName != (TChar*)0), "");
    KNL_ASSERT((priority < TCLC_LOWEST_PRIORITY), "");
    KNL_ASSERT((pError != (TError*)0), "");

    property &= IPC_USER_MUTEX_PROP;
    state = xMutexCreate(pMutex, pName, priority, property, pError);
    return state;
}


/*************************************************************************************************
 *  功能: 互斥量解除初始化                                                                       *
 *  参数: (1) pMutex   互斥量结构地址                                                            *
 *        (2) pError   详细调用结果                                                              *
 *  返回: (1) eSuccess 操作成功                                                                  *
 *        (2) eFailure 操作失败                                                                  *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclDeleteMutex(TMutex* pMutex, TError* pError)
{
    TState state;
    KNL_ASSERT((pMutex != (TMutex*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xMutexDelete(pMutex, pError);
    return state;
}


/*************************************************************************************************
 *  功能: 线程获得互斥互斥量                                                                     *
 *  参数: (1) pMutex 互斥量结构地址                                                              *
 *        (2) option   访问邮箱的模式                                                            *
 *        (3) timeo    时限阻塞模式下访问互斥量的时限长度                                        *
 *        (4) pError   详细调用结果                                                              *
 *  返回: (1) eSuccess 操作成功                                                                  *
 *        (2) eFailure 操作失败                                                                  *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclLockMutex(TMutex* pMutex, TOption option, TTimeTick timeo, TError* pError)
{
    TState state;
    KNL_ASSERT((pMutex != (TMutex*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");
    KNL_ASSERT((timeo < TCLM_MAX_VALUE32), "");

    /* 调整操作选项，屏蔽不需要支持的选项 */
    option &= IPC_USER_MUTEX_OPTION;
    state = xMutexLock(pMutex, option, timeo, pError);
    return state;
}


/*************************************************************************************************
 *  功能: 释放互斥互斥量                                                                         *
 *  参数: (1) pMutex   互斥量结构地址                                                            *
 *        (2) pError   详细调用结果                                                              *
 *  返回: (1) eSuccess 操作成功                                                                  *
 *        (2) eFailure 操作失败                                                                  *
 *  说明：mutex支持所有权的概念，所以线程释放mutex的操作都是立刻返回的,会释放mutex操作不会导致   *
 *        线程阻塞在mutex的线程阻塞队列上                                                        *
 *************************************************************************************************/
TState TclFreeMutex(TMutex* pMutex, TError* pError)
{
    TState state;
    KNL_ASSERT((pMutex != (TMutex*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xMutexFree(pMutex, pError);
    return state;
}


/*************************************************************************************************
 *  功能: 清除互斥量阻塞队列                                                                     *
 *  参数: (1) pMutex   互斥量结构地址                                                            *
 *        (2) pError   详细调用结果                                                              *
 *  返回: (1) eFailure                                                                           *
 *        (2) eSuccess                                                                           *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclResetMutex(TMutex* pMutex, TError* pError)
{
    TState state;
    KNL_ASSERT((pMutex != (TMutex*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xMutexReset(pMutex, pError);
    return state;
}


/*************************************************************************************************
 *  功能：互斥量阻塞终止函数,将指定的线程从互斥量的线程阻塞队列中终止阻塞并唤醒                  *
 *  参数：(1) pMutex  互斥量结构地址                                                             *
 *        (2) option     参数选项                                                                *
 *        (3) pThread 线程地址                                                                   *
 *        (4) pError  详细调用结果                                                               *
 *  返回：(1) eSuccess                                                                           *
 *        (2) eFailure                                                                           *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclFlushMutex(TMutex* pMutex, TError* pError)
{
    TState state;
    KNL_ASSERT((pMutex != (TMutex*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xMutexFlush(pMutex, pError);
    return state;
}
#endif


#if ((TCLC_IPC_ENABLE)&&(TCLC_IPC_MAILBOX_ENABLE))
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
TState TclCreateMailBox(TMailBox* pMailbox, TChar* pName, TProperty property, TError* pError)
{
    TState state;
    KNL_ASSERT((pMailbox != (TMailBox*)0), "");
    KNL_ASSERT((pName != (TChar*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    property &= IPC_USER_MBOX_PROP;
    state = xMailBoxCreate(pMailbox, pName, property, pError);
    return state;
}


/*************************************************************************************************
 *  功能：重置邮箱                                                                               *
 *  参数：(1) pMailbox   邮箱的地址                                                              *
 *        (2) pError     详细调用结果                                                            *
 *  返回: (1) eFailure   操作失败                                                                *
 *        (2) eSuccess   操作成功                                                                *
 *  说明：注意线程的等待结果都是TCLE_IPC_DELETE                                                   *
 *************************************************************************************************/
TState TclDeleteMailBox(TMailBox* pMailbox, TError* pError)
{
    TState state;
    KNL_ASSERT((pMailbox != (TMailBox*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xMailBoxDelete(pMailbox, pError);
    return state;
}


/*************************************************************************************************
 *  功能: 线程/ISR从邮箱中接收邮件                                                               *
 *  参数: (1) pMailbox 邮箱结构地址                                                              *
 *        (2) pMail2   保存邮件结构地址的指针变量                                                *
 *        (3) option   访问邮箱的模式                                                            *
 *        (4) timeo    时限阻塞模式下访问邮箱的时限长度                                          *
 *        (5) pError   详细调用结果                                                              *
 *  返回: (1) eFailure 操作失败                                                                  *
 *        (2) eSuccess 操作成功                                                                  *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclReceiveMail(TMailBox* pMailbox, TMail* pMail2, TOption option, TTimeTick timeo,
                      TError* pError)
{
    TState state;
    KNL_ASSERT((pMailbox != (TMailBox*)0), "");
    KNL_ASSERT((pMail2 != (TMail*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");
    KNL_ASSERT((timeo < TCLM_MAX_VALUE32), "");

    /* 调整操作选项，屏蔽不需要支持的选项 */
    option &= IPC_USER_MBOX_OPTION;
    state = xMailBoxReceive(pMailbox, pMail2, option, timeo, pError);
    return state;
}


/*************************************************************************************************
 *  功能: 线程/ISR发送邮件                                                                       *
 *  参数: (1) pMailbox 邮箱结构地址                                                              *
 *        (2) pMail2   保存邮件结构地址的指针变量                                                *
 *        (3) option   访问邮箱的模式                                                            *
 *        (4) timeo    时限阻塞模式下访问邮箱的时限长度                                          *
 *        (5) pError   详细调用结果                                                              *
 *  返回: (1) eFailure 操作失败                                                                  *
 *        (2) eSuccess 操作成功                                                                  *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclSendMail(TMailBox* pMailbox, TMail* pMail2, TOption option,
                   TTimeTick timeo, TError* pError)
{
    TState state;

    KNL_ASSERT((pMailbox != (TMailBox*)0), "");
    KNL_ASSERT((pMail2 != (TMail*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");
    KNL_ASSERT((timeo < TCLM_MAX_VALUE32), "");

    /* 调整操作选项，屏蔽不需要支持的选项 */
    option &= IPC_USER_MBOX_OPTION;
    state = xMailBoxSend(pMailbox, pMail2, option, timeo, pError);

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
TState TclIsrSendMail(TMailBox* pMailbox, TMail* pMail2, TOption option, TError* pError)
{
    TState state;
    KNL_ASSERT((pMailbox != (TMailBox*)0), "");
    KNL_ASSERT((pMail2 != (TMail*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    /* 调整操作选项，屏蔽不需要支持的选项 */
    option &= IPC_OPT_UARGENT;
    state = xMailBoxSend(pMailbox, pMail2, option, (TTimeTick)0, pError);
    return state;
}


/*************************************************************************************************
 *  功能：邮箱广播函数,向所有读阻塞队列中的线程广播邮件                                          *
 *  参数: (1) pMailbox  邮箱结构地址                                                             *
 *        (2) pMail2   保存邮件结构地址的指针变量                                                *
 *        (3) pError    详细调用结果                                                             *
 *  返回：(1) eSuccess  成功                                                                     *
 *        (2) eFailure  失败                                                                     *
 *  说明：只有邮箱的线程阻塞队列中存在读邮箱的线程的时候，才能把消息发送给队列中的线程           *
 *************************************************************************************************/
TState TclBroadcastMail(TMailBox* pMailbox, TMail* pMail2, TError* pError)
{
    TState state;
    KNL_ASSERT((pMailbox != (TMailBox*)0), "");
    KNL_ASSERT((pMail2 != (TMail*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xMailBoxBroadcast(pMailbox, pMail2, pError);
    return state;
}


/*************************************************************************************************
 *  功能: 清除邮箱阻塞队列                                                                       *
 *  参数: (1) pMailbox 邮箱结构地址                                                              *
 *        (2) pError   详细调用结果                                                              *
 *  返回: (1) eFailure                                                                           *
 *        (2) eSuccess                                                                           *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclResetMailBox(TMailBox* pMailbox, TError* pError)
{
    TState state;
    KNL_ASSERT((pMailbox != (TMailBox*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xMailboxReset(pMailbox, pError);
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
TState TclFlushMailBox(TMailBox* pMailbox, TError* pError)
{
    TState state;
    KNL_ASSERT((pMailbox != (TMailBox*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xMailBoxFlush(pMailbox, pError);
    return state;
}
#endif


#if ((TCLC_IPC_ENABLE)&&(TCLC_IPC_MQUE_ENABLE))
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
TState TclCreateMsgQueue(TMsgQueue* pMsgQue, TChar* pName, void** pPool2, TBase32 capacity, TProperty property,
                         TError* pError)
{
    TState state;
    KNL_ASSERT((pMsgQue != (TMsgQueue*)0), "");
    KNL_ASSERT((pName != (TChar*)0), "");
    KNL_ASSERT((pPool2 != (void*)0), "");
    KNL_ASSERT((capacity != 0U), "");
    KNL_ASSERT((pError != (TError*)0), "");

    property &= IPC_USER_MQUE_PROP;
    state = xMQCreate(pMsgQue, pName, pPool2, capacity, property, pError);
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
 *  返回: (2) eSuccess   操作成功                                                                *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclReceiveMessage(TMsgQueue* pMsgQue, TMessage* pMsg2, TOption option, TTimeTick timeo,
                         TError* pError)
{
    TState state;
    KNL_ASSERT((pMsgQue != (TMsgQueue*)0), "");
    KNL_ASSERT((pMsg2 != (TMessage*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");
    KNL_ASSERT((timeo < TCLM_MAX_VALUE32), "");

    /* 调整操作选项，屏蔽不需要支持的选项 */
    option &= IPC_USER_MSGQ_OPTION;
    state = xMQReceive(pMsgQue, pMsg2, option, timeo, pError);
    return state;
}


/*************************************************************************************************
 *  功能: 用于线程/ISR向消息队列中发送消息                                                       *
 *  参数: (1) pMsgQue  消息队列结构地址                                                          *
 *        (2) pMsg2    保存消息结构地址的指针变量                                                *
 *        (3) option   访问消息队列的模式                                                        *
 *        (4) timeo    时限阻塞模式下访问邮箱的时限长度                                          *
 *        (5) pError   详细调用结果                                                              *
 *  返回: (1) eFailure   操作失败                                                                *
 *  返回: (2) eSuccess   操作成功                                                                *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclSendMessage(TMsgQueue* pMsgQue, TMessage* pMsg2, TOption option, TTimeTick timeo,
                      TError* pError)
{
    TState state;
    KNL_ASSERT((pMsgQue != (TMsgQueue*)0), "");
    KNL_ASSERT((pMsg2 != (TMessage*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");
    KNL_ASSERT((timeo < TCLM_MAX_VALUE32), "");

    /* 调整操作选项，屏蔽不需要支持的选项 */
    option &= IPC_USER_MSGQ_OPTION;
    state = xMQSend(pMsgQue, pMsg2, option, timeo, pError);
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
    TState state;
    KNL_ASSERT((pMsgQue != (TMsgQueue*)0), "");
    KNL_ASSERT((pMsg2 != (TMessage*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    /* 调整操作选项，屏蔽不需要支持的选项 */
    option &= IPC_OPT_UARGENT;
    state = xMQSend(pMsgQue, pMsg2, option, (TTimeTick)0, pError);
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
    TState state;
    KNL_ASSERT((pMsgQue != (TMsgQueue*)0), "");
    KNL_ASSERT((pMsg2 != (TMessage*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xMQBroadcast(pMsgQue, pMsg2, pError);
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
    TState state;
    KNL_ASSERT((pMsgQue != (TMsgQueue*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xMQDelete(pMsgQue, pError);
    return state;
}


/*************************************************************************************************
 *  功能: 清除消息队列阻塞队列                                                                   *
 *  参数: (1) pMsgQue  消息队列结构地址                                                          *
 *        (2) pError   详细调用结果                                                              *
 *  返回: (1) eFailure                                                                           *
 *        (2) eSuccess                                                                           *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclResetMsgQueue(TMsgQueue* pMsgQue, TError* pError)
{
    TState state;
    KNL_ASSERT((pMsgQue != (TMsgQueue*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xMQReset(pMsgQue, pError);
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
    TState state;
    KNL_ASSERT((pMsgQue != (TMsgQueue*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xMQFlush(pMsgQue, pError);
    return state;
}
#endif


#if ((TCLC_IPC_ENABLE) && (TCLC_IPC_FLAGS_ENABLE))
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
    TState state;
    KNL_ASSERT((pFlags != (TFlags*)0), "");
    KNL_ASSERT((pName != (TChar*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    property &= IPC_USER_FLAG_PROP;
    state = xFlagsCreate(pFlags, pName, property, pError);
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
    TState state;
    KNL_ASSERT((pFlags != (TFlags*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xFlagsDelete(pFlags, pError);
    return state;
}


/*************************************************************************************************
 *  功能：线程/ISR向事件标记发送事件                                                             *
 *  参数：(1) pFlags   事件标记的地址                                                            *
 *        (2) pPattern 需要接收的标记的组合                                                      *
 *        (3) pError   详细调用结果                                                              *
 *  返回: (1) eFailure   操作失败                                                                *
 *        (2) eSuccess   操作成功                                                                *
 *  说明：本函数不会引起当前线程阻塞                                                             *
 *************************************************************************************************/
TState TclSendFlags(TFlags* pFlags, TBitMask pattern, TError* pError)
{
    TState state;
    KNL_ASSERT((pFlags != (TFlags*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xFlagsSend(pFlags, pattern, pError);
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
    TState state;

    KNL_ASSERT((pFlags != (TFlags*)0), "");
    KNL_ASSERT((option & (IPC_OPT_AND | IPC_OPT_OR)) != 0U, "");
    KNL_ASSERT((pError != (TError*)0), "");
    KNL_ASSERT((timeo < TCLM_MAX_VALUE32), "");

    /* 调整操作选项，屏蔽不需要支持的选项 */
    option &= IPC_USER_FLAG_OPTION;
    state = xFlagsReceive(pFlags, pPattern, option, timeo, pError);
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
    TState state;

    KNL_ASSERT((pFlags != (TFlags*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xFlagsReset(pFlags, pError);
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
    KNL_ASSERT((pFlags != (TFlags*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xFlagsFlush(pFlags, pError);
    return state;
}
#endif


#if (TCLC_TIMER_ENABLE)
/*************************************************************************************************
 *  功能：用户定时器初始化函数                                                                   *
 *  参数：(1) pTimer   定时器地址                                                                *
 *        (2) pName    定时器名称                                                                *
 *        (3) property 定时器属性                                                                *
 *        (4) ticks    定时器滴答数目                                                            *
 *        (5) pRoutine 用户定时器回调函数                                                        *
 *        (6) pData    用户定时器回调函数参数                                                    *
 *        (7) pError   详细调用结果                                                              *
 *        (8) priority 定时器优先级                                                              *
 *  返回: (1) eSuccess 操作成功                                                                  *
 *        (2) eFailure 操作失败                                                                  *
 *  说明                                                                                         *
 *************************************************************************************************/
TState TclCreateTimer(TTimer* pTimer, TChar* pName, TProperty property, TTimeTick ticks,
                      TTimerRoutine pRoutine, TArgument data, TPriority priority, TError* pError)
{
    TState state;
    KNL_ASSERT((pTimer != (TTimer*)0), "");
    KNL_ASSERT((pName != (TChar*)0), "");
    KNL_ASSERT((pRoutine != (TTimerRoutine)0), "");
    KNL_ASSERT((ticks > 0U), "");
    KNL_ASSERT((ticks < (TCLM_MAX_VALUE64 >> 1U)), "");
    KNL_ASSERT((pError != (TError*)0), "");

    property &= TIMER_USER_PROPERTY;
    state = xTimerCreate(pTimer, pName, property, ticks, pRoutine, data, priority, pError);
    return state;
}


/*************************************************************************************************
 *  功能：内核定时器取消初始化                                                                   *
 *  参数：(1) pTimer   定时器结构地址                                                            *
 *  返回: (1) eSuccess 操作成功                                                                  *
 *        (2) eFailure 操作失败                                                                  *
 *        (3) pError   详细调用结果                                                              *
 *  说明                                                                                         *
 *************************************************************************************************/
TState TclDeleteTimer(TTimer* pTimer, TError* pError)
{
    TState state;
    KNL_ASSERT((pTimer != (TTimer*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xTimerDelete(pTimer, pError);
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
TState TclStartTimer(TTimer* pTimer, TTimeTick lagticks, TError* pError)
{
    TState state;
    KNL_ASSERT((pTimer != (TTimer*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");
    KNL_ASSERT((lagticks < (TCLM_MAX_VALUE64 >> 1U)), "");

    state = xTimerStart(pTimer, lagticks, pError);
    return state;
}


/*************************************************************************************************
 *  功能：停止用户定时器函数                                                                     *
 *  参数：(1) pTimer   定时器地址                                                                *
 *  返回: (1) eSuccess 操作成功                                                                  *
 *        (2) eFailure 操作失败                                                                  *
 *        (3) pError   详细调用结果                                                              *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclStopTimer(TTimer* pTimer, TError* pError)
{
    TState state;
    KNL_ASSERT((pTimer != (TTimer*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xTimerStop(pTimer, pError);
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
TState TclConfigTimer(TTimer* pTimer, TTimeTick ticks, TPriority priority, TError* pError)
{
    TState state;
    KNL_ASSERT((pTimer != (TTimer*)0), "");
    KNL_ASSERT((ticks > 0U), "");
    KNL_ASSERT((ticks < TCLM_MAX_VALUE32), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xTimerConfig(pTimer, ticks, priority, pError);
    return state;
}
#endif


#if ((TCLC_IRQ_ENABLE) && (TCLC_IRQ_DAEMON_ENABLE))
/*************************************************************************************************
 *  功能：提交中断请求                                                                           *
 *  参数：(1) pIRQ      中断请求结构地址                                                         *
 *        (2) priority  中断请求优先级                                                           *
 *        (3) pEntry    中断处理回调函数                                                         *
 *        (4) data      中断处理回调参数                                                         *
 *  返回: (1) eFailure  操作失败                                                                 *
 *        (2) eSuccess  操作成功                                                                 *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclPostIRQ(TIrq* pIRQ, TPriority priority, TIrqEntry pEntry, TArgument data, TError* pError)
{
    TState state;
    KNL_ASSERT((pIRQ != (TIrq*)0), "");

    state = xIrqPostRequest(pIRQ, priority, pEntry, data, pError);
    return state;
}


/*************************************************************************************************
 *  功能：撤销中断请求                                                                           *
 *  参数：(1) TIrq      中断请求结构地址                                                         *
 *  返回: (1) eFailure  操作失败                                                                 *
 *        (2) eSuccess  操作成功                                                                 *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclCancelIRQ(TIrq* pIRQ, TError* pError)
{
    TState state;
    KNL_ASSERT((pIRQ != (TIrq*)0), "");

    state = xIrqCancelRequest(pIRQ, pError);
    return state;
}
#endif


#if ((TCLC_MEMORY_ENABLE) && (TCLC_MEMORY_POOL_ENABLE))
/*************************************************************************************************
 *  功能: 初始化内存页池                                                                         *
 *  参数: (1) pPool      内存页池结构地址                                                        *
 *        (2) pAddr      内存池数据区地址                                                        *
 *        (3) pages      内存池中内存页数目                                                      *
 *        (4) pgsize     内存页大小                                                              *
 *        (5) pError     详细调用结果                                                            *
 *  返回: (1) eSuccess   操作成功                                                                *
 *        (2) eFailure   操作失败                                                                *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclCreateMemoryPool(TMemPool* pPool, void* pAddr, TBase32 pages, TBase32 pgsize, TError* pError)
{
    TState state;
    KNL_ASSERT((pPool != (TMemPool*)0), "");
    KNL_ASSERT((pAddr != (void*)0), "");
    KNL_ASSERT((pages != 0U), "");
    KNL_ASSERT((pgsize > 0U), "");
    KNL_ASSERT((pages <= TCLC_MEMORY_POOL_PAGES), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xMemPoolCreate(pPool, pAddr, pages, pgsize, pError);
    return state;
}


/*************************************************************************************************
 *  功能: 销毁内存池                                                                             *
 *  参数: (1) pPool      内存池结构地址                                                          *
 *        (2) pError     详细调用结果                                                            *
 *  返回: (1) eSuccess   操作成功                                                                *
 *        (2) eFailure   操作失败                                                                *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclDeleteMemoryPool(TMemPool* pPool, TError* pError)
{
    TState state;
    KNL_ASSERT((pPool != (TMemPool*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xMemPoolDelete(pPool, pError);
    return state;
}


/*************************************************************************************************
 *  功能: 从内存池中申请分配内存                                                                 *
 *  参数: (1) pPool      内存池结构地址                                                          *
 *        (2) pObject2   保存申请到的内存的指针变量                                              *
 *        (3) pError     详细调用结果                                                            *
 *  返回: (1) eSuccess   操作成功                                                                *
 *        (2) eFailure   操作失败                                                                *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclMallocPoolMemory(TMemPool* pPool, void** pAddr2, TError* pError)
{
    TState state;
    KNL_ASSERT((pPool != (TMemPool*)0), "");
    KNL_ASSERT((pAddr2 != (void**)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xPoolMemMalloc(pPool, pAddr2, pError);
    return state;
}


/*************************************************************************************************
 *  功能: 向内存池中释放内存                                                                     *
 *  参数: (1) pPool      内存池结构地址                                                          *
 *        (2) pObject    待释放内存的地址                                                        *
 *        (3) pError     详细调用结果                                                            *
 *  返回: (1) eSuccess   操作成功                                                                *
 *        (2) eFailure   操作失败                                                                *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclFreePoolMemory(TMemPool* pPool, void* pAddr, TError* pError)
{
    TState state;
    KNL_ASSERT((pPool != (TMemPool*)0), "");
    KNL_ASSERT((pAddr != (void*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xPoolMemFree(pPool, pAddr, pError);
    return state;
}
#endif


#if ((TCLC_MEMORY_ENABLE) && (TCLC_MEMORY_BUDDY_ENABLE))
/*************************************************************************************************
 *  功能：初始化伙伴内存管理控制结构                                                             *
 *  参数：(1) pBuddy    伙伴系统分配器内存地址                                                   *
 *        (2) pAddr     可供分配的内存地址                                                       *
 *        (3) pagesize  内存页大小                                                               *
 *        (4) pages     可供分配的内存页总数                                                     *
 *        (5) pError    详细调用结果                                                             *
 *  返回: (1) eSuccess  操作成功                                                                 *
 *        (2) eFailure  操作失败                                                                 *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclCreateMemoryBuddy(TMemBuddy* pBuddy, TChar* pAddr, TBase32 pages, TBase32 pagesize, TError* pError)
{
    TState state;
    KNL_ASSERT((pBuddy != (TMemBuddy*)0), "");
    KNL_ASSERT((pAddr  != (TChar*)0), "");
    KNL_ASSERT((pages  > 0U), "");
    KNL_ASSERT((pages  <= TCLC_MEMORY_BUDDY_PAGES), "");
    KNL_ASSERT((pagesize > 0U), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xBuddyCreate(pBuddy, pAddr, pages, pagesize, pError);
    return state;
}


/*************************************************************************************************
 *  功能：销毁伙伴内存管理控制结构                                                               *
 *  参数：(1) pBuddy     伙伴系统分配器内存地址                                                  *
 *        (2) pError     详细调用结果                                                            *
 *  返回: (1) eSuccess   操作成功                                                                *
 *        (2) eFailure   操作失败                                                                *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclDeleteMemoryBuddy(TMemBuddy* pBuddy, TError* pError)
{
    TState state;
    KNL_ASSERT((pBuddy != (TMemBuddy*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xBuddyDelete(pBuddy, pError);
    return state;
}


/*************************************************************************************************
 *  功能：从伙伴系统中申请内存                                                                   *
 *  参数：(1) pBuddy    伙伴系统分配器对象地址                                                   *
 *        (2) len       需要分配的内存长度                                                       *
 *        (3) pAddr2    分配得到的内存地址指针                                                   *
 *        (4) pError    详细调用结果                                                             *
 *  返回: (1) eSuccess  操作成功                                                                 *
 *        (2) eFailure  操作失败                                                                 *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclMallocBuddyMemory(TMemBuddy* pBuddy, int len, void** pAddr2, TError* pError)
{
    TState state;
    KNL_ASSERT((pBuddy != (TMemBuddy*)0), "");
    KNL_ASSERT((len > 0U), "");
    KNL_ASSERT((pAddr2 != (void**)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xBuddyMemMalloc(pBuddy, len, pAddr2, pError);
    return state;
}


/*************************************************************************************************
 *  功能：向伙伴系统中释放内存                                                                   *
 *  参数：(1) pBuddy    伙伴系统分配器对象地址                                                   *
 *        (2) pAddr     待释放的内存地址                                                         *
 *        (3) pError    详细调用结果                                                             *
 *  返回: (1) eSuccess  操作成功                                                                 *
 *        (2) eFailure  操作失败                                                                 *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclFreeBuddyMemory(TMemBuddy* pBuddy,  void* pAddr, TError* pError)
{
    TState state;
    KNL_ASSERT((pBuddy != (TMemBuddy*)0), "");
    KNL_ASSERT((pAddr  != (char*)0), "");
    KNL_ASSERT((pError != (TError*)0), "");

    state = xBuddyMemFree(pBuddy, pAddr, pError);
    return state;
}
#endif

