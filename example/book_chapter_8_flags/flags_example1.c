#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH8_FLAGS_EXAMPLE1)

/* 用户线程参数 */
#define THREAD_LED_STACK_BYTES  (512)
#define THREAD_LED_PRIORITY     (5)
#define THREAD_LED_SLICE        (20)

#define THREAD_CTRL_STACK_BYTES (512)
#define THREAD_CTRL_PRIORITY    (6)
#define THREAD_CTRL_SLICE       (20)

/* 用户线程栈定义 */
static TBase32 ThreadLedOnStack[THREAD_LED_STACK_BYTES/4];
static TBase32 ThreadLedOffStack[THREAD_LED_STACK_BYTES/4];
static TBase32 ThreadCTRLStack[THREAD_CTRL_STACK_BYTES/4];

/* 用户线程定义 */
static TThread ThreadLedOn;
static TThread ThreadLedOff;
static TThread ThreadCTRL;

/* 用户事件标记 */
static TFlags LedFlags;
#define LED1_ON_FLG  (0x1<<0)
#define LED2_ON_FLG  (0x1<<1)
#define LED3_ON_FLG  (0x1<<2)

#define LED1_OFF_FLG (0x1<<4)
#define LED2_OFF_FLG (0x1<<5)
#define LED3_OFF_FLG (0x1<<6)


/* Led线程的主函数 */
static void ThreadLedOnEntry(TArgument arg)
{
    TState state;
    TError error;
    TBitMask pattern;
    TOption option;

    while (eTrue)
    {
        /* Led线程以阻塞方式获取事件，得到后点亮Led */
        pattern = LED1_ON_FLG | LED2_ON_FLG | LED3_ON_FLG;
        option = TCLO_IPC_WAIT | TCLO_IPC_OR | TCLO_IPC_CONSUME;

        state = TclReceiveFlags(&LedFlags, &pattern, option, 0, &error);
        if (state == eSuccess)
        {
            if (pattern & LED1_ON_FLG)
            {
                EvbLedControl(LED1, LED_ON);
            }

            if (pattern & LED2_ON_FLG)
            {
                EvbLedControl(LED2, LED_ON);
            }

            if (pattern & LED3_ON_FLG)
            {
                EvbLedControl(LED3, LED_ON);
            }
        }
    }
}


/* Led线程的主函数 */
static void ThreadLedOffEntry(TArgument arg)
{
    TState state;
    TError error;
    TBitMask pattern;
    TOption option;

    while (eTrue)
    {
        /* Led线程以阻塞方式获取事件，得到后熄灭Led */
        pattern = LED1_OFF_FLG | LED2_OFF_FLG | LED3_OFF_FLG;
        option = TCLO_IPC_WAIT | TCLO_IPC_OR | TCLO_IPC_CONSUME;

        state = TclReceiveFlags(&LedFlags, &pattern, option, 0, &error);
        if (state == eSuccess)
        {
            if (pattern & LED1_OFF_FLG)
            {
                EvbLedControl(LED1, LED_OFF);
            }

            if (pattern & LED2_OFF_FLG)
            {
                EvbLedControl(LED2, LED_OFF);
            }

            if (pattern & LED3_OFF_FLG)
            {
                EvbLedControl(LED3, LED_OFF);
            }
        }
    }
}

/* CTRL线程的主函数 */
static void ThreadCtrlEntry(TArgument arg)
{
    TState state;
    TError error;

    while (eTrue)
    {
        /* CTRL线程释放事件标记 */
        state =  TclSendFlags(&LedFlags, LED1_ON_FLG, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        /* CTRL线程延时1秒 */
        state =  TclDelayThread(0, TCLM_MLS2TICKS(1000), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

        /* CTRL线程释放事件标记 */
        state =  TclSendFlags(&LedFlags, LED2_ON_FLG, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        /* CTRL线程延时1秒 */
        state =   TclDelayThread(0, TCLM_MLS2TICKS(1000), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

        /* CTRL线程释放事件标记 */
        state =  TclSendFlags(&LedFlags, LED3_ON_FLG, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        /* CTRL线程延时1秒 */
        state =  TclDelayThread(0, TCLM_MLS2TICKS(1000), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

        /* CTRL线程释放事件标记 */
        state =  TclSendFlags(&LedFlags, LED1_OFF_FLG | LED2_OFF_FLG, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        /* CTRL线程延时1秒 */
        state = TclDelayThread(0, TCLM_MLS2TICKS(1000), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

        /* CTRL线程释放事件标记 */
        state =  TclSendFlags(&LedFlags, LED3_OFF_FLG, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        /* CTRL线程延时1秒 */
        state =  TclDelayThread(0, TCLM_MLS2TICKS(1000), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");
    }
}


/* 用户应用入口函数 */
static void AppSetupEntry(void)
{
    TState state;
    TError error;

    /* 初始化事件标记 */
    state = TclCreateFlags(&LedFlags, TCLP_IPC_DUMMY, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IPC_NONE), "");

    /* 初始化Led设备控制线程 */
    state = TclCreateThread(&ThreadLedOn,
                           &ThreadLedOnEntry, (TArgument)0,
                           ThreadLedOnStack, THREAD_LED_STACK_BYTES,
                           THREAD_LED_PRIORITY, THREAD_LED_SLICE,
                           &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    state = TclCreateThread(&ThreadLedOff,
                           &ThreadLedOffEntry, (TArgument)0,
                           ThreadLedOffStack, THREAD_LED_STACK_BYTES,
                           THREAD_LED_PRIORITY, THREAD_LED_SLICE, 
                           &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 初始化CTRL线程 */
    state = TclCreateThread(&ThreadCTRL,
                          &ThreadCtrlEntry, (TArgument)0,
                          ThreadCTRLStack, THREAD_CTRL_STACK_BYTES,
                          THREAD_CTRL_PRIORITY, THREAD_CTRL_SLICE, 
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 激活Led线程 */
    state = TclActivateThread(&ThreadLedOn, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    state = TclActivateThread(&ThreadLedOff, &error);
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
