/*!
    \file  usbd_std.c
    \brief USB device standard routines
    \note  about USB standard, please refer to the USB2.0 protocol
*/

/*
    Copyright (C) 2016 GigaDevice

    2014-09-06, V1.0.0, firmware for GD32F150
    2016-01-15, V1.0.1, firmware for GD32F150
    2016-04-30, V3.0.0, firmware update for GD32F150
*/

#include "usbd_std.h"

uint8_t g_device_address = 0;

/* USB enumeration handle functions */
static void usbd_getdescriptor     (usbd_core_handle_struct *pudev, usb_device_req_struct *req);
static void usbd_setaddress        (usbd_core_handle_struct *pudev, usb_device_req_struct *req);
static void usbd_setconfiguration  (usbd_core_handle_struct *pudev, usb_device_req_struct *req);
static void usbd_getconfiguration  (usbd_core_handle_struct *pudev, usb_device_req_struct *req);
static void usbd_getstatus         (usbd_core_handle_struct *pudev, usb_device_req_struct *req);
static void usbd_setfeature        (usbd_core_handle_struct *pudev, usb_device_req_struct *req);
static void usbd_clearfeature      (usbd_core_handle_struct *pudev, usb_device_req_struct *req);
static void usbd_reserved          (usbd_core_handle_struct *pudev, usb_device_req_struct *req);
static void usbd_setdescriptor     (usbd_core_handle_struct *pudev, usb_device_req_struct *req);
static void usbd_getinterface      (usbd_core_handle_struct *pudev, usb_device_req_struct *req);
static void usbd_setinterface      (usbd_core_handle_struct *pudev, usb_device_req_struct *req);
static void usbd_synchframe        (usbd_core_handle_struct *pudev, usb_device_req_struct *req);

static uint8_t* usbd_device_descriptor_get (usbd_core_handle_struct *pudev, uint8_t index, uint16_t *pLen);
static uint8_t* usbd_configuration_descriptor_get (usbd_core_handle_struct *pudev, uint8_t index, uint16_t *pLen);
static uint8_t* usbd_string_descriptor_get (usbd_core_handle_struct *pudev, uint8_t index, uint16_t *pLen);

/* standard device request handler */
static void (*standard_device_request[])(usbd_core_handle_struct *pudev, usb_device_req_struct *req) =
{
    usbd_getstatus,
    usbd_clearfeature,
    usbd_reserved,
    usbd_setfeature,
    usbd_reserved,
    usbd_setaddress,
    usbd_getdescriptor,
    usbd_setdescriptor,
    usbd_getconfiguration,
    usbd_setconfiguration,
    usbd_getinterface,
    usbd_setinterface,
    usbd_synchframe,
};

/* get standard descriptor handler */
static uint8_t* (*standard_descriptor_get[])(usbd_core_handle_struct *pudev, uint8_t index, uint16_t *pLen) = 
{
    usbd_device_descriptor_get,
    usbd_configuration_descriptor_get,
    usbd_string_descriptor_get
};

/*!
    \brief      USB setup stage processing
    \param[in]  pudev: pointer to USB device instance
    \param[out] none
    \retval     USB device operation status
*/
uint8_t  usbd_setup_transaction (usbd_core_handle_struct *pudev)
{
    usb_device_req_struct req;

    usbd_setup_request_parse (pudev, &req);

    switch (req.bmRequestType & USB_REQ_MASK) {
    /* standard device request */
    case USB_STANDARD_REQ:
        usbd_standard_request(pudev, &req);
        break;
    /* device class request */
    case USB_CLASS_REQ:
        usbd_device_class_request(pudev, &req);
        break;
    /* vendor defined request */
    case USB_VENDOR_REQ:
        usbd_vendor_request(pudev, &req);
        break;
    default:
        usbd_ep_stall(pudev, 0x00);
        break;
    }
    return USBD_OK;
}

