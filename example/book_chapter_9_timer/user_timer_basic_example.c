#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH9_TIMER_BASIC_EXAMPLE)

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
static TTimer Led3Timer;

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

/* 用户定时器3的回调函数，间隔1秒，点亮或熄灭Led3 */
static void BlinkLed3(TArgument data)
{
    static TIndex index = 0;
    if (index % 2)
    {
        EvbLedControl(LED3, LED_OFF);
    }
    else
    {
        EvbLedControl(LED3, LED_ON);
    }
    index++;
}

/* Led线程主函数 */
static void ThreadLedEntry(TArgument data)
{
    TState state;
    TError error;

    /* 初始化用户定时器1 */
    state = TclCreateTimer(&Led1Timer,
                           TCLP_TIMER_PERIODIC,
                           TCLM_MLS2TICKS(1000),
                           &BlinkLed1,
                           (TArgument)0,
                           &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_TIMER_NONE), "");

    /* 初始化用户定时器2 */
    state = TclCreateTimer(&Led2Timer,
                           TCLP_TIMER_PERIODIC,
                           TCLM_MLS2TICKS(1000),
                           &BlinkLed2,
                           (TArgument)0,
                           &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_TIMER_NONE), "");

    /* 初始化用户定时器3 */
    state = TclCreateTimer(&Led3Timer,
                           TCLP_TIMER_PERIODIC,
                           TCLM_MLS2TICKS(1000),
                           &BlinkLed3,
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

    /* 启动用户定时器3 */
    state = TclStartTimer(&Led3Timer, TCLM_MLS2TICKS(500), &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_TIMER_NONE), "");

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
