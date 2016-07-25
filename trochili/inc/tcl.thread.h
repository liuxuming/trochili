/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TCL_THREAD_H
#define _TCL_THREAD_H

#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.object.h"
#include "tcl.ipc.h"
#include "tcl.timer.h"

/* 线程运行错误码定义                 */
#define THREAD_DIAG_NORMAL            (TBitMask)(0x0)     /* 线程正常                                */
#define THREAD_DIAG_STACK_OVERFLOW    (TBitMask)(0x1<<0)  /* 线程栈溢出                              */
#define THREAD_DIAG_STACK_ALARM       (TBitMask)(0x1<<1)  /* 线程栈告警                              */
#define THREAD_DIAG_INVALID_EXIT      (TBitMask)(0x1<<2)  /* 线程非法退出                            */
#define THREAD_DIAG_INVALID_STATE     (TBitMask)(0x1<<3)  /* 线程操作失败                            */

/* 线程调用错误码定义                 */
#define THREAD_ERR_NONE               (TError)(0x0)
#define THREAD_ERR_UNREADY            (TError)(0x1<<0)    /* 线程结构未初始化                        */
#define THREAD_ERR_ACAPI              (TError)(0x1<<1)    /* 线程不接受操作                          */
#define THREAD_ERR_FAULT              (TError)(0x1<<2)    /* 一般性错误，操作条件不满足              */
#define THREAD_ERR_STATUS             (TError)(0x1<<3)    /* 线程状态错误                            */
#define THREAD_ERR_PRIORITY           (TError)(0x1<<4)    /* 线程优先级错误                          */


/* 线程属性定义                       */
#define THREAD_PROP_NONE              (TProperty)(0x0)    /*                              */
#define THREAD_PROP_READY             (TProperty)(0x1<<0) /* 线程初始化完毕标记位,
                                                             本成员在结构体中的位置跟汇编代码相关    */
#define THREAD_PROP_PRIORITY_FIXED    (TProperty)(0x1<<1) /* 线程优先级安全标记                               */
#define THREAD_PROP_PRIORITY_SAFE     (TProperty)(0x1<<2) /* 线程优先级安全标记                               */
#define THREAD_PROP_CLEAN_STACK       (TProperty)(0x1<<3) /* 主动清空线程栈空间                               */
#define THREAD_PROP_DAEMON            (TProperty)(0x1<<4) /* 守护线程标记位 */
#define THREAD_PROP_ASR               (TProperty)(0x1<<5) /* 用户中断处理线程标记位 */
#define THREAD_PROP_ROOT              (TProperty)(0x1<<6) /* ROOT线程标记位 */

/* 线程权限控制，各种线程API操作时的许可位 */
#define THREAD_ACAPI_NONE             (TBitMask)(0x0)
#define THREAD_ACAPI_DEINIT           (TBitMask)(0x1<<0)
#define THREAD_ACAPI_ACTIVATE         (TBitMask)(0x1<<1)
#define THREAD_ACAPI_DEACTIVATE       (TBitMask)(0x1<<2)
#define THREAD_ACAPI_SUSPEND          (TBitMask)(0x1<<3)
#define THREAD_ACAPI_RESUME           (TBitMask)(0x1<<4)
#define THREAD_ACAPI_DELAY            (TBitMask)(0x1<<5)
#define THREAD_ACAPI_UNDELAY          (TBitMask)(0x1<<6)
#define THREAD_ACAPI_YIELD            (TBitMask)(0x1<<7)
#define THREAD_ACAPI_SET_PRIORITY     (TBitMask)(0x1<<8)
#define THREAD_ACAPI_SET_SLICE        (TBitMask)(0x1<<9)
#define THREAD_ACAPI_UNBLOCK          (TBitMask)(0x1<<10)
#define THREAD_ACAPI_ALL \
    (THREAD_ACAPI_DEINIT|\
    THREAD_ACAPI_ACTIVATE|\
    THREAD_ACAPI_DEACTIVATE|\
    THREAD_ACAPI_SUSPEND|\
    THREAD_ACAPI_RESUME|\
    THREAD_ACAPI_DELAY|\
    THREAD_ACAPI_UNDELAY|\
    THREAD_ACAPI_SET_PRIORITY|\
    THREAD_ACAPI_SET_SLICE|\
    THREAD_ACAPI_UNBLOCK|\
    THREAD_ACAPI_YIELD)
#define THREAD_ACAPI_ASR \
    (THREAD_ACAPI_DEINIT | \
    THREAD_ACAPI_DEACTIVATE |\
    THREAD_ACAPI_SUSPEND |\
    THREAD_ACAPI_RESUME)

/* 线程状态定义  */
enum ThreadStausdef
{
    eThreadRunning   = (TBitMask)(0x1<<0),     /* 运行                                             */
    eThreadReady     = (TBitMask)(0x1<<1),     /* 就绪                                             */
    eThreadDormant   = (TBitMask)(0x1<<2),     /* 休眠                                             */
    eThreadBlocked   = (TBitMask)(0x1<<3),     /* 阻塞                                             */
    eThreadDelayed   = (TBitMask)(0x1<<4),     /* 延时挂起                                         */
    eThreadSuspended = (TBitMask)(0x1<<5),     /* 就绪挂起                                         */
};
typedef enum ThreadStausdef TThreadStatus;

/* 
 * 线程队列结构定义，该结构大小随内核支持的优先级范围而变化，
 * 可以实现固定时间的线程优先级计算算法                                                          
 */
