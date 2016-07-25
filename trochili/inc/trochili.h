/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TROCHILI_H
#define _TROCHILI_H

#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.object.h"
#include "tcl.cpu.h"
#include "tcl.thread.h"
#include "tcl.kernel.h"
#include "tcl.timer.h"
#include "tcl.debug.h"
#include "tcl.ipc.h"
#include "tcl.mutex.h"
#include "tcl.semaphore.h"
#include "tcl.mailbox.h"
#include "tcl.message.h"
#include "tcl.flags.h"
#include "tcl.mem.pool.h"
#include "tcl.mem.buddy.h"


#define TCLM_ASSERT KNL_ASSERT

/* 时间单位和时钟节拍转换，用户程序使用 */
#define TCLM_TICKS(ticks)       (ticks)
#define TCLM_TICKS2SEC(ticks)   ((ticks)/ TCLC_TIME_TICK_RATE)
#define TCLM_TICKS2MLS(ticks)   (((ticks)*1000/ TCLC_TIME_TICK_RATE))
#define TCLM_SEC2TICKS(sec)     ((sec)* TCLC_TIME_TICK_RATE)
#define TCLM_MLS2TICKS(ms)      (((ms)* TCLC_TIME_TICK_RATE) / 1000)

extern void TclStartKernel(TUserEntry       pUserEntry,
                           TCpuSetupEntry   pCpuEntry,
                           TBoardSetupEntry pBoardEntry,
                           TTraceEntry      pTraceEntry);
extern void TclSetSysIdleEntry(TSysIdleEntry pEntry);
extern void TclGetCurrentThread(TThread** pThread2);
extern void TclGetTimeJiffies(TTimeTick* pJiffies);
extern void TclGetTimeStamp(TTimeStamp* pStamp);
extern TState TclUnlockScheduler(void);
extern TState TclLockScheduler(void);
extern void TclTrace(const char* pNote);

/* 线程操作结果，用户程序使用 */
#define TCLE_THREAD_NONE             (THREAD_ERR_NONE)
#define TCLE_THREAD_UNREADY          (THREAD_ERR_UNREADY)
#define TCLE_THREAD_ACAPI            (THREAD_ERR_ACAPI)
#define TCLE_THREAD_FAULT            (THREAD_ERR_FAULT)
#define TCLE_THREAD_STATUS           (THREAD_ERR_STATUS)
#define TCLE_THREAD_PRIORITY         (THREAD_ERR_PRIORITY)

extern TState TclCreateThread(TThread* pThread,
                            TThreadEntry pEntry,
                            TBase32 argument,
                            void* pStack,
                            TBase32 bytes,
                            TPriority priority,
                            TTimeTick ticks,
                            TError* pError);
extern TState TclDeleteThread(TThread* pThread, TError* pError);
extern TState TclActivateThread(TThread* pThread, TError* pError);
extern TState TclDeactivateThread(TThread* pThread, TError* pError);
extern TState TclSuspendThread(TThread* pThread, TError* pError);
extern TState TclResumeThread(TThread* pThread, TError* pError);
extern TState TclSetThreadPriority(TThread* pThread, TPriority priority, TError* pError);
extern TState TclSetThreadSlice(TThread* pThread, TTimeTick ticks, TError* pError);
extern TState TclUnblockThread(TThread* pThread, TError* pError);
extern TState TclYieldThread(TError* pError);

#if (TCLC_TIMER_ENABLE)
extern TState TclDelayThread(TThread* pThread, TTimeTick ticks, TError* pError);
extern TState TclUnDelayThread(TThread* pThread, TError* pError);
#endif

#if (TCLC_TIMER_ENABLE)

/* 用户定时器属性定义，用户程序使用 */
#define TCLP_TIMER_DUMMY           (TIMER_PROP_NONE)
#define TCLP_TIMER_PERIODIC        (TIMER_PROP_PERIODIC)
#define TCLP_TIMER_URGENT          (TIMER_PROP_URGENT)

/* 用户定时器操作结果，用户程序使用 */
#define TCLE_TIMER_NONE            (TIMER_ERR_NONE)
#define TCLE_TIMER_FAULT           (TIMER_ERR_FAULT)
#define TCLE_TIMER_UNREADY         (TIMER_ERR_UNREADY)

extern TState TclCreateTimer(TTimer* pTimer, TProperty property, TTimeTick ticks,
                               TTimerRoutine routine, TArgument data, TError* pError);
extern TState TclDeleteTimer(TTimer* pTimer, TError* pError);
extern TState TclStartTimer(TTimer* pTimer, TTimeTick lagticks, TError* pError);
extern TState TclStopTimer(TTimer* pTimer, TError* pError);
extern TState TclConfigTimer(TTimer* pTimer, TTimeTick ticks, TError* pError);

#endif


#if (TCLC_IRQ_ENABLE)

