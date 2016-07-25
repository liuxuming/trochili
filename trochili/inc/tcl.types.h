/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TCL_TYPES_H
#define _TCL_TYPES_H

/* 各种数据类型定义，内核移植时必须确认 */
typedef unsigned char      TByte;
typedef char               TChar;
typedef unsigned int       TBase32;
typedef unsigned int       TAddr32;
typedef unsigned int       TReg32;
typedef unsigned int       TIndex;
typedef unsigned int       TPriority;
typedef unsigned int       TBitMask;
typedef unsigned int       TOption;
typedef unsigned int       TProperty;
typedef unsigned long long TTimeTick;
typedef unsigned long long TTimeStamp;
typedef unsigned int       TError;
typedef unsigned int       TArgument;

/* 布尔类型定义                    */
typedef enum
{
    eFalse = 0U,
    eTrue  = 1U
} TBool;

/* API与各函数返回值的统一定义  */
typedef enum
{
    eFailure,                            /* 函数调用失败                                         */
    eSuccess,                            /* 函数调用成功                                         */		
    eError,                              /* 函数调用发生错误                                     */    
} TState;

/* 寄存器读写宏定义 */
#define TCLM_GET_REG32(r)   (*((volatile unsigned int*)(r)))
#define TCLM_SET_REG32(r,v) (*((volatile unsigned int*)(r)) = ((unsigned int)(v)))

/* 数值上限定义 */
#define TCLM_MAX_VALUE08    (0xff)
#define TCLM_MAX_VALUE16    (0xffff)
#define TCLM_MAX_VALUE32    (0xffffffff)
#define TCLM_MAX_VALUE64    (0xffffffffffffffff)

#endif /* _TCL_TYPES_H */

