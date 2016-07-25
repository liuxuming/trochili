#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH11_MEMORY_POOL_TEST)

/* 用户线程参数 */
#define THREAD_LED_STACK_BYTES  (512)
#define THREAD_LED_PRIORITY     (5)
#define THREAD_LED_SLICE        (20)

/* 用户线程定义 */
static TThread* pThreadLed1;

/* 用户线程栈定义 */
static TBase32 ThreadLed1Stack[THREAD_LED_STACK_BYTES/4];


#define blk_num          (33)
struct test_blck
{
    int i;
    int j;
    int k;
    int m;
    int n;
};
static struct test_blck test_array[blk_num];
static struct test_blck* p0;
static struct test_blck* p1;
static struct test_blck* p2;
static struct test_blck* p3;

static TMemPool blkPool;


/* Led1线程的主函数 */
static void ThreadLed1Entry(TBase32 pArg)
{
    TError error;
    TState state;
    
    TBase32 index = 1;
    while (eTrue)
    {
        state = TclDelayThread((TThread*)0, TCLM_MLS2TICKS(1000), &error);
        EvbLedControl(Led1, LED_ON);

        state = TclDelayThread((TThread*)0, TCLM_MLS2TICKS(1000), &error);
        EvbLedControl(Led1, LED_OFF);

        index++;
    }
}




/* 用户应用程序入口函数 */
static void AppSetupEntry(void)
{
    TError error;
    TState state;
    int i;
	memset(&blkPool, 0U, sizeof(blkPool));
    TclCreateMemoryPool(&blkPool, (void*)test_array, blk_num, sizeof(struct test_blck), &error);

    TclMallocPoolMemory(&blkPool, (void**)(&p0), &error);
    for (i=0; i<(blk_num-2); i++)
        TclMallocPoolMemory(&blkPool, (void**)(&p1), &error);
    TclMallocPoolMemory(&blkPool, (void**)(&p2), &error);

    state = TclMallocPoolMemory(&blkPool, (void**)(&p3), &error);
    state = TclFreePoolMemory(&blkPool, (char*)(p1)+1, &error);
	
    state = TclFreePoolMemory(&blkPool, (void*)(p0), &error);
    state = TclFreePoolMemory(&blkPool, (void*)(p1), &error);
    state = TclFreePoolMemory(&blkPool, (void*)(p2), &error);
    state = TclMallocPoolMemory(&blkPool, (void**)(&p3), &error);
    state = TclMallocPoolMemory(&blkPool, (void**)(&p3), &error);
	state = TclMallocPoolMemory(&blkPool, (void**)(&p3), &error);
		
    /* 初始化Led1设备控制线程 */
    state = TclCreateThread(&ThreadLed1,
                  &ThreadLed1Entry, (TBase32)0,
                  ThreadLed1Stack, THREAD_LED_STACK_BYTES,
                  THREAD_LED_PRIORITY, THREAD_LED_SLICE, &error);

    /* 激活Led线程 */
    state = TclActivateThread(&ThreadLed1, &error);

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

