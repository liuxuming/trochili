#include "tcl.gd32f190.h"
#include "colibri190bsp.h"

/* 配置使能评估板上的设备 */
void EvbSetupEntry(void)
{
   EvbUart1Config();
   EvbLedConfig();
   EvbKeyConfig();
}


void EvbTraceEntry(const char* str)
{
    EvbUart1WriteStr(str);
}


