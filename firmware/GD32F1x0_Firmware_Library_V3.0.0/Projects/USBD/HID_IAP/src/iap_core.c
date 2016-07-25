/*!
    \file  iap_core.c
    \brief IAP driver
*/

/*
    Copyright (C) 2016 GigaDevice

    2014-09-06, V1.0.0, firmware for GD32F150
    2016-01-15, V1.0.1, firmware for GD32F150
    2016-04-30, V3.0.0, firmware update for GD32F150
*/

#include "iap_core.h"

#define USBD_VID                     0x28E9
#define USBD_PID                     0x028B

uint8_t report_buf[65];
uint8_t option_byte[17] = {0};

/* state machine variables */
uint8_t device_status[17];
uint8_t bin_address[17];

uint8_t usbd_customhid_report_id = 0;
uint8_t flag = 0;

static uint32_t  usbd_customhid_altset = 0;
static uint32_t  usbd_customhid_protocol = 0;
static uint32_t  usbd_customhid_idlestate = 0;

static uint16_t transfer_times = 0;
static uint16_t page_count = 0;
static uint32_t file_length = 0;
static uint32_t base_address = APP_LOADED_ADDR;

/* Note:it should use the C99 standard when compiling the below codes */
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

const usb_descriptor_configuration_set_struct configuration_descriptor = 
{
    .Config = 
    {
        .Header = 
         {
             .bLength = sizeof(usb_descriptor_configuration_struct), 
             .bDescriptorType = USB_DESCTYPE_CONFIGURATION 
         },
        .wTotalLength = IAP_CONFIG_SET_DESC_SIZE,
        .bNumInterfaces = 0x01,
        .bConfigurationValue = 0x01,
        .iConfiguration = 0x00,
        .bmAttributes = 0x80,
        .bMaxPower = 0x32
    },

    .HID_Interface = 
    {
        .Header = 
         {
             .bLength = sizeof(usb_descriptor_interface_struct), 
             .bDescriptorType = USB_DESCTYPE_INTERFACE 
         },
        .bInterfaceNumber = 0x00,
        .bAlternateSetting = 0x00,
        .bNumEndpoints = 0x02,
        .bInterfaceClass = 0x03,
        .bInterfaceSubClass = 0x00,
        .bInterfaceProtocol = 0x01,
        .iInterface = 0x00
    },

    .HID_VendorHID = 
    {
        .Header = 
         {
             .bLength = sizeof(usb_hid_descriptor_hid_struct), 
             .bDescriptorType = HID_DESCTYPE 
         },
        .bcdHID = 0x0111,
        .bCountryCode = 0x00,
        .bNumDescriptors = 0x01,
        .bDescriptorType = HID_REPORT_DESCTYPE,
        .wDescriptorLength = IAP_REPORT_DESC_SIZE,
    },

    .HID_ReportINEndpoint = 
    {
        .Header = 
         {
             .bLength = sizeof(usb_descriptor_endpoint_struct), 
             .bDescriptorType = USB_DESCTYPE_ENDPOINT 
         },
        .bEndpointAddress = IAP_IN_EP,
        .bmAttributes = 0x03,
        .wMaxPacketSize = IAP_IN_PACKET,
        .bInterval = 0x20
    },

    .HID_ReportOUTEndpoint = 
    {
        .Header = 
         {
             .bLength = sizeof(usb_descriptor_endpoint_struct), 
             .bDescriptorType = USB_DESCTYPE_ENDPOINT 
         },
        .bEndpointAddress = IAP_OUT_EP,
        .bmAttributes = 0x03,
        .wMaxPacketSize = IAP_OUT_PACKET,
        .bInterval = 0x20
    }
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
    USB_SERIAL_STRING_SIZE,       /* bLength */
    USB_DESCTYPE_STRING,          /* bDescriptorType */
};

void *const usbd_strings[] = 
{
    [USBD_LANGID_STR_IDX] = (uint8_t *)&usbd_language_id_desc,
    [USBD_MFC_STR_IDX] = USBD_STRING_DESC("GigaDevice"),
    [USBD_PRODUCT_STR_IDX] = USBD_STRING_DESC("GD32 USB IAP in FS Mode"),
    [USBD_SERIAL_STR_IDX] = usbd_serial_string
};

