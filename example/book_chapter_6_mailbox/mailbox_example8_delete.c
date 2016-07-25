#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH6_MAILBOX_EXAMPLE8)

/* 用户线程参数 */
#define THREAD_LED_STACK_BYTES  (512)
#define THREAD_LED_PRIORITY     (5)
#define THREAD_LED_SLICE        (20)

#define THREAD_CTRL_STACK_BYTES (512)
#define THREAD_CTRL_PRIORITY    (4)
#define THREAD_CTRL_SLICE       (20)

/* 用户线程栈定义 */
static TBase32 ThreadLedStack[THREAD_LED_STACK_BYTES/4];
static TBase32 ThreadCTRLStack[THREAD_CTRL_STACK_BYTES/4];

/* 用户线程定义 */
static TThread ThreadLed;
static TThread ThreadCTRL;

/* 用户邮件类型定义 */
typedef struct
{
    TIndex Index;
    TByte Value;
} TLedMail;

/* 用户邮箱定义 */
static TMailBox LedMailbox;


/* Led线程的主函数 */
static void ThreadLedEntry(TArgument data)
{
    TState state;
    TError error;
    TLedMail* pMail;

    while (eTrue)
    {
        /* Led线程以阻塞方式接收邮件 */
        state = TclReceiveMail(&LedMailbox, (TMail*)(&pMail),
                               TCLO_IPC_WAIT, 0, &error);
        if ((state != eSuccess) && (error & TCLE_IPC_DELETE))
        {
            EvbLedControl(LED1, LED_ON);
        }

        /* Led线程以阻塞方式接收邮件 */
        state = TclReceiveMail(&LedMailbox, (TMail*)(&pMail),
                               TCLO_IPC_WAIT, 0, &error);
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
        /* CTRL线程延时1秒 */
        state = TclDelayThread((TThread*)0, TCLM_MLS2TICKS(1000), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

        /* CTRL线程重置邮箱，然后再次初始化邮箱 */
        state = TclDeleteMailBox(&LedMailbox, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        state = TclCreateMailBox(&LedMailbox, TCLP_IPC_DUMMY, &error);
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
    state = TclCreateMailBox(&LedMailbox, TCLP_IPC_DUMMY, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IPC_NONE), "");

    /* 初始化Led设备控制线程 */
    state =TclCreateThread(&ThreadLed,
                         &ThreadLedEntry, (TArgument)0,
                         ThreadLedStack, THREAD_LED_STACK_BYTES,
                         THREAD_LED_PRIORITY, THREAD_LED_SLICE,
                         &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 初始化CTRL线程 */
    state =TclCreateThread(&ThreadCTRL,
                         &ThreadCtrlEntry, (TArgument)0,
                         ThreadCTRLStack, THREAD_CTRL_STACK_BYTES,
                         THREAD_CTRL_PRIORITY, THREAD_CTRL_SLICE,
                         &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 激活Led线程 */
    state =TclActivateThread(&ThreadLed, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 激活CTRL线程 */
    state =TclActivateThread(&ThreadCTRL, &error);
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
