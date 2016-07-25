#include "example.h"
#include "trochili.h"


#if (EVB_EXAMPLE == CH7_MESSAGE_EXAMPLE4)

/* 用户线程参数 */
#define THREAD_LED_STACK_BYTES  (512)
#define THREAD_LED_PRIORITY     (7)
#define THREAD_LED_SLICE        (20)

/* 用户线程定义 */
static TThread ThreadLed1;
static TThread ThreadLed2;

/* 用户线程栈定义 */
static TBase32 ThreadLed1Stack[THREAD_LED_STACK_BYTES/4];
static TBase32 ThreadLed2Stack[THREAD_LED_STACK_BYTES/4];

/* 用户消息类型定义 */
typedef struct
{
    TBase32 Index;
    TBase32 Value;
} TLedMsg;

/* 用户消息队列定义 */
#define MQ_POOL_LEN (32)
static TMsgQueue LedMQ1;
static TMsgQueue LedMQ2;
static void* Led1MsgPool[MQ_POOL_LEN];
static void* Led2MsgPool[MQ_POOL_LEN];

/* 用户消息定义 */
static TLedMsg LedMsg1;
static TLedMsg LedMsg2;

/* Led1的线程的主函数 */
static void ThreadLed1Entry(TArgument arg)
{
    TState state;
    TError error;
    TLedMsg* pMsg1 = &LedMsg1;
    TLedMsg* pMsg2 = &LedMsg2;

    while (eTrue)
    {
        /* Led1线程以阻塞方式发送Led1点亮消息 */
        pMsg2->Index = LED2;
        pMsg2->Value = LED_ON;
        state = TclSendMessage(&LedMQ2, (void**)(&pMsg2), TCLO_IPC_WAIT,  0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        /* Led1线程以阻塞方式接收消息，根据消息内容来点亮或者熄灭Led1 */
        state = TclReceiveMessage(&LedMQ1, (void**)(&pMsg1), TCLO_IPC_WAIT, 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");
        EvbLedControl(pMsg1->Index, pMsg1->Value);

        pMsg2->Index = LED2;
        pMsg2->Value = LED_OFF;
        state = TclSendMessage(&LedMQ2, (void**)(&pMsg2), TCLO_IPC_WAIT,  0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        /* Led1线程以阻塞方式接收消息，根据消息内容来点亮或者熄灭Led1 */
        state = TclReceiveMessage(&LedMQ1, (void**)(&pMsg1), TCLO_IPC_WAIT, 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");
        EvbLedControl(pMsg1->Index, pMsg1->Value);
    }
}


/* Led2线程的主函数 */
static void ThreadLed2Entry(TArgument arg)
{
    TState state;
    TError error;
    TLedMsg* pMsg1 = &LedMsg1;
    TLedMsg* pMsg2 = &LedMsg2;

    while (eTrue)
    {
        /* Led2线程以阻塞方式接收消息，根据消息内容来点亮或者熄灭Led2 */
        state = TclReceiveMessage(&LedMQ2, (void**)(&pMsg2), TCLO_IPC_WAIT, 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");
        EvbLedControl(pMsg2->Index, pMsg2->Value);

        /* Led2线程延时1秒 */
        state = TclDelayThread(0, TCLM_MLS2TICKS(1000), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

        /* Led2线程以阻塞方式发送Led1点亮消息 */
        pMsg1->Index = LED1;
        pMsg1->Value = LED_ON;
        state = TclSendMessage(&LedMQ1, (void**)(&pMsg1), TCLO_IPC_WAIT, 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        /* Led2线程以阻塞方式接收消息，根据消息内容来点亮或者熄灭Led2 */
        state = TclReceiveMessage(&LedMQ2, (void**)(&pMsg2), TCLO_IPC_WAIT, 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");
        EvbLedControl(pMsg2->Index, pMsg2->Value);

        /* Led2线程延时1秒 */
        state = TclDelayThread(0, TCLM_MLS2TICKS(1000), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

        /* Led2线程以阻塞方式发送Led1熄灭消息 */
        pMsg1->Index = LED1;
        pMsg1->Value = LED_OFF;
        state = TclSendMessage(&LedMQ1, (void**)(&pMsg1), TCLO_IPC_WAIT, 0, &error);
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
    state = TclCreateMsgQueue(&LedMQ1, (void**)(&Led1MsgPool),
                    MQ_POOL_LEN, TCLP_IPC_DUMMY, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IPC_NONE), "");

    state = TclCreateMsgQueue(&LedMQ2, (void**)(&Led2MsgPool),
                    MQ_POOL_LEN, TCLP_IPC_DUMMY, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IPC_NONE), "");

    /* 初始化Led设备控制线程1 */
    state = TclCreateThread(&ThreadLed1,
                  &ThreadLed1Entry, (TArgument)0,
                  ThreadLed1Stack, THREAD_LED_STACK_BYTES,
                  THREAD_LED_PRIORITY, THREAD_LED_SLICE,
                  &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 初始化Led设备控制线程2 */
    state = TclCreateThread(&ThreadLed2,
                  &ThreadLed2Entry, (TArgument)0,
                  ThreadLed2Stack, THREAD_LED_STACK_BYTES,
                  THREAD_LED_PRIORITY, THREAD_LED_SLICE,
                  &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 激活Led线程1 */
    state = TclActivateThread(&ThreadLed1,&error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 激活Led线程2 */
    state = TclActivateThread(&ThreadLed2,&error);
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
