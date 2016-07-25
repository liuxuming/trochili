#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH13_BOARD_TEST_EXAMPLE)

/* 用户线程参数 */
#define THREAD_LED_STACK_BYTES         (512)
#define THREAD_LED_PRIORITY            (5)
#define THREAD_LED_SLICE               (20)


/* 用户线程栈定义 */
static TBase32 ThreadLedStack[THREAD_LED_STACK_BYTES/4];

/* 用户线程定义 */
static TThread ThreadLed;

/* 用户定时器结构 */
static TTimer LedTimer1;
static TTimer LedTimer2;

/* 用户定时器1的回调函数，间隔1秒，点亮或熄灭Led1 */
static void Blink1(TArgument data)
{
    static int index = 0;

    if (index % 2)
    {
        EvbLedControl(LED1, LED_OFF);
        TclTrace("LED1 OFF\r\n");
    }
    else
    {
        EvbLedControl(LED1, LED_ON);
        TclTrace("LED1 ON\r\n");
    }
    index++;
}

/* 用户定时器1的回调函数，间隔1秒，点亮或熄灭Led1 */
static void Blink2(TArgument data)
{
    static int index = 0;

    if (index % 2)
    {
        EvbLedControl(LED2, LED_OFF);
        TclTrace("LED2 OFF\r\n");
    }
    else
    {
        EvbLedControl(LED2, LED_ON);
        TclTrace("LED2 ON\r\n");
    }
    index++;
}

/* 评估板按键中断处理函数 */
static TBitMask EvbKeyISR(TArgument data)
{
    TState state;
    TError error;

    TBase32 keyid;
    static TBase32 index1 = 1;
    static TBase32 index2 = 1;

    keyid = EvbKeyScan();
    if (keyid == 1)
    {
        if (index1 % 2)
        {
            /* 启动用户定时器 */
            state = TclStartTimer(&LedTimer1, 0U, &error);
            TCLM_ASSERT((state == eSuccess), "");
            TCLM_ASSERT((error == TCLE_TIMER_NONE), "");
        }
        else
        {
            /* 关闭用户定时器 */
            state = TclStopTimer(&LedTimer1, &error);
            TCLM_ASSERT((state == eSuccess), "");
            TCLM_ASSERT((error == TCLE_TIMER_NONE), "");
        }
        index1++;
    }
    else
    {
        if (index2 % 2)
        {
            /* 启动用户定时器 */
            state = TclStartTimer(&LedTimer2, 0U, &error);
            TCLM_ASSERT((state == eSuccess), "");
            TCLM_ASSERT((error == TCLE_TIMER_NONE), "");
        }
        else
        {
            /* 关闭用户定时器 */
            state = TclStopTimer(&LedTimer2, &error);
            TCLM_ASSERT((state == eSuccess), "");
            TCLM_ASSERT((error == TCLE_TIMER_NONE), "");
        }
        index2++;
    }

    return TCLR_IRQ_DONE;
}


/* Led线程的主函数 */
static void ThreadLedEntry(TArgument data)
{
    TState state;
    TError error;

    while (eTrue)
    {
        state = TclDelayThread(0, TCLM_MLS2TICKS(1000), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");
        EvbLedControl(LED3, LED_ON);
        TclTrace("LED3 ON\r\n");

        state = TclDelayThread(0, TCLM_MLS2TICKS(1000), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");
        EvbLedControl(LED3, LED_OFF);
        TclTrace("LED3 OFF\r\n");
    }
}


/* 用户应用程序入口函数 */
static void AppSetupEntry(void)
{
    TState state;
    TError error;

    /* 设置和KEY相关的外部中断向量 */
    state = TclSetIrqVector(KEY_IRQ_ID, &EvbKeyISR, (TThread*)0, (TArgument)0, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IRQ_NONE), "");

    /* 初始化用户定时器 */
    state = TclCreateTimer(&LedTimer1, TCLP_TIMER_PERIODIC,
                         TCLM_MLS2TICKS(1000), &Blink1, (TArgument)0, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_TIMER_NONE), "");

    /* 初始化用户定时器 */
    state = TclCreateTimer(&LedTimer2, TCLP_TIMER_PERIODIC,
                         TCLM_MLS2TICKS(1000), &Blink2, (TArgument)0, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_TIMER_NONE), "");

    /* 初始化Led线程 */
    state = TclCreateThread(&ThreadLed,
                          &ThreadLedEntry, (TArgument)0,
                          ThreadLedStack, THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY, THREAD_LED_SLICE,
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 激活Led线程 */
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
