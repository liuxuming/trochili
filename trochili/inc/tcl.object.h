/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TCL_OBJECT_H
#define _TCL_OBJECT_H
#include "tcl.types.h"
#include "tcl.config.h"

/* 对象类型枚举定义 */
enum ObjectTypeDef
{
    OsThreadObject = 0,
    OsTimerObject,
    OsSemaphoreObject,
    OsMutexObject,
    OsMailboxObject,
    OsMessageQueueObject,
    OsFlagObject
};
typedef enum ObjectTypeDef TObjectType;

/* 内核对象节点结构定义 */
struct LinkNodeDef
{
    struct LinkNodeDef*  Prev;
    struct LinkNodeDef*  Next;
    struct LinkNodeDef** Handle;
    void*    Owner;
    TBase32* Data;
};
typedef struct LinkNodeDef TLinkNode;

/* 内核对象进入各个链表时的节点位置 */
typedef enum LinkPosDef
{
    OsLinkHead,
    OsLinkTail
} TLinkPos;

/* 内核对象结构定义 */
struct ObjectDef
{
    TBase32       ID;                                    /* 内核对象编号     */
    TObjectType   Type;                                  /* 内核对象类型     */
    TChar         Name[TCL_OBJ_NAME_LEN];                /* 内核对象名称     */
    void*         Owner;                                 /* 内核对象宿主     */
    TLinkNode     LinkNode;                              /* 内核对象链接节点 */
};
typedef struct ObjectDef TObject;

extern void OsObjQueueAddFifoNode(TLinkNode** pHandle2, TLinkNode* pNode, TLinkPos pos);
extern void OsObjQueueAddPriorityNode(TLinkNode** pHandle2, TLinkNode* pNode);
extern void OsObjQueueRemoveNode(TLinkNode** pHandle2, TLinkNode* pNode);
extern void OsObjListAddNode(TLinkNode** pHandle2, TLinkNode* pNode, TLinkPos pos);
extern void OsObjListRemoveNode(TLinkNode** pHandle2, TLinkNode* pNode);
extern void OsObjListAddPriorityNode(TLinkNode** pHandle2, TLinkNode* pNode);
extern void OsObjListAddDiffNode(TLinkNode** pHandle2, TLinkNode* pNode);
extern void OsObjListRemoveDiffNode(TLinkNode** pHandle2, TLinkNode* pNode);

#endif /* _TCL_OBJECT_H */

