/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.kernel.h"
#include "tcl.debug.h"


/*************************************************************************************************
 *  功能：内核出现严重问题而停止继续运行                                                         *
 *  参数：(1) pNote  提示字符串                                                                  *
 *        (2) pFile  出错的文件名                                                                *
 *        (3) pFunc  出错的函数名                                                                *
 *        (4) line   出错的代码行                                                                *
 *  返回：无                                                                                     *
 *  说明：这是内核诊断到自身发生错误时采取的行动                                                 *
 *************************************************************************************************/
void uDebugPanic(const char* pNote, const char* pFile, const char* pFunc, int line)
{
    CpuDisableInt();		
    uKernelTrace(pNote);
    uKernelVariable.DBGLog.File = pFile;
    uKernelVariable.DBGLog.Func = pFunc;
    uKernelVariable.DBGLog.Line = line;
    uKernelVariable.DBGLog.Note = pNote;
    while (eTrue)
    {
        ;
    }
}

