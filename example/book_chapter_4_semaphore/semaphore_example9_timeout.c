#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH4_SEMAPHORE_EXAMPLE9)

/* 用户线程参数 */
#define THREAD_LED_STACK_BYTES  (512)
#define THREAD_LED_PRIORITY     (5)
#define THREAD_LED_SLICE        (20)

/* 用户线程栈定义 */
static TBase32 ThreadLedStack[THREAD_LED_STACK_BYTES/4];

/* 用户线程定义 */
TThread ThreadLed;

/* 用户信号量定义 */
static TSemaphore LedSemaphore;

/* Led线程的主函数 */
static void ThreadLedEntry(TArgument arg)
{
    TState state;
    TError error;

    while (eTrue)
    {
        state = eSuccess;
        error = TCLE_IPC_NONE;

        /* Led线程以阻塞方式获取信号量，得到后点亮Led */
        state = TclObtainSemaphore(&LedSemaphore, TCLO_IPC_WAIT| TCLO_IPC_TIMEO,
                                   TCLM_SEC2TICKS(1), &error);
        if ((state == eFailure) && (error == TCLE_IPC_TIMEO))
        {
            EvbLedControl(LED1, LED_ON);
        }

        state = eSuccess;
        error = TCLE_IPC_NONE;

        /* Led线程以阻塞方式获取信号量，得到后熄灭Led */
        state = TclObtainSemaphore(&LedSemaphore, TCLO_IPC_WAIT| TCLO_IPC_TIMEO,
                                   TCLM_SEC2TICKS(1), &error);
        if ((state == eFailure) && (error == TCLE_IPC_TIMEO))
        {
            EvbLedControl(LED1, LED_OFF);
        }
    }
}


/* 用户应用入口函数 */
static void AppSetupEntry(void)
{
    TState state;
    TError error;

    /* 初始化信号量 */
    state = TclCreateSemaphore(&LedSemaphore, "led semaphore", 0, 1,
                               TCLP_IPC_DEFAULT, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IPC_NONE), "");

    /* 初始化Led设备控制线程 */
    state = TclCreateThread(&ThreadLed, "led thread",
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
    /* 注册各个内核函数,启动内核 */
    TclStartKernel(&AppSetupEntry,
                   &OsCpuSetupEntry,
                   &EvbSetupEntry,
                   &EvbTraceEntry);
    return 1;
}
#endif