/* ISR返回值 */
#define TCLR_IRQ_DONE       (IRQ_ISR_DONE)       /* 中断处理程序结束                    */
#define TCLR_IRQ_ASR        (IRQ_CALL_ASR)       /* 请求调用高级异步中断处理线程        */
 
#define TCLE_IRQ_NONE       (IRQ_ERR_NONE)       
#define TCLE_IRQ_FAULT      (IRQ_ERR_FAULT)      /* 一般性错误                          */
#define TCLE_IRQ_UNREADY    (IRQ_ERR_UNREADY)    /* 中断请求结构未初始化                */

extern TState TclSetIrqVector(TIndex irqn, TISR pISR, TThread* pDaemon, TArgument data, TError* pError);
extern TState TclCleanIrqVector(TIndex vector, TError* pError);
extern TState TclCreateAsyISR(TThread* pThread,
                            TThreadEntry pEntry,
                            TBase32 Argument,
                            void* pStack,
                            TBase32 bytes,
                            TError* pError);
extern TState TclDeleteAsyISR(TThread* pThread, TError* pError);
#endif

#if ((TCLC_IRQ_ENABLE) && (TCLC_IRQ_DAEMON_ENABLE))
extern TState TclPostIRQ(TIrq* pIRQ, TPriority priority, TIrqEntry pEntry, TArgument data, TError* pError);
extern TState TclCancelIRQ(TIrq* pIRQ, TError* pError);
#endif


#if (TCLC_IPC_ENABLE)
/* IPC操作结果，用户程序使用 */
#define TCLE_IPC_NONE            (IPC_ERR_NONE)
#define TCLE_IPC_FAULT           (IPC_ERR_FAULT)
#define TCLE_IPC_UNREADY         (IPC_ERR_UNREADY)
#define TCLE_IPC_TIMEO           (IPC_ERR_TIMEO)
#define TCLE_IPC_DELETE          (IPC_ERR_DELETE)
#define TCLE_IPC_RESET           (IPC_ERR_RESET)
#define TCLE_IPC_FLUSH           (IPC_ERR_FLUSH)
#define TCLE_IPC_ABORT           (IPC_ERR_ABORT)
#define TCLE_IPC_FORBIDDEN       (IPC_ERR_FORBIDDEN)
#define TCLE_IPC_INVALID_VALUE   (IPC_ERR_INVALID_VALUE)
#define TCLE_IPC_INVALID_STATUS  (IPC_ERR_INVALID_STATUS)
#define TCLE_IPC_FLAGS           (IPC_ERR_FLAGS)

/* IPC对象属性，用户程序使用 */
#define TCLP_IPC_DUMMY           (IPC_PROPERTY)
#define TCLP_IPC_PREEMP_AUXIQ    (IPC_PROP_PREEMP_AUXIQ)
#define TCLP_IPC_PREEMP_PRIMIQ   (IPC_PROP_PREEMP_PRIMIQ)
#define TCLP_IPC_PREEMPTIVE      (IPC_PROP_PREEMP_PRIMIQ|IPC_PROP_PREEMP_AUXIQ)

/* IPC选项，用户程序使用 */
#define TCLO_IPC_DUMMY           (IPC_OPTION)
#define TCLO_IPC_WAIT            (IPC_OPT_WAIT)
#define TCLO_IPC_TIMED           (IPC_OPT_TIMED)
#define TCLO_IPC_UARGENT         (IPC_OPT_UARGENT)
#define TCLO_IPC_AND             (IPC_OPT_AND)
#define TCLO_IPC_OR              (IPC_OPT_OR)
#define TCLO_IPC_CONSUME         (IPC_OPT_CONSUME)

#endif

#if (TCLC_IPC_ENABLE && TCLC_IPC_SEMAPHORE_ENABLE)
extern TState TclCreateSemaphore(TSemaphore* pSemaphore, TBase32 value, TBase32 mvalue,
                               TProperty property, TError* pError);
extern TState TclDeleteSemaphore(TSemaphore* pSemaphore, TError* pError);
extern TState TclResetSemaphore(TSemaphore* pSemaphore, TError* pError);
extern TState TclFlushSemaphore(TSemaphore* pSemaphore, TError* pError);
extern TState TclObtainSemaphore(TSemaphore* pSemaphore, TOption option, TTimeTick timeo,
                                 TError* pError);
extern TState TclReleaseSemaphore(TSemaphore* pSemaphore, TOption option, TTimeTick timeo,
                                  TError* pError);
extern TState TclIsrReleaseSemaphore(TSemaphore* pSemaphore);
#endif

#if ((TCLC_IPC_ENABLE) && (TCLC_IPC_MUTEX_ENABLE))
extern TState TclCreateMutex(TMutex* pMutex, TPriority priority, TProperty property, TError* pError);
extern TState TclDeleteMutex(TMutex* pMutex, TError* pError);
extern TState TclLockMutex(TMutex* pMutex, TOption option, TTimeTick timeo, TError* pError);
extern TState TclFreeMutex(TMutex* pMutex, TError* pError);
extern TState TclResetMutex(TMutex* pMutex, TError* pError);
extern TState TclFlushMutex(TMutex* pMutex, TError* pError);
#endif

