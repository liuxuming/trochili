/*!
    \file  usbd_int.c
    \brief USB device interrupt routines
*/

/*
    Copyright (C) 2016 GigaDevice

    2014-09-06, V1.0.0, firmware for GD32F150
    2016-01-15, V1.0.1, firmware for GD32F150
    2016-04-30, V3.0.0, firmware update for GD32F150
*/

#include "usbd_int.h"

extern uint32_t g_interrupt_mask;
extern __IO uint8_t g_suspend_enabled;
extern __IO uint8_t g_remote_wakeup_on;
extern __IO uint8_t g_ESOF_count;

#ifdef LPM_ENABLED
__IO uint32_t  L1_remote_wakeup = 0;
__IO uint32_t  besl = 0;
__IO uint16_t  LPM_intf = 0;
#endif /* LPM_ENABLED */

static uint8_t  usbd_intf_lpst    (usbd_core_handle_struct *pudev);
static uint8_t  usbd_intf_sof     (usbd_core_handle_struct *pudev);
static uint8_t  usbd_intf_esof    (usbd_core_handle_struct *pudev);
static uint8_t  usbd_intf_reset   (usbd_core_handle_struct *pudev);
static uint8_t  usbd_intf_suspend (usbd_core_handle_struct *pudev);
static uint8_t  usbd_intf_wakeup  (usbd_core_handle_struct *pudev);

/*!
    \brief      USB interrupt events service routine
    \param[in]  none
    \param[out] none
    \retval     none
*/
void  usbd_isr (void)
{
    __IO uint16_t interrupt_flag = 0;
    __IO uint16_t ctlr = 0;

    interrupt_flag = USBD_REG_GET(USBD_INTF);

#ifdef LPM_ENABLED
    LPM_intf = USBD_REG_GET(USBD_LPMINTF); 
#endif /* LPM_ENABLED */

    if (g_interrupt_mask & INTF_STIF & interrupt_flag) {
        /* the endpoint successful transfer interrupt service */
        usbd_intf_lpst(&usb_device_dev);
    }

    if (g_interrupt_mask & INTF_RSTIF & interrupt_flag) {
        /* clear reset interrupt flag in INTF */
        USBD_REG_SET(USBD_INTF, (uint16_t)CLR_RSTIF);

        /* USB reset interrupt handle */
        usbd_intf_reset(&usb_device_dev);
    }

    if (g_interrupt_mask & INTF_WKUPIF & interrupt_flag) {
        /* clear wakeup interrupt flag in INTF */
        USBD_REG_SET(USBD_INTF, (uint16_t)CLR_WKUPIF);

        /* USB wakeup interrupt handle */
        usbd_intf_wakeup(&usb_device_dev);

#ifdef LPM_ENABLED
        /* clear L1 remote wakeup flag */
        L1_remote_wakeup = 0;
#endif /* LPM_ENABLED */
    }

    if (g_interrupt_mask & INTF_SPSIF & interrupt_flag) {
        if(!(USBD_REG_GET(USBD_CTL) & CTL_RSREQ)) {
            /* process library core layer suspend routine*/
            usbd_intf_suspend(&usb_device_dev);

            /* clear of suspend interrupt flag bit must be done after setting of CTLR_SETSPS */
            USBD_REG_SET(USBD_INTF, (uint16_t)CLR_SPSIF);
        }
    }

    if (g_interrupt_mask & INTF_SOFIF & interrupt_flag) {
        /* clear SOF interrupt flag in INTF */
        USBD_REG_SET(USBD_INTF, (uint16_t)CLR_SOFIF);

        /* USB SOF interrupt handle */
        usbd_intf_sof(&usb_device_dev);
    }

    if (g_interrupt_mask & INTF_ESOFIF & interrupt_flag) {
        /* clear ESOF interrupt flag in INTF */
        USBD_REG_SET(USBD_INTF, (uint16_t)CLR_ESOFIF);

        /* USB ESOF interrupt handle */
        usbd_intf_esof(&usb_device_dev);
    }

#ifdef LPM_ENABLED
    if (LPM_intf & USBD_REG_GET(USBD_LPMINTF)) {
        /* clear L1 ST bit in LPM INTF */
        USBD_REG_SET(USBD_LPMINTF, LPM_CLR_STIF);

        /* read BESL field from subendpoint0 register which coressponds to HIRD parameter in LPM spec */
        besl = (USBD_REG_GET(USBD_SEP0) & ATTR_HIRD) >> 4;

        /* read BREMOTEWAKE bit from subendpoint0 register which corresponding to bRemoteWake bit in LPM request */
        L1_remote_wakeup = (USBD_REG_GET(USBD_SEP0) & ATTR_BREMOTEWAKE) >> 8;

        /* process USB device core layer suspend routine */
        usbd_intf_suspend(&usb_device_dev); 

        /* enter USB model in suspend and system in low power mode (DEEP_SLEEP mode) */
        usbd_suspend();
    }
#endif /* LPM_ENABLED */
}

