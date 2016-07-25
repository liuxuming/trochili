#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH6_MAILBOX_EXAMPLE6)

/* 用户线程参数 */
#define THREAD_LED_STACK_BYTES  (512)
#define THREAD_LED_PRIORITY     (5)
#define THREAD_LED_SLICE        (20)

#define THREAD_CTRL_STACK_BYTES (512)
#define THREAD_CTRL_PRIORITY    (6)
#define THREAD_CTRL_SLICE       (20)

/* 用户线程定义 */
static TThread ThreadLed1;
static TThread ThreadLed2;
static TThread ThreadLed3;
static TThread ThreadCTRL;

/* 用户线程栈定义 */
static TBase32 ThreadLed1Stack[THREAD_LED_STACK_BYTES/4];
static TBase32 ThreadLed2Stack[THREAD_LED_STACK_BYTES/4];
static TBase32 ThreadLed3Stack[THREAD_LED_STACK_BYTES/4];
static TBase32 ThreadCTRLStack[THREAD_CTRL_STACK_BYTES/4];

/* 用户邮件类型定义 */
typedef struct
{
    TIndex Index;
    TByte Value;
} TLedMail;

/* 用户邮箱和邮件定义 */
static TMailBox LedMailbox;
static TLedMail LedMail;


/* Led1的线程的主函数 */
static void ThreadLed1Entry(TArgument arg)
{
    TState state;
    TError error;
    TLedMail* pMail;

    while (eTrue)
    {
        /* Led3线程以阻塞方式接收邮件，并根据邮件内容控制Led3的点亮或者熄灭 */
        state = TclReceiveMail(&LedMailbox, (TMail*)(&pMail),
                               TCLO_IPC_WAIT, 0, &error);
        if (state == eSuccess)
        {
            EvbLedControl(LED1, pMail->Value);
        }
    }
}

/* Led2的线程的主函数 */
static void ThreadLed2Entry(TArgument arg)
{
    TState state;
    TError error;
    TLedMail* pMail;

    while (eTrue)
    {
        /* Led3线程以阻塞方式接收邮件，并根据邮件内容控制Led3的点亮或者熄灭 */
        state = TclReceiveMail(&LedMailbox, (TMail*)(&pMail),
                               TCLO_IPC_WAIT, 0, &error);
        if (state == eSuccess)
        {
            EvbLedControl(LED2, pMail->Value);
        }
    }
}

/* Led3线程的主函数 */
static void ThreadLed3Entry(TArgument arg)
{
    TState state;
    TError error;
    TLedMail* pMail;

    while (eTrue)
    {
        /* Led3线程以阻塞方式接收邮件，并根据邮件内容控制Led3的点亮或者熄灭 */
        state = TclReceiveMail(&LedMailbox, (TMail*)(&pMail),
                               TCLO_IPC_WAIT, 0, &error);
        if (state == eSuccess)
        {
            EvbLedControl(LED3, pMail->Value);
        }
    }
}


/* CTRL线程的主函数 */
static void ThreadCtrlEntry(TArgument arg)
{
    TState state;
    TError error;
    TLedMail* pMail;

    pMail = &LedMail;
    while (eTrue)
    {
        /* 控制线程延时1秒后广播所有Led打开的邮件 */
        state = TclDelayThread(0, TCLM_MLS2TICKS(1000), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

        pMail->Index = LEDX;
        pMail->Value = LED_ON;
        state = TclBroadcastMail(&LedMailbox, (TMail*)(&pMail), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        /* 控制线程延时1秒后广播所有Led熄灭的邮件 */
        state = TclDelayThread(0, TCLM_MLS2TICKS(1000), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

        pMail->Index = LEDX;
        pMail->Value = LED_OFF;
        state = TclBroadcastMail(&LedMailbox, (TMail*)(&pMail), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");
    }
}


/* 用户应用入口函数 */
static void AppSetupEntry(void)
{
    TState state;
    TError error;

    /* 初始化邮箱 */
    state =  TclCreateMailBox(&LedMailbox, TCLP_IPC_DUMMY, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IPC_NONE), "");

    /* 初始化Led1设备控制线程 */
    state =  TclCreateThread(&ThreadLed1,
                           &ThreadLed1Entry, (TArgument)0,
                           ThreadLed1Stack, THREAD_LED_STACK_BYTES,
                           THREAD_LED_PRIORITY, THREAD_LED_SLICE,
                           &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 初始化Led2设备控制线程 */
    state =  TclCreateThread(&ThreadLed2,
                           &ThreadLed2Entry, (TArgument)0,
                           ThreadLed2Stack, THREAD_LED_STACK_BYTES,
                           THREAD_LED_PRIORITY, THREAD_LED_SLICE,
                           &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 初始化Led3设备控制线程 */
    state =  TclCreateThread(&ThreadLed3,
                           &ThreadLed3Entry, (TArgument)0,
                           ThreadLed3Stack, THREAD_LED_STACK_BYTES,
                           THREAD_LED_PRIORITY, THREAD_LED_SLICE,
                           &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 初始化CTRL线程 */
    state =  TclCreateThread(&ThreadCTRL,
                           &ThreadCtrlEntry, (TArgument)0,
                           ThreadCTRLStack, THREAD_CTRL_STACK_BYTES,
                           THREAD_CTRL_PRIORITY, THREAD_CTRL_SLICE,
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

    /* 激活Led线程3 */
    state =  TclActivateThread(&ThreadLed3, &error);
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
