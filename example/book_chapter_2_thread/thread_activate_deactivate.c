/*  线程激活和休眠API演示 */

#include "trochili.h"
#include "example.h"

#if (EVB_EXAMPLE == CH2_THREAD_EXAMPLE2)

/* 用户线程参数 */
#define THREAD_LED_STACK_BYTES   (512)
#define THREAD_LED_PRIORITY      (5)
#define THREAD_LED_SLICE         (20)

#define THREAD_CTRL_STACK_BYTES  (512)
#define THREAD_CTRL_PRIORITY     (4)
#define THREAD_CTRL_SLICE        (20)

/* 用户线程定义 */
static TThread ThreadLedOn;
static TThread ThreadLedOff;
static TThread ThreadCTRL;

/* 用户线程栈 */
static TBase32 ThreadLedOnStack[THREAD_LED_STACK_BYTES/4];
static TBase32 ThreadLedOffStack[THREAD_LED_STACK_BYTES/4];
static TBase32 ThreadCTRLStack[THREAD_CTRL_STACK_BYTES/4];

/* 线程LedOn的线程函数 */
static void ThreadLedOnEntry(TArgument data)
{
    while (eTrue)
    {
        EvbLedControl(LED1, LED_ON);
    }
}

/* 线程LedOff的线程函数 */
static void ThreadLedOffEntry(TArgument data)
{
    while (eTrue)
    {
        EvbLedControl(LED1, LED_OFF);
    }
}

/* 线程CTRL的线程函数 */
static void ThreadCtrlEntry(TArgument data)
{
    TError error;
    TState state;

    while (eTrue)
    {
        /* 激活Led设备点亮线程 */
        state = TclActivateThread(&ThreadLedOn, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

        /* 控制线程延时1秒 */
        state = TclDelayThread(&ThreadCTRL, TCLM_SEC2TICKS(1), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

        /* 休眠Led设备点亮线程 */
        state = TclDeactivateThread(&ThreadLedOn, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

        /* 激活Led设备熄灭线程 */
        state = TclActivateThread(&ThreadLedOff, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

        /* 控制线程延时1秒 */
        state = TclDelayThread(&ThreadCTRL, TCLM_SEC2TICKS(1), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

        /* 休眠Led设备熄灭线程 */
        state = TclDeactivateThread(&ThreadLedOff, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");
    }
}


/* 用户应用入口函数 */
static void AppSetupEntry(void)
{
    TError error;
    TState state;
    /* 初始化Led设备点亮线程 */
    state = TclCreateThread(&ThreadLedOn,
                          &ThreadLedOnEntry, (TArgument)0,
                          ThreadLedOnStack, THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY, THREAD_LED_SLICE,
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 初始化Led设备熄灭线程 */
    state = TclCreateThread(&ThreadLedOff,
                          &ThreadLedOffEntry, (TArgument)0,
                          ThreadLedOffStack, THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY, THREAD_LED_SLICE,
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 初始化CTRL线程 */
    state = TclCreateThread(&ThreadCTRL,
                          &ThreadCtrlEntry, (TArgument)0,
                          ThreadCTRLStack, THREAD_CTRL_STACK_BYTES,
                          THREAD_CTRL_PRIORITY, THREAD_CTRL_SLICE,
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 激活CTRL线程 */
    state = TclActivateThread(&ThreadCTRL, &error);
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

