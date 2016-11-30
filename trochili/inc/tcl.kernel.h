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
#include "tcl.timer.h"
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

/* 系统Fault处理函数类型定义                    */
typedef void (*TSysFaultEntry)(void* pKernelVariable);

/* 系统Warning处理函数类型定义                  */
typedef void (*TSysWarningEntry)(void* pKernelVariable);

/* 代码运行环境类型定义，系统代码有三种运行环境 */
typedef enum
{
    OsOriginState  = 0,                               /* 代码运行在内核初态                    */
    OsExtremeState = 1,                               /* 代码运行在中断态                      */
    OsThreadState  = 2,                               /* 代码运行在线程态                      */
} TKernelState;

#define OS_KERNEL_DIAG_ERROR_NONE      (0U)           /* 线程栈溢出                            */
#define OS_KERNEL_DIAG_THREAD_ERROR    (0x1<<0U)      /* 线程错误                              */
#define OS_KERNEL_DIAG_SCHED_ERROR     (0x1<<1U)      /* 内核禁止线程调度                      */
#define OS_KERNEL_DIAG_TIMER_ERROR     (0x1<<2U)      /* 定时器错误                            */
#define OS_KERNEL_DIAG_IPC_ERROR       (0x1<<3U)      /* 在中断里操作了互斥量                  */

/* 内核变量结构定义，记录了内核运行时的各种数据 */
struct KernelVariableDef
{
    TBase32          SchedLockTimes;                  /* 决定是否允许线程调度                  */
    TThread*         NomineeThread;                   /* 内核候选线程指针                      */
    TThread*         CurrentThread;                   /* 内核当前线程指针                      */
    TKernelState     State;                           /* 记录代码执行时所处的运行状态          */
    TBase32          IntrNestTimes;                   /* 记录内核被中断的嵌套次数              */
    TTimeTick        Jiffies;                         /* 系统运行总的节拍数                    */
	TBase32          JiffyCycles;                     /* 系统jiffy轮回次数                     */
    TBitMask         Diagnosis;                       /* 内核运行状况记录                      */
    TDBGLog          DBGLog;                          /* 内核运行状况记录                      */

    TBoardSetupEntry BoardSetupEntry;                 /* 板级初始化程序入口                    */
    TCpuSetupEntry   CpuSetupEntry;                   /* 处理器初始化程序入口                  */
    TUserEntry       UserEntry;                       /* 用户程序入口                          */
    TTraceEntry      TraceEntry;                      /* 内核打印函数                          */
    TSysIdleEntry    SysIdleEntry;                    /* 内核IDLE函数                          */
    TSysFaultEntry   SysFaultEntry;                   /* 内核FAULT函数                         */
    TSysWarningEntry SysWarningEntry;                 /* 内核WARNING函数                       */
    TThread*         RootThread;                      /* 内核ROOT线程指针                      */

#if (TCLC_TIMER_ENABLE)
    TThread*         TimerDaemon;                     /* 用户定时器线程指针                    */
    TTimerList*      TimerList;                       /* 用户定时器链表指针                    */
#endif

#if (TCLC_IRQ_ENABLE)
    TAddr32*         IrqMapTable;                     /* 内核中断映射表                        */
    TIrqVector*      IrqVectorTable;                  /* 内核中断向量表                        */
#endif

#if ((TCLC_IRQ_ENABLE)&&(TCLC_IRQ_DAEMON_ENABLE))
    TThread*         IrqDaemon;                       /* IRQ线程指针                           */
#endif

    TThreadQueue*    ThreadAuxiliaryQueue;            /* 内核线程辅助队列指针                  */
    TThreadQueue*    ThreadReadyQueue;                /* 内核进就绪队列结指针                  */
    TLinkNode*       ThreadTimerList;                 /* 线程定时器链表指针                    */

    TLinkNode*       ObjectList;                      /* 内核对象的队列节点                    */
    TBase32          ObjectID;                        /* 内核对象编号生成计数                  */
};
typedef struct KernelVariableDef TKernelVariable;


extern TKernelVariable OsKernelVariable;


extern void OsKernelAddObject(TObject* pObject, TChar* pName, TObjectType type, void* pOwner);
extern void OsKernelRemoveObject(TObject* pObject);
extern void OsKernelEnterIntrState(void);
extern void OsKernelLeaveIntrState(void);
extern void OsKernelTickISR(void);


extern void TclStartKernel(TUserEntry pUserEntry,
                           TCpuSetupEntry   pCpuEntry,
                           TBoardSetupEntry pBoardEntry,
                           TTraceEntry      pTraceEntry);
extern void TclSetSysIdleEntry(TSysIdleEntry pEntry);
extern void TclSetSysFaultEntry(TSysFaultEntry pEntry);
extern void TclSetSysWarningEntry(TSysWarningEntry pEntry);
extern void TclGetCurrentThread(TThread** pThread2);
extern void TclGetTimeStamp(TBase32* pCycles, TTimeTick* pJiffies);
extern TState TclUnlockScheduler(void);
extern TState TclLockScheduler(void);
extern void TclTrace(const char* pNote);


#endif /* _TCL_KERNEL_H */

