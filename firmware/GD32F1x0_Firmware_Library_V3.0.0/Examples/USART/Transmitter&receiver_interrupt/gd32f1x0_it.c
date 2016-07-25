/*!
    \file  gd32f1x0_it.c
    \brief interrupt service routines
*/

/*
    Copyright (C) 2016 GigaDevice

    2014-12-26, V1.0.0, firmware for GD32F1x0(x=3,5)
    2016-01-15, V2.0.0, firmware for GD32F1x0(x=3,5,7,9)
    2016-04-30, V3.0.0, firmware update for GD32F1x0(x=3,5,7,9)
*/


#include "gd32f1x0_it.h"
#include "gd32f1x0_eval.h"

extern uint8_t transfersize;
extern uint8_t receivesize;
extern __IO uint8_t txcount; 
extern __IO uint16_t rxcount; 
extern uint8_t receiver_buffer[32];
extern uint8_t transmitter_buffer[];
/*!
    \brief      this function handles NMI exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void NMI_Handler(void)
{
}

/*!
    \brief      this function handles HardFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void HardFault_Handler(void)
{
    /* if Hard Fault exception occurs, go to infinite loop */
    while (1);
}

/*!
    \brief      this function handles MemManage exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void MemManage_Handler(void)
{
    /* if Memory Manage exception occurs, go to infinite loop */
    while (1);
}

/*!
    \brief      this function handles BusFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void BusFault_Handler(void)
{
    /* if Bus Fault exception occurs, go to infinite loop */
    while (1);
}

/*!
    \brief      this function handles UsageFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void UsageFault_Handler(void)
{
    /* if Usage Fault exception occurs, go to infinite loop */
    while (1);
}

/*!
    \brief      this function handles SVC exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void SVC_Handler(void)
{
}

/*!
    \brief      this function handles DebugMon exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void DebugMon_Handler(void)
{
}

/*!
    \brief      this function handles PendSV exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void PendSV_Handler(void)
{
}

/*!
    \brief      this function handles USART RBNE interrupt request and TBE interrupt request
    \param[in]  none
    \param[out] none
    \retval     none
*/
void USART0_IRQHandler(void)
{
    if(RESET != usart_interrupt_flag_get(EVAL_COM1, USART_STAT_RBNE,USART_INT_RBNEIE)){
        /* receive data */
        receiver_buffer[rxcount++] = (usart_data_receive(EVAL_COM1) & 0x7F);
        if(rxcount == receivesize){
            usart_interrupt_disable(EVAL_COM1, USART_INT_RBNEIE);
        }
    }

    if(RESET != usart_interrupt_flag_get(EVAL_COM1, USART_STAT_TC,USART_INT_TBEIE)){
        /* transmit data */
        usart_data_transmit(EVAL_COM1, transmitter_buffer[txcount++]);
        if(txcount == transfersize){
            usart_interrupt_disable(EVAL_COM1, USART_INT_TBEIE);
        }
    }
}

