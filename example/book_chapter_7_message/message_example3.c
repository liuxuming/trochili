#include "example.h"
#include "trochili.h"


#if (EVB_EXAMPLE == CH7_MESSAGE_EXAMPLE3)

/* 用户线程参数 */
#define THREAD_LED_STACK_BYTES  (512)
#define THREAD_LED_PRIORITY     (7)
#define THREAD_LED_SLICE        (20)

#define THREAD_CTRL_STACK_BYTES (512)
#define THREAD_CTRL_PRIORITY    (6)
#define THREAD_CTRL_SLICE       (20)

/* 用户线程定义 */
static TThread ThreadLed;
static TThread ThreadCTRL;

/* 用户线程栈定义 */
static TBase32 ThreadLedStack[THREAD_LED_STACK_BYTES/4];
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

/* 用户信号量定义 */
static TSemaphore LedSemaphore;

/* Led的线程的主函数 */
static void ThreadLedEntry(TArgument arg)
{
    TState state;
    TLedMsg* pMsg;
    TError error;

    while (eTrue)
    {
        /* Led线程以阻塞方式接收消息 */
        state = TclReceiveMessage(&LedMQ, (TMessage*)(&pMsg),
                                  TCLO_IPC_WAIT, 0, &error);
        if (state == eSuccess)
        {
            /* 如果成功则按照消息的内容来点亮或者熄灭Led */
            EvbLedControl(pMsg->Index, pMsg->Value);

            /* Led线程休眠1秒 */
            state = TclDelayThread(0, TCLM_MLS2TICKS(1000), &error);
            TCLM_ASSERT((state == eSuccess), "");
            TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

            /* Led线程释放信号量 */
            state = TclReleaseSemaphore(&LedSemaphore, TCLO_IPC_WAIT, 0, &error);
            TCLM_ASSERT((state == eSuccess), "");
            TCLM_ASSERT((error == TCLE_IPC_NONE), "");
        }
    }
}


/* CTRL线程的主函数 */
static void ThreadCtrlEntry(TArgument arg)
{
    TState state;
    TError error;
    TLedMsg* pMsg = &LedMsg;
    while (eTrue)
    {
        /* CTRL线程以阻塞方式发送Led点亮消息 */
        pMsg->Index = LED1;
        pMsg->Value = LED_ON;
        state = TclSendMessage(&LedMQ, (TMessage*)(&pMsg), TCLO_IPC_WAIT, 0 , &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        /* CTRL线程以阻塞方式获得信号量 */
        state = TclObtainSemaphore(&LedSemaphore, TCLO_IPC_WAIT, 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        /* CTRL线程以阻塞方式发送Led熄灭消息 */
        pMsg->Index = LED1;
        pMsg->Value = LED_OFF;
        state = TclSendMessage(&LedMQ, (TMessage*)(&pMsg), TCLO_IPC_WAIT , 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        /* CTRL线程以阻塞方式获得信号量 */
        state = TclObtainSemaphore(&LedSemaphore, TCLO_IPC_WAIT, 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");
    }
}


/* 用户应用入口函数 */
static void AppSetupEntry(void)
{
    TState state;
    TError error;

    /* 初始化消息队列和信号量 */
    state = TclCreateMsgQueue(&LedMQ, (void**)(&LedMsgPool),
                            MQ_POOL_LEN, TCLP_IPC_DUMMY, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    state = TclCreateSemaphore(&LedSemaphore, 0, 1, TCLP_IPC_DUMMY, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

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


