#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH8_FLAGS_EXAMPLE3)

/* 用户线程参数 */
#define THREAD_LED_STACK_BYTES  (512)
#define THREAD_LED_PRIORITY     (5)
#define THREAD_LED_SLICE        (20)

#define THREAD_CTRL_STACK_BYTES (512)
#define THREAD_CTRL_PRIORITY    (4)
#define THREAD_CTRL_SLICE       (20)

/* 用户线程定义 */
static TThread ThreadLED1;
static TThread ThreadLed2;
static TThread ThreadLed3;
static TThread ThreadCTRL;

/* 用户线程栈定义 */
static TBase32 ThreadLedStack1[THREAD_LED_STACK_BYTES/4];
static TBase32 ThreadLedStack2[THREAD_LED_STACK_BYTES/4];
static TBase32 ThreadLedStack3[THREAD_LED_STACK_BYTES/4];
static TBase32 ThreadCTRLStack[THREAD_CTRL_STACK_BYTES/4];

/* 用户事件标记定义 */
static TFlags LedFlags;

/* Led线程0的主函数 */
static void ThreadLED1Entry(TArgument data)
{
    TState state;
    TError error;
    TBitMask pattern;
    TOption option;

    while (eTrue)
    {
        /* Led线程以阻塞方式获取事件，当发现事件标记flush后点亮Led */
        pattern = 0xa0;
        option = TCLO_IPC_WAIT | TCLO_IPC_OR | TCLO_IPC_CONSUME;
        state = TclReceiveFlags(&LedFlags, &pattern, option, 0, &error);
        if ((state != eSuccess) && (error & TCLE_IPC_RESET))
        {
            EvbLedControl(LED1, LED_ON);
        }

        /* Led线程以阻塞方式获取事件，当发现事件标记flush后熄灭Led */
        pattern = 0xa0;
        option = TCLO_IPC_WAIT | TCLO_IPC_OR | TCLO_IPC_CONSUME;
        state = TclReceiveFlags(&LedFlags, &pattern, option, 0, &error);
        if ((state != eSuccess) && (error & TCLE_IPC_RESET))
        {
            EvbLedControl(LED1, LED_OFF);
        }
    }
}

/* Led2线程的主函数 */
static void ThreadLed2Entry(TArgument data)
{
    TState state;
    TError error;
    TBitMask pattern;
    TOption option;

    while (eTrue)
    {
        /* Led线程以阻塞方式获取事件，当发现事件标记flush后点亮Led */
        pattern = 0xa1;
        option = TCLO_IPC_WAIT | TCLO_IPC_OR | TCLO_IPC_CONSUME;
        state = TclReceiveFlags(&LedFlags, &pattern, option, 0, &error);
        if ((state != eSuccess) && (error & TCLE_IPC_RESET))
        {
            EvbLedControl(LED2, LED_ON);
        }

        /* Led线程以阻塞方式获取事件，当发现事件标记flush后熄灭Led */
        pattern = 0xa1;
        option = TCLO_IPC_WAIT | TCLO_IPC_OR | TCLO_IPC_CONSUME;
        state = TclReceiveFlags(&LedFlags, &pattern, option, 0, &error);
        if ((state != eSuccess) && (error & TCLE_IPC_RESET))
        {
            EvbLedControl(LED2, LED_OFF);
        }
    }
}

/* Led2线程的主函数 */
static void ThreadLed3Entry(TArgument data)
{
    TState state;
    TError error;
    TBitMask pattern;
    TOption option;

    while (eTrue)
    {
        /* Led线程以阻塞方式获取事件，当发现事件标记flush后点亮Led */
        pattern = 0xa2;
        option = TCLO_IPC_WAIT | TCLO_IPC_OR | TCLO_IPC_CONSUME;
        state = TclReceiveFlags(&LedFlags, &pattern, option, 0, &error);
        if ((state != eSuccess) && (error & TCLE_IPC_RESET))
        {
            EvbLedControl(LED3, LED_ON);
        }

        /* Led线程以阻塞方式获取事件，当发现事件标记flush后熄灭Led */
        pattern = 0xa2;
        option = TCLO_IPC_WAIT | TCLO_IPC_OR | TCLO_IPC_CONSUME;
        state = TclReceiveFlags(&LedFlags, &pattern, option, 0, &error);
        if ((state != eSuccess) && (error & TCLE_IPC_RESET))
        {
            EvbLedControl(LED3, LED_OFF);
        }
    }
}



/* CTRL线程的主函数 */
static void ThreadCtrlEntry(TArgument data)
{
    TState state;
    TError error;

    while (eTrue)
    {
        /* CTRL线程延时1秒后FLUSH事件标记 */
        state = TclDelayThread(&ThreadCTRL, TCLM_MLS2TICKS(1000), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

        state = TclResetFlags(&LedFlags, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");
    }
}


/* 用户应用入口函数 */
static void AppSetupEntry(void)
{
    TState state;
    TError error;

    /* 初始化事件标记 */
    state = TclCreateFlags(&LedFlags, TCLP_IPC_DUMMY, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IPC_NONE), "");

    /* 初始化LED1设备线程 */
    state = TclCreateThread(&ThreadLED1,
                          &ThreadLED1Entry, (TArgument)0,
                          ThreadLedStack1, THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY, THREAD_LED_SLICE,
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 初始化Led2设备线程 */
    state = TclCreateThread(&ThreadLed2,
                          &ThreadLed2Entry, (TArgument)0,
                          ThreadLedStack2, THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY, THREAD_LED_SLICE,
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 初始化Led3设备线程 */
    state = TclCreateThread(&ThreadLed3,
                          &ThreadLed3Entry, (TArgument)0,
                          ThreadLedStack3, THREAD_LED_STACK_BYTES,
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

    /* 激活Led线程1 */
    state = TclActivateThread(&ThreadLED1, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 激活Led线程2 */
    state = TclActivateThread(&ThreadLed2, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 激活Led线程3 */
    state = TclActivateThread(&ThreadLed3, &error);
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
