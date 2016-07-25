/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TCLC_CPU_H
#define _TCLC_CPU_H

#include "tcl.types.h"

extern void CpuSetupEntry(void);
extern void CpuStartTickClock(void);
extern void CpuBuildThreadStack(TAddr32* pTop, void* pStack, TBase32 bytes,
                                void* pEntry, TArgument argument);
extern void CpuConfirmThreadSwitch(void);
extern void CpuCancelThreadSwitch(void);
extern void CpuDisableInt(void);
extern void CpuEnableInt(void);
extern void CpuEnterCritical(TReg32* pValue);
extern void CpuLeaveCritical(TReg32 value);
extern void CpuLoadRootThread(void);
extern TPriority CpuCalcHiPRIO(TBase32 data);

#endif /* _TCLC_CPU_H */

