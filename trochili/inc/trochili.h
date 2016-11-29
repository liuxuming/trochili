/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TROCHILI_H
#define _TROCHILI_H

#include "tcl.types.h"
#include "tcl.config.h"

#if (TCLC_ASSERT_ENABLE)
#include "tcl.debug.h"
#define TCLM_ASSERT OS_ASSERT
#endif


#include "tcl.kernel.h"
/* 时间单位和时钟节拍转换，用户程序使用 */
#define TCLM_TICKS(ticks)        (ticks)
#define TCLM_TICKS2SEC(ticks)    ((ticks) / TCLC_TIME_TICK_RATE)
#define TCLM_TICKS2MLS(ticks)    ((((ticks) * 1000) / TCLC_TIME_TICK_RATE))
#define TCLM_SEC2TICKS(sec)      ((sec)* TCLC_TIME_TICK_RATE)
#define TCLM_MLS2TICKS(ms)       (((ms)* TCLC_TIME_TICK_RATE) / 1000)


#include "tcl.thread.h"
#define TCLE_THREAD_NONE         (OS_THREAD_ERR_NONE)
#define TCLE_THREAD_UNREADY      (OS_THREAD_ERR_UNREADY)
#define TCLE_THREAD_ACAPI        (OS_THREAD_ERR_ACAPI)
#define TCLE_THREAD_FAULT        (OS_THREAD_ERR_FAULT)
#define TCLE_THREAD_STATUS       (OS_THREAD_ERR_STATUS)
#define TCLE_THREAD_PRIORITY     (OS_THREAD_ERR_PRIORITY)


#if (TCLC_TIMER_ENABLE)
#include "tcl.timer.h"

#define TCLP_TIMER_DEFAULT       (OS_TIMER_PROP_DEAULT)
#define TCLP_TIMER_PERIODIC      (OS_TIMER_PROP_PERIODIC)
#define TCLP_TIMER_ACCURATE      (OS_TIMER_PROP_ACCURATE)

#define TCLE_TIMER_NONE          (OS_TIMER_ERR_NONE)
#define TCLE_TIMER_FAULT         (OS_TIMER_ERR_FAULT)
#define TCLE_TIMER_UNREADY       (OS_TIMER_ERR_UNREADY)
#define TCLE_TIMER_STATUS        (OS_TIMER_ERR_STATUS)
#endif


#if (TCLC_IRQ_ENABLE)
#include "tcl.irq.h"

#define TCLR_IRQ_DONE            (OS_IRQ_DONE)
#define TCLR_IRQ_DAEMON          (OS_IRQ_DAEMON)

#define TCLE_IRQ_NONE            (OS_IRQ_ERR_NONE)
#define TCLE_IRQ_FAULT           (OS_IRQ_ERR_FAULT)
#define TCLE_IRQ_UNREADY         (OS_IRQ_ERR_UNREADY)
#endif


#if (TCLC_IPC_ENABLE)
#include "tcl.ipc.h"

#define TCLE_IPC_NONE            (OS_IPC_ERR_NONE)
#define TCLE_IPC_FAULT           (OS_IPC_ERR_FAULT)
#define TCLE_IPC_UNREADY         (OS_IPC_ERR_UNREADY)
#define TCLE_IPC_NORAML          (OS_IPC_ERR_NORMAL)
#define TCLE_IPC_TIMEO           (OS_IPC_ERR_TIMEO)
#define TCLE_IPC_DELETE          (OS_IPC_ERR_DELETE)
#define TCLE_IPC_RESET           (OS_IPC_ERR_RESET)
#define TCLE_IPC_FLUSH           (OS_IPC_ERR_FLUSH)
#define TCLE_IPC_ABORT           (OS_IPC_ERR_ABORT)
#define TCLE_IPC_ACAPI           (OS_IPC_ERR_ACAPI)

#define TCLP_IPC_DEFAULT         (OS_IPC_PROP_DEFAULT)
#define TCLP_IPC_PREEMP_AUXIQ    (OS_IPC_PROP_PREEMP_AUXIQ)
#define TCLP_IPC_PREEMP_PRIMIQ   (OS_IPC_PROP_PREEMP_PRIMIQ)
#define TCLP_IPC_PREEMPTIVE      (OS_IPC_PROP_PREEMP_PRIMIQ | OS_IPC_PROP_PREEMP_AUXIQ)

#define TCLO_IPC_DEFAULT         (OS_IPC_OPT_DEFAULT)
#define TCLO_IPC_WAIT            (OS_IPC_OPT_WAIT)
#define TCLO_IPC_TIMEO           (OS_IPC_OPT_TIMEO)
#define TCLO_IPC_UARGENT         (OS_IPC_OPT_UARGENT)
#define TCLO_IPC_AND             (OS_IPC_OPT_AND)
#define TCLO_IPC_OR              (OS_IPC_OPT_OR)
#define TCLO_IPC_CONSUME         (OS_IPC_OPT_CONSUME)
#endif

#if (TCLC_IPC_ENABLE && TCLC_IPC_SEMAPHORE_ENABLE)
#include "tcl.semaphore.h"
#endif

#if ((TCLC_IPC_ENABLE) && (TCLC_IPC_MUTEX_ENABLE))
#include "tcl.mutex.h"
#endif

#if ((TCLC_IPC_ENABLE) && (TCLC_IPC_FLAGS_ENABLE))
#include "tcl.flags.h"
#endif

#if ((TCLC_IPC_ENABLE) && (TCLC_IPC_MAILBOX_ENABLE))
#include "tcl.mailbox.h"
#endif

#if ((TCLC_IPC_ENABLE) && (TCLC_IPC_MQUE_ENABLE))
#include "tcl.message.h"
#endif

#if (TCLC_MEMORY_ENABLE)
#include "tcl.memory.h"
#define TCLE_MEMORY_NONE         (OS_MEM_ERR_NONE)
#define TCLE_MEMORY_FAULT        (OS_MEM_ERR_FAULT)
#define TCLE_MEMORY_UNREADY      (OS_MEM_ERR_UNREADY)
#define TCLE_MEMORY_NOMEM        (OS_MEM_ERR_NO_MEM)
#define TCLE_MEMORY_BADADDR      (OS_MEM_ERR_BAD_ADDR)
#define TCLE_MEMORY_DBLFREE      (OS_MEM_ERR_DBL_FREE)
#endif

#if (TCLC_MEMORY_ENABLE && TCLC_MEMORY_POOL_ENABLE)
#include "tcl.mem.pool.h"
#endif

#if (TCLC_MEMORY_ENABLE && TCLC_MEMORY_BUDDY_ENABLE)
#include "tcl.mem.buddy.h"
#endif

#endif /* _TROCHILI_H */

