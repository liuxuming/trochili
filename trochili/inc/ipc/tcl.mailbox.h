/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TCL_MAILBOX_H
#define _TCL_MAILBOX_H

#include "tcl.config.h"
#include "tcl.types.h"
#include "tcl.object.h"
#include "tcl.ipc.h"

#if ((TCLC_IPC_ENABLE)&&(TCLC_IPC_MAILBOX_ENABLE))

/* 邮箱状态定义 */
typedef enum
{
    OsMailboxEmpty,                  /* 邮箱数据空              */
    OsMailboxFull                    /* 邮箱数据满              */
} TMailboxStatus;

/* 邮件结构定义 */
typedef void* TMail;

/* 邮箱结构定义 */
struct MailboxDef
{
    TProperty      Property;         /* 线程的调度策略等属性配置 */
    TMail          Mail;             /* 邮箱的邮件对象           */
    TMailboxStatus Status;           /* 邮箱的状态               */
    TIpcQueue      Queue;            /* 邮箱的线程阻塞队列       */
    TObject        Object;
};
typedef struct MailboxDef TMailbox;


extern TState TclCreateMailbox(TMailbox* pMailbox, TChar* pName, TProperty property, TError* pError);
extern TState TclDeleteMailbox(TMailbox* pMailbox, TError* pError);
extern TState TclReceiveMail(TMailbox* pMailbox, TMail* pMail2, TOption option,
                             TTimeTick timeo, TError* pError);
extern TState TclSendMail(TMailbox* pMailbox, TMail* pMail2, TOption option, TTimeTick timeo,
                          TError* pError);
extern TState TclIsrSendMail(TMailbox* pMailbox, TMail* pMail2, TOption option, TError* pError);
extern TState TclBroadcastMail(TMailbox* pMailbox, TMail* pMail2, TError* pError);
extern TState TclResetMailbox(TMailbox* pMailbox, TError* pError);
extern TState TclFlushMailbox(TMailbox* pMailbox, TError* pError);

#endif

#endif /* _TOCHILI_MAILBOX_H */

