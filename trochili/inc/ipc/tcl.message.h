/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TCL_MQUEUE_H
#define _TCL_MQUEUE_H

#include "tcl.config.h"
#include "tcl.types.h"
#include "tcl.object.h"
#include "tcl.ipc.h"

#if ((TCLC_IPC_ENABLE)&&(TCLC_IPC_MQUE_ENABLE))

/* 消息队列状态定义 */
enum MQStatus
{
    OsMQEmpty,                /* 消息队列空              */
    OsMQFull,                 /* 消息队列满              */
    OsMQPartial               /* 消息队列非空非满        */
};
typedef enum MQStatus TMQStatus;

/* 消息数据结构定义 */
typedef void* TMessage;

/* 消息队列结构定义 */
struct MessageQueueCB
{
    TProperty Property;       /* 消息队列属性配置        */
    void**    MsgPool;        /* 消息缓冲池              */
    TBase32   Capacity;       /* 消息队列容量            */
    TBase32   MsgEntries;     /* 消息队列中消息的数目    */
    TIndex    Head;           /* 消息队列写指针位置      */
    TIndex    Tail;           /* 消息队列读指针位置      */
    TMQStatus Status;         /* 消息队列状态            */
    TIpcQueue Queue;          /* 消息队列的线程阻塞队列  */
    TObject   Object;
};
typedef struct MessageQueueCB TMsgQueue;


extern TState TclCreateMsgQueue(TMsgQueue* pMsgQue, TChar* pName, void** pPool2, TBase32 capacity,
                                TProperty property, TError* pError);
extern TState TclDeleteMsgQueue(TMsgQueue* pMsgQue, TError* pError);
extern TState TclReceiveMessage(TMsgQueue* pMsgQue, TMessage* pMsg2, TOption option,
                                TTimeTick timeo, TError* pError);
extern TState TclSendMessage(TMsgQueue* pMsgQue, TMessage* pMsg2, TOption option,
                             TTimeTick timeo, TError* pError);
extern TState TclIsrSendMessage(TMsgQueue* pMsgQue, TMessage* pMsg2, TOption option, TError* pError);
extern TState TclBroadcastMessage(TMsgQueue* pMsgQue, TMessage* pMsg2, TError* pError);
extern TState TclFlushMsgQueue(TMsgQueue* pMsgQue, TError* pError);
extern TState TclResetMsgQueue(TMsgQueue* pMsgQue, TError* pError);
#endif

#endif /* _TCL_MQUEUE_H */

