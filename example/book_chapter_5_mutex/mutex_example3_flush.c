#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH5_MUTEX_EXAMPLE3)

/* 用户线程参数 */
#define THREAD_LED_STACK_BYTES  (512)
#define THREAD_LED_PRIORITY     (8)
#define THREAD_LED_SLICE        (2000)

#define THREAD_CTRL_STACK_BYTES (512)
#define THREAD_CTRL_PRIORITY    (6)
#define THREAD_CTRL_SLICE       (2000)

/* 用户线程栈定义 */
static TThread ThreadLed1;
static TThread ThreadLed2;
static TThread ThreadLed3;
static TThread ThreadCTRL;

/* 用户线程栈定义 */
static TBase32 ThreadLed1Stack[THREAD_LED_STACK_BYTES/4];
static TBase32 ThreadLed2Stack[THREAD_LED_STACK_BYTES/4];
static TBase32 ThreadLed3Stack[THREAD_LED_STACK_BYTES/4];
static TBase32 ThreadCTRLStack[THREAD_CTRL_STACK_BYTES/4];

/* 用户互斥量定义 */
static TMutex LedMutex;

/* 用户互斥量优先级天花板 */
#define LED_MUTEX_PRIORITY   (4)

/* Led1线程的主函数 */
static void ThreadLed1Entry(TArgument data)
{
    TState state;
    TError error;

    while (eTrue)
    {
        /* Led线程以阻塞方式获取互斥量，被强制解除阻塞后点亮Led */
        state = TclLockMutex(&LedMutex, TCLO_IPC_WAIT, 0, &error);
        if ((state != eSuccess) && (error & TCLE_IPC_FLUSH))
        {
            EvbLedControl(LED1, LED_ON);
        }

        /* Led线程以阻塞方式获取互斥量，被强制解除阻塞后熄灭Led */
        state = TclLockMutex(&LedMutex, TCLO_IPC_WAIT, 0, &error);
        if ((state != eSuccess) && (error & TCLE_IPC_FLUSH))
        {
            EvbLedControl(LED1, LED_OFF);
        }
    }
}


/* Led2线程的主函数 */
static void ThreadLed2Entry(TArgument data)
{
    TState state;
    TError error;

    while (eTrue)
    {
        /* Led线程以阻塞方式获取互斥量，被强制解除阻塞后点亮Led */
        state = TclLockMutex(&LedMutex, TCLO_IPC_WAIT, 0, &error);
        if ((state != eSuccess) && (error & TCLE_IPC_FLUSH))
        {
            EvbLedControl(LED2, LED_ON);
        }

        /* Led线程以阻塞方式获取互斥量，被强制解除阻塞后熄灭Led */
        state = TclLockMutex(&LedMutex, TCLO_IPC_WAIT, 0, &error);
        if ((state != eSuccess) && (error & TCLE_IPC_FLUSH))
        {
            EvbLedControl(LED2, LED_OFF);
        }
    }
}

/* Led3线程的主函数 */
static void ThreadLed3Entry(TArgument data)
{
    TState state;
    TError error;

    while (eTrue)
    {
        /* Led线程以阻塞方式获取互斥量，被强制解除阻塞后点亮Led */
        state = TclLockMutex(&LedMutex, TCLO_IPC_WAIT, 0, &error);
        if ((state != eSuccess) && (error & TCLE_IPC_FLUSH))
        {
            EvbLedControl(LED3, LED_ON);
        }

        /* Led线程以阻塞方式获取互斥量，被强制解除阻塞后熄灭Led */
        state = TclLockMutex(&LedMutex, TCLO_IPC_WAIT, 0, &error);
        if ((state != eSuccess) && (error & TCLE_IPC_FLUSH))
        {
            EvbLedControl(LED3, LED_OFF);
        }
    }
}

/* CTRL线程的主函数 */
static void ThreadCtrlEntry(TArgument data)
{
    TState state;
    TError error;

    /* CTRL线程以阻塞方式获取互斥量 */
    state = TclLockMutex(&LedMutex, TCLO_IPC_WAIT, 0, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IPC_NONE), "");

    while (eTrue)
    {
        /* CTRL线程延时1秒后强制解除全部线程的阻塞 */
        state = TclDelayThread((TThread*)0, TCLM_MLS2TICKS(1000), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

        state = TclFlushMutex(&LedMutex, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");
    }
}


/* 用户应用程序入口函数 */
static void AppSetupEntry(void)
{
    TState state;
    TError error;

    /* 初始化互斥量 */
    state = TclCreateMutex(&LedMutex, LED_MUTEX_PRIORITY, TCLP_IPC_DUMMY, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IPC_NONE), "");

    /* 初始化Led1设备控制线程 */
    state = TclCreateThread(&ThreadLed1,
                          &ThreadLed1Entry, (TArgument)0,
                          ThreadLed1Stack, THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY, THREAD_LED_SLICE, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 初始化Led2设备控制线程 */
    state = TclCreateThread(&ThreadLed2,
                          &ThreadLed2Entry, (TArgument)0,
                          ThreadLed2Stack, THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY, THREAD_LED_SLICE, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 初始化Led3设备控制线程 */
    state = TclCreateThread(&ThreadLed3,
                          &ThreadLed3Entry, (TArgument)0,
                          ThreadLed3Stack, THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY, THREAD_LED_SLICE, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 初始化CTRL线程 */
    state = TclCreateThread(&ThreadCTRL,
                          &ThreadCtrlEntry, (TArgument)0,
                          ThreadCTRLStack, THREAD_CTRL_STACK_BYTES,
                          THREAD_CTRL_PRIORITY, THREAD_CTRL_SLICE, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 激活Led设备控制线程 */
    state = TclActivateThread(&ThreadLed1, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    state = TclActivateThread(&ThreadLed2, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    state = TclActivateThread(&ThreadLed3, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 激活CTRL线程 */
    state = TclActivateThread(&ThreadCTRL, &error);
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
