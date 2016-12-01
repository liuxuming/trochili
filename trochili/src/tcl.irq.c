/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#include <string.h>

#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.object.h"
#include "tcl.cpu.h"
#include "tcl.kernel.h"
#include "tcl.thread.h"
#include "tcl.debug.h"
#include "tcl.irq.h"

#if (TCLC_IRQ_ENABLE)

/* 中断向量描述符属性 */
#define IRQ_VECTOR_PROP_NONE   (TProperty)(0x0)
#define IRQ_VECTOR_PROP_READY  (TProperty)(0x1<<0)
#define IRQ_VECTOR_PROP_LOCKED (TProperty)(0x1<<1)

#if (TCLC_IRQ_DAEMON_ENABLE)
/* IRQ请求队列类型定义 */
typedef struct IrqListDef
{
    TLinkNode* Handle;
} TIrqList;
#endif

/* 内核中断向量表 */
static TIrqVector IrqVectorTable[TCLC_IRQ_VECTOR_NUM];

/* MCU中断号到内核中断向量的转换表 */
static TAddr32 IrqMapTable[TCLC_CPU_IRQ_NUM];


/*************************************************************************************************
 *  功能：中断处理函数总入口                                                                     *
 *  参数：(1) irqn 中断号                                                                        *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void OsIrqEnterISR(TIndex irqn)
{
    TReg32      imask;
    TIrqVector* pVector;
    TISR        pISR;
    TArgument   data;
    TBitMask    retv;

    OS_ASSERT((irqn < TCLC_CPU_IRQ_NUM), "");
    OsCpuEnterCritical(&imask);

    /* 获得和中断号对应的中断向量 */
    pVector = (TIrqVector*)(IrqMapTable[irqn]);
    if ((pVector != (TIrqVector*)0) &&
            (pVector->Property & IRQ_VECTOR_PROP_READY))
    {
        /* 在处理中断对应的向量时，禁止其他代码修改向量 */
        pVector->Property |= IRQ_VECTOR_PROP_LOCKED;

        /* 在中断环境下调用低级中断处理函数 */
        if (pVector->ISR != (TISR)0)
        {
            pISR = pVector->ISR;
            data = pVector->Argument;
            OsCpuLeaveCritical(imask);
            retv = pISR(data);
            OsCpuEnterCritical(&imask);

            /*
             * 如果需要则调用中断处理线程DAEMON(内核中断守护线程),
             * 注意此时DAEMON可能处于OsThreadReady状态
             */
#if (TCLC_IRQ_DAEMON_ENABLE)
            if (retv & OS_IRQ_DAEMON)
            {
                OsThreadResumeFromISR(OsKernelVariable.IrqDaemon);

            }
#endif
        }
        pVector->Property &= (~IRQ_VECTOR_PROP_LOCKED);
    }

    OsCpuLeaveCritical(imask);
}


