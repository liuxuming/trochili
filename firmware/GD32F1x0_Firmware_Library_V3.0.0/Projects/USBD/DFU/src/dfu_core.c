/*!
    \file  dfu_core.c
    \brief USB DFU device class core functions
*/

/*
    Copyright (C) 2016 GigaDevice

    2014-09-06, V1.0.0, firmware for GD32F150
    2016-01-15, V1.0.1, firmware for GD32F150
    2016-04-30, V3.0.0, firmware update for GD32F150
*/

#include "dfu_core.h"
#include "systick.h"

#define USBD_VID                     0x28e9
#define USBD_PID                     0x0189

/* DFU requests management functions */
static void dfu_detach     (void *pudev, usb_device_req_struct *req);
static void dfu_dnload     (void *pudev, usb_device_req_struct *req);
static void dfu_upload     (void *pudev, usb_device_req_struct *req);
static void dfu_getstatus  (void *pudev, usb_device_req_struct *req);
static void dfu_clrstatus  (void *pudev, usb_device_req_struct *req);
static void dfu_getstate   (void *pudev, usb_device_req_struct *req);
static void dfu_abort      (void *pudev, usb_device_req_struct *req);

static void dfu_mode_leave (void *pudev);

static uint8_t dfu_getstatus_complete (void *pudev);

static void (*dfu_request_process[])(void *pudev, usb_device_req_struct *req) = 
{
    dfu_detach,
    dfu_dnload,
    dfu_upload,
    dfu_getstatus,
    dfu_clrstatus,
    dfu_getstate,
    dfu_abort
};

/* data management variables */
static __IO uint32_t g_usbd_dfu_altset = 0;

/* note:it should use the c99 standard when compiling the below codes */
/* USB standard device descriptor */
const usb_descriptor_device_struct device_descripter =
{
    .Header = 
     {
         .bLength = USB_DEVICE_DESC_SIZE, 
         .bDescriptorType = USB_DESCTYPE_DEVICE
     },
    .bcdUSB = 0x0200,
    .bDeviceClass = 0x00,
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,
    .bMaxPacketSize0 = USBD_EP0_MAX_SIZE,
    .idVendor = USBD_VID,
    .idProduct = USBD_PID,
    .bcdDevice = 0x0100,
    .iManufacturer = USBD_MFC_STR_IDX,
    .iProduct = USBD_PRODUCT_STR_IDX,
    .iSerialNumber = USBD_SERIAL_STR_IDX,
    .bNumberConfigurations = USBD_CFG_MAX_NUM
};

/* USB device configuration descriptor */
const usb_descriptor_configuration_set_struct configuration_descriptor = 
{
    .Config = 
    {
        .Header = 
         {
             .bLength = sizeof(usb_descriptor_configuration_struct), 
             .bDescriptorType = USB_DESCTYPE_CONFIGURATION 
         },
        .wTotalLength = USB_DFU_CONFIG_DESC_SIZE,
        .bNumInterfaces = 0x01,
        .bConfigurationValue = 0x01,
        .iConfiguration = 0x00,
        .bmAttributes = 0x80,
        .bMaxPower = 0x32
    },

    .DFU_Interface = 
    {
        .Header = 
         {
             .bLength = sizeof(usb_descriptor_interface_struct), 
             .bDescriptorType = USB_DESCTYPE_INTERFACE 
         },
        .bInterfaceNumber = 0x00,
        .bAlternateSetting = 0x00,
        .bNumEndpoints = 0x00,
        .bInterfaceClass = 0xFE,
        .bInterfaceSubClass = 0x01,
        .bInterfaceProtocol = 0x02,
        .iInterface = 0x00
    },

    .DFU_Function_Desc = 
    {
        .Header = 
         {
            .bLength = sizeof(usb_dfu_function_descriptor_struct), 
            .bDescriptorType = DFU_DESC_TYPE 
         },
        .bmAttributes = 0x0B,
        .wDetachTimeOut = 0x00FF,
        .wTransferSize = TRANSFER_SIZE,
        .bcdDFUVersion = 0x011A,
    },
};

/* USB language ID Descriptor */
const usb_descriptor_language_id_struct usbd_language_id_desc = 
{
    .Header = 
     {
         .bLength = sizeof(usb_descriptor_language_id_struct), 
         .bDescriptorType = USB_DESCTYPE_STRING
     },
    .wLANGID = ENG_LANGID
};

/* USB serial string */
uint8_t usbd_serial_string[USB_SERIAL_STRING_SIZE] =
{
    USB_SERIAL_STRING_SIZE,
    USB_DESCTYPE_STRING,
};

