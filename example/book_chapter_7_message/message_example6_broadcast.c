#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH7_MESSAGE_EXAMPLE6)

/* 用户线程参数 */
#define THREAD_LED_STACK_BYTES  (512)
#define THREAD_LED_PRIORITY     (7)
#define THREAD_LED_SLICE        (20)

#define THREAD_CTRL_STACK_BYTES (512)
#define THREAD_CTRL_PRIORITY    (8)
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

/* 用户消息类型定义 */
typedef struct
{
    TBase32 Index;
    TBase32 Value;
} TLedMsg;

/* 用户消息队列定义 */
#define MQ_POOL_LEN (32)
static TMsgQueue LedMQ;
static void* LedMsgPool[MQ_POOL_LEN];

/* 用户消息定义 */
static TLedMsg LedMsg;

/* Led1线程的主函数 */
static void ThreadLed1Entry(TArgument data)
{
    TState state;
    TError error;
    TLedMsg* pMsg;

    while (eTrue)
    {
        /* Led3线程以阻塞方式接收消息，按照消息内容来点亮或者熄灭Led3 */
        state = TclReceiveMessage(&LedMQ, (void**)(&pMsg), TCLO_IPC_WAIT,
                                  0, &error);
        if (state == eSuccess)
        {
            EvbLedControl(LED1, pMsg->Value);
        }
    }
}

/* Led2线程的主函数 */
static void ThreadLed2Entry(TArgument data)
{
    TState state;
    TError error;
    TLedMsg* pMsg;

    while (eTrue)
    {
        /* Led3线程以阻塞方式接收消息，按照消息内容来点亮或者熄灭Led3 */
        state = TclReceiveMessage(&LedMQ, (void**)(&pMsg), TCLO_IPC_WAIT,
                                  0, &error);
        if (state == eSuccess)
        {
            EvbLedControl(LED2, pMsg->Value);
        }
    }
}

/* Led3线程的主函数 */
static void ThreadLed3Entry(TArgument data)
{
    TState state;
    TError error;
    TLedMsg* pMsg;

    while (eTrue)
    {
        /* Led3线程以阻塞方式接收消息，按照消息内容来点亮或者熄灭Led3 */
        state = TclReceiveMessage(&LedMQ, (void**)(&pMsg), TCLO_IPC_WAIT,
                                  0, &error);
        if (state == eSuccess)
        {
            EvbLedControl(LED3, pMsg->Value);
        }
    }
}

/* CTRL线程的主函数 */
static void ThreadCtrlEntry(TArgument data)
{
    TState state;
    TError error;
    TLedMsg* pMsg = &LedMsg;

    while (eTrue)
    {
        /* CTRL线程延时1秒，然后广播消息，把所有Led都点亮 */
        state = TclDelayThread((TThread*)0, TCLM_MLS2TICKS(1000), &error);
        pMsg->Index = LEDX;
        pMsg->Value = LED_ON;
        TclBroadcastMessage(&LedMQ, (TMessage*)&pMsg, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        /* CTRL线程延时1秒，然后广播消息，把所有Led都熄灭 */
        state =TclDelayThread((TThread*)0, TCLM_MLS2TICKS(1000), &error);
        pMsg->Index = LEDX;
        pMsg->Value = LED_OFF;
        state = TclBroadcastMessage(&LedMQ, (TMessage*)&pMsg, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");
    }
}


/* 用户应用入口函数 */
static void AppSetupEntry(void)
{
    TState state;
    TError error;

    /* 初始化消息队列 */
    state = TclCreateMsgQueue(&LedMQ, (void**)(&LedMsgPool),
                            MQ_POOL_LEN, TCLP_IPC_DUMMY, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IPC_NONE), "");

    /* 初始化Led设备1控制线程 */
    state = TclCreateThread(&ThreadLed1,
                          &ThreadLed1Entry, (TArgument)0,
                          ThreadLed1Stack, THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY, THREAD_LED_SLICE,
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 初始化Led设备2控制线程 */
    state = TclCreateThread(&ThreadLed2,
                          &ThreadLed2Entry, (TArgument)0,
                          ThreadLed2Stack, THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY, THREAD_LED_SLICE,
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 初始化Led设备3控制线程 */
    state = TclCreateThread(&ThreadLed3,
                          &ThreadLed3Entry, (TArgument)0,
                          ThreadLed3Stack, THREAD_LED_STACK_BYTES,
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

    /* 激活Led线程1 */
    state = TclActivateThread(&ThreadLed1, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 激活Led线程2 */
    state = TclActivateThread(&ThreadLed2, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 激活Led线程3 */
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


