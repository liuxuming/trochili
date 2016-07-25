/*!
    \file  usbd_core.c
    \brief USB device driver
*/

/*
    Copyright (C) 2016 GigaDevice

    2014-09-06, V1.0.0, firmware for GD32F150
    2016-01-15, V1.0.1, firmware for GD32F150
    2016-04-30, V3.0.0, firmware update for GD32F150
*/

#include "usbd_core.h"
#include "usbd_std.h"

uint32_t g_interrupt_mask = 0;
uint32_t g_free_buf_addr = ENDP_BUF_ADDR;

usbd_ep_buf_struct *pbuf_reg = (usbd_ep_buf_struct *)USBD_RAM;

/*!
    \brief      device core register initialization
    \param[in]  none
    \param[out] none
    \retval     none
*/
void  usbd_core_init (usbd_core_handle_struct *pudev)
{
    /* disable remote wakeup feature */
    pudev->remote_wakeup = 0;

    /* just reset the CLOSE bit */
    USBD_REG_SET(USBD_CTL, CTL_SETRST);

    /* may be need wait some time(tSTARTUP) ... */

    /* clear SETRST bit in USBD_CTL register */
    USBD_REG_SET(USBD_CTL, 0);

    /* clear all pending interrupts */
    USBD_REG_SET(USBD_INTF, 0);

    /* set allocation buffer address */
    USBD_REG_SET(USBD_BADDR, BUFFER_ADDRESS & 0xFFF8);

    g_interrupt_mask = IER_MASK;
  
#ifdef LPM_ENABLED
    /* enable L1REQ interrupt */
    USBD_REG_SET(USBD_LPMCTL, LPMCTL_STIE);
#endif /* LPM_ENABLED */

    /* enable all interrupts mask bits */
    USBD_REG_SET(USBD_CTL, g_interrupt_mask);
}

/*!
    \brief      free buffer used from application by toggling the SW_BUF byte
    \param[in]  ep_id: endpoint identifier (0..7)
    \param[in]  dir: endpoint direction which can be OUT(0) or IN(1)
    \param[out] none
    \retval     None
*/
void  user_buffer_free (uint8_t ep_id, uint8_t dir)
{
    if (DBUF_EP_OUT == dir) {
        USBD_SWBUF_RX_TOGGLE(ep_id);
    } else if (DBUF_EP_IN == dir) {
        USBD_SWBUF_TX_TOGGLE(ep_id);
    }
}

/*!
    \brief      device core register configure when stop device
    \param[in]  none
    \param[out] none
    \retval     none
*/
void  usbd_core_deinit (void)
{
    /* disable all interrupts and set USB reset */
    USBD_REG_SET(USBD_CTL, CTL_SETRST);

    /* clear all interrupt flags */
    USBD_REG_SET(USBD_INTF, 0);

    /* close device */
    USBD_REG_SET(USBD_CTL, CTL_SETRST | CTL_CLOSE);
}

/*!
    \brief      endpoint initialization
    \param[in]  pudev: pointer to USB core instance
    \param[in]  pep_desc: pointer to endpoint descriptor
    \param[out] none
    \retval     none
*/
void  usbd_ep_init (usbd_core_handle_struct *pudev, const void *pep_desc)
{
    usb_ep_struct *pEP;
    uint8_t EP_ID = ((usb_descriptor_endpoint_struct *)pep_desc)->bEndpointAddress & 0x0F;
    uint32_t reg_value = ((usb_descriptor_endpoint_struct *)pep_desc)->wMaxPacketSize;

    if (((usb_descriptor_endpoint_struct *)pep_desc)->bEndpointAddress >> 7) {
        pEP = &pudev->in_ep[EP_ID];

        pEP->maxpacket = reg_value;

        /* set the endpoint transmit buffer address */
        (pbuf_reg + EP_ID)->tx_addr = g_free_buf_addr;

        reg_value = (reg_value + 1) & ~1;

        /* configure the endpoint status as NAK status */
        USBD_ENDP_TX_STATUS_SET(EP_ID, EPTX_NAK);
    } else {
        pEP = &pudev->out_ep[EP_ID];

        pEP->maxpacket = reg_value;

        /* set the endpoint receive buffer address */
        (pbuf_reg + EP_ID)->rx_addr = g_free_buf_addr;

        if (reg_value > 62) {
            reg_value = (reg_value + 31) & ~31;
            (pbuf_reg + EP_ID)->rx_count = ((reg_value << 5) - 1) | 0x8000;
        } else {
            reg_value = (reg_value + 1) & ~1;
            (pbuf_reg + EP_ID)->rx_count = reg_value << 9;
        }

        /* configure the endpoint status as NAK status */
        USBD_ENDP_RX_STATUS_SET(EP_ID, EPRX_NAK);
    }

    g_free_buf_addr += reg_value;

    /* set the endpoint type */
    switch (((usb_descriptor_endpoint_struct *)pep_desc)->bmAttributes & USB_EPTYPE_MASK) {
    case ENDP_CONTROL:
        reg_value = EP_CONTROL;
        break;
    case ENDP_BULK:
        reg_value = EP_BULK;
        break;
    case ENDP_INT:
        reg_value = EP_INTERRUPT;
        break;
    case ENDP_ISOC:
        reg_value = EP_ISO;
        break;
    default:
        break;
    }

    USBD_REG_SET(USBD_EPxCS(EP_ID), reg_value | EP_ID);
}