void *const usbd_strings[] = 
{
    [USBD_LANGID_STR_IDX] = (uint8_t *)&usbd_language_id_desc,
    [USBD_MFC_STR_IDX] = USBD_STRING_DESC("GigaDevice"),
    [USBD_PRODUCT_STR_IDX] = USBD_STRING_DESC("GD32 USB DFU in FS Mode"),
    [USBD_SERIAL_STR_IDX] = usbd_serial_string
};

dfu_status_struct dfu_status;

uint8_t manifest_state = MANIFEST_COMPLETE;

struct 
{
    uint8_t buf[TRANSFER_SIZE];
    uint16_t data_len;
    uint16_t block_num;
    uint32_t base_addr;
} prog_struct = {{0}, 0, 0, APP_LOADED_ADDR};

/*!
    \brief      initialize the MSC device
    \param[in]  pudev: pointer to usb device instance
    \param[in]  config_index: configuration index
    \param[out] none
    \retval     usb device operation status
*/
usbd_status_enum dfu_init (void *pudev, uint8_t config_index)
{
    /* unlock the internal flash */
    fmc_unlock();

    systick_config();

    return USBD_OK;
}

/*!
    \brief      de-initialize the MSC device
    \param[in]  pudev: pointer to usb device instance
    \param[in]  config_index: configuration index
    \param[out] none
    \retval     usb device operation status
*/
usbd_status_enum  dfu_deinit (void *pudev, uint8_t config_index)
{
    /* restore device default state */
    dfu_status.bStatus = STATUS_OK;
    dfu_status.bState = STATE_dfuIDLE;

    prog_struct.block_num = 0;
    prog_struct.data_len = 0;

    /* lock the internal flash */
    fmc_lock();

    return USBD_OK;
}

