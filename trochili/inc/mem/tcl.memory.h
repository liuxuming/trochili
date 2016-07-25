/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TCLC_MEMORY_H
#define _TCLC_MEMORY_H

#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.object.h"

#if (TCLC_MEMORY_ENABLE)

#define MEM_ERR_NONE               (0x0U)
#define MEM_ERR_FAULT              (0x1<<1)                      /* 一般性错误                 */
#define MEM_ERR_UNREADY            (0x1<<2)                      /* 内存管理结构未初始化       */
#define MEM_ERR_NO_MEM             (0x1<<3)                      /* 可分配内存不足             */
#define MEM_ERR_BAD_ADDR           (0x1<<4)                      /* 释放的内存地址非法         */
#define MEM_ERR_DBL_FREE           (0x1<<5)                      /* 释放的内存没有被分配       */
#define MEM_ERR_POOL_FULL          (0x1<<6)                      /* 释放的内存没有被分配       */

#define MEM_PROP_READY (0x1 << 0)

#endif

#endif /* _TCLC_MEMORY_H */

