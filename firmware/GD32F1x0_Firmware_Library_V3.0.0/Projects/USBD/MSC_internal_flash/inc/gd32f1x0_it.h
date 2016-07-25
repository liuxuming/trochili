/*!
    \file  gd32f1x0_it.h
    \brief the header file of the ISR
*/

/*
    Copyright (C) 2016 GigaDevice

    2014-09-06, V1.0.0, firmware for GD32F150
    2016-01-15, V1.0.1, firmware for GD32F150
    2016-04-30, V3.0.0, firmware update for GD32F150
*/

#ifndef GD32F1X0_IT_H
#define GD32F1X0_IT_H

#include "gd32f1x0.h"

/* function declarations */
/* this function handles NMI exception */
void NMI_Handler(void);
/* this function handles HardFault exception */
void HardFault_Handler(void);
/* this function handles MemManage exception */
void MemManage_Handler(void);
/* this function handles BusFault exception */
void BusFault_Handler(void);
/* this function handles UsageFault exception */
void UsageFault_Handler(void);
/* this function handles SVC exception */
void SVC_Handler(void);
/* this function handles DebugMon exception */
void DebugMon_Handler(void);
/* this function handles PendSV exception */
void PendSV_Handler(void);
/* this function handles SysTick exception */
void SysTick_Handler(void);
/*  this function handles USB FS exception */
void USBD_LP_IRQHandler (void);
/* this function handles USB WakeUp interrupt exception.*/
void USBDWakeUp_IRQHandler  (void);

#endif /* GD32F1X0_IT_H */
