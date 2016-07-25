/*!
    \file  gd32f1x0_it.c
    \brief main interrupt service routines
*/

/*
    Copyright (C) 2016 GigaDevice

    2014-09-06, V1.0.0, firmware for GD32F150
    2016-01-15, V1.0.1, firmware for GD32F150
    2016-04-30, V3.0.0, firmware update for GD32F150
*/

#include "gd32f1x0_it.h"
#include "usbd_int.h"

uint8_t Send_Buffer[4] = {0x00, 0x01, 0x00, 0x01};

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
    \brief      this function handles USBD interrupt
    \param[in]  none
    \param[out] none
    \retval     none
*/
void  USBD_LP_IRQHandler (void)
{
    usbd_isr();
}

/*!
    \brief      this function handles EXTI0_1_IRQ Handler.
    \param[in]  none
    \param[out] none
    \retval     none
*/
void  EXTI0_1_IRQHandler (void)
{
    if (RESET != exti_interrupt_flag_get(WAKEUP_KEY_EXTI_LINE)) {
        if (USBD_CONFIGURED == usb_device_dev.status) {
            Send_Buffer[0] = 0x15; 

            if (RESET == gd_eval_keygetstate(KEY_WAKEUP)) {
                if(Send_Buffer[1]) {
                    Send_Buffer[1] = 0x00;
                } else {
                    Send_Buffer[1] = 0x01;
                }
            }

            custom_hid_report_send (&usb_device_dev, Send_Buffer, 2);
        }

        /* clear the EXTI line interrupt flag */
        exti_interrupt_flag_clear(WAKEUP_KEY_EXTI_LINE);
    }
}


/*!
    \brief      this function handles EXTI14_15_IRQ Handler.
    \param[in]  none
    \param[out] none
    \retval     none
*/
void  EXTI4_15_IRQHandler (void)
{
    if (RESET != exti_interrupt_flag_get(TAMPER_KEY_EXTI_LINE)) {
        if(USBD_CONFIGURED == usb_device_dev.status) {
            Send_Buffer[2] = 0x16; 

            if (RESET == gd_eval_keygetstate(KEY_TAMPER)) {
                if(Send_Buffer[3]) {
                    Send_Buffer[3] = 0x00;
                } else {
                    Send_Buffer[3] = 0x01;
                }
            }

            custom_hid_report_send (&usb_device_dev, &Send_Buffer[2], 2);
        }

        /* clear the EXTI line interrupt flag */
        exti_interrupt_flag_clear(TAMPER_KEY_EXTI_LINE);
    }
}

#ifdef USBD_LOWPWR_MODE_ENABLE

/*!
    \brief      this function handles USBD wakeup interrupt request.
    \param[in]  none
    \param[out] none
    \retval     none
*/
void  USBDWakeUp_IRQHandler (void)
{
    exti_int_flag_clear(EXTI_18);
}

#endif /* USBD_LOWPWR_MODE_ENABLE */
