/* 线程优先级修改API演示 */

#include "trochili.h"
#include "example.h"

#if (EVB_EXAMPLE == CH2_THREAD_EXAMPLE6)

/* 创建线程时需要的参数 */
#define THREAD_LED_STACK_BYTES  (512)
#define THREAD_LED_PRIORITY     (5)
#define THREAD_LED_SLICE        (0xFFFFFFFF)

/* 线程定义 */
static TThread ThreadLed1;
static TThread ThreadLed2;
/* 线程栈定义 */
static TBase32 ThreadLed1Stack[THREAD_LED_STACK_BYTES/4];
static TBase32 ThreadLed2Stack[THREAD_LED_STACK_BYTES/4];

/* 线程做无用操作，起到空转效果 */
static delay(TBase32 count)
{
    while(count--);
}


/* 线程Led1的入口函数 */
static void ThreadLed1Entry(TArgument data)
{
    TState state;
    TError error;

    TByte turn = 0;
    while (eTrue)
    {
        EvbLedControl(LED1, LED_ON);
        delay(0x8FFFFF);
        EvbLedControl(LED1, LED_OFF);
        delay(0x8FFFFF);
        turn ++;
        if (turn == 2)
        {
            state = TclSetThreadPriority(&ThreadLed2, THREAD_LED_PRIORITY - 1, &error);
            TCLM_ASSERT((state == eSuccess), "");
            TCLM_ASSERT((error == TCLE_THREAD_NONE), "");
            turn = 0;
        }
    }
}


/* 线程Led2的入口函数 */
static void ThreadLed2Entry(TArgument data)
{
    TState state;
    TError error;

    TByte turn = 0;
    while (eTrue)
    {
        EvbLedControl(LED2, LED_ON);
        delay(0x8FFFFF);
        EvbLedControl(LED2, LED_OFF);
        delay(0x8FFFFF);
        turn ++;
        if (turn == 2)
        {
            state = TclSetThreadPriority(&ThreadLed2, THREAD_LED_PRIORITY + 1, &error );
            TCLM_ASSERT((state == eSuccess), "");
            TCLM_ASSERT((error == TCLE_THREAD_NONE), "");
            turn = 0;
        }
    }
}


/* 用户应用入口函数 */
static void AppSetupEntry(void)
{
    TState state;
    TError error;

    /* 初始化Led1设备控制线程 */
    state = TclCreateThread(&ThreadLed1,
                          &ThreadLed1Entry, (TArgument)0,
                          ThreadLed1Stack, THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY, THREAD_LED_SLICE,
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 初始化Led2设备控制线程 */
    state = TclCreateThread(&ThreadLed2,
                          &ThreadLed2Entry, (TArgument)0,
                          ThreadLed2Stack, THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY + 1, THREAD_LED_SLICE,
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 激活Led1设备控制线程 */
    state = TclActivateThread(&ThreadLed1, &error);

    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 激活Led2设备控制线程 */
    state = TclActivateThread(&ThreadLed2, &error);
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
