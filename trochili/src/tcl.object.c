/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.debug.h"
#include "tcl.object.h"

/*************************************************************************************************
 *  功能：按照先入先出规则将节点加入到指定的双向循环链表                                         *
 *  参数：(1) pHandle2 指向链表头指针的指针                                                      *
 *        (2) pNode    节点指针                                                                  *
 *        (3) pos      队首队尾标记                                                              *
 *  返回：无                                                                                     *
 *  说明：无                                                                                     *
 *************************************************************************************************/
void uObjQueueAddFifoNode(TLinkNode** pHandle2, TLinkNode* pNode, TLinkPos pos)
{
    KNL_ASSERT((pNode->Handle == (TLinkNode**)0), "");

    if (*pHandle2)
    {
        /* 如果队列不空，则把节点追加在队尾 */
        pNode->Prev = (*pHandle2)->Prev;
        pNode->Prev->Next = pNode;
        pNode->Next = *pHandle2;
        pNode->Next->Prev = pNode;
        if (pos == eLinkPosHead)
        {
            (*pHandle2) = (*pHandle2)->Prev;
        }
    }
    else
    {
        /* 否则初始化第一个节点和头节点指针 */
        *pHandle2 = pNode;
        pNode->Prev = pNode;
        pNode->Next = pNode;
    }
    pNode->Handle = pHandle2;
}


/*************************************************************************************************
 *  功能：按照先优先级降序规则将节点加入到指定的双向循环链表                                     *
 *  参数：(1) pHandle2 指向链表头指针的指针                                                      *
 *        (2) pNode    节点指针                                                                  *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void uObjQueueAddPriorityNode(TLinkNode** pHandle2, TLinkNode* pNode)
{
    TLinkNode* pTemp = (TLinkNode*)0;

    KNL_ASSERT((pNode->Handle == (TLinkNode**)0), "");

    /* 检查队列是否为空 */
    if (*pHandle2)
    {
        pTemp = (*pHandle2);

        /* 新节点比头结点优先级还高(此时需要更新head指针)  */
        if (*(pTemp->Data) > *(pNode->Data))
        {
            (*pHandle2) = pNode;
        }
        else
        {
            /*
             * 在对列中找了新节点的位置，或者新节点的优先级比所有节点的优先级都低
             * (此时不需要更新head指针)
             */
            pTemp = pTemp->Next;
            while ((*(pTemp->Data) <= *(pNode->Data)) && (pTemp != (*pHandle2)))
            {
                pTemp = pTemp->Next;
            }
        }

        /* 插入新节点到队列  */
        pNode->Prev = pTemp->Prev;
        pNode->Prev->Next = pNode;
        pNode->Next = pTemp;
        pNode->Next->Prev = pNode;
    }
    else
    {
        (*pHandle2) = pNode;
        pNode->Prev = pNode;
        pNode->Next = pNode;
    }
    pNode->Handle = pHandle2;
}


/*************************************************************************************************
 *  功能：将节点从指定的双向循环链表中移出                                                       *
 *  参数：(1) pHandle2 指向链表头指针的指针                                                      *
 *        (2) pNode    节点指针                                                                  *
 *        (3) pos      队首队尾标记                                                              *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void uObjQueueRemoveNode(TLinkNode** pHandle2, TLinkNode* pNode)
{
    KNL_ASSERT((pNode->Handle == pHandle2), "");

    /* 检查是否队列中只有一个节点，如果是就把头结点指针清空 */
    if (pNode->Prev == pNode)
    {
        *pHandle2 = (TLinkNode*)0;
    }
    else
    {
        /* 如果队列中不止一个节点，则把当前节点删除 */
        pNode->Prev->Next = pNode->Next;
        pNode->Next->Prev = pNode->Prev;

        /* 如果给节点是头结点，则更新头结点指针，将其下移 */
        if (pNode == *pHandle2)
        {
            *pHandle2 = pNode->Next;
        }
    }

    /* 清空节点前后的链接信息 */
    pNode->Next = (TLinkNode*)0;
    pNode->Prev = (TLinkNode*)0;
    pNode->Handle = (TLinkNode**)0;
}


