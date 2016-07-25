/* 本例演示线程间异步单向邮件收发。两个线程间不存在同步关系，各自独立收/发邮件 */
#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH6_MAILBOX_EXAMPLE1)

/* 用户线程参数 */
#define THREAD_LED_STACK_BYTES  (512)
#define THREAD_LED_PRIORITY     (6)
#define THREAD_LED_SLICE        (20)

#define THREAD_CTRL_STACK_BYTES (512)
#define THREAD_CTRL_PRIORITY    (5)
#define THREAD_CTRL_SLICE       (20)

/* 用户线程定义 */
static TThread ThreadLed;
static TThread ThreadCTRL;

/* 用户线程栈定义 */
static TBase32 ThreadLedStack[THREAD_LED_STACK_BYTES/4];
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

/* Led线程的主函数 */
static void ThreadLedEntry(TArgument arg)
{
    TState state;
    TError error;
    TLedMail* pMail;

    while (eTrue)
    {
        /* Led线程通过邮箱接收邮件，采用非阻塞方式发送 */
        state = TclReceiveMail(&LedMailbox, (TMail*)(&pMail), 0, 0, &error);
        if (state == eSuccess)
        {
            EvbLedControl(pMail->Index, pMail->Value);
        }
    }
}

/* CTRL线程的主函数 */
static void ThreadCtrlEntry(TArgument arg)
{
    TState state;
    TError error;
    TLedMail* pMail = &LedMail;

    while (eTrue)
    {
        /* 控制线程通过邮箱发送Led点亮邮件，采用非阻塞方式发送 */
        pMail->Index = LED1;
        pMail->Value = LED_ON;
        state = TclSendMail(&LedMailbox, (TMail*)(&pMail), 0, 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");;

        /* 控制线程延时1秒 */
        state = TclDelayThread(0, TCLM_MLS2TICKS(1000), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

        /* 控制线程通过邮箱发送Led熄灭邮件，采用非阻塞方式发送 */
        pMail->Index = LED1;
        pMail->Value = LED_OFF;
        state = TclSendMail(&LedMailbox, (TMail*)(&pMail), 0, 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        /* 控制线程延时1秒 */
        state = TclDelayThread(0, TCLM_MLS2TICKS(1000), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");
    }
}


/* 用户应用程序入口函数 */
static void AppSetupEntry(void)
{
    TState state;
    TError error;

    state = TclCreateMailBox(&LedMailbox, TCLP_IPC_DUMMY, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IPC_NONE), "");

    /* 初始化Led设备控制线程 */
    state = TclCreateThread(&ThreadLed,
                          &ThreadLedEntry, (TBase32)0U,
                          ThreadLedStack, THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY, THREAD_LED_SLICE,
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 初始化CTRL线程 */
    state = TclCreateThread(&ThreadCTRL,
                          &ThreadCtrlEntry, (TBase32)0U,
                          ThreadCTRLStack, THREAD_CTRL_STACK_BYTES,
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
