#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH4_SEMAPHORE_EXAMPLE2)

/* 用户线程参数 */
#define THREAD_LED_STACK_BYTES  (512)
#define THREAD_LED_PRIORITY     (5)
#define THREAD_LED_SLICE        (20)

/* 用户线程栈定义 */
static TBase32 ThreadLed1Stack[THREAD_LED_STACK_BYTES/4];
static TBase32 ThreadLed2Stack[THREAD_LED_STACK_BYTES/4];

/* 用户线程定义 */
static TThread ThreadLed1;
static TThread ThreadLed2;

/* 用户信号量定义 */
static TSemaphore LedSemaphore1;
static TSemaphore LedSemaphore2;


/* Led线程1的主函数 */
static void ThreadLed1Entry(TArgument arg)
{
    TState state;
    TError error;

    while (eTrue)
    {
        /* Led线程1以阻塞方式获取信号量1，如果成功就点亮Led1*/
        state = TclObtainSemaphore(&LedSemaphore1, TCLO_IPC_WAIT,
                                   0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        EvbLedControl(LED1, LED_ON);

        /* Led线程1延时1秒后关闭Led1 */
        state = TclDelayThread((TThread*)0, TCLM_MLS2TICKS(1000), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

        EvbLedControl(LED1, LED_OFF);

        /* Led线程1以非阻塞方式释放信号量2 */
        state = TclReleaseSemaphore(&LedSemaphore2, TCLO_IPC_DUMMY,
                                    0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");
    }
}


/* Led线程2的主函数 */
static void ThreadLed2Entry(TArgument arg)
{
    TState state;
    TError error;

    while (eTrue)
    {
        /* Led线程2点亮Led2 */
        EvbLedControl(LED2, LED_ON);

        /* Led线程2延时1秒后关闭Led2 */
        state =TclDelayThread((TThread*)0, TCLM_MLS2TICKS(1000), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

        EvbLedControl(LED2, LED_OFF);

        /* Led线程2以非阻塞方式释放信号量1 */
        state =TclReleaseSemaphore(&LedSemaphore1, TCLO_IPC_DUMMY,
                                   0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        /* Led线程2以阻塞方式获取信号量2 */
        state = TclObtainSemaphore(&LedSemaphore2, TCLO_IPC_WAIT,
                                   0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");
    }
}


/* 用户应用入口函数 */
static void AppSetupEntry(void)
{
    TState state;
    TError error;

    /* 初始化信号量1 */
    state = TclCreateSemaphore(&LedSemaphore1, 0, 1, TCLP_IPC_DUMMY, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IPC_NONE), "");

    /* 初始化信号量2 */
    state = TclCreateSemaphore(&LedSemaphore2, 0, 1, TCLP_IPC_DUMMY, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IPC_NONE), "");

    /* 初始化Led线程1 */
    state = TclCreateThread(&ThreadLed1,
                          &ThreadLed1Entry, (TArgument)0,
                          ThreadLed1Stack, THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY, THREAD_LED_SLICE,
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 初始化Led线程2 */
    state = TclCreateThread(&ThreadLed2,
                          &ThreadLed2Entry, (TArgument)0,
                          ThreadLed2Stack, THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY + 1, THREAD_LED_SLICE,
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 激活Led线程1 */
    state = TclActivateThread(&ThreadLed1, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 激活Led线程2 */
    state = TclActivateThread(&ThreadLed2, &error);
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