/*!
    \brief      data out stage processing
    \param[in]  pudev: pointer to USB device instance
    \param[in]  ep_id: endpoint identifier(0..7)
    \param[out] none
    \retval     USB device operation status
*/
uint8_t  usbd_out_transaction (usbd_core_handle_struct *pudev, uint8_t ep_id)
{
    usb_ep_struct *ep = &pudev->out_ep[ep_id];

    if (0 == ep_id) {
        if (0 != pudev->ctl_count) {
            if (ep->trs_len > ep->maxpacket) {
                /* one data packet has been received, update trs_len */
                ep->trs_len -= ep->maxpacket;

                /* continue to receive remain data */
                usbd_ep_rx(pudev, EP0_OUT, ep->trs_buf, ep->trs_len);
            } else {
                if (USBD_CONFIGURED == pudev->status) {
                    /* device class handle */
                    pudev->class_data_handler(pudev, USBD_RX, EP0);
                }

                /* enter the control transaction status stage */
                USBD_CONTRL_STATUS_TX();

                pudev->ctl_count = 0;
            }
        } else {
            /* clear endpoint status_out status */
            USBD_STATUS_OUT_CLEAR(EP0);
        }
    } else {
        if (USBD_CONFIGURED == pudev->status) {
            pudev->class_data_handler(pudev, USBD_RX, ep_id);
        }
    }
    return USBD_OK;
}

/*!
    \brief      data in stage processing
    \param[in]  pudev: pointer to USB device instance
    \param[in]  ep_id: endpoint identifier(0..7)
    \param[out] none
    \retval     USB device operation status
*/
uint8_t  usbd_in_transaction (usbd_core_handle_struct *pudev, uint8_t ep_id)
{
    usb_ep_struct *ep = &pudev->in_ep[ep_id];

    if (0 == ep_id) {
        if (0 != pudev->ctl_count) {
            if (ep->trs_len > ep->maxpacket) {
                /* one data packet has been transmited, update trs_len */
                ep->trs_len -= ep->maxpacket;

                /* continue to transmit remain data */
                usbd_ep_tx(pudev, EP0_IN, ep->trs_buf, ep->trs_len);
            } else {
#ifndef USB_DFU
                /* transmit length is maxpacket multiple, so send zero length packet */
                if ((0 == (ep->trs_len % ep->maxpacket)) && (ep->trs_len < pudev->ctl_count)) {
                    usbd_ep_tx(pudev, EP0_IN, NULL, 0);

                    pudev->ctl_count = 0;
                } else
#endif /* USB_DFU */
                {
                    ep->trs_len = 0;

                    if (USBD_CONFIGURED == pudev->status) {
                        pudev->class_data_handler(pudev, USBD_TX, EP0); 
                    }

                    USBD_CONTRL_STATUS_RX();

                    pudev->ctl_count = 0;
                }
            }
        } else {
            if (0 != g_device_address) {
                USBD_REG_SET(USBD_DADDR, DADDR_USBEN | g_device_address);
                g_device_address = 0;
            }
        }
    } else {
        ep->trs_len -= ep->trs_count;

        if (0 == ep->trs_len) {
            if (USBD_CONFIGURED == pudev->status) {
                pudev->class_data_handler(pudev, USBD_TX, ep_id);
            }
        } else {
            usbd_ep_tx(pudev, ep_id, ep->trs_buf, ep->trs_len);
        }
    }

    return USBD_OK;
}

/*!
    \brief      handle USB standard device request
    \param[in]  pudev: pointer to USB device instance
    \param[in]  req: pointer to USB device request
    \param[out] none
    \retval     USB device operation status
*/
uint8_t  usbd_standard_request (usbd_core_handle_struct *pudev, usb_device_req_struct *req)
{
    /* call device request handle function */
    (*standard_device_request[req->bRequest])(pudev, req);

    return USBD_OK;
}

/*!
    \brief      handle USB device class request
    \param[in]  pudev: pointer to USB device instance
    \param[in]  req: pointer to USB device class request
    \param[out] none
    \retval     USB device operation status
*/
uint8_t  usbd_device_class_request (usbd_core_handle_struct *pudev, usb_device_req_struct *req)
{
    usbd_status_enum ret;

    switch (pudev->status) {
    case USBD_CONFIGURED:
        if (LOWBYTE(req->wIndex) <= USBD_ITF_MAX_NUM) {
            /* call device class handle function */
            ret = (usbd_status_enum)(pudev->class_req_handler(pudev, req));

            if ((0 == req->wLength) && (USBD_OK == ret)) {
                /* no data stage */
                USBD_CONTRL_STATUS_TX();
            }
        } else {
            usbd_enum_error(pudev, req);
        }
        break;
    default:
        usbd_enum_error(pudev, req);
        break;
    }

    return USBD_OK;
}

