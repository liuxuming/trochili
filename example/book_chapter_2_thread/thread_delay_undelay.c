/*  线程延时API演示 */

#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH2_THREAD_EXAMPLE4)

/* 创建线程时需要的参数 */
#define THREAD_LED_STACK_BYTES  (512)
#define THREAD_LED_PRIORITY     (5)
#define THREAD_LED_SLICE        (65535)

#define THREAD_CTRL_STACK_BYTES (512)
#define THREAD_CTRL_PRIORITY    (6)
#define THREAD_CTRL_SLICE       (65535)

/* 线程定义、线程栈定义 */
static TThread ThreadLed1;
static TThread ThreadLed2;
static TThread ThreadCtrl;
static TBase32 ThreadLed1Stack[THREAD_LED_STACK_BYTES/4];
static TBase32 ThreadLed2Stack[THREAD_LED_STACK_BYTES/4];
static TBase32 ThreadCtrlStack[THREAD_LED_STACK_BYTES/4];

/* Led线程的线程主函数 */
static void ThreadLed1Entry(TArgument data)
{
    TError error;
    TState state;

    while (eTrue)
    {
        state = TclDelayThread(0, TCLM_SEC2TICKS(1), &error);
        if(state == eSuccess)
        {
            EvbLedControl(LED1, LED_OFF);
        }

        state = TclDelayThread(0, TCLM_SEC2TICKS(1), &error);
        if(state == eSuccess)
        {
            EvbLedControl(LED1, LED_ON);
        }
    }
}


/* Led线程的线程主函数 */
static void ThreadLed2Entry(TArgument data)
{
    TError error;
    TState state;

    while (eTrue)
    {
        state = TclDelayThread(0, TCLM_SEC2TICKS(60), &error);
        if(state == eSuccess)
        {
            EvbLedControl(LED2, LED_OFF);
        }

        state = TclDelayThread(0, TCLM_SEC2TICKS(60), &error);
        if(state == eSuccess)
        {
            EvbLedControl(LED2, LED_ON);
        }
    }
}


/* Led线程的线程主函数 */
static void ThreadCtrlEntry(TArgument data)
{
    TError error;
    TState state;

    static int i = 0;

    while (eTrue)
    {
        i = 0xfffff;
        while (i--);
        state = TclUnDelayThread(&ThreadLed2, &error);
			  state = state;
    }
}


/* 用户应用入口函数 */
static void AppSetupEntry(void)
{
    TError error;
    TState state;

    /* 初始化Led1控制线程 */
    state = TclCreateThread(&ThreadLed1,
                            &ThreadLed1Entry,
                            (TArgument)(&ThreadLed1),
                            ThreadLed1Stack,
                            THREAD_LED_STACK_BYTES,
                            THREAD_LED_PRIORITY,
                            THREAD_LED_SLICE,
                            &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 初始化Led2控制线程 */
    state = TclCreateThread(&ThreadLed2,
                            &ThreadLed2Entry,
                            (TArgument)(&ThreadLed2),
                            ThreadLed2Stack,
                            THREAD_LED_STACK_BYTES,
                            THREAD_LED_PRIORITY,
                            THREAD_LED_SLICE,
                            &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 初始化Led2控制线程 */
    state = TclCreateThread(&ThreadCtrl,
                            &ThreadCtrlEntry,
                            (TArgument)(&ThreadLed2),
                            ThreadCtrlStack,
                            THREAD_CTRL_STACK_BYTES,
                            THREAD_CTRL_PRIORITY,
                            THREAD_CTRL_SLICE,
                            &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 激活Led1控制线程 */
    state = TclActivateThread(&ThreadLed1, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");


    /* 激活Led2控制线程 */
    state = TclActivateThread(&ThreadLed2, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 激活Led设备控制线程 */
    state = TclActivateThread(&ThreadCtrl, &error);
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


