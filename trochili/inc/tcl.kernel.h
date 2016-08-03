/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TCL_KERNEL_H
#define _TCL_KERNEL_H

#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.cpu.h"
#include "tcl.thread.h"
#include "tcl.irq.h"
#include "tcl.debug.h"

/* 用户入口函数类型定义                         */
typedef void (*TUserEntry)(void);

/* 板级初始化函数类型定义                       */
typedef void (*TBoardSetupEntry)(void);

/* 处理器初始化函数类型定义                     */
typedef void (*TCpuSetupEntry)(void);

/* 板级字符串打印函数类型定义                   */
typedef void (*TTraceEntry)(const char* pStr);

/* 系统IDLE函数类型定义                         */
typedef void (*TSysIdleEntry)(void);

/* 系统Fault处理函数类型定义                         */
typedef void (*TSysFaultEntry)(void);


/* 代码运行环境类型定义，系统代码有三种运行环境 */
typedef enum
{
    eOriginState = 0,                                 /* 代码运行在内核初态                    */
    eIntrState   = 1,                                 /* 代码运行在中断态                      */
    eThreadState = 2,                                 /* 代码运行在线程态                      */
} TKernelState;

#define KERNEL_DIAG_ERROR_NONE      (0U)       /* 线程栈溢出                                   */
#define KERNEL_DIAG_THREAD_ERROR    (0x1<<0U)  /* 线程栈溢出                                   */
#define KERNEL_DIAG_SCHED_ERROR     (0x1<<2U)  /* 线程栈溢出                                   */
#define KERNEL_DIAG_TIMER_ERROR     (0x1<<3U)  /* 线程栈溢出                                   */

/* 内核变量结构定义，记录了内核运行时的各种数据 */
struct KernelVariableDef
{
    TBase32          SchedLockTimes;                  /* 决定是否允许线程调度                    */
    TThread*         NomineeThread;                   /* 内核候选线程指针                        */
    TThread*         CurrentThread;                   /* 内核当前线程指针                        */
    TKernelState     State;                           /* 记录代码执行时所处的运行状态            */
    TBase32          IntrNestTimes;                   /* 记录内核被中断的嵌套次数                */
    TTimeTick        Jiffies;                         /* 系统运行总的节拍数                      */
    TBase32          ObjID;                           /* 内核对象编号生成计数                    */
    TBitMask         Diagnosis;                       /* 内核运行状况记录                        */
    TDBGLog          DBGLog;
    TBoardSetupEntry BoardSetupEntry;                 /* 板级初始化程序入口                      */
    TCpuSetupEntry   CpuSetupEntry;                   /* 处理器初始化程序入口                    */
    TUserEntry       UserEntry;                       /* 记录用户程序入口                        */
    TTraceEntry      TraceEntry;                      /* 内核打印函数                            */
    TSysIdleEntry    SysIdleEntry;                    /* 内核IDLE函数                            */
	TSysFaultEntry   SysFaultEntry;                   /* 内核FAULT函数                           */
    TThread*         RootThread;                      /* 内核Root线程指针                        */

#if ((TCLC_TIMER_ENABLE)&&(TCLC_TIMER_DAEMON_ENABLE))
    TThread*         TimerDaemon;                     /* 用户定时器线程指针                      */
#endif

#if ((TCLC_IRQ_ENABLE)&&(TCLC_IRQ_DAEMON_ENABLE))
    TThread*         IrqDaemon;                       /* IRQ线程指针                             */
#endif

#if (TCLC_TIMER_ENABLE)
    TTimerList*      TimerList;
#endif

#if (TCLC_IRQ_ENABLE)
    TAddr32*         IrqMapTable;
    TIrqVector*      IrqVectorTable;
#endif

    TThreadQueue*    ThreadAuxiliaryQueue;            /* 内核线程辅助队列指针                    */
    TThreadQueue*    ThreadReadyQueue;                /* 内核进就绪队列结指针                    */
};
typedef struct KernelVariableDef TKernelVariable;


extern TKernelVariable uKernelVariable;

extern void uKernelTrace(const char* pNote);
extern void xKernelEnterIntrState(void);
extern void xKernelLeaveIntrState(void);
extern void xKernelTrace(const char* pNote);
extern void xKernelTickISR(void);
extern TState xKernelUnlockSched(void);
extern TState xKernelLockSched(void);
extern void xKernelSetIdleEntry(TSysIdleEntry pEntry);
extern void xKernelGetCurrentThread(TThread** pThread2);
extern void xKernelGetJiffies(TTimeTick* pJiffies);
extern void xKernelStart(TUserEntry       pUserEntry,
                         TCpuSetupEntry   pCpuEntry,
                         TBoardSetupEntry pBoardEntry,
                         TTraceEntry      pTraceEntry);


#endif /* _TCL_KERNEL_H */

