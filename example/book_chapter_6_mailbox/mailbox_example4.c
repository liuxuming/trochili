#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH6_MAILBOX_EXAMPLE4)

/* 用户线程参数 */
#define THREAD_LED_STACK_BYTES  (512)
#define THREAD_LED_PRIORITY     (5)
#define THREAD_LED_SLICE        (20)

/* 用户线程定义 */
static TThread ThreadLed1;
static TThread ThreadLed2;

/* 用户线程栈定义 */
static TBase32 ThreadLed1Stack[THREAD_LED_STACK_BYTES/4];
static TBase32 ThreadLed2Stack[THREAD_LED_STACK_BYTES/4];

/* 用户邮件类型定义 */
typedef struct
{
    TIndex Index;
    TByte Value;
} TLedMail;


static TMailBox Led1MailBox;
static TMailBox Led2MailBox;
static TLedMail Led1Mail;
static TLedMail Led2Mail;


/* Led1线程的主函数 */
static void ThreadLed1Entry(TArgument data)
{
    TState state;
    TError error;

    TLedMail* pMail1;
    TLedMail* pMail2;

    pMail2 = &Led2Mail;
    while (eTrue)
    {
        /* Led1线程以阻塞方式接收Led1控制邮件 */
        state = TclReceiveMail(&Led1MailBox, (TMail*)(&pMail1), TCLO_IPC_WAIT, 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        /* Led1线程控制Led1的点亮或熄灭 */
        EvbLedControl(pMail1->Index, pMail1->Value);

        /* Led1线程延时1秒 */
        state = TclDelayThread((TThread*)0, TCLM_MLS2TICKS(1000), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

        /* Led1线程以阻塞方式发送Led2点亮邮件 */
        pMail2->Index = LED2;
        pMail2->Value = LED_ON;
        state = TclSendMail(&Led2MailBox, (TMail*)(&pMail2), TCLO_IPC_WAIT, 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        /* Led1线程以阻塞方式接收Led1控制邮件 */
        state = TclReceiveMail(&Led1MailBox, (TMail*)(&pMail1), TCLO_IPC_WAIT, 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        /* Led1线程控制Led1的点亮或熄灭 */
        EvbLedControl(pMail1->Index, pMail1->Value);

        /* Led1线程延时1秒 */
        state = TclDelayThread((TThread*)0, TCLM_MLS2TICKS(1000), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

        /* Led1线程以阻塞方式发送Led2熄灭邮件 */
        pMail2->Index = LED2;
        pMail2->Value = LED_OFF;
        state = TclSendMail(&Led2MailBox, (TMail*)(&pMail2), TCLO_IPC_WAIT, 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");
    }
}

/* Led2线程的主函数 */
static void ThreadLed2Entry(TArgument data)
{
    TState state;
    TError error;

    TLedMail* pMail1;
    TLedMail* pMail2;

    pMail1 = &Led1Mail;
    while (eTrue)
    {
        /* Led2线程以阻塞方式发送Led1点亮邮件 */
        pMail1->Index = LED1;
        pMail1->Value = LED_ON;
        state = TclSendMail(&Led1MailBox, (TMail*)(&pMail1), TCLO_IPC_WAIT, 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        /* Led2线程以阻塞方式接收Led2控制邮件 */
        state = TclReceiveMail(&Led2MailBox, (TMail*)(&pMail2), TCLO_IPC_WAIT, 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        /* Led2线程控制Led2的点亮或熄灭 */
        EvbLedControl(pMail2->Index, pMail2->Value);

        /* Led2线程以阻塞方式发送Led1熄灭邮件 */
        pMail1->Index = LED1;
        pMail1->Value = LED_OFF;
        state = TclSendMail(&Led1MailBox, (TMail*)(&pMail1), TCLO_IPC_WAIT, 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        /* Led2线程以阻塞方式接收Led2控制邮件 */
        state = TclReceiveMail(&Led2MailBox, (TMail*)(&pMail2), TCLO_IPC_WAIT, 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        /* Led1线程控制Led1的点亮或熄灭 */
        EvbLedControl(pMail2->Index, pMail2->Value);
    }
}

/* 用户应用入口函数 */
static void AppSetupEntry(void)
{
    TState state;
    TError error;

    /* 初始化Led邮箱 */
    state = TclCreateMailBox(&Led2MailBox, TCLP_IPC_DUMMY, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IPC_NONE), "");

    state = TclCreateMailBox(&Led1MailBox, TCLP_IPC_DUMMY, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IPC_NONE), "");

    /* 初始化Led1设备控制线程 */
    state = TclCreateThread(&ThreadLed1,
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
                           THREAD_LED_PRIORITY + 1, THREAD_LED_SLICE,
                           &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 激活Led设备控制线程 */
    state = TclActivateThread(&ThreadLed1, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    state = TclActivateThread(&ThreadLed2, &error);
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
