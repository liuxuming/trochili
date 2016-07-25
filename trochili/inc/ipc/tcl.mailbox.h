/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TCL_MAILBOX_H
#define _TCL_MAILBOX_H

#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.ipc.h"
#include "tcl.thread.h"

#if ((TCLC_IPC_ENABLE)&&(TCLC_IPC_MAILBOX_ENABLE))

/* 邮件类型定义 */
typedef enum
{
    eNormalMail,  /* 普通邮件  */
    eUrgentMail   /* 紧急邮件  */
} TMailType;

/* 邮箱状态定义 */
typedef enum
{
    eMailBoxEmpty,  /* 邮箱数据空 */
    eMailBoxFull    /* 邮箱数据满 */
} TMailBoxStatus;

/* 邮件结构定义 */
typedef void* TMail;

/* 邮箱结构定义 */
struct MailBoxDef
{
    TProperty      Property;         /* 线程的调度策略等属性配置 */
    TMail          Mail;             /* 邮箱的邮件对象           */
    TMailBoxStatus Status;           /* 邮箱的状态               */
    TIpcQueue      Queue;            /* 邮箱的线程阻塞队列       */
};
typedef struct MailBoxDef TMailBox;

extern TState xMailBoxSend(TMailBox* pMailbox, TMail* pMail2, TOption option, TTimeTick timeo, TError* pError);
extern TState xMailBoxReceive(TMailBox* pMailbox, TMail* pMail2, TOption option, TTimeTick timeo, TError* pError);
extern TState xMailBoxCreate(TMailBox* pMailbox, TProperty property, TError* pError);
extern TState xMailBoxDelete(TMailBox* pMailbox, TError* pError);
extern TState xMailBoxFlush(TMailBox* pMailbox, TError* pError);
extern TState xMailboxReset(TMailBox* pMailbox, TError* pError);
extern TState xMailBoxBroadcast(TMailBox* pMailbox, TMail* pMail2, TError* pError);

#endif

#endif /* _TOCHILI_MAILBOX_H */

