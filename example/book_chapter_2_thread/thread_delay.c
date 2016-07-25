/*  线程延时API演示 */

#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH2_THREAD_EXAMPLE4)

/* 创建线程时需要的参数 */
#define THREAD_LED_STACK_BYTES  (512)
#define THREAD_LED_PRIORITY     (5)
#define THREAD_LED_SLICE        (65535)

/* 线程定义、线程栈定义 */
static TThread ThreadLed;
static TBase32 ThreadLedStack[THREAD_LED_STACK_BYTES/4];

/* Led线程的线程主函数 */
static void ThreadLedEntry(TArgument data)
{
    TError error;
    TState state;
    TThread* pThread;

    /* 参数2为线程的内核参数，由内核自动设置为当前线程的地址 */
    pThread = (TThread*)data;
    while (eTrue)
    {
        EvbLedControl(LED1, LED_ON);
        state = TclDelayThread(pThread, TCLM_SEC2TICKS(1), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

        EvbLedControl(LED1, LED_OFF);
        state = TclDelayThread(pThread, TCLM_SEC2TICKS(1), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");
    }
}

/* 用户应用入口函数 */
static void AppSetupEntry(void)
{
    TError error;
    TState state;

    /* 初始化Led设备控制线程 */
    state = TclCreateThread(&ThreadLed,
                          &ThreadLedEntry,
                          (TArgument)(&ThreadLed),
                          ThreadLedStack,
                          THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY,
                          THREAD_LED_SLICE,
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 激活Led设备控制线程 */
    state = TclActivateThread(&ThreadLed, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");
}


/* 处理器BOOT之后会调用main函数，必须提供 */
int main(void)
{
    /* 注册各个内核函数，启动内核 */
    TclStartKernel(&AppSetupEntry,
                   &CpuSetupEntry,
                   &EvbSetupEntry,
                   &EvbTraceEntry);

    return 1;
}

#endif


