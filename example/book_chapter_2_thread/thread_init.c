
#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH2_THREAD_EXAMPLE1)

#define THREAD_LED_STACK_BYTES (512)
#define THREAD_LED_PRIORITY    (5)
#define THREAD_LED_SLICE       (20)

static TThread ThreadLed;
static TBase32 ThreadLedStack[THREAD_LED_STACK_BYTES/4];


static delay(TBase32 count)
{
    while (count--)
        ;
}

/* 线程Led的入口函数 */
static void ThreadLedEntry(TArgument data)
{
    while (eTrue)
    {
        delay(0x8FFFFF);
        EvbLedControl(LED1, LED_ON);

        delay(0x8FFFFF);
        EvbLedControl(LED1, LED_OFF);

        delay(0x8FFFFF);
        EvbLedControl(LED2, LED_ON);

        delay(0x8FFFFF);
        EvbLedControl(LED2, LED_OFF);

        delay(0x8FFFFF);
        EvbLedControl(LED3, LED_ON);

        delay(0x8FFFFF);
        EvbLedControl(LED3, LED_OFF);
    }
}

/* 用户应用入口函数 */
static void AppSetupEntry(void)
{
    TError error;
    TState state;

    /* 初始化Led设备控制线程 */
    state = TclCreateThread(&ThreadLed,
                          &ThreadLedEntry, (TArgument)0,
                          ThreadLedStack,  THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY, THREAD_LED_SLICE,
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");
	
    /* 激活Led点亮线程 */
    state =TclActivateThread(&ThreadLed, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    TclTrace("example start!\r\n");
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