/*************************************************************************************************
 *  功能：将节点加入到双向链表的指定位置                                                         *
 *  范围：全局                                                                                   *
 *  参数：(1) pHandle2 指向链表头指针的指针                                                      *
 *        (2) pNode    节点指针                                                                  *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void uObjListAddNode(TLinkNode** pHandle2, TLinkNode* pNode, TLinkPos pos)
{
    TLinkNode* pTail;
    KNL_ASSERT((pNode->Handle == (TLinkNode**)0), "");

    /* 如果队列中有个节点 */
    if (*pHandle2)
    {
        if (pos == eLinkPosHead)
        {
            pNode->Next = *pHandle2;
            pNode->Prev = (TLinkNode*)0;
            (*pHandle2)->Prev = pNode;
            *pHandle2 = pNode;
        }
        else
        {
            pTail= *pHandle2;
            while(pTail->Next)
            {
                pTail = pTail->Next;
            }

            pNode->Next = (TLinkNode*)0;
            pNode->Prev = pTail;
            pTail->Next = pNode;
        }
    }
    /* 如果队列中没有节点 */
    else
    {
        *pHandle2 = pNode;
        pNode->Next = (TLinkNode*)0;
        pNode->Prev = (TLinkNode*)0;
    }
    pNode->Handle = pHandle2;
}


/*************************************************************************************************
 *  功能：按照先优先级降序规则将节点加入到指定的双向链表                                         *
 *  参数：(1) pHandle2 指向链表头指针的指针                                                      *
 *        (2) pNode    节点指针                                                                  *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void uObjListAddPriorityNode(TLinkNode** pHandle2, TLinkNode* pNode)
{
    TLinkNode* pCursor = (TLinkNode*)0;
    TLinkNode* pTail = (TLinkNode*)0;

    /* 如果链表为空，则将新节点设为头结点 */
    if ((*pHandle2) == (TLinkNode*)0)
    {
        *pHandle2 = pNode;
        pNode->Next = (TLinkNode*)0;
        pNode->Prev = (TLinkNode*)0;
    }
    else
    {
        /* 否则遍历队列，查找合适位置 */
        pCursor = *pHandle2;
        while (pCursor != (TLinkNode*)0)
        {
            pTail = pCursor;
            if (*(pNode->Data) >= *(pCursor->Data))
            {
                pCursor = pCursor->Next;
            }
            else
            {
                break;
            }
        }

        /* 游标不为空说明在链表中找到合适的节点，需要把新节点插入到该节点之前 */
        if (pCursor != (TLinkNode*)0)
        {
            /* 如果是链表头节点 */
            if (pCursor->Prev == (TLinkNode*)0)
            {
                *pHandle2 = pNode;
                pCursor->Prev = pNode;
                pNode->Prev   = (TLinkNode*)0;
                pNode->Next    = pCursor;
            }
            /* 如果是链表其它节点(链表中间的节点或者尾节点) */
            else
            {
                pNode->Prev         = pCursor->Prev;
                pNode->Next         = pCursor;
                pCursor->Prev->Next = pNode;
                pCursor->Prev       = pNode;
            }
        }
        /* 如果游标为空说明没有找到合适的节点，所以只能把新节点放在链表最后 */
        else
        {
            pTail->Next = pNode;
            pNode->Prev = pTail;
            pNode->Next = (TLinkNode*)0;
        }
    }
    pNode->Handle = pHandle2;
}


/*************************************************************************************************
 *  功能：将节点从指定的双向链表中移出                                                           *
 *  范围：全局                                                                                   *
 *  参数：(1) pHandle2 指向链表头指针的指针                                                      *
 *        (2) pNode    节点指针                                                                  *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void uObjListRemoveNode(TLinkNode** pHandle2, TLinkNode* pNode)
{
    KNL_ASSERT((pNode->Handle == pHandle2), "");

    /* 如果队列中只有一个节点 */
    if ((pNode->Prev == (TLinkNode*)0) && (pNode->Next == (TLinkNode*)0))
    {
        *pHandle2 = (TLinkNode*)0;
    }
    /* 如果队列中有多个节点并且需要删除队列尾部的节点 */
    else if ((pNode->Prev != (TLinkNode*)0) && (pNode->Next == (TLinkNode*)0))
    {
        pNode->Prev->Next = (TLinkNode*)0;
    }
    /* 如果队列中有多个节点并且需要删除队列头部的节点 */
    else if ((pNode->Prev == (TLinkNode*)0) && (pNode->Next != (TLinkNode*)0))
    {
        pNode->Next->Prev = (TLinkNode*)0;
        *pHandle2 = pNode->Next;
    }
    /* 如果队列中有多个节点并且需要删除队列中部的节点 */
    else
    {
        pNode->Prev->Next = pNode->Next;
        pNode->Next->Prev = pNode->Prev;
    }

    /* 设置被删除节点的链接 */
    pNode->Next = (TLinkNode*)0;
    pNode->Prev = (TLinkNode*)0;
    pNode->Handle = (TLinkNode**)0;
}