/* USB custom HID device report descriptor */
const uint8_t iap_report_descriptor[IAP_REPORT_DESC_SIZE] =
{
    0x05, 0x01,     /* USAGE_PAGE (Generic Desktop) */
    0x09, 0x00,     /* USAGE (Custom Device)        */
    0xa1, 0x01,     /* COLLECTION (Application)     */

    /* IAP command and data */
    0x85, 0x01,     /* REPORT_ID (0x01)          */
    0x09, 0x01,     /* USAGE (IAP command)       */
    0x15, 0x00,     /* LOGICAL_MINIMUM (0)       */
    0x25, 0xff,     /* LOGICAL_MAXIMUM (255)     */
    0x75, 0x08,     /* REPORT_SIZE (8)           */
    0x95, 0x3f,     /* REPORT_COUNT (63)         */
    0x91, 0x82,     /* OUTPUT (Data,Var,Abs,Vol) */

    /* device status and option byte */  
    0x85, 0x02,     /* REPORT_ID (0x02)               */
    0x09, 0x02,     /* USAGE (Status and option byte) */
    0x15, 0x00,     /* LOGICAL_MINIMUM (0)            */
    0x25, 0xff,     /* LOGICAL_MAXIMUM (255)          */
    0x75, 0x08,     /* REPORT_SIZE (8)                */
    0x95, 0x10,     /* REPORT_COUNT (16)              */
    0x81, 0x82,     /* INPUT (Data,Var,Abs,Vol)       */

    0xc0            /* END_COLLECTION            */
};

/* IAP requests management functions */
static void  iap_req_erase     (void *pudev);
static void  iap_req_dnload    (void *pudev);
static void  iap_req_optionbyte(void *pudev);
static void  iap_req_leave     (void *pudev);
static void  iap_address_send  (void *pudev);

static void  iap_data_write (uint8_t *data, uint32_t addr, uint32_t len);

/*!
    \brief      initialize the HID device
    \param[in]  pudev: pointer to USB device instance
    \param[in]  config_index: configuration index
    \param[out] none
    \retval     USB device operation status
*/
usbd_status_enum iap_init (void *pudev, uint8_t config_index)
{
    /* initialize Tx endpoint */
    usbd_ep_init(pudev, &(configuration_descriptor.HID_ReportINEndpoint));

    /* initialize Rx endpoint */
    usbd_ep_init(pudev, &(configuration_descriptor.HID_ReportOUTEndpoint));

    /* unlock the internal flash */
    fmc_unlock();

    /* prepare receive Data */
    usbd_ep_rx(pudev, IAP_OUT_EP, report_buf, IAP_OUT_PACKET);

    return USBD_OK;
}

/*!
    \brief      de-initialize the HID device
    \param[in]  pudev: pointer to USB device instance
    \param[in]  config_index: configuration index
    \param[out] none
    \retval     USB device operation status
*/
usbd_status_enum  iap_deinit (void *pudev, uint8_t config_index)
{
    /* deinitialize HID endpoints */
    usbd_ep_deinit (pudev, IAP_IN_EP);
    usbd_ep_deinit (pudev, IAP_OUT_EP);

    /* lock the internal flash */
    fmc_lock();

    return USBD_OK;
}