struct ThreadQueueDef
{
    TBitMask   PriorityMask;                 /* 队列中就绪优先级掩码                             */
    TObjNode*  Handle[TCLC_PRIORITY_NUM];    /* 队列中线程分队列                                 */
	//TBase32    Number[TCLC_PRIORITY_NUM];    /* 队列中线程分队列中的线程数目                     */
};
typedef struct ThreadQueueDef TThreadQueue;

/* 线程主函数类型定义                                                                            */
typedef void (*TThreadEntry)(TArgument data);

/* 内核线程结构定义，用于保存线程的基本信息                                                      */
struct ThreadDef
{
    TProperty     Property;                  /* 线程的属性,本成员在结构体中的位置跟汇编代码相关  */
    TThreadStatus Status;                    /* 线程状态,本成员在结构体中的位置跟汇编代码相关    */
    TAddr32       StackTop;                  /* 线程栈顶指针,本成员在结构体中的位置跟汇编代码相关*/
    TAddr32       StackBase;                 /* 线程栈底指针                                     */
#if (TCLC_THREAD_STACK_CHECK_ENABLE)
    TBase32       StackAlarm;                /* 线程栈用量警告                                   */
    TBase32       StackBarrier;              /* 线程栈顶围栏                                     */
#endif
    TBitMask      ACAPI;                     /* 线程可接受的API                                  */
    TPriority     Priority;                  /* 线程当前优先级                                   */
    TPriority     BasePriority;              /* 线程基本优先级                                   */
    TTimeTick     Ticks;                     /* 时间片中还剩下的ticks数目                        */
    TTimeTick     BaseTicks;                 /* 时间片长度（ticks数目）                          */
    TTimeTick     Jiffies;                   /* 线程总的运行时钟节拍数                           */
    TThreadEntry  Entry;                     /* 线程的主函数                                     */
    TArgument     Argument;                  /* 线程主函数的用户参数,用户来赋值                  */
    TBitMask      Diagnosis;                 /* 线程运行错误码                                   */
#if (TCLC_TIMER_ENABLE)
    TTimer        Timer;                     /* 线程自带定时器                                   */
#endif

#if (TCLC_IPC_ENABLE)
    TIpcContext  IpcContext;                 /* 线程互斥、同步或者通信的上下文                   */
#endif

#if ((TCLC_IPC_ENABLE) && (TCLC_IPC_MUTEX_ENABLE))
    TObjNode*     LockList;                  /* 线程占有的锁的队列                               */
#endif

    TBase32       SyncValue;
    TThreadQueue* Queue;                     /* 指向线程所属线程队列的指针                       */
    TBase32       ThreadID;                  /* 线程ID                                           */
    TObjNode      ObjNode;                   /* 线程所在队列的节点                               */
};
typedef struct ThreadDef TThread;

#define NODE2THREAD(NODE) ((TThread*)((TByte*)(NODE)-OFF_SET_OF(TThread, ObjNode)))


extern TThreadQueue uThreadAuxiliaryQueue;   /* 内核线程辅助队列                                 */
extern TThreadQueue SetThreadReadyQueue;    /* 内核进就绪队列结                                 */

extern void uThreadLeaveQueue(TThreadQueue* pQueue, TThread* pThread);
extern void uThreadEnterQueue(TThreadQueue* pQueue, TThread* pThread, TQueuePos pos);
extern void uThreadSchedule(void);
extern void uThreadTickISR(void);
extern void uThreadModuleInit(void);
extern void uThreadResumeFromISR(TThread* pThread);
extern void uThreadSuspendSelf(void);
extern void uThreadCreate(TThread* pThread,
                        TThreadStatus status,
                        TProperty property,
                        TBitMask acapi,
                        TThreadEntry pEntry,
                        TArgument argument,
                        void* pStack,
                        TBase32 bytes,
                        TPriority priority,
                        TTimeTick ticks);
extern TState uThreadDelete(TThread* pThread, TError* pError);
extern TState uThreadSetPriority(TThread* pThread, TPriority priority,
                                 TBool flag, TBool* pHiRP, TError* pError);
extern TState xThreadCreate(TThread* pThread,
                          TThreadStatus status,
                          TProperty     property,
                          TBitMask      acapi,
                          TThreadEntry  pEntry,
                          TBase32         argument,
                          void*         pStack,
                          TBase32         bytes,
                          TPriority     priority,
                          TTimeTick     ticks,
                          TError*       pError);
extern TState xThreadDelete(TThread* pThread, TError* pError);
extern TState xThreadActivate(TThread* pThread, TError* pError);
extern TState xThreadDeactivate(TThread* pThread, TError* pError);
extern TState xThreadSuspend(TThread* pThread, TError* pError);
extern TState xThreadResume(TThread* pThread, TError* pError);
extern TState xThreadUnblock(TThread* pThread, TError* pError);
#if (TCLC_TIMER_ENABLE)
extern TState xThreadDelay(TThread* pThread, TTimeTick ticks, TError* pError);
extern TState xThreadUndelay(TThread* pThread, TError* pError);
#endif

extern TState xThreadYield(TError* pError);
extern TState xThreadSetPriority(TThread* pThread, TPriority priority, TError* pError);
extern TState xThreadSetTimeSlice(TThread* pThread, TTimeTick ticks, TError* pError);

#endif /*_TCL_THREAD_H */

