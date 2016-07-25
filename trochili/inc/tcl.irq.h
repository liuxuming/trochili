/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TCL_IRQ_H
#define _TCL_IRQ_H

#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.cpu.h"
#include "tcl.debug.h"
#include "tcl.thread.h"

#if (TCLC_IRQ_ENABLE)

/* ISR返回值 */
#define IRQ_ISR_DONE           (TBitMask)(0x0)       /* 中断处理程序结束              */
#define IRQ_CALL_ASR           (TBitMask)(0x1<<0)    /* 请求调用高级异步中断处理线程  */

#define IRQ_ERR_NONE           (TError)(0x0)
#define IRQ_ERR_FAULT          (TError)(0x1<<0)      /* 一般性错误                    */
#define IRQ_ERR_UNREADY        (TError)(0x1<<1)      /* 中断请求对象未初始化          */
#define IRQ_ERR_LOCKED         (TError)(0x1<<2)      /* 中断请求对象加锁              */

#define IRQ_PROP_NONE          (TProperty)(0x0)      /* IRQ就绪标记                   */
#define IRQ_PROP_READY         (TProperty)(0x1<<0)   /* IRQ就绪标记                   */

/* ISR函数类型定义 */
typedef TBitMask (*TISR)(TArgument data);

/* 中断向量结构定义 */
typedef struct
{
    TProperty  Property;
    TIndex     IRQn;                                 /* 物理中断号                    */
    TISR       ISR;                                  /* 同步中断处理函数              */
    TThread*   ASR;                                  /* 异步中断处理线程              */
    TArgument  Argument;                             /* 中断向量参数                  */
} TIrqVector;

/* ISR函数类型定义 */
typedef TBitMask (*TISR)(TArgument data);

#if (TCLC_IRQ_DAEMON_ENABLE)
/* IRQ回调函数类型定义 */
typedef void(*TIrqEntry)(TArgument data);            

/* IRQ请求结构定义 */
typedef struct IrqDef
{
    TProperty Property;
    TPriority Priority;                              /* IRQ优先级                     */
    TIrqEntry Entry;                                 /* IRQ回调函数                   */
    TArgument Argument;                              /* IRQ回调参数                   */
    TObjNode  ObjNode;                               /* IRQ所在队列的链表指针         */
} TIrq;
#endif

extern void uIrqModuleInit(void);
extern void xIrqEnterISR(TIndex irqn);
extern TState xIrqSetVector(TIndex irqn, TISR pISR, TThread* pASR, TArgument data, TError* pError);
extern TState xIrqCleanVector(TIndex irqn, TError* pError);

#if (TCLC_IRQ_DAEMON_ENABLE)
extern TState xIrqPostRequest(TIrq* pIRQ, TPriority priority, 
                              TIrqEntry pEntry, TArgument data, TError* pError);
extern TState xIrqCancelRequest(TIrq* pIRQ, TError* pError);
extern void uIrqCreateDaemon(void);
#endif

#endif

#endif /* _TCL_IRQ_H */

