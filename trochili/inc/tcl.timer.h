/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TCL_TIMER_H_
#define _TCL_TIMER_H_

#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.object.h"

#if (TCLC_TIMER_ENABLE)

#define TIMER_ERR_NONE               (0x0)
#define TIMER_ERR_FAULT              (0x1<<0)           /* 一般性错误                          */
#define TIMER_ERR_UNREADY            (0x1<<1)           /* 定时器管理结构未初始化              */

/* 定时器状态枚举定义 */
enum TimerStatusDef
{
    eTimerDormant = 0,                                   /* 定时器的初始状态                   */
    eTimerActive,                                        /* 定时器激活状态                     */
};
typedef enum TimerStatusDef TTimerStatus;

/* 定时器属性标记定义 */
#define TIMER_PROP_DEAULT         (0x0)                 /* 定时器空属性标记                     */
#define TIMER_PROP_READY          (0x1<<0)              /* 定时器就绪标记                       */
#define TIMER_PROP_EXPIRED        (0x1<<1)              /* 定时器期满标记                       */
#define TIMER_PROP_PERIODIC       (0x1<<2)              /* 用户周期回调定时器                   */
#define TIMER_PROP_ACCURATE       (0x1<<3)              /* 用户精准定时器                       */

#define TIMER_USER_PROPERTY    (TIMER_PROP_PERIODIC| TIMER_PROP_ACCURATE)

/* 定时器运行错误码定义 */
#define TIMER_DIAG_NORMAL         (TBitMask)(0x0)       /* 定时器正常                            */
#define TIMER_DIAG_OVERFLOW       (TBitMask)(0x1<<0)    /* 定时器计数溢出                        */

/* 用户定时器回调函数类型定义 */
typedef void(*TTimerRoutine)(TArgument data, TTimeTick ticks);

/* 定时器结构定义 */
struct TimerDef
{
    TProperty     Property;                              /* 定时器属性                           */
    TTimerStatus  Status;                                /* 定时器状态                           */
    TTimeTick     MatchTicks;                            /* 定时器到时时刻                       */
    TTimeTick     PeriodTicks;                           /* 定时器延时计数                       */
    TTimerRoutine Routine;                               /* 用户定时器回调函数                   */
    TArgument     Argument;                              /* 定时器延时回调参数                   */
    TPriority     Priority;                              /* 定时器回调优先级                     */
    TTimeTick     ExpiredTicks;                          /* 定时器期满时刻                       */
    TLinkNode     ExpiredNode;                           /* 定时器期满队列的链表指针             */
    TBitMask      Diagnosis;                             /* 定时器运行错误码                     */
    TLinkNode     LinkNode;                              /* 定时器所在队列的链表指针             */
    TObject       Object;
};
typedef struct TimerDef TTimer;


/* 定时器队列结构定义 */
struct TimerListDef
{
    TLinkNode*    DormantHandle;
    TLinkNode*    ActiveHandle[TCLC_TIMER_WHEEL_SIZE];
    TLinkNode*    ExpiredHandle;
};
typedef struct TimerListDef TTimerList;

#define TCLM_NODE2TIMER(NODE) ((TTimer*)((TByte*)(NODE)-OFF_SET_OF(TTimer, LinkNode)))

extern void uTimerModuleInit(void);
extern void uTimerTickUpdate(void);

extern TState xTimerCreate(TTimer* pTimer, TChar* pName, TProperty property, TTimeTick ticks,
                           TTimerRoutine pRoutine, TArgument data, TPriority priority, TError* pError);
extern TState xTimerDelete(TTimer * pTimer, TError* pError);
extern TState xTimerStart(TTimer* pTimer, TTimeTick lagticks, TError* pError);
extern TState xTimerStop(TTimer* pTimer, TError* pError);
extern TState xTimerConfig(TTimer* pTimer, TTimeTick ticks, TPriority priority, TError* pError);
extern void uTimerCreateDaemon(void);
#endif


#endif /*_TCL_TIMER_H_*/

