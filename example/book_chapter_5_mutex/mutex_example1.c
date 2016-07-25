#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH5_MUTEX_EXAMPLE1)

/* 用户线程参数 */
#define THREAD_LED_STACK_BYTES  (512)
#define THREAD_LED_PRIORITY     (8)
#define THREAD_LED_SLICE        (0xff)

/* 用户线程定义 */
static TThread ThreadLedOn;
static TThread ThreadLedOff;

/* 用户线程栈定义 */
static TBase32 ThreadLedOnStack[THREAD_LED_STACK_BYTES/4];
static TBase32 ThreadLedOffStack[THREAD_LED_STACK_BYTES/4];

/* 用户互斥量定义 */
static TMutex LedMutex;

/* 用户互斥量优先级天花板 */
#define LED_MUTEX_PRIORITY   (4)

/* Led点亮线程的主函数 */
static void ThreadLedOnEntry(TArgument data)
{
    TError error;
    TState state;
    while (eTrue)
    {
        /* Led点亮线程以阻塞方式锁定互斥量，如果成功则点亮Led */
        state = TclLockMutex(&LedMutex, TCLO_IPC_WAIT, 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");
        EvbLedControl(LED1, LED_ON);

        /* Led点亮线程延时1秒 */
        state = TclDelayThread(&ThreadLedOn, TCLM_MLS2TICKS(1000), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

        /* Led点亮线程释放互斥量 */
        state = TclFreeMutex(&LedMutex, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");
    }
}

/* Led熄灭线程的主函数 */
static void ThreadLedOffEntry(TArgument data)
{
    TError error;
    TState state;

    while (eTrue)
    {
        /* Led熄灭线程以阻塞方式锁定互斥量，如果成功则熄灭Led */
        state = TclLockMutex(&LedMutex, TCLO_IPC_WAIT, 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        EvbLedControl(LED1, LED_OFF);

        /* Led熄灭线程延时1秒 */
        state = TclDelayThread(&ThreadLedOff, TCLM_MLS2TICKS(1000), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

        /* Led熄灭线程释放互斥量 */
        state = TclFreeMutex(&LedMutex, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");
    }
}

/* 用户应用入口函数 */
static void AppSetupEntry(void)
{
    TState state;
    TError error;

    /* 初始化互斥量 */
    state = TclCreateMutex(&LedMutex, LED_MUTEX_PRIORITY, TCLP_IPC_DUMMY, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IPC_NONE), "");

    /* 初始化Led点亮线程 */
    state = TclCreateThread(&ThreadLedOn,
                          &ThreadLedOnEntry, (TArgument)0,
                          ThreadLedOnStack, THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY, THREAD_LED_SLICE, 
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 初始化Led熄灭线程 */
    state = TclCreateThread(&ThreadLedOff,
                          &ThreadLedOffEntry, (TArgument)0,
                          ThreadLedOffStack, THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY + 1, THREAD_LED_SLICE, 
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 激活Led点亮线程 */
    state = TclActivateThread(&ThreadLedOn, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 激活Led熄灭线程 */
    state = TclActivateThread(&ThreadLedOff, &error);
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