/*************************************************************************************************
 *  功能：设置中断向量函数                                                                       *
 *  参数：(1) irqn     中断号                                                                    *
 *        (2) pISR     ISR处理函数                                                               *
 *        (3) data     应用提供的回调数据                                                        *
 *        (4) pError   详细调用结果                                                              *
 *  返回: (1) eFailure 操作失败                                                                  *
 *        (2) eSuccess 操作成功                                                                  *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclSetIrqVector(TIndex irqn, TISR pISR, TArgument data, TError* pError)
{
    TState state = eFailure;
    TError error = OS_IRQ_ERR_FAULT;
    TReg32 imask;
    TIndex index;
    TIrqVector* pVector;

    OS_ASSERT((irqn < TCLC_CPU_IRQ_NUM), "");
    OS_ASSERT((pISR != (TISR)0), "");

    OsCpuEnterCritical(&imask);

    /* 如果指定的中断号已经注册过中断向量，那么直接更新 */
    if (IrqMapTable[irqn] != (TAddr32)0)
    {
        pVector = (TIrqVector*)(IrqMapTable[irqn]);

        /* 更新之前确保没有被锁定 */
        if ((pVector->Property & IRQ_VECTOR_PROP_LOCKED))
        {
            error = OS_IRQ_ERR_LOCKED;
        }
        else
        {
            /* 更新中断向量对应的中断服务程序 */
            pVector->ISR      = pISR;
            pVector->Argument = data;

            error = OS_IRQ_ERR_NONE;
            state = eSuccess;
        }
    }
    else
    {
        /* 为该中断号申请中断向量项 */
        for (index = 0; index < TCLC_IRQ_VECTOR_NUM; index++)
        {
            pVector = (TIrqVector*)IrqVectorTable + index;
            if (!(pVector->Property & IRQ_VECTOR_PROP_READY))
            {
                /*
                 * 建立中断号和对应的中断向量的联系,
                 * 设置中断向量对应的中断服务程序
                 */
                IrqMapTable[irqn] = (TAddr32)pVector;
                pVector->IRQn     = irqn;
                pVector->ISR      = pISR;
                pVector->Argument = data;
                pVector->Property = IRQ_VECTOR_PROP_READY;

                error = OS_IRQ_ERR_NONE;
                state = eSuccess;
                break;
            }
        }
    }

    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能：清空中断向量函数                                                                       *
 *  参数：(1) irqn   中断编号                                                                    *
 *        (2) pError 详细调用结果                                                                *
 *  返回: (1) eSuccess 操作成功                                                                  *
 *        (2) eFailure 操作失败                                                                  *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclCleanIrqVector(TIndex irqn, TError* pError)
{
    TState state = eFailure;
    TError error = OS_IRQ_ERR_FAULT;
    TReg32 imask;
    TIrqVector* pVector;

    OS_ASSERT((irqn < TCLC_CPU_IRQ_NUM), "");

    OsCpuEnterCritical(&imask);

    /* 找到该中断向量并且清空相关信息 */
    if (IrqMapTable[irqn] != (TAddr32)0)
    {
        pVector = (TIrqVector*)(IrqMapTable[irqn]);
        if ((pVector->Property & IRQ_VECTOR_PROP_READY) &&
                (pVector->IRQn == irqn))
        {
            if (!(pVector->Property & IRQ_VECTOR_PROP_LOCKED))
            {
                IrqMapTable[irqn] = (TAddr32)0;
                memset(pVector, 0U, sizeof(TIrqVector));
                error = OS_IRQ_ERR_NONE;
                state = eSuccess;
            }
            else
            {
                error = OS_IRQ_ERR_LOCKED;
            }
        }
    }

    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}



#if (TCLC_IRQ_DAEMON_ENABLE)

/* IRQ守护线程定义和栈定义 */
static TThread IrqDaemonThread;
static TBase32 IrqDaemonStack[TCLC_IRQ_DAEMON_STACK_BYTES >> 2];

/* IRQ守护线程不接受任何线程管理API操作 */
#define IRQ_DAEMON_ACAPI (OS_THREAD_ACAPI_NONE)

/* IRQ请求队列 */
static TIrqList IrqReqList;


