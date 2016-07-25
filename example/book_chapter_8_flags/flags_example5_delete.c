#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH8_FLAGS_EXAMPLE5)

/* 用户线程参数 */
#define THREAD_LED_STACK_BYTES  (512)
#define THREAD_LED_PRIORITY     (5)
#define THREAD_LED_SLICE        (20)

#define THREAD_CTRL_STACK_BYTES (512)
#define THREAD_CTRL_PRIORITY    (4)
#define THREAD_CTRL_SLICE       (20)

/* 用户线程定义 */
static TThread ThreadLed;
static TThread ThreadCTRL;

/* 用户线程栈定义 */
static TBase32 ThreadLedStack[THREAD_LED_STACK_BYTES/4];
static TBase32 ThreadCTRLStack[THREAD_CTRL_STACK_BYTES/4];

/* 用户事件标记定义 */
static TFlags LedFlags;

/* Led线程的主函数 */
static void ThreadLedEntry(TArgument data)
{
    TState state;
    TError error;
    TBitMask pattern;
    TOption option;

    while (eTrue)
    {
        /* Led线程以阻塞方式获取事件，当发现事件标记重置后点亮Led */
        pattern = 0x1;
        option = TCLO_IPC_WAIT | TCLO_IPC_OR | TCLO_IPC_CONSUME;
        state = TclReceiveFlags(&LedFlags, &pattern, option, 0, &error);
        if ((state != eSuccess) && (error & TCLE_IPC_DELETE))
        {
            EvbLedControl(LED1, LED_ON);
        }

        /* Led线程以阻塞方式获取事件标记，当发现事件标记重置后熄灭Led */
        pattern = 0x1;
        option = TCLO_IPC_WAIT | TCLO_IPC_OR | TCLO_IPC_CONSUME;
        state = TclReceiveFlags(&LedFlags, &pattern, option, 0, &error);
        if ((state != eSuccess) && (error & TCLE_IPC_DELETE))
        {
            EvbLedControl(LED1, LED_OFF);
        }
    }
}


/* CTRL线程的主函数 */
static void ThreadCtrlEntry(TArgument data)
{
    TState state;
    TError error;
    while (eTrue)
    {
        /* CTRL线程延时1秒，此后Led线程才得到运行机会 */
        state = TclDelayThread(&ThreadCTRL, TCLM_MLS2TICKS(1000), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

        /* CTRL线程删除事件标记 */
        state =TclDeleteFlags(&LedFlags, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        state =TclCreateFlags(&LedFlags, TCLP_IPC_DUMMY, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");
    }
}


/* 用户应用入口函数 */
static void AppSetupEntry(void)
{
    TState state;
    TError error;

    /* 初始化Led事件标记 */
    state = TclCreateFlags(&LedFlags, TCLP_IPC_DUMMY, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IPC_NONE), "");

    /* 初始化Led设备控制线程 */
    state = TclCreateThread(&ThreadLed,
                          &ThreadLedEntry, (TArgument)0,
                          ThreadLedStack, THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY, THREAD_LED_SLICE,
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 初始化CTRL线程 */
    state = TclCreateThread(&ThreadCTRL,
                          &ThreadCtrlEntry, (TArgument)0,
                          ThreadCTRLStack,THREAD_CTRL_STACK_BYTES,
                          THREAD_CTRL_PRIORITY, THREAD_CTRL_SLICE,
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 激活Led线程 */
    state = TclActivateThread(&ThreadLed, &error);
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
