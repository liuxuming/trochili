#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH9_TIMER_CONFIG_EXAMPLE)

/* 创建线程时需要的参数 */
#define THREAD_LED_STACK_BYTES  (512)
#define THREAD_LED_PRIORITY     (5)
#define THREAD_LED_SLICE        (32U)

/* Led线程结构和栈 */
static TThread ThreadLed;
static TBase32 ThreadLedStack[THREAD_LED_STACK_BYTES/4];

/* 用户定时器结构 */
static TTimer Led1Timer;
static TTimer Led2Timer;

static TTimeTick ticks1 = 500;
static TTimeTick ticks2 = 500;

/* 用户定时器1的回调函数，间隔1秒，点亮或熄灭Led1 */
static void BlinkLed1(TArgument data)
{
    static TIndex index = 0;
    if (index % 2)
    {
        EvbLedControl(LED1, LED_OFF);
    }
    else
    {
        EvbLedControl(LED1, LED_ON);
    }
    index++;
}

/* 用户定时器2的回调函数，间隔1秒，点亮或熄灭Led2 */
static void BlinkLed2(TArgument data)
{
    static TIndex index = 0;
    if (index % 2)
    {
        EvbLedControl(LED2, LED_OFF);
    }
    else
    {
        EvbLedControl(LED2, LED_ON);
    }
    index++;
}


/* 评估板按键中断处理函数 */
static TBitMask EvbKeyISR(TArgument data)
{
    TState state;
    TError error;

    TBase32 keyid;

  	keyid = EvbKeyScan();
    if (keyid == 1)
    {
        /* 关闭用户定时器 */
        state = TclStopTimer(&Led1Timer, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_TIMER_NONE), "");

        ticks1 *= 2;
        ticks1 = (ticks1 > 2000)? 2000:ticks1;

        /* 配置用户定时器 */
        state = TclConfigTimer(&Led1Timer, TCLM_MLS2TICKS(ticks1), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_TIMER_NONE), "");

        /* 启动用户定时器 */
        state = TclStartTimer(&Led1Timer, 0U, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_TIMER_NONE), "");

    }
    else
    {
        /* 关闭用户定时器 */
        state = TclStopTimer(&Led1Timer, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_TIMER_NONE), "");

        ticks1 /= 2;
        ticks1 = (ticks1 < 500)? 500:ticks1;

        /* 配置用户定时器 */
        state = TclConfigTimer(&Led1Timer, TCLM_MLS2TICKS(ticks1), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_TIMER_NONE), "");

        /* 启动用户定时器 */
        state = TclStartTimer(&Led1Timer, 0U, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_TIMER_NONE), "");
    }

    return TCLR_IRQ_DONE;
}


/* Led线程主函数 */
static void ThreadLedEntry(TArgument data)
{
    TState state;
    TError error;

    /* 初始化用户定时器1 */
    state = TclCreateTimer(&Led1Timer,
                           TCLP_TIMER_PERIODIC,
                           TCLM_MLS2TICKS(ticks1),
                           &BlinkLed1,
                           (TArgument)0,
                           &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_TIMER_NONE), "");

    /* 初始化用户定时器2 */
    state = TclCreateTimer(&Led2Timer,
                           TCLP_TIMER_PERIODIC,
                           TCLM_MLS2TICKS(ticks2),
                           &BlinkLed2,
                           (TArgument)0,
                           &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_TIMER_NONE), "");


    /* 启动用户定时器1 */
    state = TclStartTimer(&Led1Timer, TCLM_MLS2TICKS(500), &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_TIMER_NONE), "");

    /* 启动用户定时器2 */
    state = TclStartTimer(&Led2Timer, TCLM_MLS2TICKS(500), &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_TIMER_NONE), "");


    /* 设置和KEY相关的外部中断向量 */
    state = TclSetIrqVector(KEY_IRQ_ID, &EvbKeyISR, (TThread*)0, (TArgument)0, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IRQ_NONE), "");

    while (eTrue)
    {
    }
}


/* 用户应用入口函数 */
static void AppSetupEntry(void)
{
    TState state;
    TError error;

    /* 初始化Led设备控制线程 */
    state = TclCreateThread(&ThreadLed,
                            &ThreadLedEntry,
                            (TArgument)0,
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
    /* 注册各个内核函数,启动内核 */
    TclStartKernel(&AppSetupEntry,
                   &CpuSetupEntry,
                   &EvbSetupEntry,
                   &EvbTraceEntry);

    return 1;
}

#endif
