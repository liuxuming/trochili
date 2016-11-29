/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TCL_FLAGS_H
#define _TCL_FLAGS_H

#include "tcl.config.h"
#include "tcl.types.h"
#include "tcl.object.h"
#include "tcl.ipc.h"

#if ((TCLC_IPC_ENABLE) && (TCLC_IPC_FLAGS_ENABLE))

/* 事件标记结构定义 */
struct FlagsDef
{
    TProperty Property;  /* 线程的调度策略等属性配置   */
    TBitMask  Value;     /* 事件标记的当前事件组       */
    TIpcQueue Queue;     /* 事件标记的线程阻塞队列     */
    TObject   Object;
};
typedef struct FlagsDef TFlags;


extern TState TclCreateFlags(TFlags* pFlags, TChar* pName, TProperty property, TError* pError);
extern TState TclDeleteFlags(TFlags* pFlags, TError* pError);
extern TState TclSendFlags(TFlags* pFlags, TBitMask pattern, TError* pError);
extern TState TclReceiveFlags(TFlags* pFlags, TBitMask* pPattern, TOption option,
                              TTimeTick timeo, TError* pError);
extern TState TclResetFlags(TFlags* pFlags, TError* pError);
extern TState TclFlushFlags(TFlags* pFlags,  TError* pError);
#endif

#endif /* _TCL_FLAGS_H */