/*!
    \brief      handle USB low priority successful transfer event 
    \param[in]  pudev: pointer to USB device instance
    \param[out] none
    \retval     USB device operation status
*/
static uint8_t  usbd_intf_lpst (usbd_core_handle_struct *pudev)
{
    usb_ep_struct *pEP = NULL;
    uint8_t EP_ID = 0;
    __IO uint16_t Intf = 0;
    __IO uint16_t ep_value = 0;

    /* wait till interrupts are not pending */
    while (0 != ((Intf = USBD_REG_GET(USBD_INTF)) & INTF_STIF)) {
        /* get endpoint number and the value of control and state register */
        EP_ID = (uint8_t)(Intf & INTF_EPNUM);
        ep_value = USBD_REG_GET(USBD_EPxCS(EP_ID));

        if (0 == (Intf & INTF_DIR)) {
            /* handle the in direction transaction */

            pEP = &(pudev->in_ep[EP_ID]);

            if (0 != (ep_value & EPxCS_TX_ST)) {
                /* clear successful transmit interrupt flag */
                USBD_ENDP_TX_STAT_CLEAR(EP_ID);

                /* just handle single buffer situation */
                pEP->trs_count = (pbuf_reg + EP_ID)->tx_count & EPTCNT_CNT;

                /* maybe mutiple packets */
                pEP->trs_buf += pEP->trs_count;

                usbd_in_transaction(pudev, EP_ID);
            }
        } else {
            /* handle the out direction transaction */

            int count = 0;

            pEP = &(pudev->out_ep[EP_ID]);

            if (0 != (ep_value & EPxCS_RX_ST)) {
                /* clear successful receive interrupt flag */
                USBD_ENDP_RX_STAT_CLEAR(EP_ID);

                count = (pbuf_reg + EP_ID)->rx_count & EPRCNT_CNT;

                if (0 != count) {
                    if (0 != (ep_value & EPxCS_SETUP)) {
                        /* handle setup packet */
                        usbd_ep_data_read(&(pudev->setup_packet[0]), \
                                           pbuf_reg->rx_addr, count);

                        usbd_setup_transaction(pudev);

                        return USBD_OK;
                    } else {
                        usbd_ep_data_read(pEP->trs_buf, (pbuf_reg + EP_ID)->rx_addr, count);
                    }
                }

                /* maybe mutiple packets */
                pEP->trs_count += count;
                pEP->trs_buf += count;
                pEP->trs_len -= count;

                if (0 == pEP->trs_len || count < pEP->maxpacket) {
                    usbd_out_transaction(pudev, EP_ID);
                    pEP->trs_count = 0;
                } else {
                    usbd_ep_rx(pudev, EP_ID, pEP->trs_buf, pEP->trs_len);
                }
            }
        }   
    }

    return USBD_OK;
}

