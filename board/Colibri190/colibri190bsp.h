#ifndef _TCL_COLIBRI_H
#define _TCL_COLIBRI_H

#include "tcl.gd32f190.h"
#include "colibri_bsp_led.h"
#include "colibri_bsp_key.h"
#include "colibri_bsp_uart.h"


#define KEY_IRQ_ID     EXTI4_15_IRQID
#define TIM_IRQ_ID     TIMER2_IRQID
#define UART_IRQ_ID    USART2_IRQID

extern void EvbSetupEntry(void);
extern void EvbTraceEntry(const char* str);
#define EVB_PRINTF     EvbTraceEntry
#endif /* _TCL_COLIBRI_H */
