#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH10_IRQ_DAEMON_EXAMPLE)

/* 用户线程参数 */
#define THREAD_LED_STACK_BYTES         (512)
#define THREAD_LED_PRIORITY            (5)
#define THREAD_LED_SLICE               (20)

/* 用户线程栈定义 */
static TBase32 ThreadLed1Stack[THREAD_LED_STACK_BYTES/4];
static TBase32 ThreadLed2Stack[THREAD_LED_STACK_BYTES/4];
static TBase32 ThreadLed3Stack[THREAD_LED_STACK_BYTES/4];

/* 用户线程定义 */
static TThread ThreadLed1;
static TThread ThreadLed2;
static TThread ThreadLed3;

/* 用户信号量定义 */
static TSemaphore LedSemaphore1;
static TSemaphore LedSemaphore2;

/* IRQ请求对象 */
static TIrq  irq1;
static TIrq  irq2;

/* 评估板按键IRQ回调函数 */
static void IrqCallback(TArgument arg)
{
    TState state;
    TError error;
    TSemaphore* pSemaphore;

    pSemaphore = (TSemaphore*)arg;
    state = TclReleaseSemaphore(pSemaphore, TCLO_IPC_DUMMY, 0U, &error);
    TCLM_ASSERT((state == eSuccess), "");
}


/* 评估板按键中断处理函数 */
static TBitMask EvbKeyISR(TArgument data)
{
    TState state;
    TError error;
    int key;

    key = EvbKeyScan();
    if (key == 1)
    {
        state = TclPostIRQ(&irq1,
                           (TPriority)5,
                           IrqCallback,
                           (TArgument)(&LedSemaphore1),
                           &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IRQ_NONE), "");

        return TCLR_IRQ_ASR;
    }
    else if (key == 2)
    {
        state = TclPostIRQ(&irq2,
                           (TPriority)5,
                           IrqCallback,
                           (TArgument)(&LedSemaphore2),
                           &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IRQ_NONE), "");

        return TCLR_IRQ_ASR;
    }
    else
    {
        return TCLR_IRQ_DONE;
    }
}


/* Led1线程的主函数 */
static void ThreadLed1Entry(TBase32 arg0)
{
    TState state;
    TError error;

    while (eTrue)
    {
        /* Led线程以阻塞方式获取信号量，如果成功则点亮Led */
        state = TclObtainSemaphore(&LedSemaphore1, TCLO_IPC_WAIT, 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");
        EvbLedControl(LED1, LED_ON);

        /* Led线程以阻塞方式获取信号量，如果成功则熄灭Led */
        state = TclObtainSemaphore(&LedSemaphore1, TCLO_IPC_WAIT, 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");
        EvbLedControl(LED1, LED_OFF);
    }
}


/* Led2线程的主函数 */
static void ThreadLed2Entry(TBase32 arg0)
{
    TState state;
    TError error;

    while (eTrue)
    {
        /* Led线程以阻塞方式获取信号量，如果成功则点亮Led */
        state = TclObtainSemaphore(&LedSemaphore2, TCLO_IPC_WAIT, 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");
        EvbLedControl(LED2, LED_ON);

        /* Led线程以阻塞方式获取信号量，如果成功则熄灭Led */
        state = TclObtainSemaphore(&LedSemaphore2, TCLO_IPC_WAIT, 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");
        EvbLedControl(LED2, LED_OFF);
    }
}


/* Led3线程的主函数 */
static void ThreadLed3Entry(TBase32 arg0)
{
    TState state;
    TError error;

    while (eTrue)
    {
        EvbLedControl(LED3, LED_ON);
        state = TclDelayThread((TThread*)0, TCLM_SEC2TICKS(1), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

        EvbLedControl(LED3, LED_OFF);
        state = TclDelayThread((TThread*)0, TCLM_SEC2TICKS(1), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");
    }
}

/* 用户应用程序入口函数 */
static void AppSetupEntry(void)
{
    TError error;
    TState state;

    /* 初始化信号量1 */
    state = TclCreateSemaphore(&LedSemaphore1, 0, 1, TCLP_IPC_DUMMY, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IPC_NONE), "");

    /* 初始化信号量2 */
    state = TclCreateSemaphore(&LedSemaphore2, 0, 1, TCLP_IPC_DUMMY, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IPC_NONE), "");

    /* 设置和KEY相关的外部中断向量 */
    state = TclSetIrqVector(KEY_IRQ_ID, &EvbKeyISR, (TThread*)0, (TArgument)0, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IRQ_NONE), "");

    /* 初始化Led1线程 */
    state = TclCreateThread(&ThreadLed1,
                          &ThreadLed1Entry,
                          (TArgument)0,
                          ThreadLed1Stack,
                          THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY,
                          THREAD_LED_SLICE,
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 初始化Led2线程 */
    state = TclCreateThread(&ThreadLed2,
                          &ThreadLed2Entry,
                          (TArgument)0,
                          ThreadLed2Stack,
                          THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY,
                          THREAD_LED_SLICE,
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 初始化Led3线程 */
    state = TclCreateThread(&ThreadLed3,
                          &ThreadLed3Entry,
                          (TArgument)0,
                          ThreadLed3Stack,
                          THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY,
                          THREAD_LED_SLICE,
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");
	
    /* 激活Led1线程 */
    state = TclActivateThread(&ThreadLed1, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 激活Led2线程 */
    state = TclActivateThread(&ThreadLed2, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 激活Led3线程 */
    state = TclActivateThread(&ThreadLed3, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");
}


/* 处理器BOOT之后会调用main函数，必须提供 */
int main(void)
{
    /* 注册各个内核函数, 启动内核 */
    TclStartKernel(&AppSetupEntry,
                   &CpuSetupEntry,
                   &EvbSetupEntry,
                   &EvbTraceEntry);

    return 1;
}

#endif

