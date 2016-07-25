/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TCL_DEBUG_H
#define _TCL_DEBUG_H

#include "tcl.types.h"
#include "tcl.config.h"

/* 内核运行错误记录 */
typedef struct DBGLogDef
{
    const TChar* Note;
    const TChar* File;
    const TChar* Func;
    TBase32        Line;
} TDBGLog;

extern void uDebugPanic(const char* pNote, const char* pFile, const char* pFunc, int line);

/* ASSERT宏定义 */
#if (TCLC_ASSERT_ENABLE)
#ifndef KNL_ASSERT
#define KNL_ASSERT(condition, note)\
    do{\
        if (!(condition))\
        {\
            uDebugPanic((note), __FILE__, __FUNCTION__, __LINE__);\
        }\
    } while(0)
#endif
#else
#define KNL_ASSERT
#endif

#endif /* _TCL_DEBUG_H */