/*!
    \brief      handle USB SOF event
    \param[in]  pudev: pointer to USB device instance
    \param[out] none
    \retval     USB device operation status
*/
static uint8_t  usbd_intf_sof (usbd_core_handle_struct *pudev)
{
    /* if necessary, user can add code here */

    return USBD_OK;
}

/*!
    \brief      handle USB expect SOF event
    \param[in]  pudev: pointer to USB device instance
    \param[out] none
    \retval     USB device operation status
*/
static uint8_t  usbd_intf_esof (usbd_core_handle_struct *pudev)
{
    /* control resume time by ESOFs */
    if(g_ESOF_count > 0)
    {
        g_ESOF_count--;

        if (0 == g_ESOF_count) {
            if (1 == g_remote_wakeup_on) {
                USBD_REG_SET(USBD_CTL, USBD_REG_GET(USBD_CTL) & ~CTL_RSREQ);
                g_remote_wakeup_on = 0;
            } else {
                USBD_REG_SET(USBD_CTL, USBD_REG_GET(USBD_CTL) | CTL_RSREQ);
                g_ESOF_count = 3;
                g_remote_wakeup_on = 1;
            }
        }
    }

    return USBD_OK;
}

/*!
    \brief      handle USB reset event
    \param[in]  pudev: pointer to USB device instance
    \param[out] none
    \retval     USB device operation status
*/
static uint8_t  usbd_intf_reset (usbd_core_handle_struct *pudev)
{
#ifdef LPM_ENABLED
    uint16_t sLPM;
#endif /* LPM_ENABLED */

    /* configure endpoint 0 buffer */
    pbuf_reg->tx_addr = g_free_buf_addr;
    g_free_buf_addr += USBD_EP0_MAX_SIZE;

    pbuf_reg->rx_addr = g_free_buf_addr;
    g_free_buf_addr += USBD_EP0_MAX_SIZE;

    /* configure endpoint 0 Rx count */
    pbuf_reg->rx_count = ((USBD_EP0_MAX_SIZE << 5) - 1) | 0x8000;

    pudev->in_ep[EP0].maxpacket = USBD_EP0_MAX_SIZE;
    pudev->out_ep[EP0].maxpacket = USBD_EP0_MAX_SIZE;

    USBD_REG_SET(USBD_EPxCS(0), EP_CONTROL | EPRX_VALID | EPTX_NAK);

    /* set device address as default address 0 */
    USBD_REG_SET(USBD_DADDR, DADDR_USBEN);

#ifdef LPM_ENABLED
    sLPM = SUBEP_VALID;
    USBD_REG_SET(USBD_LPMCTL, 0x8000);
    USBD_REG_SET(USBD_SEP0, sLPM);
#endif /* LPM_ENABLED */

    pudev->status = USBD_DEFAULT;

    return USBD_OK;
}

/*!
    \brief      handle USB suspend event
    \param[in]  pudev: pointer to USB device instance
    \param[out] none
    \retval     USB device operation status
*/
static uint8_t  usbd_intf_suspend (usbd_core_handle_struct *pudev)
{
    /* store the device current status */
    pudev->prev_status = pudev->status;

    /* set device in suspended state */
    pudev->status = USBD_SUSPENDED;

    /* enter USB in suspend and mcu system in low power mode */
    if (g_suspend_enabled) {
        usbd_suspend();
    } else {
        /* if not possible then resume after xx ms */
        g_ESOF_count = 3;
    }

    return USBD_OK;
}

/*!
    \brief      handle USB wakeup event
    \param[in]  pudev: pointer to USB device instance
    \param[out] none
    \retval     USB device operation status
*/
static uint8_t  usbd_intf_wakeup (usbd_core_handle_struct *pudev)
{
    /* restore the old status */
    pudev->status = pudev->prev_status;

    if (0 == g_remote_wakeup_on) {
        resume_mcu();
    }

    return USBD_OK;
}
