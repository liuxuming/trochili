/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TCLC_IPC_H
#define _TCLC_IPC_H

#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.object.h"

#if (TCLC_IPC_ENABLE)

/* IPC线程阻塞队列结构定义 */
struct IpcBlockedQueueDef
{
    TProperty* Property;                                /* 线程阻塞队列属性                          */
    TLinkNode*  PrimaryHandle;                           /* 队列中基本线程分队列                      */
    TLinkNode*  AuxiliaryHandle;                         /* 队列中辅助线程分队列                      */
};
typedef struct IpcBlockedQueueDef TIpcQueue;


/* IPC操作结果，内核代码使用 */
#define IPC_ERR_NONE             (TError)(0x0)          /* 操作成功                                  */
#define IPC_ERR_FAULT            (TError)(0x1<<0)       /* 函数/参数用法错误                         */
#define IPC_ERR_UNREADY          (TError)(0x1<<1)       /* IPC对象没有被初始化                       */
#define IPC_ERR_NORMAL           (TError)(0x1<<2)       /* 信号量:信号量的数值不能满足操作           */ 
                                                        /* 邮箱和消息队列:状态不能被满足操作         */
                                                        /* 事件标记:待发送的事件已经存在 or 
                                                                    接收事件时期待的事件不能被满足   */
														/* 互斥量： 互斥量已被其他线程占用 or 
                                                                    互斥量不属于当前线程             */
#define IPC_ERR_TIMEO            (TError)(0x1<<3)       /* 因阻塞时限到达线程被唤醒                  */
#define IPC_ERR_DELETE           (TError)(0x1<<4)       /* IPC对象被销毁，线程被唤醒                 */
#define IPC_ERR_RESET            (TError)(0x1<<5)       /* IPC对象被重置，线程被唤醒                 */
#define IPC_ERR_FLUSH            (TError)(0x1<<6)       /* IPC阻塞队列上的线程被迫中止               */
#define IPC_ERR_ABORT            (TError)(0x1<<7)       /* IPC阻塞队列上的线程被迫中止               */
#define IPC_ERR_ACAPI            (TError)(0x1<<9)       /* 线程不能以阻塞方式访问IPC对象             */

/* IPC对象属性，内核代码使用 */
#define IPC_PROP_DEFAULT         (TProperty)(0x0)
#define IPC_PROP_READY           (TProperty)(0x1<<0)    /* IPC对象已经被初始化                       */
#define IPC_PROP_PREEMP_AUXIQ    (TProperty)(0x1<<1)    /* 辅助线程阻塞队列采用优先级调度方案        */
#define IPC_PROP_PREEMP_PRIMIQ   (TProperty)(0x1<<2)    /* 基本线程阻塞队列采用优先级调度方案        */
#define IPC_PROP_AUXIQ_AVAIL     (TProperty)(0x1<<17)   /* 辅助线程阻塞队列里存在被阻塞的线程        */
#define IPC_PROP_PRIMQ_AVAIL     (TProperty)(0x1<<18)   /* 基本线程阻塞队列里存在被阻塞的线程        */

#define IPC_RESET_SEMAPHORE_PROP (IPC_PROP_READY | IPC_PROP_PREEMP_PRIMIQ)
#define IPC_RESET_MUTEX_PROP     (IPC_PROP_READY | IPC_PROP_PREEMP_PRIMIQ)
#define IPC_RESET_MBOX_PROP      (IPC_PROP_READY | IPC_PROP_PREEMP_PRIMIQ | IPC_PROP_PREEMP_AUXIQ)
#define IPC_RESET_MQUE_PROP      (IPC_PROP_READY | IPC_PROP_PREEMP_PRIMIQ | IPC_PROP_PREEMP_AUXIQ)
#define IPC_RESET_FLAG_PROP      (IPC_PROP_READY | IPC_PROP_PREEMP_PRIMIQ)

#define IPC_USER_SEMAPHORE_PROP (IPC_PROP_PREEMP_PRIMIQ)
#define IPC_USER_MUTEX_PROP     (IPC_PROP_PREEMP_PRIMIQ)
#define IPC_USER_MBOX_PROP      (IPC_PROP_PREEMP_PRIMIQ | IPC_PROP_PREEMP_AUXIQ)
#define IPC_USER_MQUE_PROP      (IPC_PROP_PREEMP_PRIMIQ | IPC_PROP_PREEMP_AUXIQ)
#define IPC_USER_FLAG_PROP      (IPC_PROP_PREEMP_PRIMIQ)