/*!
    \brief      handle the HID class-specific requests
    \param[in]  pudev: pointer to USB device instance
    \param[in]  req: device class-specific request
    \param[out] none
    \retval     USB device operation status
*/
usbd_status_enum iap_req_handler (void *pudev, usb_device_req_struct *req)
{
    uint16_t len = 0;
    uint8_t *pbuf = NULL;
    uint8_t USBD_CUSTOMHID_Report_LENGTH = 0;

    switch (req->bmRequestType & USB_REQ_MASK) {
    case USB_CLASS_REQ:
        switch (req->bRequest) {
        case GET_REPORT:
            /* no use for this driver */
            break;
        case GET_IDLE:
            usbd_ep_tx (pudev, EP0_IN, (uint8_t *)&usbd_customhid_idlestate, 1);
            break;
        case GET_PROTOCOL:
            usbd_ep_tx (pudev, EP0_IN, (uint8_t *)&usbd_customhid_protocol, 1);
            break;
        case SET_REPORT:
            flag = 1;
            usbd_customhid_report_id = (uint8_t)(req->wValue);
            USBD_CUSTOMHID_Report_LENGTH = (uint8_t)(req->wLength);
            usbd_ep_rx (pudev, EP0_OUT, report_buf, USBD_CUSTOMHID_Report_LENGTH);
            break;
        case SET_IDLE:
            usbd_customhid_idlestate = (uint8_t)(req->wValue >> 8);
            break;
        case SET_PROTOCOL:
            usbd_customhid_protocol = (uint8_t)(req->wValue);
            break;
        default:
            usbd_enum_error (pudev, req);
            return USBD_FAIL; 
            }
            break;
    case USB_STANDARD_REQ:
        /* standard device request */
        switch(req->bRequest) {
        case USBREQ_GET_DESCRIPTOR:
            switch(req->wValue >> 8) {
            case HID_REPORT_DESCTYPE:
                len = MIN(IAP_REPORT_DESC_SIZE, req->wLength);
                pbuf = (uint8_t *)iap_report_descriptor;
                break;
            case HID_DESCTYPE:
                len = MIN(IAP_CONFIG_DESC_SIZE, req->wLength);
                pbuf = (uint8_t *)(&(configuration_descriptor.HID_VendorHID));
                break;
            default:
                break;
            }
            usbd_ep_tx (pudev, EP0_IN, pbuf, len);
            break;
        case USBREQ_GET_INTERFACE:
            usbd_ep_tx (pudev, EP0_IN, (uint8_t *)&usbd_customhid_altset, 1);
            break;
        case USBREQ_SET_INTERFACE:
            usbd_customhid_altset = (uint8_t)(req->wValue);
            break;
        default:
            break;
        }
        break;
    }

    return USBD_OK;
}

/*!
    \brief      handle data stage
    \param[in]  pudev: pointer to USB device instance
    \param[in]  rx_tx: data transfer direction:
      \arg        USBD_TX
      \arg        USBD_RX
    \param[in]  ep_id: endpoint identifier
    \param[out] none
    \retval     USB device operation status
*/
usbd_status_enum  iap_data_handler (void *pudev, usbd_dir_enum rx_tx, uint8_t ep_id)
{
    if((USBD_TX == rx_tx) && ((IAP_IN_EP & 0x7F) == ep_id)) {
        return USBD_OK;
    } else if ((USBD_RX == rx_tx) && (IAP_OUT_EP == ep_id)) {
        switch (report_buf[0]) {
        case 0x01:
            switch(report_buf[1]) {
            case IAP_DNLOAD:
                iap_req_dnload(pudev);
                break;
            case IAP_ERASE:
                iap_req_erase(pudev);
                break;
            case IAP_OPTION_BYTE:
                iap_req_optionbyte(pudev);
                break;
            case IAP_LEAVE:
                iap_req_leave(pudev);
                break;
            case IAP_GETBIN_ADDRESS:
                iap_address_send(pudev);
                break;
            default:
                usbd_enum_error(pudev, NULL);
                return USBD_FAIL;
            }
            break;
        default:
            break;
        }

        usbd_ep_rx(pudev, IAP_OUT_EP, report_buf, IAP_OUT_PACKET);

        return USBD_OK;
    }

    return USBD_FAIL;
}

/*!
    \brief      send iap report
    \param[in]  pudev: pointer to USB device instance
    \param[in]  report: pointer to HID report
    \param[in]  len: data length
    \param[out] none
    \retval     USB device operation status
*/
uint8_t  iap_report_send (usbd_core_handle_struct *pudev, uint8_t *report, uint16_t len)
{
    usbd_ep_tx (pudev, IAP_IN_EP, report, len);

    return USBD_OK;
}