/*!
    \brief      handle USB vendor request
    \param[in]  pudev: pointer to USB device instance
    \param[in]  req: pointer to USB vendor request
    \param[out] none
    \retval     USB device operation status
*/
uint8_t  usbd_vendor_request (usbd_core_handle_struct *pudev, usb_device_req_struct *req)
{
    /* added by user */

    return USBD_OK;
}

/*!
    \brief      no operation, just for reserved
    \param[in]  pudev: pointer to USB device instance
    \param[in]  req: pointer to USB device request
    \param[out] none
    \retval     none
*/
static void  usbd_reserved (usbd_core_handle_struct *pudev, usb_device_req_struct *req)
{
    /* no operation */
}

/*!
    \brief      get the device descriptor
    \brief[in]  index: no use
    \param[in]  none
    \param[out] pLen: data length pointer
    \retval     descriptor buffer pointer
*/
static uint8_t* usbd_device_descriptor_get (usbd_core_handle_struct *pudev, uint8_t index, uint16_t *pLen)
{
    *pLen = pudev->dev_desc[0];

    return pudev->dev_desc;
}

/*!
    \brief      get the configuration descriptor
    \brief[in]  index: no use
    \param[in]  none
    \param[out] pLen: data length pointer
    \retval     descriptor buffer pointer
*/
static uint8_t* usbd_configuration_descriptor_get (usbd_core_handle_struct *pudev, uint8_t index, uint16_t *pLen)
{
    *pLen = pudev->config_desc[2];

    return pudev->config_desc;
}

/*!
    \brief      get string descriptor
    \param[in]  index: string descriptor index
    \param[in]  pLen: pointer to string length
    \param[out] none
    \retval     none
*/
static uint8_t* usbd_string_descriptor_get (usbd_core_handle_struct *pudev, uint8_t index, uint16_t *pLen)
{
    uint8_t *desc = pudev->strings[index];

    *pLen = desc[0];

    return desc;
}

/*!
    \brief      handle Get_Status request
    \param[in]  pudev: pointer to USB device instance
    \param[in]  req: pointer to USB device request
    \param[out] none
    \retval     none
*/
static void  usbd_getstatus (usbd_core_handle_struct *pudev, usb_device_req_struct *req)
{
    /* no need */
}

/*!
    \brief      handle USB Clear_Feature request
    \param[in]  pudev: pointer to USB device instance
    \param[in]  req: USB device request
    \param[out] none
    \retval     none
*/
static void  usbd_clearfeature (usbd_core_handle_struct *pudev, usb_device_req_struct *req)
{
    uint8_t ep_addr = 0;

    switch (req->bmRequestType & USB_REQ_RECIPIENT_MASK) {
    case USB_REQTYPE_DEVICE:
        switch (pudev->status) {
        case USBD_ADDRESSED:
        case USBD_CONFIGURED:
            /* clear device remote wakeup feature */
            if (USB_FEATURE_REMOTE_WAKEUP == req->wValue) {
                pudev->remote_wakeup = 0;
                pudev->class_req_handler(pudev, req);
                USBD_CONTRL_STATUS_TX();
            } else if (USB_FEATURE_TEST_MODE == req->wValue) {
                /* can not clear test_mode feature */
                usbd_enum_error(pudev, req);
            }
            break;
        default:
            break;
        }
        break;
    case USB_REQTYPE_INTERFACE:
        switch (pudev->status) {
        case USBD_ADDRESSED:
            usbd_enum_error (pudev, req);
            break;
        case USBD_CONFIGURED:
            if (LOWBYTE(req->wIndex) <= USBD_ITF_MAX_NUM) {
                /* no operation */
            } else {
                usbd_enum_error(pudev, req);
            }
            break;
        default:
            break;
        }
        break;
    case USB_REQTYPE_ENDPOINT:
       ep_addr = LOWBYTE(req->wIndex);
       switch (pudev->status) {
       case USBD_ADDRESSED:
           if (IS_NOT_EP0(ep_addr)) {
               usbd_enum_error(pudev, req);
           }
           break;
       case USBD_CONFIGURED:
           /* clear endpoint halt feature */
           if (USB_FEATURE_ENDP_HALT == req->wValue) {
               if (IS_NOT_EP0(ep_addr)) {
                   usbd_ep_clear_stall(pudev, ep_addr);
               }
           }
           pudev->class_req_handler(pudev, req);
           USBD_CONTRL_STATUS_TX();
           break;
       default:
           break;
       }
       break;
    default:
        usbd_enum_error(pudev, req);
        break;
    }
}

