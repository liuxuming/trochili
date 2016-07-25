/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#include "string.h"

#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.cpu.h"
#include "tcl.thread.h"
#include "tcl.timer.h"
#include "tcl.debug.h"
#include "tcl.kernel.h"

/* 内核关键参数集合 */
TKernelVariable uKernelVariable;

static void CreateRootThread(void);

/*************************************************************************************************
 *  功能：板级字符串打印函数                                                                     *
 *  参数：(1) pStr 待打印的字符串                                                                *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void uKernelTrace(const char* pStr)
{
    if (uKernelVariable.TraceEntry != (TTraceEntry)0)
    {
        uKernelVariable.TraceEntry(pStr);
    }
}


/*************************************************************************************************
 *  功能：板级字符串打印函数                                                                     *
 *  参数：(1) pStr 待打印的字符串                                                                *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void xKernelTrace(const char* pNote)
{
    TReg32 imask;

    CpuEnterCritical(&imask);
    if (uKernelVariable.TraceEntry != (TTraceEntry)0)
    {
        uKernelVariable.TraceEntry(pNote);
    }
    CpuLeaveCritical(imask);
}


/*************************************************************************************************
 *  功能：内核进入中断处理程序                                                                   *
 *  参数：无                                                                                     *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void xKernelEnterIntrState(void)
{
    TReg32 imask;
    CpuEnterCritical(&imask);

    uKernelVariable.IntrNestTimes++;
    uKernelVariable.State = eIntrState;

    CpuLeaveCritical(imask);
}


/*************************************************************************************************
 *  功能：内核退出中断处理程序                                                                   *
 *  参数：无                                                                                     *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void xKernelLeaveIntrState(void)
{
    TReg32 imask;

    CpuEnterCritical(&imask);

    KNL_ASSERT((uKernelVariable.IntrNestTimes > 0U), "");
    uKernelVariable.IntrNestTimes--;
    if (uKernelVariable.IntrNestTimes == 0U)
    {
        /* 
         * 如果还有其它中断标记在挂起或激活，说明当前中断是最高优先级中断，虽然没有发生嵌套，
         * 但是返回后将进入低级别的中断，所以这种情况不必要进行任务切换，理应留给最后一个并且是
         * 最低级别的那个中断在退出中断时来完成。
         * 此处的线程调度体现的是"抢占" 
         */
        if (uKernelVariable.Schedulable == eTrue)
        {
            uThreadSchedule();
        }
        uKernelVariable.State = eThreadState;
    }

    CpuLeaveCritical(imask);
}


/*************************************************************************************************
 *  功能：时钟节拍中断处理函数                                                                   *
 *  参数：无                                                                                     *
 *  返回：无                                                                                     *
 *  说明：定时器处理函数首先运行，随后进行线程调度处理                                           *
 *************************************************************************************************/
void xKernelTickISR(void)
{
    TReg32 imask;

    CpuEnterCritical(&imask);

    uKernelVariable.Jiffies++;

#if (TCLC_TIMER_ENABLE)
    uTimerTickISR();
#endif

    uThreadTickISR();

    CpuLeaveCritical(imask);
}


/*************************************************************************************************
 *  功能：关闭任务调度功能                                                                       *
 *  参数：无                                                                                     *
 *  返回：无                                                                                     *
 *  说明：本函数只能被线程调用                                                                   *
 *************************************************************************************************/
TState xKernelLockSched(void)
{
    TState state = eFailure;
    TReg32 imask;

    CpuEnterCritical(&imask);
    if (uKernelVariable.State == eThreadState)
    {
        uKernelVariable.Schedulable = eFalse;
        state = eSuccess;
    }
    CpuLeaveCritical(imask);
    return state;
}


/*************************************************************************************************
 *  功能：开启任务调度功能                                                                       *
 *  参数：无                                                                                     *
 *  返回：无                                                                                     *
 *  说明：本函数只能被线程调用                                                                   *
 *************************************************************************************************/
TState xKernelUnlockSched(void)
{
    TState state = eFailure;
    TReg32 imask;

    CpuEnterCritical(&imask);
    if (uKernelVariable.State == eThreadState)
    {
        uKernelVariable.Schedulable = eTrue;
        state = eSuccess;
    }
    CpuLeaveCritical(imask);
    return state;
}


/*************************************************************************************************
 *  功能：设置系统Idle函数供IDLE线程调用                                                         *
 *  参数：(1) pEntry 系统Idle函数                                                                *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void xKernelSetIdleEntry(TSysIdleEntry pEntry)
{
    TReg32 imask;

    CpuEnterCritical(&imask);
    uKernelVariable.SysIdleEntry = pEntry;
    CpuLeaveCritical(imask);
}


/*************************************************************************************************
 *  功能：获得系统当前线程指针                                                                   *
 *  参数：(1) pThread2 返回当前线程指针                                                          *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void xKernelGetCurrentThread(TThread** pThread2)
{
    TReg32 imask;

    CpuEnterCritical(&imask);
    *pThread2 = uKernelVariable.CurrentThread;
    CpuLeaveCritical(imask);
}


/*************************************************************************************************
 *  功能：获得系统已运行时钟节拍数                                                               *
 *  参数：(1) pJiffies 返回系统已运行时钟节拍数                                                  *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void xKernelGetJiffies(TTimeTick* pJiffies)
{
    TReg32 imask;

    CpuEnterCritical(&imask);
    *pJiffies = uKernelVariable.Jiffies;
    CpuLeaveCritical(imask);
}


/*************************************************************************************************
 *  功能：内核启动函数                                                                           *
 *  参数：(1) pUserEntry  应用初始化函数                                                         *
 *        (2) pCpuEntry   处理器初始化函数                                                       *
 *        (3) pBoardEntry 板级设备初始化函数                                                     *
 *        (4) pTraceEntry 调试输出函数                                                           *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void xKernelStart(TUserEntry pUserEntry,
                  TCpuSetupEntry pCpuEntry,
                  TBoardSetupEntry pBoardEntry,
                  TTraceEntry pTraceEntry)
{
    /* 关闭处理器中断 */
    CpuDisableInt();

    /* 初始化基本内核参数 */
    memset(&uKernelVariable, 0, sizeof(uKernelVariable));
    uKernelVariable.UserEntry       = pUserEntry;
    uKernelVariable.CpuSetupEntry   = pCpuEntry;
    uKernelVariable.BoardSetupEntry = pBoardEntry;
    uKernelVariable.TraceEntry      = pTraceEntry;
    uKernelVariable.Schedulable     = eTrue;

    /* 初始化所有内核模块 */
    uThreadModuleInit();                    /* 初始化线程管理模块           */
