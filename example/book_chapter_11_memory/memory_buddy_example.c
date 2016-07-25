#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH11_MEMORY_BUDDY_EXAMPLE)
#define THREAD_MEMORY_STACK_BYTES  (512)
#define THREAD_MEMORY_PRIORITY     (5)
#define THREAD_MEMORY_SLICE        (65535u)

/* MEM线程结构和栈 */
static TThread ThreadMem;
static TBase32 ThreadMemStack[THREAD_MEMORY_STACK_BYTES/4];

#define MEMORY_PAGE_SIZE (32)
#define MEMORY_PAGES    (TCLC_MEMORY_BUDDY_PAGES)

static TMemBuddy mem;
static char memPool[MEMORY_PAGES * MEMORY_PAGE_SIZE];
static void ThreadMemEntry(TArgument arg)
{
    TState state;
    void* addr0;
    void* addr1;
    void* addr2;
    void* addr3;
    TError error;


    state = TclCreateMemBuddy(&mem, memPool, MEMORY_PAGES, MEMORY_PAGE_SIZE, &error);

    while (eTrue)
    {
#if 0
        state = TclMallocBuddyMem(&mem, MEMORY_PAGE_SIZE*0, &addr0, &error);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 malloc error\r\n");
            while(1);
        }

#endif
#if 0
        state = TclMallocBuddyMem(&mem, MEMORY_PAGE_SIZE, &addr0, &error);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 malloc error\r\n");
            while(1);
        }
#endif

#if 0
        state = TclMallocBuddyMem(&mem, MEMORY_PAGE_SIZE*2, &addr0, &error);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 malloc error\r\n");
            while(1);
        }

#endif

#if 0
        state = TclMallocBuddyMem(&mem, MEMORY_PAGE_SIZE*3, &addr0);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 malloc error\r\n");
            while(1);
        }

#endif

#if 0
        //  //3.2.3.0.2,2,2
        state = TclMallocBuddyMem(&mem, MEMORY_PAGE_SIZE*4, &addr0, &error);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 malloc error\r\n");
            while(1);
        }
        //  //4,3,3,2,2,2,2
        state = TclFreeBuddyMem(&mem, addr0, &error);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 free error\r\n");
            while(1);
        }
#endif
#if 0
// 3.0.3.2.2.2.2
        state = TclMallocBuddyMem(&mem, MEMORY_PAGE_SIZE*5, &addr0);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 malloc error\r\n");
            while(1);
        }

        state = TclFreeBuddyMem(&mem, addr0);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 free error\r\n");
            while(1);
        }
#endif
#if 1
// 3.0.3.2.2.2.2
        state = TclMallocBuddyMem(&mem, MEMORY_PAGE_SIZE*6, &addr0, &error);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 malloc error\r\n");
            while(1);
        }

        state = TclFreeBuddyMem(&mem, addr0, &error);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 free error\r\n");
            while(1);
        }
#endif

#if 0
// 3.0.3.2.2.2.2
        state = TclMallocBuddyMem(&mem, MEMORY_PAGE_SIZE*7, &addr0);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 malloc error\r\n");
            while(1);
        }

        state = TclFreeBuddyMem(&mem, addr0);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 free error\r\n");
            while(1);
        }
#endif

#if 0
// 3.0.3.2.2.2.2
        state = TclMallocBuddyMem(&mem, MEMORY_PAGE_SIZE*8, &addr0);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 malloc error\r\n");
            while(1);
        }

        state = TclFreeBuddyMem(&mem, addr0);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 free error\r\n");
            while(1);
        }
#endif

#if 0
// 0.3.3.2.2.2.2
        state = TclMallocBuddyMem(&mem, MEMORY_PAGE_SIZE*9, &addr0);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 malloc error\r\n");
            while(1);
        }

        state = TclFreeBuddyMem(&mem, addr0);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 free error\r\n");
            while(1);
        }
#endif

#if 0
        // 0.3.3.2.2.2.2
        state = TclMallocBuddyMem(&mem, MEMORY_PAGE_SIZE*15, &addr0);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 malloc error\r\n");
            while(1);
        }

        state = TclFreeBuddyMem(&mem, addr0);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 free error\r\n");
            while(1);
        }
#endif

#if 0
// 0.3.3.2.2.2.2
        state = TclMallocBuddyMem(&mem, MEMORY_PAGE_SIZE*16, &addr0);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 malloc error\r\n");
            while(1);
        }

        state = TclFreeBuddyMem(&mem, addr0);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 free error\r\n");
            while(1);
        }
#endif

#if 0
// req len overflow
        state = TclMallocBuddyMem(&mem, MEMORY_PAGE_SIZE*17, &addr0);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 malloc error\r\n");
            while(1);
        }

        state = TclFreeBuddyMem(&mem, addr0);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 free error\r\n");
            while(1);
        }
