#include "example.h"
#include "trochili.h"


#if (EVB_EXAMPLE == CH7_MESSAGE_EXAMPLE2)

/* 用户线程参数 */
#define THREAD_LED_STACK_BYTES  (512)
#define THREAD_LED_PRIORITY     (7)
#define THREAD_LED_SLICE        (20)

/* 用户线程定义 */
static TThread ThreadLed;

/* 用户线程栈定义 */
static TBase32 ThreadLedStack[THREAD_LED_STACK_BYTES/4];

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

/* Led线程的主函数 */
static void ThreadLedEntry(TArgument data)
{
    TError error;
    TState state;
    TLedMsg* pMsg;

    while (eTrue)
    {
        /* Led线程以阻塞方式接收消息，如果成功则按照消息的
        内容来点亮或者熄灭Led */
        state = TclReceiveMessage(&LedMQ, (void**)(&pMsg),
                                  TCLO_IPC_WAIT, 0, &error);
        if (state == eSuccess)
        {
            EvbLedControl(pMsg->Index, pMsg->Value);
        }
    }
}

/* 评估板按键中断处理函数 */
static TBitMask EvbKeyISR(TArgument data)
{
    TState state;
    TLedMsg* pMsg = &LedMsg;
    static TBase32 turn = 0;

    if (EvbKeyScan())
    {
        if (turn % 2)
        {
            /* KeyISR发送Led点亮的消息 */
            pMsg->Index = LED1;
            pMsg->Value = LED_ON;
            state = TclIsrSendMessage(&LedMQ, (TMessage*)(&pMsg));
            TCLM_ASSERT((state == eSuccess), "");
        }
        else
        {
            /* KeyISR发送Led熄灭的消息 */
            pMsg->Index = LED1;
            pMsg->Value = LED_OFF;
            state = TclIsrSendMessage(&LedMQ, (TMessage*)(&pMsg));
            TCLM_ASSERT((state == eSuccess), "");
        }
        turn++;
    }

    return 0;
}


/* 用户应用程序入口函数 */
static void AppSetupEntry(void)
{
    TState state;
    TError error;

    /* 设置和KEY相关的外部中断向量 */
    state = TclSetIrqVector(KEY_IRQ_ID, &EvbKeyISR, (TThread*)0, (TArgument)0, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IRQ_NONE), "");

    /* 初始化消息队列 */
    state = TclCreateMsgQueue(&LedMQ, (void**)(&LedMsgPool),
                            MQ_POOL_LEN, TCLP_IPC_DUMMY, &error);
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

    /* 激活Led线程 */
    state = TclActivateThread(&ThreadLed, &error);
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