/*!
    \brief      configure the endpoint when it is disabled
    \param[in]  pudev: pointer to USB core instance
    \param[in]  ep_addr: endpoint address
                  in this parameter:
                    bit0..bit6: endpoint number (0..7)
                    bit7: endpoint direction which can be IN(1) or OUT(0)
    \param[out] none
    \retval     none
*/
void  usbd_ep_deinit (usbd_core_handle_struct *pudev, uint8_t ep_addr)
{

    uint8_t EP_ID = ep_addr & 0x7F;

    if (ep_addr >> 7) {
        USBD_DTG_TX_CLEAR(EP_ID);

        /* configure the endpoint status as DISABLED */
        USBD_ENDP_TX_STATUS_SET(EP_ID, EPTX_DISABLED); 
    } else {
        USBD_DTG_RX_CLEAR(EP_ID);

        /* configure the endpoint status as DISABLED */
        USBD_ENDP_RX_STATUS_SET(EP_ID, EPRX_DISABLED);
    }
}

/*!
    \brief      endpoint prepare to receive data
    \param[in]  pudev: pointer to usb core instance
    \param[in]  ep_addr: endpoint address
                  in this parameter:
                    bit0..bit6: endpoint number (0..7)
                    bit7: endpoint direction which can be IN(1) or OUT(0)
    \param[in]  pbuf: user buffer address pointer
    \param[in]  buf_len: buffer length
    \param[out] none
    \retval     none
*/
void  usbd_ep_rx (usbd_core_handle_struct *pudev, uint8_t ep_addr, uint8_t *pbuf, uint16_t buf_len)
{
    usb_ep_struct *pEP;
    uint8_t EP_ID = ep_addr & 0x7F;

    pEP = &pudev->out_ep[EP_ID];

    /* configure the transaction level parameters */
    pEP->trs_buf = pbuf;
    pEP->trs_len = buf_len;

    /* enable endpoint to receive */
    USBD_ENDP_RX_STATUS_SET(EP_ID, EPRX_VALID);
}

/*!
    \brief      endpoint prepare to transmit data
    \param[in]  pudev: pointer to USB core instance
    \param[in]  ep_addr: endpoint address
                  in this parameter:
                    bit0..bit6: endpoint number (0..7)
                    bit7: endpoint direction which can be IN(1) or OUT(0)
    \param[in]  pbuf: transmit buffer address pointer
    \param[in]  buf_len: buffer length
    \param[out] none
    \retval     none
*/
void  usbd_ep_tx (usbd_core_handle_struct *pudev, uint8_t ep_addr, uint8_t *pbuf, uint16_t buf_len)
{
    usb_ep_struct *pEP;
    __IO uint32_t len = 0;
    uint8_t EP_ID = ep_addr & 0x7F;

    pEP = &pudev->in_ep[EP_ID];

    /* configure the transaction level parameters */
    pEP->trs_buf = pbuf;
    pEP->trs_len = buf_len;
    pEP->trs_count = 0;

    /* transmit length is more than one packet */
    if (pEP->trs_len > pEP->maxpacket) {
        len = pEP->maxpacket;
    } else {
        len = pEP->trs_len;
    }

    usbd_ep_data_write(pEP->trs_buf, (pbuf_reg + EP_ID)->tx_addr, len);
    (pbuf_reg + EP_ID)->tx_count = len;

    /* enable endpoint to transmit */
    USBD_ENDP_TX_STATUS_SET(EP_ID, EPTX_VALID);
}

