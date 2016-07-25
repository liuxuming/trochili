/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TCL_MUTEX_H
#define _TCL_MUTEX_H

#include "tcl.types.h"
#include "tcl.object.h"
#include "tcl.thread.h"

#if ((TCLC_IPC_ENABLE)&&(TCLC_IPC_MUTEX_ENABLE))

/* 互斥信号量结构定义 */
struct MutexDef
{
    TProperty Property;      /* 队列中线程的调度策略等属性配置 */
    TThread*  Owner;         /* 占有互斥信号量的线程指针       */
    TBase32   Nest;          /* 互斥信号量嵌套加锁深度         */
    TPriority Priority;      /* ceiling value                  */
    TIpcQueue Queue;         /* 互斥信号量的线程阻塞队列       */
    TObjNode  LockNode;      /* 用来组成互斥量链表             */   
};
typedef struct MutexDef TMutex;

extern TState xMutexCreate(TMutex* pMutex, TPriority priority, TProperty property, TError* pError);
extern TState xMutexDelete(TMutex* pMutex, TError* pError);
extern TState xMutexLock(TMutex* pMutex, TOption option, TTimeTick timeo, TError* pError);
extern TState xMutexFree(TMutex* pMutex, TError* pError);
extern TState xMutexReset(TMutex* pMutex, TError* pError);
extern TState xMutexFlush(TMutex* pMutex, TError* pError);

#endif

#endif /*_TCL_MUTEX_H*/