#endif

#if 0//34
        state = TclMallocBuddyMem(&mem, MEMORY_PAGE_SIZE*3, &addr0);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 malloc error\r\n");
            while(1);
        }

        state = TclMallocBuddyMem(&mem, MEMORY_PAGE_SIZE*4, &addr1);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr1 malloc error\r\n");
            while(1);
        }
#endif

#if 0//78
        state = TclMallocBuddyMem(&mem, MEMORY_PAGE_SIZE*7, &addr0);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 malloc error\r\n");
            while(1);
        }

        state = TclMallocBuddyMem(&mem, MEMORY_PAGE_SIZE*8, &addr1);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr1 malloc error\r\n");
            while(1);
        }
#endif

#if 0//12312311
        state = TclMallocBuddyMem(&mem, MEMORY_PAGE_SIZE*1, &addr0);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 malloc error\r\n");
            while(1);
        }

        state = TclMallocBuddyMem(&mem, MEMORY_PAGE_SIZE*2, &addr1);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr1 malloc error\r\n");
            while(1);
        }

        state = TclMallocBuddyMem(&mem, MEMORY_PAGE_SIZE*3, &addr1);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr1 malloc error\r\n");
            while(1);
        }

        state = TclMallocBuddyMem(&mem, MEMORY_PAGE_SIZE*1, &addr0);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 malloc error\r\n");
            while(1);
        }

        state = TclMallocBuddyMem(&mem, MEMORY_PAGE_SIZE*2, &addr1);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr1 malloc error\r\n");
            while(1);
        }

        state = TclMallocBuddyMem(&mem, MEMORY_PAGE_SIZE*3, &addr1);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr1 malloc error\r\n");
            while(1);
        }

        state = TclMallocBuddyMem(&mem, MEMORY_PAGE_SIZE*1, &addr0);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 malloc error\r\n");
            while(1);
        }

        state = TclMallocBuddyMem(&mem, MEMORY_PAGE_SIZE*1, &addr0);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 malloc error\r\n");
            while(1);
        }
#endif

#if 0// 1.1.1.1
        state = TclMallocBuddyMem(&mem, MEMORY_PAGE_SIZE*1, &addr0);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 malloc error\r\n");
            while(1);
        }

        state = TclMallocBuddyMem(&mem, MEMORY_PAGE_SIZE*1, &addr1);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 malloc error\r\n");
            while(1);
        }

        state = TclMallocBuddyMem(&mem, MEMORY_PAGE_SIZE*1, &addr2);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 malloc error\r\n");
            while(1);
        }

        state = TclMallocBuddyMem(&mem, MEMORY_PAGE_SIZE*1, &addr3);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 malloc error\r\n");
            while(1);
        }

        state = TclFreeBuddyMem(&mem, addr0);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 free error\r\n");
            while(1);
        }

        state = TclFreeBuddyMem(&mem, addr3);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 free error\r\n");
            while(1);
        }

        state = TclFreeBuddyMem(&mem, addr1);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 free error\r\n");
            while(1);
        }

        state = TclFreeBuddyMem(&mem, addr2);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 free error\r\n");
            while(1);
        }
#endif

#if 0//
        state = TclMallocBuddyMem(&mem, MEMORY_PAGE_SIZE*1, &addr0);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 malloc error\r\n");
            while(1);
        }

        state = TclMallocBuddyMem(&mem, MEMORY_PAGE_SIZE*1, &addr1);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 malloc error\r\n");
            while(1);
        }

        state = TclMallocBuddyMem(&mem, MEMORY_PAGE_SIZE*1, &addr2);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 malloc error\r\n");
            while(1);
        }

        state = TclMallocBuddyMem(&mem, MEMORY_PAGE_SIZE*1, &addr3);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 malloc error\r\n");
            while(1);
        }

        state = TclFreeBuddyMem(&mem, addr0);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 free error\r\n");
            while(1);
        }

        state = TclFreeBuddyMem(&mem, addr3);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 free error\r\n");
            while(1);
        }

        state = TclFreeBuddyMem(&mem, addr2);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 free error\r\n");
            while(1);
        }

        state = TclFreeBuddyMem(&mem, addr1);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 free error\r\n");
            while(1);
        }
#endif

#if 0//
        state = TclMallocBuddyMem(&mem, MEMORY_PAGE_SIZE*1, &addr0, &error);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 malloc error\r\n");
            while(1);
        }

        state = TclMallocBuddyMem(&mem, MEMORY_PAGE_SIZE*1, &addr1, &error);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 malloc error\r\n");
            while(1);
        }

        state = TclMallocBuddyMem(&mem, MEMORY_PAGE_SIZE*1, &addr2, &error);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 malloc error\r\n");
            while(1);
        }

        state = TclMallocBuddyMem(&mem, MEMORY_PAGE_SIZE*1, &addr3, &error);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 malloc error\r\n");
            while(1);
        }

        state = TclFreeBuddyMem(&mem, addr0, &error);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 free error\r\n");
            while(1);
        }

        state = TclFreeBuddyMem(&mem, addr1, &error);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 free error\r\n");
            while(1);
        }

        state = TclFreeBuddyMem(&mem, addr2, &error);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 free error\r\n");
            while(1);
        }

        state = TclFreeBuddyMem(&mem, addr3, &error);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 free error\r\n");
            while(1);
        }