#if (TCLC_TIMER_ENABLE)
    uTimerModuleInit();                     /* 初始化定时器模块             */
#endif
#if (TCLC_IRQ_ENABLE)
    uIrqModuleInit();                       /* 初始化中断管理模块           */
#endif

    CreateRootThread();                     /* 初始化内核ROOT线程并且激活   */
#if ((TCLC_TIMER_ENABLE) && (TCLC_TIMER_DAEMON_ENABLE))
    uTimerCreateDaemon();                   /* 初始化内核定时器线程并且激活 */
#endif
#if ((TCLC_IRQ_ENABLE) && (TCLC_IRQ_DAEMON_ENABLE))
    uIrqCreateDaemon();                     /* 初始化内核IRQ线程并且激活    */
#endif

    uKernelVariable.CpuSetupEntry();        /* 调用处理器和板级初始化函数   */
    uKernelVariable.BoardSetupEntry();      /* 调用处理器和板级初始化函数   */

    CpuLoadRootThread();                    /* 启动内核ROOT线程             */

    /* 打开处理器中断 */
    CpuEnableInt();

    /* 
     * 本段代码应该永远不会被执行，若运行到此，说明移植时出现问题。
     * 这里的循环代码起到兜底作用，避免处理器进入非正常状态
     */
    while (eTrue)
    {
        uDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }
}


/* 内核ROOT线程定义和栈定义 */
static TThread RootThread;
static TBase32 RootThreadStack[TCLC_ROOT_THREAD_STACK_BYTES >> 2];

/* 内核ROOT线程不接受任何线程管理API操作 */
#define THREAD_ACAPI_ROOT (THREAD_ACAPI_NONE)

/*************************************************************************************************
 *  功能：内核ROOT线程函数                                                                       *
 *  参数：(1) argument 线程的参数                                                                *
 *  返回：无                                                                                     *
 *  说明：该函数首先开启多任务机制，然后调度其它线程运行                                         *
 *        注意线程栈容量大小的问题，这个线程函数不要做太多工作                                   *
 *************************************************************************************************/
static void xRootThreadEntry(TBase32 argument)
{
    /* 关闭处理器中断 */
    CpuDisableInt();

    /* 标记内核进入多线程模式 */
    uKernelVariable.State = eThreadState;

    /* 临时关闭线程调度功能 */
    uKernelVariable.Schedulable = eFalse;

    /* 
     * 调用用户入口函数，初始化用户程序。
     * 该函数运行在eThreadState,但是禁止Schedulable的状态下
     */
    if(uKernelVariable.UserEntry == (TUserEntry)0)
    {
        uDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }
    uKernelVariable.UserEntry();

    /* 开启线程调度功能 */
    uKernelVariable.Schedulable = eTrue;

    /* 打开系统时钟节拍 */
    CpuStartTickClock();

    /* 打开处理器中断 */
    CpuEnableInt();

    /* 调用IDLE Hook函数，此时多线程机制已经打开 */
    while (eTrue)
    {
        if (uKernelVariable.SysIdleEntry != (TSysIdleEntry)0)
        {
            uKernelVariable.SysIdleEntry();
        }
    }
}


/*************************************************************************************************
 *  功能：用来创建内核ROOT线程                                                                   *
 *  参数：无                                                                                     *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
static void CreateRootThread(void)
{
    /* 检查内核是否处于初始状态 */
    if(uKernelVariable.State != eOriginState)
    {
        uDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }

    /* 初始化内核ROOT线程 */
    uThreadCreate(&RootThread,
                  eThreadReady,
                  THREAD_PROP_PRIORITY_FIXED|\
                  THREAD_PROP_CLEAN_STACK|\
                  THREAD_PROP_ROOT,
                  THREAD_ACAPI_ROOT,
                  xRootThreadEntry,
                  (TArgument)0,
                  (void*)RootThreadStack,
                  (TBase32)TCLC_ROOT_THREAD_STACK_BYTES,
                  (TPriority)TCLC_ROOT_THREAD_PRIORITY,
                  (TTimeTick)TCLC_ROOT_THREAD_SLICE);

    /* 初始化相关的内核变量 */
    uKernelVariable.RootThread    = &RootThread;
    uKernelVariable.NomineeThread = &RootThread;
    uKernelVariable.CurrentThread = &RootThread;
}