/*************************************************************************************************
 *  功能：提交中断请求                                                                           *
 *  参数：(1) pIRQ      中断请求结构地址                                                         *
 *        (2) pEntry    中断处理回调函数                                                         *
 *        (3) data      中断处理回调参数                                                         *
 *        (4) priority  中断请求优先级                                                           *
 *        (5) pError    详细调用结果                                                             *
 *  返回: (1) eFailure  操作失败                                                                 *
 *        (2) eSuccess  操作成功                                                                 *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclPostIRQ(TIrq* pIRQ, TIrqEntry pEntry, TArgument data, TPriority priority,
                         TError* pError)
{
    TState state = eFailure;
    TError error = OS_IRQ_ERR_FAULT;
    TReg32 imask;

    OS_ASSERT((pIRQ != (TIrq*)0), "");

    OsCpuEnterCritical(&imask);

    if (!(pIRQ->Property & OS_IRQ_PROP_READY))
    {
        pIRQ->Property        = OS_IRQ_PROP_READY;
        pIRQ->Entry           = pEntry;
        pIRQ->Argument        = data;
        pIRQ->Priority        = priority;
        pIRQ->LinkNode.Next   = (TLinkNode*)0;
        pIRQ->LinkNode.Prev   = (TLinkNode*)0;
        pIRQ->LinkNode.Handle = (TLinkNode**)0;
        pIRQ->LinkNode.Data   = (TBase32*)(&(pIRQ->Priority));
        pIRQ->LinkNode.Owner  = (void*)pIRQ;
        OsObjListAddPriorityNode(&(IrqReqList.Handle), &(pIRQ->LinkNode));

        error = OS_IRQ_ERR_NONE;
        state = eSuccess;
    }

    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能：撤销中断请求                                                                           *
 *  参数：(1) pIRQ      中断请求结构地址                                                         *
 *        (2) pError    详细调用结果                                                             *
 *  返回: (1) eFailure  操作失败                                                                 *
 *        (2) eSuccess  操作成功                                                                 *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclCancelIRQ(TIrq* pIRQ, TError* pError)
{
    TState state = eFailure;
    TError error = OS_IRQ_ERR_UNREADY;
    TReg32 imask;

    OS_ASSERT((pIRQ != (TIrq*)0), "");

    OsCpuEnterCritical(&imask);
    if (pIRQ->Property & OS_IRQ_PROP_READY)
    {
        OsObjListRemoveNode( pIRQ->LinkNode.Handle, &(pIRQ->LinkNode));
        memset(pIRQ, 0U, sizeof(TIrq));

        error = OS_IRQ_ERR_NONE;
        state = eSuccess;
    }
    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能：内核中的IRQ守护线程函数                                                                *
 *  参数：(1) argument IRQ守护线程的参数                                                         *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
static void IrqDaemonEntry(TArgument argument)
{
    TReg32    imask;
    TIrq*     pIRQ;
    TIrqEntry pEntry;
    TArgument data;

    /*
     * 从队列中逐个获得IRQ请求兵在线程环境下处理该IRQ回调事务
     * 如果IRQ请求队列为空则将IRQ守护线程挂起
     */
    while(eTrue)
    {
        OsCpuEnterCritical(&imask);
        if (IrqReqList.Handle == (TLinkNode*)0)
        {
            OsThreadSuspendSelf();
            OsCpuLeaveCritical(imask);
        }
        else
        {
            pIRQ   = (TIrq*)(IrqReqList.Handle->Owner);
            pEntry = pIRQ->Entry;
            data   = pIRQ->Argument;
            OsObjListRemoveNode(pIRQ->LinkNode.Handle, &(pIRQ->LinkNode));
            memset(pIRQ, 0U, sizeof(TIrq));
            OsCpuLeaveCritical(imask);

            pEntry(data);
        }
    }
}


#endif


/*************************************************************************************************
 *  功能：定时器模块初始化                                                                       *
 *  参数：无                                                                                     *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void OsIrqModuleInit(void)
{
    /* 检查内核是否处于初始状态 */
    if(OsKernelVariable.State != OsOriginState)
    {
        OsDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }

    /* 初始化相关的内核变量 */
    OsKernelVariable.IrqMapTable = IrqMapTable;
    OsKernelVariable.IrqVectorTable = IrqVectorTable;

    memset(IrqMapTable, 0U, sizeof(IrqMapTable));
    memset(IrqVectorTable, 0U, sizeof(IrqVectorTable));

#if (TCLC_IRQ_DAEMON_ENABLE)
    memset(&IrqReqList, 0U, sizeof(TIrqList));

    /* 初始化内核中断服务线程 */
    OsThreadCreate(&IrqDaemonThread,
                   "kernel irq daemon",
                   OsThreadSuspended,
                   OS_THREAD_PROP_PRIORITY_FIXED | \
                   OS_THREAD_PROP_CLEAN_STACK | \
                   OS_THREAD_PROP_KERNEL_DAEMON,
                   IRQ_DAEMON_ACAPI,
                   IrqDaemonEntry,
                   (TArgument)0,
                   (void*)IrqDaemonStack,
                   (TBase32)TCLC_IRQ_DAEMON_STACK_BYTES,
                   (TPriority)TCLC_IRQ_DAEMON_PRIORITY,
                   (TTimeTick)TCLC_IRQ_DAEMON_SLICE);

    /* 初始化相关的内核变量 */
    OsKernelVariable.IrqDaemon = &IrqDaemonThread;
#endif
}

#undef IRQ_VECTOR_PROP_NONE
#undef IRQ_VECTOR_PROP_READY
#undef IRQ_VECTOR_PROP_LOCKED
#endif

