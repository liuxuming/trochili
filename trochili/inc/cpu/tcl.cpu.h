/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TCLC_CPU_H
#define _TCLC_CPU_H

#include "tcl.types.h"

extern void OsCpuSetupEntry(void);
extern void OsCpuStartTickClock(void);
extern void OsCpuBuildThreadStack(TAddr32* pTop, void* pStack, TBase32 bytes,
                                void* pEntry, TArgument argument);
extern void OsCpuConfirmThreadSwitch(void);
extern void OsCpuCancelThreadSwitch(void);
extern void OsCpuDisableInt(void);
extern void OsCpuEnableInt(void);
extern void OsCpuEnterCritical(TReg32* pValue);
extern void OsCpuLeaveCritical(TReg32 value);
extern void OsCpuLoadRootThread(void);
extern TPriority OsCpuCalcHiPRIO(TBase32 data);

#endif /* _TCLC_CPU_H */