/*!
    \brief      handle the IAP_DNLOAD request
    \param[in]  pudev: pointer to usb device instance
    \param[out] none
    \retval     none
*/
static void iap_req_dnload(void *pudev)
{
    if (0 != transfer_times) {
        if (1 == transfer_times) {
            iap_data_write(&report_buf[2], base_address, file_length % TRANSFER_SIZE);

            device_status[0] = 0x02;
            device_status[1] = 0x02;
            iap_report_send (pudev, device_status, 17);
        } else {
            iap_data_write(&report_buf[2], base_address, TRANSFER_SIZE);

            base_address += TRANSFER_SIZE;
        }

        transfer_times --;
    }
}

/*!
    \brief      handle the IAP_ERASE request
    \param[in]  pudev: pointer to usb device instance
    \param[out] none
    \retval     none
*/
static void iap_req_erase(void *pudev)
{
    uint32_t i, addr = 0;

    /* get base address to erase */
    base_address  = report_buf[2];
    base_address |= report_buf[3] << 8;
    base_address |= report_buf[4] << 16;
    base_address |= report_buf[5] << 24;

    page_count = report_buf[6];

    /* get file length */
    file_length = report_buf[7];
    file_length |= report_buf[8] << 8;
    file_length |= report_buf[9] << 16;
    file_length |= report_buf[10] << 24;

    if (0 == (file_length % TRANSFER_SIZE)) {
        transfer_times = file_length / TRANSFER_SIZE;
    } else {
        transfer_times = file_length / TRANSFER_SIZE + 1;
    }

    /* check if the address is in protected area */
    if (IS_PROTECTED_AREA(base_address)) {
        return;
    }

    addr = base_address;

    for (i = 0; i < page_count; i ++) {
        /* call the standard flash erase-page function */
        fmc_page_erase(addr);

        addr += PAGE_SIZE;
    }

    device_status[0] = 0x02;
    device_status[1] = 0x01;

    usbd_ep_tx(pudev, IAP_IN_EP, device_status, IAP_IN_PACKET);
}

/*!
    \brief      handle the IAP_OPTION_BYTE request
    \param[in]  pudev: pointer to usb device instance
    \param[out] none
    \retval     none
*/
static void  iap_req_optionbyte(void *pudev)
{
    uint8_t i = 0;
    uint32_t address = 0x1FFFF800;

    option_byte[0]= 0x02;

    for (i = 1; i < 17; i++) {
        option_byte[i] = *(uint8_t *)address;
        address++;
    }

    iap_report_send (pudev, option_byte, IAP_IN_PACKET);
}

/*!
    \brief      handle the IAP_LEAVE request
    \param[in]  pudev: pointer to usb device instance
    \param[out] none
    \retval     none
*/
static void  iap_req_leave(void *pudev)
{
    /* lock the internal flash */
    fmc_lock();

    /* generate system reset to allow jumping to the user code */
    NVIC_SystemReset();
}

/*!
    \brief      handle the IAP_SEND_ADDRESS request
    \param[in]  pudev: pointer to usb device instance
    \param[out] none
    \retval     none
*/
static void  iap_address_send(void *pudev)
{
    bin_address[0] = 0x02;

    bin_address[1] = (uint8_t)(APP_LOADED_ADDR);
    bin_address[2] = (uint8_t)(APP_LOADED_ADDR >> 8);
    bin_address[3] = (uint8_t)(APP_LOADED_ADDR >> 16);
    bin_address[4] = (uint8_t)(APP_LOADED_ADDR >> 24);

    iap_report_send (pudev, bin_address, IAP_IN_PACKET);
}

/*!
    \brief      write data to sectors of memory
    \param[in]  data: data to be written
    \param[in]  addr: sector address/code
    \param[in]  len: length of data to be written (in bytes)
    \param[out] none
    \retval     MAL_OK if all operations are OK, MAL_FAIL else
*/
static void  iap_data_write (uint8_t *data, uint32_t addr, uint32_t len)
{
    uint32_t idx = 0;

    /* check if the address is in protected area */
    if (IS_PROTECTED_AREA(addr)) {
        return;
    }

    if (len & 0x03) {/* not an aligned data */
        for (idx = len; idx < ((len & 0xFFFC) + 4); idx++) {
            data[idx] = 0xFF;
        }
    }

    /* data received are word multiple */
    for (idx = 0; idx < len; idx += 4) {
        fmc_word_program(addr, *(uint32_t *)(data + idx));
        addr += 4;
    }
}
