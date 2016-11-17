/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TCL_SEMAPHORE_H
#define _TCL_SEMAPHORE_H

#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.object.h"
#include "tcl.ipc.h"
#include "tcl.thread.h"

#if ((TCLC_IPC_ENABLE)&&(TCLC_IPC_SEMAPHORE_ENABLE))

/* 计数信号量、二值结构定义 */
struct SemaphoreDef
{
    TProperty Property;         /* 队列中线程的调度策略等属性配置     */
    TBase32   Value;            /* 计数信号量的当前数值               */
    TBase32   LimitedValue;     /* 计数信号量的最大数值               */
    TBase32   InitialValue;     /* 计数信号量的初始数值               */
    TIpcQueue Queue;            /* 信号量的线程阻塞队列               */
    TObject   Object;
};
typedef struct SemaphoreDef TSemaphore;

extern TState xSemaphoreRelease(TSemaphore* pSemaphore, TOption option, TTimeTick timeo, TError* pError);
extern TState xSemaphoreObtain(TSemaphore* pSemaphore, TOption option, TTimeTick timeo, TError* pError);
extern TState xSemaphoreCreate(TSemaphore* pSemaphore, TChar* pName, TBase32 value, TBase32 mvalue,
	                                   TProperty property, TError* pError);
extern TState xSemaphoreDelete(TSemaphore* pSemaphore, TError* pError);
extern TState xSemaphoreReset(TSemaphore* pSemaphore, TError* pError);
extern TState xSemaphoreFlush(TSemaphore* pSemaphore, TError* pError);
#endif

#endif /*_TCL_SEMAPHORE_H*/