/*************************************************************************************************
 *  功能：将节点加入到指定的双向差分链表                                                         *
 *  范围：全局                                                                                   *
 *  参数：(1) pHandle2 指向链表头指针的指针                                                      *
 *        (2) pNode    节点指针                                                                  *
 *  返回：无                                                                                     *
 *  说明：双向非循环链表                                                                         *
 *************************************************************************************************/
void uObjListAddDiffNode(TLinkNode** pHandle2, TLinkNode* pNode)
{
    TLinkNode* pCursor = (TLinkNode*)0;
    TLinkNode* pTail = (TLinkNode*)0;
    KNL_ASSERT((pNode->Handle == (TLinkNode**)0), "");

    /* 如果差分链表为空，则将新节点设为头结点 */
    if ((*pHandle2) == (TLinkNode*)0)
    {
        (*pHandle2) = pNode;
        pNode->Next = (TLinkNode*)0;
        pNode->Prev = (TLinkNode*)0;
    }
    /* 否则遍历队列，查找合适位置 */
    else
    {
        pCursor = *pHandle2;
        while (pCursor != (TLinkNode*)0)
        {
            pTail = pCursor;
            if (*(pNode->Data) >= *(pCursor->Data))
            {
                *(pNode->Data) -= *(pCursor->Data);
                pCursor = pCursor->Next;
            }
            else
            {
                *(pCursor->Data) -= *(pNode->Data);
                break;
            }
        }

        /* 游标不为空说明在链表中找到合适的节点，需要把新节点插入到该节点之前 */
        if (pCursor != (TLinkNode*)0)
        {
            /* 如果是链表头节点 */
            if (pCursor->Prev == (TLinkNode*)0)
            {
                *pHandle2 = pNode;
                pCursor->Prev = pNode;
                pNode->Prev = (TLinkNode*)0;
                pNode->Next = pCursor;
            }
            /* 如果是链表其它节点(链表中间的节点或者尾节点) */
            else
            {
                pNode->Prev = pCursor->Prev;
                pNode->Next = pCursor;
                pCursor->Prev->Next = pNode;
                pCursor->Prev = pNode;
            }
        }
        /* 如果游标为空说明没有找到合适的节点，所以只能把新节点放在链表最后 */
        else
        {
            pTail->Next = pNode;
            pNode->Prev = pTail;
            pNode->Next = (TLinkNode*)0;
        }
    }
    pNode->Handle = pHandle2;
}


/*************************************************************************************************
 *  功能：将节点从指定的双向差分链表中移出                                                       *
 *  范围：全局                                                                                   *
 *  参数：(1) pHandle2 指向链表头指针的指针                                                      *
 *        (2) pNode    节点指针                                                                  *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void uObjListRemoveDiffNode(TLinkNode** pHandle2, TLinkNode* pNode)
{
    KNL_ASSERT((pNode->Handle == pHandle2), "");

    /* 如果差分链表中只有一个节点 */
    if ((pNode->Next == (TLinkNode*)0) && (pNode->Prev == (TLinkNode*)0))
    {
        *pHandle2 = (TLinkNode*)0;
    }
    /* 如果删除的是链表的尾节点 */
    else if (pNode->Next == (TLinkNode*)0)
    {
        pNode->Prev->Next = (TLinkNode*)0;
    }
    /* 如果删除的是链表的头节点 */
    else if (pNode->Prev == (TLinkNode*)0)
    {
        *(pNode->Next->Data) += *(pNode->Data);
        *pHandle2 = pNode->Next;
        pNode->Next->Prev = (TLinkNode*)0;
    }
    /* 如果删除的是链表中间的节点 */
    else
    {
        *(pNode->Next->Data) += *(pNode->Data);
        pNode->Next->Prev = pNode->Prev;
        pNode->Prev->Next = pNode->Next;
    }

    /* 该节点从链表中删除 */
    pNode->Prev = (TLinkNode*)0;
    pNode->Next = (TLinkNode*)0;
    pNode->Handle = (TLinkNode**)0;
}

