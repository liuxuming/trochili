#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH10_IRQ_ISR_EXAMPLE)

/* 用户线程参数 */
#define THREAD_LED_STACK_BYTES  (512)
#define THREAD_LED_PRIORITY     (5)
#define THREAD_LED_SLICE        (20)


/* 用户线程栈定义 */
static TBase32 ThreadLedStack[THREAD_LED_STACK_BYTES/4];

/* 用户线程定义 */
static TThread ThreadLed;

/* 用户信号量定义 */
static TSemaphore LedSemaphore;

/* Led线程的主函数 */
static void ThreadLedEntry(TArgument data)
{
    TError error;
    TState state;

    while (eTrue)
    {
        /* Led线程以阻塞方式获取信号量，如果成功则点亮Led */
        state = TclObtainSemaphore(&LedSemaphore, TCLO_IPC_WAIT, 0, &error);
        if (state == eSuccess)
        {
            TCLM_ASSERT((error == TCLE_IPC_NONE), "");
            EvbLedControl(LED1, LED_ON);
        }

        /* Led线程以阻塞方式获取信号量，如果成功则熄灭Led */
        state = TclObtainSemaphore(&LedSemaphore, TCLO_IPC_WAIT, 0, &error);
        if (state == eSuccess)
        {
            TCLM_ASSERT((error == TCLE_IPC_NONE), "");
            EvbLedControl(LED1, LED_OFF);
        }
    }
}


/* 评估板按键中断处理函数 */
static TBitMask EvbKeyISR(TArgument data)
{
    TState state;
    if (EvbKeyScan())
    {
        /* ISR以非阻塞方式(必须)释放信号量 */
        state = TclIsrReleaseSemaphore(&LedSemaphore);
        TCLM_ASSERT((state == eSuccess), "");
    }
    return TCLR_IRQ_DONE;
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

    /* 初始化信号量 */
    state = TclCreateSemaphore(&LedSemaphore, 0, 1, TCLP_IPC_DUMMY, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IPC_NONE), "");

    /* 初始化Led线程 */
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
                   &CpuSetupEntry,
                   &EvbSetupEntry,
                   &EvbTraceEntry);
    return 1;
}


#endif