/*!
    \brief      handle USB Set_Feature request
    \param[in]  pudev: pointer to USB device instance
    \param[in]  req: pointer to USB device request
    \param[out] none
    \retval     none
*/
static void  usbd_setfeature (usbd_core_handle_struct *pudev, usb_device_req_struct *req)
{
    uint8_t ep_addr = 0;

    switch (req->bmRequestType & USB_REQ_RECIPIENT_MASK) {
    case USB_REQTYPE_DEVICE:
        switch (pudev->status) {
        case USBD_ADDRESSED:
        case USBD_CONFIGURED:
            /* set device remote wakeup feature */
            if (USB_FEATURE_REMOTE_WAKEUP == req->wValue) {
                pudev->remote_wakeup = 1;
                USBD_CONTRL_STATUS_TX();
            }
            break;
        default:
            break;
        }
        break;
    case USB_REQTYPE_INTERFACE:
        switch (pudev->status) {
        case USBD_ADDRESSED:
            usbd_enum_error(pudev, req);
            break;
        case USBD_CONFIGURED:
            if (LOWBYTE(req->wIndex) <= USBD_ITF_MAX_NUM) {
                /* no operation */
            } else {
                usbd_enum_error(pudev, req);
            }
            break;
        default:
            break;
        }
        break;
    case USB_REQTYPE_ENDPOINT:
        switch (pudev->status) {
        case USBD_ADDRESSED:
            if (IS_NOT_EP0(ep_addr)) {
                usbd_enum_error(pudev, req);
            }
            break;
        case USBD_CONFIGURED:
            /* set endpoint halt feature */
            if (USB_FEATURE_ENDP_HALT == req->wValue) {
                if (IS_NOT_EP0(ep_addr)) {
                    usbd_ep_stall(pudev, ep_addr);
                }
            }
            USBD_CONTRL_STATUS_TX();
            break;
        default:
            break;
        }
        break;
    default:
        usbd_enum_error(pudev, req);
        break;
    }
}

/*!
    \brief      handle USB Set_Address request
    \param[in]  pudev: pointer to USB device instance
    \param[in]  req: pointer to USB device request
    \param[out] none
    \retval     none
*/
static void  usbd_setaddress (usbd_core_handle_struct *pudev, usb_device_req_struct *req)
{
    if ((0 == req->wIndex) && (0 == req->wLength)) {
        g_device_address = (uint8_t)(req->wValue) & 0x7F;

        if (USBD_CONFIGURED == pudev->status) {
            usbd_enum_error(pudev, req);
        } else {
            USBD_CONTRL_STATUS_TX();

            if (0 != g_device_address) {
                pudev->status  = USBD_ADDRESSED;
            } else {
                pudev->status  = USBD_DEFAULT;
            }
        }
    } else {
        usbd_enum_error(pudev, req);
    }
}

/*!
    \brief      handle USB Get_Descriptor request
    \param[in]  pudev: pointer to USB device instance
    \param[in]  req: pointer to USB device request
    \param[out] none
    \retval     none
*/
static void  usbd_getdescriptor (usbd_core_handle_struct *pudev, usb_device_req_struct *req)
{
    if (USB_REQTYPE_DEVICE == (req->bmRequestType & USB_REQ_RECIPIENT_MASK)) {
        uint8_t desc_index = req->wValue >> 8;
        if (desc_index <= 0x03) {
            uint16_t len = 0;
            uint8_t *pbuf = NULL;

            /* call corresponding descriptor get function */
            pbuf = standard_descriptor_get[desc_index - 1](pudev, (uint8_t)(req->wValue) & 0xFF, &len);

            if ((0 != len) && (0 != req->wLength)) {
                len = MIN(len, req->wLength);

                usbd_ep_tx (pudev, EP0_IN, pbuf, len);
            }
        } else {
            usbd_enum_error(pudev, req);
        }
    } else if (USB_REQTYPE_INTERFACE == (req->bmRequestType & USB_REQ_RECIPIENT_MASK)) {
        if (NULL != pudev->class_data_handler) {
            /* get device class special descriptor */
            pudev->class_req_handler(pudev, req);
        }
    }
}

