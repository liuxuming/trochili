/*  用户级线程调度演示 */

#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH2_THREAD_EXAMPLE5)

/* 创建线程时需要的参数 */
#define THREAD_LED_STACK_BYTES  (512)
#define THREAD_LED_PRIORITY     (5)
#define THREAD_LED_SLICE        (0xffffffff)

/* 线程定义 */
  TThread ThreadLedOn;
  TThread ThreadLedOff;
/* 线程栈定义 */
static TBase32 ThreadLedOnStack[THREAD_LED_STACK_BYTES/4];
static TBase32 ThreadLedOffStack[THREAD_LED_STACK_BYTES/4];


/* 线程做无用操作，起到空转效果 */
static delay(TBase32 count)
{
    while(count --);
}


/* 线程LedOn的线程函数 */
static void ThreadLedOnEntry(TArgument data)
{
    TError error;
    TState state;

    while (eTrue)
    {
        EvbLedControl(LED1, LED_ON);
        delay(0x8FFFFF);
        state = TclYieldThread(&error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");
    }
}


/* 线程LedOff的线程函数 */
static void ThreadLedOffEntry(TArgument data)
{
    TError error;
    TState state;

    while (eTrue)
    {
        EvbLedControl(LED1, LED_OFF);
        delay(0x8FFFFF);
        state = TclYieldThread(&error);
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
    state = TclCreateThread(&ThreadLedOn,
                          &ThreadLedOnEntry, (TArgument)0,
                          ThreadLedOnStack, THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY, THREAD_LED_SLICE,
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 初始化Led设备控制线程 */
    state = TclCreateThread(&ThreadLedOff,
                          &ThreadLedOffEntry, (TArgument)0,
                          ThreadLedOffStack, THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY, THREAD_LED_SLICE,
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 激活Led设备控制线程 */
    state = TclActivateThread(&ThreadLedOn, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 激活Led设备控制线程 */
    state = TclActivateThread(&ThreadLedOff, &error);
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


