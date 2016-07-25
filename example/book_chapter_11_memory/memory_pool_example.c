#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH11_MEMORY_POOL_EXAMPLE)

/* 用户线程参数 */
#define THREAD_LED_STACK_BYTES  (512)
#define THREAD_LED_PRIORITY     (5)
#define THREAD_LED_SLICE        (20)

#define DATA_BLOCK_BYTES  THREAD_LED_STACK_BYTES


/* 用户线程定义 */
static TThread* pThreadLed1;
static TThread* pThreadLed2;
static TThread* pThreadLed3;


/* 用户线程栈定义 */
static TBase32* pThreadStack1;
static TBase32* pThreadStack2;
static TBase32* pThreadStack3;


static TByte DataMemory[DATA_BLOCK_BYTES * 6];
static TMemPool DataMemoryPool;

/* Led1\Led2\led3线程的主函数 */
static void ThreadLedEntry(TArgument data)
{
    TError error;
    TState state;

    while (eTrue)
    {
        state = TclDelayThread((TThread*)0, TCLM_MLS2TICKS(1000), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");
        EvbLedControl(data, LED_ON);

        state = TclDelayThread((TThread*)0, TCLM_MLS2TICKS(1000), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");
        EvbLedControl(data, LED_OFF);
    }
}

/* 用户应用程序入口函数 */
static void AppSetupEntry(void)
{
    TError error;
    TState state;

    state = TclCreateMemoryPool(&DataMemoryPool, (void*)DataMemory, 6, DATA_BLOCK_BYTES, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_MEMORY_NONE), "");

    state = TclMallocPoolMemory(&DataMemoryPool, (void**)(&pThreadLed1), &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_MEMORY_NONE), "");

    state = TclMallocPoolMemory(&DataMemoryPool, (void**)(&pThreadLed2), &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_MEMORY_NONE), "");

    state = TclMallocPoolMemory(&DataMemoryPool, (void**)(&pThreadLed3), &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_MEMORY_NONE), "");

    state = TclMallocPoolMemory(&DataMemoryPool, (void**)(&pThreadStack1), &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_MEMORY_NONE), "");

    state = TclMallocPoolMemory(&DataMemoryPool, (void**)(&pThreadStack2), &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_MEMORY_NONE), "");

    state = TclMallocPoolMemory(&DataMemoryPool, (void**)(&pThreadStack3), &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_MEMORY_NONE), "");

    /* 初始化Led线程1 */
    state = TclCreateThread(pThreadLed1,
                          &ThreadLedEntry, (TArgument)LED1,
                          pThreadStack1, THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY, THREAD_LED_SLICE,
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 初始化Led线程2*/
    state = TclCreateThread(pThreadLed2,
                          &ThreadLedEntry,(TArgument)LED2,
                          pThreadStack2, THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY, THREAD_LED_SLICE,
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 初始化Led线程3 */
    state = TclCreateThread(pThreadLed3,
                          &ThreadLedEntry, (TArgument)LED3,
                          pThreadStack3, THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY, THREAD_LED_SLICE,
                          &error);

    /* 激活Led线程1 */
    state = TclActivateThread(pThreadLed1, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 激活Led线程2 */
    state = TclActivateThread(pThreadLed2, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 激活Led线程3 */
    state = TclActivateThread(pThreadLed3, &error);
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