/*!
    \brief      handle the MSC class-specific requests
    \param[in]  pudev: pointer to usb device instance
    \param[in]  req: device class-specific request
    \param[out] none
    \retval     usb device operation status
*/
usbd_status_enum dfu_req_handler (void *pudev, usb_device_req_struct *req)
{
    switch (req->bmRequestType & USB_REQ_MASK) {
    case USB_CLASS_REQ:
        if (req->bRequest < DFU_REQ_MAX) {
            dfu_request_process[req->bRequest](pudev, req);
        } else {
            usbd_enum_error(pudev, req);
            return USBD_FAIL;
        }
        break;
    case USB_STANDARD_REQ:
        /* standard device request */
        switch (req->bRequest) {
        case USBREQ_GET_INTERFACE:
            usbd_ep_tx (pudev, EP0_IN, (uint8_t *)&g_usbd_dfu_altset, 1);
            break;
        case USBREQ_SET_INTERFACE:
            g_usbd_dfu_altset = (uint8_t)(req->wValue);
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }

    return USBD_OK;
}

/*!
    \brief      handle data Stage
    \param[in]  pudev: pointer to usb device instance
    \param[in]  rx_tx: the flag of Rx or Tx
    \param[in]  ep_id: the endpoint ID
    \param[out] none
    \retval     usb device operation status
*/
usbd_status_enum  dfu_data_handler (void *pudev, usbd_dir_enum rx_tx, uint8_t ep_id)
{
    if ((USBD_TX == rx_tx) && ((DFU_IN_EP & 0x7F) == ep_id)) {
        dfu_getstatus_complete(pudev);

        return USBD_OK;
    }

    return USBD_FAIL;
}

/*!
    \brief      handle data IN stage in control endpoint 0
    \param[in]  pudev: pointer to usb device instance
    \param[out] none
    \retval     usb device operation status
  */
static uint8_t  dfu_getstatus_complete (void *pudev)
{
    uint32_t addr;

    if (STATE_dfuDNBUSY == dfu_status.bState) {
        /* decode the special command */
        if (0 == prog_struct.block_num) {
            if (1 == prog_struct.data_len){
                if (GET_COMMANDS == prog_struct.buf[0]) {
                    /* no operation */
                } else {
                    goto command_error;
                }
            } else if (5 == prog_struct.data_len) {
                if(SET_ADDRESS_POINTER == prog_struct.buf[0]) {
                    /* set flash operation address */
                    prog_struct.base_addr = *(uint32_t *)(prog_struct.buf + 1);
                } else if(ERASE == prog_struct.buf[0]) {
                    prog_struct.base_addr = *(uint32_t *)(prog_struct.buf + 1);

                    fmc_page_erase(prog_struct.base_addr);
                } else {
                    goto command_error;
                }
            } else {
command_error:  usbd_enum_error(pudev, NULL);
            }
        } else if (prog_struct.block_num > 1) {  /* regular download command */
            /* preform the write operation */
            uint32_t idx = 0;

            /* decode the required address */
            addr = (prog_struct.block_num - 2) * TRANSFER_SIZE + prog_struct.base_addr;

            if (prog_struct.data_len & 0x03) { /* not an aligned data */
                for (idx = prog_struct.data_len; idx < ((prog_struct.data_len & 0xFFFC) + 4); idx++) {
                    prog_struct.buf[idx] = 0xFF;
                }
            }

            /* data received are word multiple */
            for (idx = 0; idx < prog_struct.data_len; idx += 4) {
                fmc_word_program(addr, *(uint32_t *)(prog_struct.buf + idx));
                addr += 4;
            }
            prog_struct.block_num = 0;
        }

        prog_struct.data_len = 0;

        /* update the device state and poll timeout */
        dfu_status.bState = STATE_dfuDNLOAD_SYNC;

        return USBD_OK;
    } else if (dfu_status.bState == STATE_dfuMANIFEST) {  /* manifestation in progress */
        /* start leaving DFU mode */
        dfu_mode_leave(pudev);
    }

    return USBD_OK;
}

/*!
    \brief      handle the DFU_DETACH request
    \param[in]  pudev: pointer to usb device instance
    \param[in]  req: DFU class request
    \param[out] none
    \retval     none.
*/
static void dfu_detach(void *pudev, usb_device_req_struct *req)
{
    switch (dfu_status.bState) {
    case STATE_dfuIDLE:
    case STATE_dfuDNLOAD_SYNC:
    case STATE_dfuDNLOAD_IDLE:
    case STATE_dfuMANIFEST_SYNC:
    case STATE_dfuUPLOAD_IDLE:
        dfu_status.bStatus = STATUS_OK;
        dfu_status.bState = STATE_dfuIDLE;
        dfu_status.iString = 0; /* iString */
        prog_struct.block_num = 0;
        prog_struct.data_len = 0;
        break;
    default:
        break;
    }

    /* check the detach capability in the DFU functional descriptor */
    if (configuration_descriptor.DFU_Function_Desc.wDetachTimeOut & DFU_DETACH_MASK) {
        /* perform an unconnected operation on USB bus */
        gpio_bit_reset(USB_PULLUP, USB_PULLUP_PIN);
        ((usbd_core_handle_struct *)pudev)->status = USBD_UNCONNECTED;

        /* perform an connected operation on USB bus */
        gpio_bit_set(USB_PULLUP, USB_PULLUP_PIN);
        ((usbd_core_handle_struct *)pudev)->status = USBD_CONNECTED;
    } else {
        /* wait for the period of time specified in detach request */
        delay_1ms (4);
    }
}

/*!
    \brief      handle the DFU_DNLOAD request
    \param[in]  pudev: pointer to usb device instance
    \param[in]  req: DFU class request
    \param[out] none
    \retval     none
*/
static void dfu_dnload(void *pudev, usb_device_req_struct *req)
{
    switch (dfu_status.bState) {
    case STATE_dfuIDLE:
    case STATE_dfuDNLOAD_IDLE:
        if (req->wLength > 0) {
            /* update the global length and block number */
            prog_struct.block_num = req->wValue;
            prog_struct.data_len = req->wLength;
            dfu_status.bState = STATE_dfuDNLOAD_SYNC;

            /* enable EP0 prepare receive the buffer */
            usbd_ep_rx (pudev, EP0_OUT, (uint8_t*)prog_struct.buf, prog_struct.data_len);
        } else {
            manifest_state = MANIFEST_IN_PROGRESS;
            dfu_status.bState = STATE_dfuMANIFEST_SYNC;
        }
        break;
    default:
        usbd_enum_error(pudev, req);
        break;
    }
}

/*!
    \brief      handles the DFU UPLOAD request.
    \param[in]  pudev: pointer to usb device instance
    \param[in]  req: DFU class request
    \param[out] none
    \retval     none
*/
static void  dfu_upload (void *pudev, usb_device_req_struct *req)
{
    uint8_t *phy_addr = NULL;
    uint32_t addr = 0;

    if(req->wLength <= 0) {
        dfu_status.bState = STATE_dfuIDLE;
        return;
    }

    switch (dfu_status.bState) {
    case STATE_dfuIDLE:
    case STATE_dfuUPLOAD_IDLE:
        /* update the global length and block number */
        prog_struct.block_num = req->wValue;
        prog_struct.data_len = req->wLength;

        /* DFU Get Command */
        if (prog_struct.block_num == 0) {
            /* update the state machine */
            dfu_status.bState = (prog_struct.data_len > 3) ? STATE_dfuIDLE : STATE_dfuUPLOAD_IDLE;

            /* store the values of all supported commands */
            prog_struct.buf[0] = GET_COMMANDS;
            prog_struct.buf[1] = SET_ADDRESS_POINTER;
            prog_struct.buf[2] = ERASE;

            /* send the status data over EP0 */
            usbd_ep_tx (pudev, EP0_IN, (uint8_t *)(&(prog_struct.buf[0])), 3);
        } else if (prog_struct.block_num > 1) {
            dfu_status.bState = STATE_dfuUPLOAD_IDLE;

            /* change is accelerated */
            addr = (prog_struct.block_num - 2) * TRANSFER_SIZE + prog_struct.base_addr;

            /* return the physical address where data are stored */
            phy_addr = (uint8_t *)(addr);

            /* send the status data over EP0 */
            usbd_ep_tx (pudev, EP0_IN, phy_addr, prog_struct.data_len);
        } else {
            dfu_status.bState = STATUS_errSTALLEDPKT;

            usbd_enum_error (pudev, req);
        }
        break;
    default:
        prog_struct.data_len = 0;
        prog_struct.block_num = 0;

        usbd_enum_error (pudev, req);
        break;
    }
}

/*!
    \brief      handle the DFU_GETSTATUS request
    \param[in]  pudev: pointer to usb device instance
    \param[out] none
    \retval     none
*/
static void  dfu_getstatus (void *pudev, usb_device_req_struct *req)
{
    switch (dfu_status.bState) {
    case STATE_dfuDNLOAD_SYNC:
        if (0 != prog_struct.data_len) {
            dfu_status.bState = STATE_dfuDNBUSY;
            if (0 == prog_struct.block_num) {
                if (ERASE == prog_struct.buf[0]) {
                    SET_POLLING_TIMEOUT(FLASH_ERASE_TIMEOUT);
                } else {
                    SET_POLLING_TIMEOUT(FLASH_WRITE_TIMEOUT);
                }
            }
        } else {
            dfu_status.bState = STATE_dfuDNLOAD_IDLE;
        }
        break;
    case STATE_dfuMANIFEST_SYNC:
        if (MANIFEST_IN_PROGRESS == manifest_state) {
            dfu_status.bState = STATE_dfuMANIFEST;
            dfu_status.bwPollTimeout0 = 1;
        } else if ((MANIFEST_COMPLETE == manifest_state) && \
            (configuration_descriptor.DFU_Function_Desc.bmAttributes & 0x04)){
            dfu_status.bState = STATE_dfuIDLE;
            dfu_status.bwPollTimeout0 = 0;
        }
        break;
    default:
        break;
    }

    /* send the status data of DFU interface to host over EP0 */
    usbd_ep_tx (pudev, EP0_IN, (uint8_t *)(&(dfu_status.bStatus)), 6);
}

/*!
    \brief      handle the DFU_CLRSTATUS request
    \param      pudev: pointer to usb device instance
    \param[out] none
    \retval     none
*/
static void  dfu_clrstatus (void *pudev, usb_device_req_struct *req)
{
    if (STATE_dfuERROR == dfu_status.bState) {
        dfu_status.bStatus = STATUS_OK;
        dfu_status.bState = STATE_dfuIDLE;
    } else {
        /* state Error */
        dfu_status.bStatus = STATUS_errUNKNOWN;
        dfu_status.bState = STATE_dfuERROR;
    }

    dfu_status.iString = 0; /* iString: index = 0 */
}

/*!
    \brief      handle the DFU_GETSTATE request
    \param[in]  pudev: pointer to usb device instance
    \param[out] none
    \retval     none
*/
static void  dfu_getstate (void *pudev, usb_device_req_struct *req)
{
    /* send the current state of the DFU interface to host */
    usbd_ep_tx (pudev, EP0_IN, &dfu_status.bState, 1);
}

/*!
    \brief      handle the DFU_ABORT request
    \param[in]  pudev: pointer to usb device instance
    \param[out] none
    \retval     none
*/
static void  dfu_abort (void *pudev, usb_device_req_struct *req)
{
    switch (dfu_status.bState){
    case STATE_dfuIDLE:
    case STATE_dfuDNLOAD_SYNC:
    case STATE_dfuDNLOAD_IDLE:
    case STATE_dfuMANIFEST_SYNC:
    case STATE_dfuUPLOAD_IDLE:
        dfu_status.bStatus = STATUS_OK;
        dfu_status.bState = STATE_dfuIDLE;
        dfu_status.iString = 0; /* iString: index = 0 */

        prog_struct.block_num = 0;
        prog_struct.data_len = 0;
        break;

    default:
        break;
    }
}

/*!
    \brief      leave DFU mode and reset device to jump to user loaded code
    \param[in]  pudev: pointer to usb device instance
    \param[out] none
    \retval     none
*/
void  dfu_mode_leave (void *pudev)
{
    manifest_state = MANIFEST_COMPLETE;

    if (configuration_descriptor.DFU_Function_Desc.bmAttributes & 0x04) {
        dfu_status.bState = STATE_dfuMANIFEST_SYNC;
    } else {
        dfu_status.bState = STATE_dfuMANIFEST_WAIT_RESET;

        /* lock the internal flash */
        fmc_lock();

        /* generate system reset to allow jumping to the user code */
        NVIC_SystemReset();
    }
}