#if ((TCLC_IPC_ENABLE) && (TCLC_IPC_FLAGS_ENABLE))
extern TState TclCreateFlags(TFlags* pFlags, TProperty property, TError* pError);
extern TState TclDeleteFlags(TFlags* pFlags, TError* pError);
extern TState TclSendFlags(TFlags* pFlags, TBitMask pattern, TError* pError);
extern TState TclReceiveFlags(TFlags* pFlags, TBitMask* pPattern, TOption option,
                              TTimeTick timeo, TError* pError);
extern TState TclResetFlags(TFlags* pFlags, TError* pError);
extern TState TclFlushFlags(TFlags* pFlags,  TError* pError);
#endif

#if ((TCLC_IPC_ENABLE) && (TCLC_IPC_MAILBOX_ENABLE))
extern TState TclCreateMailBox(TMailBox* pMailbox, TProperty property, TError* pError);
extern TState TclDeleteMailBox(TMailBox* pMailbox, TError* pError);
extern TState TclReceiveMail(TMailBox* pMailbox, TMail* pMail2, TOption option,
                             TTimeTick timeo, TError* pError);
extern TState TclSendMail(TMailBox* pMailbox, TMail* pMail2, TOption option, TTimeTick timeo,
                          TError* pError);
extern TState TclIsrSendMail(TMailBox* pMailbox, TMail* pMail2);
extern TState TclBroadcastMail(TMailBox* pMailbox, TMail* pMail2, TError* pError);
extern TState TclResetMailBox(TMailBox* pMailbox, TError* pError);
extern TState TclFlushMailBox(TMailBox* pMailbox, TError* pError);
#endif

#if ((TCLC_IPC_ENABLE) && (TCLC_IPC_MQUE_ENABLE))
extern TState TclCreateMsgQueue(TMsgQueue* pMsgQue, void** pPool2, TBase32 capacity,
                              TProperty property, TError* pError);
extern TState TclDeleteMsgQueue(TMsgQueue* pMsgQue, TError* pError);
extern TState TclReceiveMessage(TMsgQueue* pMsgQue, TMessage* pMsg2, TOption option,
                                TTimeTick timeo, TError* pError);
extern TState TclSendMessage(TMsgQueue* pMsgQue, TMessage* pMsg2, TOption option,
                             TTimeTick timeo, TError* pError);
extern TState TclIsrSendMessage(TMsgQueue* pMsgQue, TMessage* pMsg2);
extern TState TclBroadcastMessage(TMsgQueue* pMsgQue, TMessage* pMsg2, TError* pError);
extern TState TclFlushMsgQueue(TMsgQueue* pMsgQue, TError* pError);
extern TState TclResetMsgQueue(TMsgQueue* pMsgQue, TError* pError);
#endif

#if (TCLC_MEMORY_ENABLE)

/* 内存操作结果，用户程序使用 */
#define TCLE_MEMORY_NONE            (MEM_ERR_NONE)
#define TCLE_MEMORY_FAULT           (MEM_ERR_FAULT)
#define TCLE_MEMORY_UNREADY         (MEM_ERR_UNREADY)
#define TCLE_MEMORY_NOMEM           (MEM_ERR_NO_MEM)
#define TCLE_MEMORY_BADADDR         (MEM_ERR_BAD_ADDR)
#define TCLE_MEMORY_DBLFREE         (MEM_ERR_DBL_FREE)
#endif

#if (TCLC_MEMORY_ENABLE && TCLC_MEMORY_POOL_ENABLE)
extern TState TclCreateMemoryPool(TMemPool* pPool, void* pAddr, TBase32 pages, TBase32 pgsize, TError* pError);
extern TState TclDeleteMemoryPool(TMemPool* pPool, TError* pError);
extern TState TclMallocPoolMemory (TMemPool* pPool, void** pAddr2, TError* pError);
extern TState TclFreePoolMemory (TMemPool* pPool, void* pAddr, TError* pError);
#endif

#if (TCLC_MEMORY_ENABLE && TCLC_MEMORY_BUDDY_ENABLE)
extern TState TclCreateMemBuddy(TMemBuddy* pBuddy, TChar* pAddr, TBase32 pages, TBase32 pagesize, TError* pError);
extern TState TclDeleteMemBuddy(TMemBuddy* pBuddy, TError* pError);
extern TState TclMallocBuddyMem(TMemBuddy* pBuddy, int len, void** pAddr2, TError* pError);
extern TState TclFreeBuddyMem(TMemBuddy* pBuddy,  void* pAddr, TError* pError);
#endif

#endif /* _TROCHILI_H */

