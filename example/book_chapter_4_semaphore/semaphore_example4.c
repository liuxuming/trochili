#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH4_SEMAPHORE_EXAMPLE4)

#define THREAD_SYNC_ENABLE (1)

/* 用户线程参数 */
#define THREAD_UART_STACK_BYTES  (512)
#define THREAD_UART_PRIORITY     (5)
#define THREAD_UART_SLICE        (100)

/* 用户线程栈定义 */
static TBase32 ThreadUartLowCaseStack[THREAD_UART_STACK_BYTES/4];
static TBase32 ThreadUartUpCaseStack[THREAD_UART_STACK_BYTES/4];

/* 用户线程定义 */
static TThread ThreadUartLowCase;
static TThread ThreadUartUpCase;

/* 用户信号量定义 */
static TSemaphore UartSemaphore;

/* 字符打印函数，通过BSP将字符串输出到评估板的串口 */
static void PrintfString(char* str)
{
    TState state;
    TError error;

    /* 当前线程以阻塞方式获取信号量，成功后通过BSP打印字符串 */
#if (THREAD_SYNC_ENABLE)
    state = TclObtainSemaphore(&UartSemaphore, TCLO_IPC_WAIT, 0, &error);
    if (state == eSuccess)
    {
        TclTrace(str);
    }
    /* 字符串完全打印后，当前线程以非阻塞方式释放信号量 */
    TclReleaseSemaphore(&UartSemaphore, 0, 0, &error);
#else
    error = error;
    state = state;
    TclTrace(str);
#endif
}

/* 打印大写字符串线程的主函数 */
static void ThreadUartLowCaseEntry(TArgument arg)
{
    while (eTrue)
    {
        PrintfString("ABCDDEFGHIJK\r\n");
    }
}

/* 打印小写字符串线程的主函数 */
static void ThreadUartUpCaseEntry(TArgument arg)
{
    while (eTrue)
    {
        PrintfString("abcdefghijk\r\n");
    }
}


/* 用户应用程序入口函数 */
static void AppSetupEntry(void)
{
    TState state;
    TError error;

    /* 初始化信号量 */
    state = TclCreateSemaphore(&UartSemaphore, 1, 1, TCLP_IPC_DUMMY, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IPC_NONE), "");

    /* 初始化UART设备控制线程 */
    state = TclCreateThread(&ThreadUartLowCase,
                          &ThreadUartLowCaseEntry, (TArgument)0,
                          ThreadUartLowCaseStack, THREAD_UART_STACK_BYTES,
                          THREAD_UART_PRIORITY, THREAD_UART_SLICE, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 初始化UART设备控制线程 */
    state = TclCreateThread(&ThreadUartUpCase,
                          &ThreadUartUpCaseEntry, (TArgument)0,
                          ThreadUartUpCaseStack, THREAD_UART_STACK_BYTES,
                          THREAD_UART_PRIORITY, THREAD_UART_SLICE, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 激活UART小写线程 */
    state = TclActivateThread(&ThreadUartLowCase, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 激活UART大写线程 */
    state = TclActivateThread(&ThreadUartUpCase, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");
}


/* 处理器BOOT之后会调用main函数，必须提供 */
int main(void)
{
    /* 注册各个内核函数,启动内核 */
    TclStartKernel(&AppSetupEntry,
                   &CpuSetupEntry,
                   &EvbSetupEntry,
                   &EvbTraceEntry);
    return 1;
}


#endif