/*!
    \brief      handle USB Set_Descriptor request
    \param[in]  pudev: pointer to USB device instance
    \param[in]  req: pointer to USB device request
    \param[out] none
    \retval     none
*/
static void  usbd_setdescriptor (usbd_core_handle_struct *pudev, usb_device_req_struct *req)
{
    /* no handle */
}

/*!
    \brief      handle USB Get_Configuration request
    \param[in]  pudev: pointer to USB device instance
    \param[in]  req: pointer to USB device request
    \param[out] none
    \retval     none
*/
static void  usbd_getconfiguration (usbd_core_handle_struct *pudev, usb_device_req_struct *req)
{
    /* no handle */
}

/*!
    \brief      handle USB Set_Configuration request
    \param[in]  pudev: pointer to USB device instance
    \param[in]  req: pointer to USB device request
    \param[out] none
    \retval     none
*/
static void  usbd_setconfiguration (usbd_core_handle_struct *pudev, usb_device_req_struct *req)
{
    static uint8_t  cfgidx;

    cfgidx = (uint8_t)(req->wValue);

    if (cfgidx > USBD_CFG_MAX_NUM) {
        usbd_enum_error(pudev, req);
    } else {
        switch (pudev->status) {
        case USBD_ADDRESSED:
            if (cfgidx){
                pudev->config_num = cfgidx;
                pudev->status = USBD_CONFIGURED;
                pudev->class_init(pudev, cfgidx);
                USBD_CONTRL_STATUS_TX();
            } else {
                USBD_CONTRL_STATUS_TX();
            }
            break;
        case USBD_CONFIGURED:
            if (0 == cfgidx) {
                pudev->status = USBD_ADDRESSED;
                pudev->config_num = cfgidx;
                pudev->class_deinit(pudev, cfgidx);
                USBD_CONTRL_STATUS_TX();
            } else if (cfgidx != pudev->config_num) {
                /* clear old configuration */
                pudev->class_deinit(pudev, pudev->config_num);

                /* set new configuration */
                pudev->config_num = cfgidx;
                pudev->class_init(pudev, cfgidx);
                USBD_CONTRL_STATUS_TX();
            } else {
                USBD_CONTRL_STATUS_TX();
            }
            break;
        default:
            break;
        }
    }
}

/*!
    \brief      handle USB Get_Interface request
    \param[in]  pudev: pointer to USB device instance
    \param[in]  req: pointer to USB device request
    \param[out] none
    \retval     none
*/
static void  usbd_getinterface (usbd_core_handle_struct *pudev, usb_device_req_struct *req)
{
    if (NULL != pudev->class_req_handler) {
        pudev->class_req_handler(pudev, req);
    }
}

/*!
    \brief      handle USB Set_Interface request
    \param[in]  pudev: pointer to USB device instance
    \param[in]  req: pointer to USB device request
    \param[out] none
    \retval     none
*/
static void  usbd_setinterface (usbd_core_handle_struct *pudev, usb_device_req_struct *req)
{
    if (NULL != pudev->class_req_handler) {
        pudev->class_req_handler(pudev, req);
    }
}

/*!
    \brief      handle USB SynchFrame request
    \param[in]  pudev: pointer to USB device instance
    \param[in]  req: pointer to USB device request
    \param[out] none
    \retval     none
*/
static void  usbd_synchframe (usbd_core_handle_struct *pudev, usb_device_req_struct *req)
{
    /* no handle */
}

/*!
    \brief      decode setup data packet
    \param[in]  pudev: pointer to USB device instance
    \param[in]  req: pointer to USB device request
    \param[out] none
    \retval     none
*/
void  usbd_setup_request_parse (usbd_core_handle_struct *pudev, usb_device_req_struct *req)
{
    req->bmRequestType = *(uint8_t *)(pudev->setup_packet);
    req->bRequest = *(uint8_t *)(pudev->setup_packet + 1);
    req->wValue = SWAPBYTE(pudev->setup_packet + 2);
    req->wIndex = SWAPBYTE(pudev->setup_packet + 4);
    req->wLength = SWAPBYTE(pudev->setup_packet + 6);

    pudev->ctl_count = req->wLength;
}

/*!
    \brief      handle USB enumeration error event
    \param[in]  pudev: pointer to USB device instance
    \param[in]  req: pointer to USB device request
    \param[out] none
    \retval     none
*/
void  usbd_enum_error (usbd_core_handle_struct *pudev, usb_device_req_struct *req)
{
    usbd_ep_stall(pudev, EP0);
}