/* 线程IPC选项，内核代码使用 */
#define IPC_OPT_DEFAULT               (TOption)(0x0)
#define IPC_OPT_WAIT             (TOption)(0x1<<1)       /* 永久方式等待IPC                          */
#define IPC_OPT_TIMEO            (TOption)(0x1<<2)       /* 时限方式等待标记                         */
#define IPC_OPT_UARGENT          (TOption)(0x1<<3)       /* 消息队列、邮件使用                       */
#define IPC_OPT_AND              (TOption)(0x1<<4)       /* 标记事件标记操作是AND类型                */
#define IPC_OPT_OR               (TOption)(0x1<<5)       /* 标记事件标记操作是OR类型                 */
#define IPC_OPT_CONSUME          (TOption)(0x1<<6)       /* 事件标记使用                             */

#define IPC_OPT_SEMAPHORE        (TOption)(0x1<<16)      /* 标记线程阻塞在信号量的线程阻塞队列中     */
#define IPC_OPT_MUTEX            (TOption)(0x1<<17)      /* 标记线程阻塞在互斥量的线程阻塞队列中     */
#define IPC_OPT_MAILBOX          (TOption)(0x1<<18)      /* 标记线程阻塞在邮箱的线程阻塞队列中       */
#define IPC_OPT_MSGQUEUE         (TOption)(0x1<<19)      /* 标记线程阻塞在消息队列的线程阻塞队列中   */
#define IPC_OPT_FLAGS            (TOption)(0x1<<20)      /* 标记线程阻塞在事件标记的线程阻塞队列中   */

#define IPC_OPT_USE_AUXIQ        (TOption)(0x1<<23)      /* 标记线程在线程阻塞队列的辅助队列中       */
#define IPC_OPT_READ_DATA        (TOption)(0x1<<24)      /* 接收邮件或者消息                         */
#define IPC_OPT_WRITE_DATA       (TOption)(0x1<<25)      /* 发送邮件或者消息                         */

#define IPC_USER_SEMAPHORE_OPTION  (IPC_OPT_WAIT|IPC_OPT_TIMEO)
#define IPC_USER_MUTEX_OPTION      (IPC_OPT_WAIT|IPC_OPT_TIMEO)
#define IPC_USER_MBOX_OPTION       (IPC_OPT_WAIT|IPC_OPT_TIMEO|IPC_OPT_UARGENT)
#define IPC_USER_MSGQ_OPTION       (IPC_OPT_WAIT|IPC_OPT_TIMEO|IPC_OPT_UARGENT)
#define IPC_USER_FLAG_OPTION       (IPC_OPT_WAIT|IPC_OPT_TIMEO|IPC_OPT_AND|IPC_OPT_OR|IPC_OPT_CONSUME)

/* NOTE: not compliant MISRA2004 18.4: Unions shall not be used. */
union IpcDataDef
{
    TBase32 Value;                                /* 保存被传输数据变量的地址址值               */
    void*   Addr1;                                /* 指向事件标记的一级指针                     */
    void**  Addr2;                                /* 指向消息或者邮件的二级指针                 */
};
typedef union IpcDataDef TIpcData;

/* 线程用于记录IPC对象的详细信息的记录结构 */
struct IpcContextDef
{
    void*        Object;                          /* 指向IPC对象地址的指针                      */
    TIpcQueue*   Queue;                           /* 线程所属IPC线程队列指针                    */
    TIpcData     Data;                            /* 和IPC对象操作相关的数据指针                */
    TBase32      Length;                          /* 和IPC对象操作相关的数据长度                */
    TOption      Option;                          /* 访问IPC对象的操作参数                      */
    TState*      State;                           /* IPC对象操作的返回值                        */
    TError*      Error;                           /* IPC对象操作的错误代码                      */
    void*        Owner;                           /* IPC对象所属线程                            */
    TLinkNode    LinkNode;                        /* 线程所在IPC队列的链表节点                  */
};
typedef struct IpcContextDef TIpcContext;


extern void uIpcInitContext(TIpcContext* pContext, void* pIpc, TBase32 data, TBase32 len, TOption option,
                            TState* pState, TError* pError);
extern void uIpcCleanContext(TIpcContext* pContext);
extern void uIpcBlockThread(TIpcContext* pContext, TIpcQueue* pQueue, TTimeTick ticks);
extern void uIpcUnblockThread(TIpcContext* pContext,  TState state, TError error, TBool* pHiRP);
extern void uIpcUnblockAll(TIpcQueue* pQueue, TState state, TError error,
                           void** pData2, TBool* pHiRP);
extern void uIpcSetPriority(TIpcContext* pContext, TPriority priority);

#endif

#endif /* _TCLC_IPC_H */

