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

#define TIMER_ERR_NONE               (0x0U)
#define TIMER_ERR_FAULT              (0x1<<0)           /* 一般性错误                          */
#define TIMER_ERR_UNREADY            (0x1<<1)           /* 定时器管理结构未初始化              */
#define TIMER_ERR_TYPE               (0x1<<2)           /* 定时器类型错误                      */


/* 定时器类型枚举定义 */
enum TimerTypeDef
{
    eThreadTimer = 0,                                    /* 用于线程普通延时定时器               */
    eIpcTimer,                                           /* 用于IPC的时限阻塞定时器              */
    eUserTimer,                                          /* 用户定时器                           */
};
typedef enum TimerTypeDef TTimerType;

/* 定时器状态枚举定义 */
enum TimerStatusDef
{
    eTimerDormant = 0,                                   /* 定时器的初始状态                     */
    eTimerActive,                                        /* 定时器激活状态                       */
#if (TCLC_TIMER_DAEMON_ENABLE)
    eTimerExpired                                        /* 用户定时器期满状态                   */
#endif
};
typedef enum TimerStatusDef TTimerStatus;

/* 定时器属性标记定义 */
#define TIMER_PROP_NONE           (0x0)                 /* 定时器空属性标记                     */
#define TIMER_PROP_READY          (0x1<<0)              /* 定时器就绪标记                       */
#define TIMER_PROP_PERIODIC       (0x1<<1)              /* 用户周期回调定时器                   */
#define TIMER_PROP_URGENT         (0x1<<2)              /* 用户紧急回调定时器                   */

/* 用户定时器回调函数类型定义 */
typedef void(*TTimerRoutine)(TArgument data);

/* 定时器结构定义 */
struct TimerDef
{
    TProperty     Property;                              /* 定时器属性                           */
    TBase32       ID;                                    /* 定时器编号                           */
    void*         Owner;                                 /* 定时器所属线程                       */
    TTimerStatus  Status;                                /* 定时器状态                           */
    TTimerType    Type;                                  /* 定时器类型                           */
    TTimeTick     MatchTicks;                            /* 定时器到时时刻                       */
    TTimeTick     PeriodTicks;                           /* 定时器延时计数                       */
    TTimerRoutine Routine;                               /* 用户定时器回调函数                   */
    TArgument     Argument;                              /* 定时器延时回调参数                   */
    TObjNode      ObjNode;                               /* 定时器所在队列的链表指针             */
};
typedef struct TimerDef TTimer;

/* 定时器队列结构定义 */
struct TimerListDef
{
    TObjNode*    DormantHandle;
    TObjNode*    ActiveHandle[TCLC_TIMER_WHEEL_SIZE];
#if (TCLC_TIMER_DAEMON_ENABLE)
    TObjNode*    ExpiredHandle;
#endif
};
typedef struct TimerListDef TTimerList;

//#define TCLM_NODE2TIMER(NODE) ((TThread*)((TByte*)(NODE)-OFF_SET_OF(TTimer, ObjNode)))

extern void uTimerModuleInit(void);
extern void uTimerCreate(TTimer* pTimer, TProperty property, TTimerType type, TTimeTick ticks,
                         TTimerRoutine pRoutine, TArgument data, void* pOwner);
extern void uTimerDelete(TTimer* pTimer);
extern void uTimerConfig(TTimer* pTimer, TTimerType type, TTimeTick ticks);
extern void uTimerStart(TTimer* pTimer, TTimeTick lagticks);
extern void uTimerStop(TTimer* pTimer);
extern void uTimerTickISR(void);

extern TState xTimerCreate(TTimer* pTimer, TProperty property, TTimeTick ticks,
                           TTimerRoutine pRoutine, TArgument data, TError* pError);
extern TState xTimerDelete(TTimer * pTimer, TError* pError);
extern TState xTimerStart(TTimer* pTimer, TTimeTick lagticks, TError* pError);
extern TState xTimerStop(TTimer* pTimer, TError* pError);
extern TState xTimerConfig(TTimer* pTimer, TTimeTick ticks, TError* pError);

#if (TCLC_TIMER_DAEMON_ENABLE)
extern void uTimerCreateDaemon(void);
#endif
#endif


#endif /*_TCL_TIMER_H_*/