/*!
    \brief      set an endpoint to STALL status
    \param[in]  pudev: pointer to usb core instance
    \param[in]  ep_addr: endpoint address
                  in this parameter:
                    bit0..bit6: endpoint number (0..7)
                    bit7: endpoint direction which can be IN(1) or OUT(0)
    \param[out] none
    \retval     none
*/
void  usbd_ep_stall (usbd_core_handle_struct *pudev, uint8_t ep_addr)
{
    uint8_t EP_ID = ep_addr & 0x7F;

    if (ep_addr >> 7) {
        USBD_ENDP_TX_STATUS_SET(EP_ID, EPTX_STALL);
    } else {
        USBD_ENDP_RX_STATUS_SET(EP_ID, EPRX_STALL);
    }

    if (0 == EP_ID) {
        /* control endpoint need to be stalled in two directions */
        USBD_ENDP_RX_TX_STATUS_SET(EP_ID, EPRX_STALL, EPTX_STALL); 
    }
}

/*!
    \brief      clear endpoint stalled status
    \param[in]  pudev: pointer to usb core instance
    \param[in]  ep_addr: endpoint address
                  in this parameter:
                    bit0..bit6: endpoint number (0..7)
                    bit7: endpoint direction which can be IN(1) or OUT(0)
    \param[out] none
    \retval     none
*/
void  usbd_ep_clear_stall (usbd_core_handle_struct *pudev, uint8_t ep_addr)
{
    uint8_t EP_ID = ep_addr & 0x7F;

    if (ep_addr >> 7) {
        USBD_DTG_TX_CLEAR(EP_ID);
        USBD_ENDP_TX_STATUS_SET(EP_ID, EPTX_VALID);
    } else {
        USBD_DTG_RX_CLEAR(EP_ID);
        USBD_ENDP_RX_STATUS_SET(EP_ID, EPRX_VALID);
    }
}

/*!
    \brief      get the endpoint status
    \param[in]  pudev: pointer to usb core instance
    \param[in]  ep_addr: endpoint address
                  in this parameter:
                    bit0..bit6: endpoint number (0..7)
                    bit7: endpoint direction which can be IN(1) or OUT(0)
    \param[out] none
    \retval     endpoint status
*/
uint8_t  usbd_ep_status_get (usbd_core_handle_struct *pudev, uint8_t ep_addr)
{
    if (ep_addr >> 7) {
        return USBD_ENDP_TX_STATUS_GET((ep_addr & 0x7F));
    } else {
        return USBD_ENDP_RX_STATUS_GET(ep_addr);
    }
}

/*!
    \brief      write datas from user fifo to USBRAM
    \param[in]  user_fifo: pointer to user fifo
    \param[in]  usbram_addr: the allocation buffer address of the endpoint
    \param[in]  bytes: the bytes count of the write datas
    \param[out] none
    \retval     none
*/
void usbd_ep_data_write(uint8_t *user_fifo, uint16_t usbram_addr, uint16_t bytes)
{
    uint32_t n;
    uint32_t *write_addr = (uint32_t *)(usbram_addr * 2 + USBD_RAM);

    for (n = 0; n < (bytes + 1) / 2; n++) {
        *write_addr++ = *((__packed uint16_t*)user_fifo);
        user_fifo += 2;
    }
}

/*!
    \brief      read datas from USBRAM to user fifo
    \param[in]  user_fifo: pointer to user fifo
    \param[in]  usbram_addr: the allocation buffer address of the endpoint
    \param[in]  bytes: the bytes count of the read datas
    \param[out] none
    \retval     none
*/
void usbd_ep_data_read(uint8_t *user_fifo, uint16_t usbram_addr, uint16_t bytes)
{
    uint32_t n;
    uint32_t *read_addr = (uint32_t *)(usbram_addr * 2 + USBD_RAM);

    for (n = 0; n < (bytes + 1) / 2; n++) {
        *((__packed uint16_t*)user_fifo) = *read_addr++;
        user_fifo += 2;
    }
}

/*!
    \brief      get the received data length
    \param[in]  pudev: pointer to USB core instance
    \param[in]  ep_id: endpoint identifier which is in (0..7)
    \param[out] none
    \retval     received data length
*/
uint16_t  usbd_rx_count_get (usbd_core_handle_struct *pudev, uint8_t ep_id)
{
    return pudev->out_ep[ep_id].trs_count;
}