#endif

#if 0//
        state = TclMallocBuddyMem(&mem, MEMORY_PAGE_SIZE*1, &addr0);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 malloc error\r\n");
            while(1);
        }

        state = TclMallocBuddyMem(&mem, MEMORY_PAGE_SIZE*1, &addr1);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 malloc error\r\n");
            while(1);
        }

        state = TclMallocBuddyMem(&mem, MEMORY_PAGE_SIZE*1, &addr2);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 malloc error\r\n");
            while(1);
        }

        state = TclMallocBuddyMem(&mem, MEMORY_PAGE_SIZE*1, &addr3);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 malloc error\r\n");
            while(1);
        }

        state = TclFreeBuddyMem(&mem, addr3);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 free error\r\n");
            while(1);
        }

        state = TclFreeBuddyMem(&mem, addr2);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 free error\r\n");
            while(1);
        }

        state = TclFreeBuddyMem(&mem, addr1);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 free error\r\n");
            while(1);
        }

        state = TclFreeBuddyMem(&mem, addr0);
        if (state != eSuccess)
        {
            EVB_PRINTF("addr0 free error\r\n");
            while(1);
        }
#endif

    }
}


/* 用户应用入口函数 */
static void AppSetupEntry(void)
{
    TState state;
    TError error;

    /* 初始化MEM设备控制线程 */
    state = TclCreateThread(&ThreadMem,
                          &ThreadMemEntry, (TArgument)0,
                          ThreadMemStack, THREAD_MEMORY_STACK_BYTES,
                          THREAD_MEMORY_PRIORITY,
                          THREAD_MEMORY_SLICE,
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 激活MEM设备控制线程 */
    state = TclActivateThread(&ThreadMem, &error);
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

#if 0
#define THREAD_MEMORY_STACK_BYTES  (512)
#define THREAD_MEMORY_PRIORITY    (5)
#define THREAD_MEMORY_SLICE       (65535u)

/* MEM线程结构和栈 */
static TThread ThreadMem;
static TBase32 ThreadMemStack[THREAD_MEMORY_STACK_BYTES/4];

#define MEM_CACHE_SIZE (32+sizeof(TMemBlock))
static TMemHeap MemHeap;
#pragma pack(4)
static char MemCache[MEM_CACHE_SIZE * 5];
#pragma pack()

static void* addr0;
static void* addr1;
static void* addr2;
static void* addr3;
static void* addr4;
static void* addr5;

static void mem_dump(TMemHeap* heap)
{
    TMemBlock* block;
    char* data;
    int i;
    /* 查找合适的内存页进行内存分配 */
    block = (TMemBlock*)(heap->Addr);

    while (block != (TMemBlock*)0)
    {
        EVB_PRINTF("\r\n" );
        EVB_PRINTF("Block.Addr = 0x%x\r\n", block);
        EVB_PRINTF("Block.Len = 0x%x\r\n", block->Len);
        EVB_PRINTF("Block.Property = 0x%x\r\n", block->Property);
        EVB_PRINTF("Block.Prev = 0x%x\r\n", block->Prev);
        EVB_PRINTF("Block.Next = 0x%x\r\n", block->Next);
        EVB_PRINTF("Block.Data = 0x%x\r\n", (char*)block + MEM_BLK_HDR_LEN );
        data = (char*)(block) + MEM_BLK_HDR_LEN;
        for(i = 0; i < (block->Len - MEM_BLK_HDR_LEN); i++)
        {
            if ((i % 16 == 0) && (i != 0))
                EVB_PRINTF("\r\n" );
            EVB_PRINTF("0x%02x ", data[i]);

        }
        EVB_PRINTF("\r\n" );
        block = block->Next;
    }
}

static void mem_prepare(void)
{
    TState state;

    state = TclMallocHeapMemory(&MemHeap, 32, &addr0);
    if (state != eSuccess)
    {
        EVB_PRINTF("0a malloc error\r\n");
        while(1);
    }
    memset(addr0, 'a' , 32);

    state = TclMallocHeapMemory(&MemHeap, 32, &addr1);
    if (state != eSuccess)
    {
        EVB_PRINTF("1b malloc error\r\n");
        while(1);
    }
    memset(addr1, 'b' , 32);

    state = TclMallocHeapMemory(&MemHeap, 32, &addr2);
    if (state != eSuccess)
    {
        EVB_PRINTF("2c malloc error\r\n");
        while(1);
    }
    memset(addr2, 'c' , 32);

    state = TclMallocHeapMemory(&MemHeap, 32, &addr3);
    if (state != eSuccess)
    {
        EVB_PRINTF("3d malloc error\r\n");
        while(1);
    }
    memset(addr3, 'd' , 32);

    state = TclMallocHeapMemory(&MemHeap, 32, &addr4);
    if (state != eSuccess)
    {
        EVB_PRINTF("4e malloc error\r\n");
        while(1);
    }
    memset(addr4, 'e' , 32);
    mem_dump(&MemHeap);

    EVB_PRINTF("\r\naddr0= 0x%02x,addr1= 0x%02x,addr2= 0x%02x,addr3= 0x%02x,addr4= 0x%02x\r\n ",
               addr0, addr1, addr2, addr3 , addr4 );
}
// 1
static void mem_check_1(void)
{
    TState state;

    mem_prepare();

    EVB_PRINTF("\r\nfree addr3= 0x%02x\r\n ", addr3 );
    TclFreeHeapMemory(&MemHeap, addr3);
    EVB_PRINTF("free addr2= 0x%02x\r\n ", addr2 );
    TclFreeHeapMemory(&MemHeap, addr2);

    state = TclMallocHeapMemory(&MemHeap, 64, &addr5);
    if (state != eSuccess)
    {
        EVB_PRINTF("5f malloc error\r\n");
    }
    EVB_PRINTF("malloc addr5= 0x%02x\r\n ", addr5 );
    memset(addr5, 'f' , 64);
    mem_dump(&MemHeap);
}

static void mem_check_2(void)
{
    TState state;
    mem_prepare();

    EVB_PRINTF("\r\nfree addr2= 0x%02x\r\n ", addr2 );
    TclFreeHeapMemory(&MemHeap, addr2);
    EVB_PRINTF("free addr3= 0x%02x\r\n ", addr3 );
    TclFreeHeapMemory(&MemHeap, addr3);


    state = TclMallocHeapMemory(&MemHeap, 64, &addr5);
    if (state != eSuccess)
    {
        EVB_PRINTF("5f malloc error\r\n");
    }
    EVB_PRINTF("malloc addr5= 0x%02x\r\n ", addr5 );
    memset(addr5, 'f' , 64);
    mem_dump(&MemHeap);

}

static void mem_check_3(void)
{
    TState state;
    mem_prepare();

    EVB_PRINTF("\r\n free addr1= 0x%02x\r\n ", addr1 );
    TclFreeHeapMemory(&MemHeap, addr1);
    EVB_PRINTF("\r\n free addr3= 0x%02x\r\n ", addr3 );
    TclFreeHeapMemory(&MemHeap, addr3);
    EVB_PRINTF("\r\n free addr2= 0x%02x\r\n ", addr2 );
    TclFreeHeapMemory(&MemHeap, addr2);

    state = TclMallocHeapMemory(&MemHeap, 64, &addr5);
    if (state != eSuccess)
    {
        EVB_PRINTF("5f malloc error\r\n");
    }
    EVB_PRINTF("\r\n malloc addr5= 0x%02x\r\n ", addr5 );
    memset(addr5, 'f' , 64);
    mem_dump(&MemHeap);

}

static void mem_check_3_1(void)
{
    TState state;
    mem_prepare();

    EVB_PRINTF("\r\n free addr1= 0x%02x\r\n ", addr1 );
    TclFreeHeapMemory(&MemHeap, addr1);
    EVB_PRINTF("\r\n free addr3= 0x%02x\r\n ", addr3 );
    TclFreeHeapMemory(&MemHeap, addr3);
    EVB_PRINTF("\r\n free addr2= 0x%02x\r\n ", addr2 );
    TclFreeHeapMemory(&MemHeap, addr2);

    state = TclMallocHeapMemory(&MemHeap, 16 * 8 - 1, &addr5);
    if (state != eSuccess)
    {
        EVB_PRINTF("5f malloc error\r\n");
        while(1);
    }
    EVB_PRINTF("\r\n malloc addr5= 0x%02x\r\n ", addr5 );
    memset(addr5, 'f' , 16 * 8 - 1);
    mem_dump(&MemHeap);

}

static void mem_check_3_2(void)
{
    TState state;
    mem_prepare();

    EVB_PRINTF("\r\n free addr1= 0x%02x\r\n ", addr1 );
    TclFreeHeapMemory(&MemHeap, addr1);
    EVB_PRINTF("\r\n free addr3= 0x%02x\r\n ", addr3 );
    TclFreeHeapMemory(&MemHeap, addr3);
    EVB_PRINTF("\r\n free addr2= 0x%02x\r\n ", addr2 );
    TclFreeHeapMemory(&MemHeap, addr2);

    state = TclMallocHeapMemory(&MemHeap, 16 * 8 , &addr5);
    if (state != eSuccess)
    {
        EVB_PRINTF("5f malloc error\r\n");
        while(1);
    }
    EVB_PRINTF("\r\n malloc addr5= 0x%02x\r\n ", addr5 );
    memset(addr5, 'f' , 16 * 8 );
    mem_dump(&MemHeap);

}

static void mem_check_3_3(void)
{
    TState state;
    mem_prepare();

    EVB_PRINTF("\r\n free addr1= 0x%02x\r\n ", addr1 );
    TclFreeHeapMemory(&MemHeap, addr1);
    EVB_PRINTF("\r\n free addr3= 0x%02x\r\n ", addr3 );
    TclFreeHeapMemory(&MemHeap, addr3);
    EVB_PRINTF("\r\n free addr2= 0x%02x\r\n ", addr2 );
    TclFreeHeapMemory(&MemHeap, addr2);

    state = TclMallocHeapMemory(&MemHeap, 16 * 8 + 1, &addr5);
    if (state != eSuccess)
    {
        EVB_PRINTF("5f malloc error\r\n");
        while(1);
    }
    EVB_PRINTF("\r\n malloc addr5= 0x%02x\r\n ", addr5 );
    memset(addr5, 'f' , 16 * 8 + 1);
    mem_dump(&MemHeap);

}

// 以下测试因为块头的影响
static void mem_check_3_4(void)
{
    TState state;
    mem_prepare();

    EVB_PRINTF("\r\n free addr1= 0x%02x\r\n ", addr1 );
    TclFreeHeapMemory(&MemHeap, addr1);
    EVB_PRINTF("\r\n free addr3= 0x%02x\r\n ", addr3 );
    TclFreeHeapMemory(&MemHeap, addr3);
    EVB_PRINTF("\r\n free addr2= 0x%02x\r\n ", addr2 );
    TclFreeHeapMemory(&MemHeap, addr2);

    state = TclMallocHeapMemory(&MemHeap, 16 * 7 + 1, &addr5);
    if (state != eSuccess)
    {
        EVB_PRINTF("5f malloc error\r\n");
        while(1);
    }
    EVB_PRINTF("\r\n malloc addr5= 0x%02x\r\n ", addr5 );
    memset(addr5, 'f' , 16 * 7 + 1);
    mem_dump(&MemHeap);

}

static void mem_check_3_5(void)
{
    TState state;
    mem_prepare();

    EVB_PRINTF("\r\n free addr1= 0x%02x\r\n ", addr1 );
    TclFreeHeapMemory(&MemHeap, addr1);
    EVB_PRINTF("\r\n free addr3= 0x%02x\r\n ", addr3 );
    TclFreeHeapMemory(&MemHeap, addr3);
    EVB_PRINTF("\r\n free addr2= 0x%02x\r\n ", addr2 );
    TclFreeHeapMemory(&MemHeap, addr2);

    state = TclMallocHeapMemory(&MemHeap, 16 * 7 , &addr5);
    if (state != eSuccess)
    {
        EVB_PRINTF("5f malloc error\r\n");
        while(1);
    }
    EVB_PRINTF("\r\n malloc addr5= 0x%02x\r\n ", addr5 );
    memset(addr5, 'f' , 16 * 7);
    mem_dump(&MemHeap);

}


static void mem_check_3_6(void)
{
    TState state;
    mem_prepare();

    EVB_PRINTF("\r\n free addr1= 0x%02x\r\n ", addr1 );
    TclFreeHeapMemory(&MemHeap, addr1);
    EVB_PRINTF("\r\n free addr3= 0x%02x\r\n ", addr3 );
    TclFreeHeapMemory(&MemHeap, addr3);
    EVB_PRINTF("\r\n free addr2= 0x%02x\r\n ", addr2 );
    TclFreeHeapMemory(&MemHeap, addr2);

    state = TclMallocHeapMemory(&MemHeap, 16 * 7 - 1, &addr5);
    if (state != eSuccess)
    {
        EVB_PRINTF("5f malloc error\r\n");
        while(1);
    }
    EVB_PRINTF("\r\n malloc addr5= 0x%02x\r\n ", addr5 );
    memset(addr5, 'f' , 16 * 7 - 1);
    mem_dump(&MemHeap);

}

// 以下测试因为块的影响
// new block
static void mem_check_3_7(void)
{
    TState state;
    mem_prepare();

    EVB_PRINTF("\r\n free addr1= 0x%02x\r\n ", addr1 );
    TclFreeHeapMemory(&MemHeap, addr1);
    EVB_PRINTF("\r\n free addr3= 0x%02x\r\n ", addr3 );
    TclFreeHeapMemory(&MemHeap, addr3);
    EVB_PRINTF("\r\n free addr2= 0x%02x\r\n ", addr2 );
    TclFreeHeapMemory(&MemHeap, addr2);

    state = TclMallocHeapMemory(&MemHeap, 16 * 7 - 4, &addr5);
    if (state != eSuccess)
    {
        EVB_PRINTF("5f malloc error\r\n");
        while(1);
    }
    EVB_PRINTF("\r\n malloc addr5= 0x%02x\r\n ", addr5 );
    memset(addr5, 'f' , 16 * 7 - 4);
    mem_dump(&MemHeap);

}
// new block
static void mem_check_3_8(void)
{
    TState state;
    mem_prepare();

    EVB_PRINTF("\r\n free addr1= 0x%02x\r\n ", addr1 );
    TclFreeHeapMemory(&MemHeap, addr1);
    EVB_PRINTF("\r\n free addr3= 0x%02x\r\n ", addr3 );
    TclFreeHeapMemory(&MemHeap, addr3);
    EVB_PRINTF("\r\n free addr2= 0x%02x\r\n ", addr2 );
    TclFreeHeapMemory(&MemHeap, addr2);

    state = TclMallocHeapMemory(&MemHeap, 16 * 7 - 5 , &addr5);
    if (state != eSuccess)
    {
        EVB_PRINTF("5f malloc error\r\n");
        while(1);
    }
    EVB_PRINTF("\r\n malloc addr5= 0x%02x\r\n ", addr5 );
    memset(addr5, 'f' , 16 * 7 - 5);
    mem_dump(&MemHeap);

}

// no new blcok
static void mem_check_3_9(void)
{
    TState state;
    mem_prepare();

    EVB_PRINTF("\r\n free addr1= 0x%02x\r\n ", addr1 );
    TclFreeHeapMemory(&MemHeap, addr1);
    EVB_PRINTF("\r\n free addr3= 0x%02x\r\n ", addr3 );
    TclFreeHeapMemory(&MemHeap, addr3);
    EVB_PRINTF("\r\n free addr2= 0x%02x\r\n ", addr2 );
    TclFreeHeapMemory(&MemHeap, addr2);

    state = TclMallocHeapMemory(&MemHeap, 16 * 7 - 3, &addr5);
    if (state != eSuccess)
    {
        EVB_PRINTF("5f malloc error\r\n");
        while(1);
    }
    EVB_PRINTF("\r\n malloc addr5= 0x%02x\r\n ", addr5 );
    memset(addr5, 'f' , 16 * 7 - 3);
    mem_dump(&MemHeap);

}
// 2
static void mem_check_4(void)
{
    TState state;
    mem_prepare();

    EVB_PRINTF("\r\n free addr0= 0x%02x\r\n ", addr0 );
    TclFreeHeapMemory(&MemHeap, addr0);
    EVB_PRINTF("\r\n free addr1= 0x%02x\r\n ", addr1 );
    TclFreeHeapMemory(&MemHeap, addr1);

    state = TclMallocHeapMemory(&MemHeap, 64, &addr5);
    if (state != eSuccess)
    {
        EVB_PRINTF("5f malloc error\r\n");
    }
    EVB_PRINTF("\r\n malloc addr5= 0x%02x\r\n ", addr5 );
    memset(addr5, 'f' , 64);
    mem_dump(&MemHeap);

}

static void mem_check_5(void)
{
    TState state;
    mem_prepare();

    EVB_PRINTF("\r\n free addr1= 0x%02x\r\n ", addr1 );
    TclFreeHeapMemory(&MemHeap, addr1);
    EVB_PRINTF("\r\n free addr0= 0x%02x\r\n ", addr0 );
    TclFreeHeapMemory(&MemHeap, addr0);

    state = TclMallocHeapMemory(&MemHeap, 64, &addr5);
    if (state != eSuccess)
    {
        EVB_PRINTF("5f malloc error\r\n");
    }
    EVB_PRINTF("\r\n malloc addr5= 0x%02x\r\n ", addr5 );
    memset(addr5, 'f' , 64);
    mem_dump(&MemHeap);

}
static void mem_check_6(void)
{
    TState state;
    mem_prepare();

    EVB_PRINTF("\r\n free addr0= 0x%02x\r\n ", addr0 );
    TclFreeHeapMemory(&MemHeap, addr0);
    EVB_PRINTF("\r\n free addr2= 0x%02x\r\n ", addr2 );
    TclFreeHeapMemory(&MemHeap, addr2);
    EVB_PRINTF("\r\n free addr1= 0x%02x\r\n ", addr1 );
    TclFreeHeapMemory(&MemHeap, addr1);

    state = TclMallocHeapMemory(&MemHeap, 64, &addr5);
    if (state != eSuccess)
    {
        EVB_PRINTF("5f malloc error\r\n");
    }
    EVB_PRINTF("\r\n malloc addr5= 0x%02x\r\n ", addr5 );
    memset(addr5, 'f' , 64);
    mem_dump(&MemHeap);

}

static void mem_check_6_1(void)
{
    TState state;
    mem_prepare();

    EVB_PRINTF("\r\n free addr0= 0x%02x\r\n ", addr0 );
    TclFreeHeapMemory(&MemHeap, addr0);
    EVB_PRINTF("\r\n free addr2= 0x%02x\r\n ", addr2 );
    TclFreeHeapMemory(&MemHeap, addr2);
    EVB_PRINTF("\r\n free addr1= 0x%02x\r\n ", addr1 );
    TclFreeHeapMemory(&MemHeap, addr1);

    state = TclMallocHeapMemory(&MemHeap, 16 * 8 - 1, &addr5);
    if (state != eSuccess)
    {
        EVB_PRINTF("5f malloc error\r\n");
        while(1);
    }
    EVB_PRINTF("\r\n malloc addr5= 0x%02x\r\n ", addr5 );
    memset(addr5, 'f' , 16 * 8 - 1);
    mem_dump(&MemHeap);

}

static void mem_check_6_2(void)
{
    TState state;
    mem_prepare();

    EVB_PRINTF("\r\n free addr0= 0x%02x\r\n ", addr0 );
    TclFreeHeapMemory(&MemHeap, addr0);
    EVB_PRINTF("\r\n free addr2= 0x%02x\r\n ", addr2 );
    TclFreeHeapMemory(&MemHeap, addr2);
    EVB_PRINTF("\r\n free addr1= 0x%02x\r\n ", addr1 );
    TclFreeHeapMemory(&MemHeap, addr1);

    state = TclMallocHeapMemory(&MemHeap, 16 * 8 , &addr5);
    if (state != eSuccess)
    {
        EVB_PRINTF("5f malloc error\r\n");
        while(1);
    }
    EVB_PRINTF("\r\n malloc addr5= 0x%02x\r\n ", addr5 );
    memset(addr5, 'f' , 16 * 8 );
    mem_dump(&MemHeap);

}

static void mem_check_6_3(void)
{
    TState state;
    mem_prepare();

    EVB_PRINTF("\r\n free addr0= 0x%02x\r\n ", addr0 );
    TclFreeHeapMemory(&MemHeap, addr0);
    EVB_PRINTF("\r\n free addr2= 0x%02x\r\n ", addr2 );
    TclFreeHeapMemory(&MemHeap, addr2);
    EVB_PRINTF("\r\n free addr1= 0x%02x\r\n ", addr1 );
    TclFreeHeapMemory(&MemHeap, addr1);

    state = TclMallocHeapMemory(&MemHeap, 16 * 8 + 1, &addr5);
    if (state != eSuccess)
    {
        EVB_PRINTF("5f malloc error\r\n");
        while(1);
    }
    EVB_PRINTF("\r\n malloc addr5= 0x%02x\r\n ", addr5 );
    memset(addr5, 'f' , 16 * 8 + 1);
    mem_dump(&MemHeap);

}
// 3
static void mem_check_7(void)
{
    TState state;
    mem_prepare();

    EVB_PRINTF("\r\n free addr3= 0x%02x\r\n ", addr3 );
    TclFreeHeapMemory(&MemHeap, addr3);
    EVB_PRINTF("\r\n free addr4= 0x%02x\r\n ", addr4 );
    TclFreeHeapMemory(&MemHeap, addr4);

    state = TclMallocHeapMemory(&MemHeap, 64, &addr5);
    if (state != eSuccess)
    {
        EVB_PRINTF("5f malloc error\r\n");
    }
    EVB_PRINTF("\r\n malloc addr5= 0x%02x\r\n ", addr5 );
    memset(addr5, 'f' , 64);
    mem_dump(&MemHeap);

}

static void mem_check_8(void)
{
    TState state;
    mem_prepare();

    EVB_PRINTF("\r\n free addr4= 0x%02x\r\n ", addr4 );
    TclFreeHeapMemory(&MemHeap, addr4);

    EVB_PRINTF("\r\n free addr3= 0x%02x\r\n ", addr3 );
    TclFreeHeapMemory(&MemHeap, addr3);

    state = TclMallocHeapMemory(&MemHeap, 64, &addr5);
    if (state != eSuccess)
    {
        EVB_PRINTF("5f malloc error\r\n");
    }
    EVB_PRINTF("\r\n malloc addr5= 0x%02x\r\n ", addr5 );
    memset(addr5, 'f' , 64);
    mem_dump(&MemHeap);

}

static void mem_check_9(void)
{
    TState state;
    mem_prepare();

    EVB_PRINTF("\r\n free addr4= 0x%02x\r\n ", addr4 );
    TclFreeHeapMemory(&MemHeap, addr4);

    EVB_PRINTF("\r\n free addr2= 0x%02x\r\n ", addr2 );
    TclFreeHeapMemory(&MemHeap, addr2);

    EVB_PRINTF("\r\n free addr3= 0x%02x\r\n ", addr3 );
    TclFreeHeapMemory(&MemHeap, addr3);

    state = TclMallocHeapMemory(&MemHeap, 64, &addr5);
    if (state != eSuccess)
    {
        EVB_PRINTF("5f malloc error\r\n");
    }
    EVB_PRINTF("\r\n malloc addr5= 0x%02x\r\n ", addr5 );
    memset(addr5, 'f' , 64);
    mem_dump(&MemHeap);

}

static void mem_check_9_1(void)
{
    TState state;
    mem_prepare();

    EVB_PRINTF("\r\n free addr4= 0x%02x\r\n ", addr4 );
    TclFreeHeapMemory(&MemHeap, addr4);

    EVB_PRINTF("\r\n free addr2= 0x%02x\r\n ", addr2 );
    TclFreeHeapMemory(&MemHeap, addr2);

    EVB_PRINTF("\r\n free addr3= 0x%02x\r\n ", addr3 );
    TclFreeHeapMemory(&MemHeap, addr3);

    state = TclMallocHeapMemory(&MemHeap, 16 * 8 - 1, &addr5);
    if (state != eSuccess)
    {
        EVB_PRINTF("5f malloc error\r\n");
        while(1);
    }
    EVB_PRINTF("\r\n malloc addr5= 0x%02x\r\n ", addr5 );
    memset(addr5, 'f' , 16 * 8 - 1);
    mem_dump(&MemHeap);

}

static void mem_check_9_2(void)
{
    TState state;
    mem_prepare();

    EVB_PRINTF("\r\n free addr4= 0x%02x\r\n ", addr4 );
    TclFreeHeapMemory(&MemHeap, addr4);

    EVB_PRINTF("\r\n free addr2= 0x%02x\r\n ", addr2 );
    TclFreeHeapMemory(&MemHeap, addr2);

    EVB_PRINTF("\r\n free addr3= 0x%02x\r\n ", addr3 );
    TclFreeHeapMemory(&MemHeap, addr3);

    state = TclMallocHeapMemory(&MemHeap, 16 * 8, &addr5);
    if (state != eSuccess)
    {
        EVB_PRINTF("5f malloc error\r\n");
        while(1);
    }
    EVB_PRINTF("\r\n malloc addr5= 0x%02x\r\n ", addr5 );
    memset(addr5, 'f' , 16 * 8);
    mem_dump(&MemHeap);

}

static void mem_check_9_3(void)
{
    TState state;
    mem_prepare();

    EVB_PRINTF("\r\n free addr4= 0x%02x\r\n ", addr4 );
    TclFreeHeapMemory(&MemHeap, addr4);

    EVB_PRINTF("\r\n free addr2= 0x%02x\r\n ", addr2 );
    TclFreeHeapMemory(&MemHeap, addr2);

    EVB_PRINTF("\r\n free addr3= 0x%02x\r\n ", addr3 );
    TclFreeHeapMemory(&MemHeap, addr3);

    state = TclMallocHeapMemory(&MemHeap, 16 * 8 + 1, &addr5);
    if (state != eSuccess)
    {
        EVB_PRINTF("5f malloc error\r\n");
        while(1);
    }
    EVB_PRINTF("\r\n malloc addr5= 0x%02x\r\n ", addr5 );
    memset(addr5, 'f' , 16 * 8 + 1);
    mem_dump(&MemHeap);

}

static void ThreadMemEntry(void* pArg)
{
    TState state;

    memset(MemCache, 'x' , sizeof(MemCache));
    state = TclInitMemoryHeap(&MemHeap, MemCache, sizeof(MemCache));
    if (state != eSuccess)
    {
        EVB_PRINTF("xMemHeapInit error!\r\n");
        while(1);
    }

    mem_check_3_9();
    while (eTrue)
        {}
}


/* 用户应用入口函数 */
static void EVB_AppEntry(void)
{
    /* 初始化MEM设备控制线程 */
    TclCreateThread(&ThreadMem,
                  &ThreadMemEntry, (void*)0,
                  ThreadMemStack, THREAD_MEMORY_STACK_BYTES,
                  THREAD_MEMORY_PRIORITY, THREAD_MEMORY_SLICE);

    /* 激活MEM设备控制线程 */
    TclActivateThread(&ThreadMem);
}


/* 处理器BOOT之后会调用main函数，必须提供 */
int main(void)
{
    /* 注册各个内核函数 */
    TclStartKernel(&EVB_AppEntry,
                   &CpuSetupEntry,
                   &EvbSetupEntry,
                   &EvbTraceEntry);

    /* 启动内核 */
    TclStartKernel();

    return 1;
}

#endif

