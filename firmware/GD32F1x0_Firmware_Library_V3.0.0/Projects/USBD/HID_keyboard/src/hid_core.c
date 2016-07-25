/*!
    \file  hid_core.c
    \brief HID class driver
*/

/*
    Copyright (C) 2016 GigaDevice

    2014-09-06, V1.0.0, firmware for GD32F150
    2016-01-15, V1.0.1, firmware for GD32F150
    2016-04-30, V3.0.0, firmware update for GD32F150
*/

#include "hid_core.h"
#include "usbd_std.h"

static uint32_t usbd_hid_altset = 0;
static uint32_t usbd_hid_protocol = 0;
static uint32_t usbd_hid_idlestate  = 0;

extern __IO uint8_t prev_transfer_complete;

extern uint8_t key_buffer[];

#define USBD_VID                     0x28e9
#define USBD_PID                     0x0380

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
        .wTotalLength = USB_HID_CONFIG_DESC_SIZE,
        .bNumInterfaces = 0x01,
        .bConfigurationValue = 0x01,
        .iConfiguration = 0x00,
        .bmAttributes = 0xA0,
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
        .bInterfaceSubClass = 0x01,
        .bInterfaceProtocol = 0x01,
        .iInterface = 0x00
    },

    .HID_VendorHID = 
    {
        .Header = 
         {
             .bLength = sizeof(usb_hid_descriptor_hid_struct), 
             .bDescriptorType = HID_DESC_TYPE 
         },
        .bcdHID = 0x0111,
        .bCountryCode = 0x00,
        .bNumDescriptors = 0x01,
        .bDescriptorType = HID_REPORT_DESCTYPE,
        .wDescriptorLength = USB_HID_REPORT_DESC_SIZE,
    },

    .HID_ReportINEndpoint = 
    {
        .Header = 
         {
             .bLength = sizeof(usb_descriptor_endpoint_struct), 
             .bDescriptorType = USB_DESCTYPE_ENDPOINT 
         },
        .bEndpointAddress = HID_IN_EP,
        .bmAttributes = 0x03,
        .wMaxPacketSize = HID_IN_PACKET,
        .bInterval = 0x40
    },

    .HID_ReportOUTEndpoint = 
    {
        .Header = 
         {
             .bLength = sizeof(usb_descriptor_endpoint_struct), 
             .bDescriptorType = USB_DESCTYPE_ENDPOINT
         },
        .bEndpointAddress = HID_OUT_EP,
        .bmAttributes = 0x03,
        .wMaxPacketSize = HID_OUT_PACKET,
        .bInterval = 0x40
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

void *const usbd_strings[] = 
{
    [USBD_LANGID_STR_IDX] = (uint8_t *)&usbd_language_id_desc,
    [USBD_MFC_STR_IDX] = USBD_STRING_DESC("GigaDevice"),
    [USBD_PRODUCT_STR_IDX] = USBD_STRING_DESC("GD32 USB Keyboard in FS Mode"),
    [USBD_SERIAL_STR_IDX] = USBD_STRING_DESC("GD32F1X0-V3.0.0-3a4b5ec")
};

const uint8_t hid_report_desc[USB_HID_REPORT_DESC_SIZE] =
{
    0x05, 0x01,  /* USAGE_PAGE (Generic Desktop) */
    0x09, 0x06,  /* USAGE (Keyboard) */
    0xa1, 0x01,  /* COLLECTION (Application) */

    0x05, 0x07,  /* USAGE_PAGE (Keyboard/Keypad) */
    0x19, 0xe0,  /* USAGE_MINIMUM (Keyboard LeftControl) */
    0x29, 0xe7,  /* USAGE_MAXIMUM (Keyboard Right GUI) */
    0x15, 0x00,  /* LOGICAL_MINIMUM (0) */
    0x25, 0x01,  /* LOGICAL_MAXIMUM (1) */
    0x95, 0x08,  /* REPORT_COUNT (8) */
    0x75, 0x01,  /* REPORT_SIZE (1) */
    0x81, 0x02,  /* INPUT (Data,Var,Abs) */

    0x95, 0x01,  /* REPORT_COUNT (1) */
    0x75, 0x08,  /* REPORT_SIZE (8) */
    0x81, 0x03,  /* INPUT (Cnst,Var,Abs) */

    0x95, 0x06,  /* REPORT_COUNT (6) */
    0x75, 0x08,  /* REPORT_SIZE (8) */
    0x25, 0xFF,  /* LOGICAL_MAXIMUM (255) */
    0x19, 0x00,  /* USAGE_MINIMUM (Reserved (no event indicated)) */
    0x29, 0x65,  /* USAGE_MAXIMUM (Keyboard Application) */
    0x81, 0x00,  /* INPUT (Data,Ary,Abs) */

    0x25, 0x01,  /* LOGICAL_MAXIMUM (1) */
    0x95, 0x02,  /* REPORT_COUNT (2) */
    0x75, 0x01,  /* REPORT_SIZE (1) */
    0x05, 0x08,  /* USAGE_PAGE (LEDs) */
    0x19, 0x01,  /* USAGE_MINIMUM (Num Lock) */
    0x29, 0x02,  /* USAGE_MAXIMUM (Caps Lock) */
    0x91, 0x02,  /* OUTPUT (Data,Var,Abs) */

    0x95, 0x01,  /* REPORT_COUNT (1) */
    0x75, 0x06,  /* REPORT_SIZE (6) */
    0x91, 0x03,  /* OUTPUT (Cnst,Var,Abs) */

    0xc0         /* END_COLLECTION */
};

/*!
    \brief      initialize the HID device
    \param[in]  pudev: pointer to USB device instance
    \param[in]  config_index: configuration index
    \param[out] none
    \retval     USB device operation status
*/
usbd_status_enum hid_init (void *pudev, uint8_t config_index)
{
    /* initialize Tx endpoint */
    usbd_ep_init(pudev, &(configuration_descriptor.HID_ReportINEndpoint));

    /* initialize Rx endpoint */
    usbd_ep_init(pudev, &(configuration_descriptor.HID_ReportOUTEndpoint));

    return USBD_OK;
}

/*!
    \brief      de-initialize the HID device
    \param[in]  pudev: pointer to USB device instance
    \param[in]  config_index: configuration index
    \param[out] none
    \retval     USB device operation status
*/
usbd_status_enum  hid_deinit (void *pudev, uint8_t config_index)
{
    /* deinitialize HID endpoints */
    usbd_ep_deinit (pudev, HID_IN_EP);
    usbd_ep_deinit (pudev, HID_OUT_EP);

    return USBD_OK;
}

/*!
    \brief      handle the HID class-specific requests
    \param[in]  pudev: pointer to USB device instance
    \param[in]  req: device class-specific request
    \param[out] none
    \retval     USB device operation status
*/
usbd_status_enum hid_req_handler (void *pudev, usb_device_req_struct *req)
{
    uint16_t len = 0;
    uint8_t *pbuf = NULL;

    switch (req->bmRequestType & USB_REQ_MASK) {
    case USB_CLASS_REQ:
        switch (req->bRequest) {
        case GET_REPORT:
            /* no use for this driver */
            break;
        case GET_IDLE:
            usbd_ep_tx (pudev, EP0_IN, (uint8_t *)&usbd_hid_idlestate, 1);
            break;
        case GET_PROTOCOL:
            usbd_ep_tx (pudev, EP0_IN, (uint8_t *)&usbd_hid_protocol, 1);
            break;
        case SET_REPORT:
            /* no use for this driver */
            break;
        case SET_IDLE:
            usbd_hid_idlestate = (uint8_t)(req->wValue >> 8);
            break;
        case SET_PROTOCOL:
            usbd_hid_protocol = (uint8_t)(req->wValue);
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
            switch (req->wValue >> 8) {
            case HID_REPORT_DESCTYPE:
                len = MIN(USB_HID_REPORT_DESC_SIZE, req->wLength);
                pbuf = (uint8_t *)hid_report_desc;
                break;
            case HID_DESC_TYPE:
                len = MIN(USB_HID_DESC_SIZE, req->wLength);
                pbuf = (uint8_t *)(&(configuration_descriptor.HID_VendorHID));
                break;
            default:
                break;
            }
            usbd_ep_tx (pudev, EP0_IN, pbuf, len);
            break;
        case USBREQ_GET_INTERFACE:
            usbd_ep_tx (pudev, EP0_IN, (uint8_t *)&usbd_hid_altset, 1);
            break;
        case USBREQ_SET_INTERFACE:
            usbd_hid_altset = (uint8_t)(req->wValue);
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
usbd_status_enum  hid_data_handler (void *pudev, usbd_dir_enum rx_tx, uint8_t ep_id)
{
    if ((USBD_TX == rx_tx) && ((HID_IN_EP & 0x7F) == ep_id)) {
        key_buffer[2] = 0x00;

        usbd_ep_tx(pudev, HID_IN_EP, key_buffer, HID_IN_PACKET);

        prev_transfer_complete = 1;

        return USBD_OK;
    }

    return USBD_FAIL;
}

/*!
    \brief      send keyboard report
    \param[in]  pudev: pointer to USB device instance
    \param[in]  report: pointer to HID report
    \param[in]  len: data length
    \param[out] none
    \retval     USB device operation status
*/
uint8_t  hid_report_send (usbd_core_handle_struct *pudev, uint8_t *report, uint16_t len)
{
    /* check if USB is configured */
    prev_transfer_complete = 0;

    usbd_ep_tx (pudev, HID_IN_EP, report, len);

    return USBD_OK;
}
