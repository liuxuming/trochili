/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TCL_TIMER_H_
#define _TCL_TIMER_H_

#include "tcl.config.h"
#include "tcl.types.h"
#include "tcl.object.h"

#if (TCLC_TIMER_ENABLE)

#define OS_TIMER_ERR_NONE            (0x0)
#define OS_TIMER_ERR_FAULT           (0x1<<0)            /* 一般性错误                          */
#define OS_TIMER_ERR_UNREADY         (0x1<<1)            /* 定时器管理结构未初始化              */
#define OS_TIMER_ERR_STATUS          (0x1<<2)            /* 定时器状态错误                      */

/* 定时器状态枚举定义 */
enum TimerStatusDef
{
    OsTimerDormant = 0,                                  /* 定时器的初始状态                    */
    OsTimerActive,                                       /* 定时器激活状态                      */
};
typedef enum TimerStatusDef TTimerStatus;

/* 定时器属性标记定义 */
#define OS_TIMER_PROP_DEAULT         (0x0)               /* 定时器空属性标记                    */
#define OS_TIMER_PROP_READY          (0x1<<0)            /* 定时器就绪标记                      */
#define OS_TIMER_PROP_EXPIRED        (0x1<<1)            /* 定时器期满标记                      */
#define OS_TIMER_PROP_PERIODIC       (0x1<<2)            /* 用户周期回调定时器                  */
#define OS_TIMER_PROP_ACCURATE       (0x1<<3)            /* 用户精准定时器                      */

#define OS_TIMER_USER_PROPERTY       (OS_TIMER_PROP_PERIODIC| OS_TIMER_PROP_ACCURATE)

/* 定时器运行错误码定义 */
#define OS_TIMER_DIAG_NORMAL         (TBitMask)(0x0)    /* 定时器正常                           */
#define OS_TIMER_DIAG_OVERFLOW       (TBitMask)(0x1<<0) /* 定时器计数溢出                       */

/* 用户定时器回调函数类型定义 */
typedef void(*TTimerRoutine)(TArgument data, TBase32 cycles, TTimeTick ticks);

/* 定时器结构定义 */
struct TimerDef
{
    TProperty     Property;                              /* 定时器属性                          */
    TTimerStatus  Status;                                /* 定时器状态                          */
    TTimeTick     MatchTicks;                            /* 定时器到时时刻                      */
    TTimeTick     PeriodTicks;                           /* 定时器定时长度                      */
    TTimerRoutine Routine;                               /* 用户定时器回调函数                  */
    TArgument     Argument;                              /* 定时器延时回调参数                  */
    TPriority     Priority;                              /* 定时器回调优先级                    */
    TBase32       ExpiredTicksCycles;                    /* 定时器期满时系统jiffy轮回次数       */
    TTimeTick     ExpiredTicks;                          /* 定时器期满时刻                      */
    TBase32       ExpiredTimes;                          /* 定时器期满次数                      */
    TLinkNode     ExpiredNode;                           /* 定时器期满队列的链表指针            */
    TBitMask      Diagnosis;                             /* 定时器运行错误码                    */
    TLinkNode     LinkNode;                              /* 定时器所在队列的链表指针            */
    TObject       Object;                                /* 定时器对象的链表指针                */
};
typedef struct TimerDef TTimer;


/* 定时器队列结构定义 */
struct TimerListDef
{
    TLinkNode* Handle[TCLC_TIMER_WHEEL_SIZE];
};
typedef struct TimerListDef TTimerList;

#define TCLM_NODE2TIMER(NODE) ((TTimer*)((TByte*)(NODE)-OFF_SET_OF(TTimer, LinkNode)))

extern void OsTimerModuleInit(void);
extern void OsTimerTickUpdate(void);

extern TState TclCreateTimer(TTimer* pTimer, TChar* pName, TProperty property, TTimeTick ticks,
                             TTimerRoutine routine, TArgument data, TPriority priority,
                             TError* pError);
extern TState TclDeleteTimer(TTimer* pTimer, TError* pError);
extern TState TclStartTimer(TTimer* pTimer, TTimeTick lagticks, TError* pError);
extern TState TclStopTimer(TTimer* pTimer, TError* pError);
extern TState TclConfigTimer(TTimer* pTimer, TTimeTick ticks, TPriority priority,
                             TError* pError);
#endif


#endif /*_TCL_TIMER_H_*/

