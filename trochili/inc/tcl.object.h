/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TCL_OBJECT_H
#define _TCL_OBJECT_H
#include "tcl.types.h"

/* 内核对象节点结构定义 */
struct ObjNodeDef
{
    struct ObjNodeDef*  Prev;
    struct ObjNodeDef*  Next;
    struct ObjNodeDef** Handle;
    void*  Owner;
    TBase32* Data;
};
typedef struct ObjNodeDef TObjNode;

#define OFF_SET_OF(TYPE, MEMBER) ((TBase32)(&(((TYPE *)0)->MEMBER)))

typedef enum QueuePosDef
{
    eQuePosHead,
    eQuePosTail
} TQueuePos;


void uObjQueueAddFifoNode(TObjNode** pHandle2, TObjNode* pNode, TQueuePos pos);
void uObjQueueAddPriorityNode(TObjNode** pHandle2, TObjNode* pNode);
void uObjQueueRemoveNode(TObjNode** pHandle2, TObjNode* pNode);
void uObjListAddNode(TObjNode** pHandle2, TObjNode* pNode, TQueuePos pos);
void uObjListRemoveNode(TObjNode** pHandle2, TObjNode* pNode);
void uObjListAddPriorityNode(TObjNode** pHandle2, TObjNode* pNode);

#endif /* _TCL_OBJECT_H */

