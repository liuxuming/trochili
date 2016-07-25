/*!
    \file  usbd_std.h
    \brief USB standard definitions
*/

/*
    Copyright (C) 2016 GigaDevice

    2014-09-06, V1.0.0, firmware for GD32F150
    2016-01-15, V1.0.1, firmware for GD32F150
    2016-04-30, V3.0.0, firmware update for GD32F150
*/

#ifndef USBD_STD_H
#define USBD_STD_H

#include "usbd_core.h"
#include <wchar.h>

#ifndef NULL
 #define NULL                                           0
#endif

/* constants definitions */
#define USB_DEV_QUALIFIER_DESC_LEN                     0x0A  /* device qualifier descriptor length */
#define USB_CFG_DESC_LEN                               0x09  /* configuration descriptor length */

#define USBD_LANGID_STR_IDX                            0x00  /* language ID string index */
#define USBD_MFC_STR_IDX                               0x01  /* manufacturer string index */
#define USBD_PRODUCT_STR_IDX                           0x02  /* product string index */
#define USBD_SERIAL_STR_IDX                            0x03  /* serial string index */
#define USBD_CONFIG_STR_IDX                            0x04  /* configuration string index */
#define USBD_INTERFACE_STR_IDX                         0x05  /* interface string index */

#define USB_STANDARD_REQ                               0x00  /* standard request */
#define USB_CLASS_REQ                                  0x20  /* device class request */
#define USB_VENDOR_REQ                                 0x40  /* vendor request */
#define USB_REQ_MASK                                   0x60  /* request type mask */

#define USB_REQTYPE_DEVICE                             0x00  /* request recipient is device */
#define USB_REQTYPE_INTERFACE                          0x01  /* request recipient is interface */
#define USB_REQTYPE_ENDPOINT                           0x02  /* request recipient is endpoint */
#define USB_REQ_RECIPIENT_MASK                         0x1f  /* request recipient mask */

#define USBREQ_GET_STATUS                              0x00  /* Get_Status standard requeset */
#define USBREQ_CLEAR_FEATURE                           0x01  /* Clear_Feature standard requeset */
#define USBREQ_SET_FEATURE                             0x03  /* Set_Feature standard requeset */
#define USBREQ_SET_ADDRESS                             0x05  /* Set_Address standard requeset */
#define USBREQ_GET_DESCRIPTOR                          0x06  /* Get_Descriptor standard requeset */
#define USBREQ_GET_CONFIGURATION                       0x08  /* Get_Configuration standard requeset */
#define USBREQ_SET_CONFIGURATION                       0x09  /* Set_Configuration standard requeset */
#define USBREQ_GET_INTERFACE                           0x0A  /* Get_Interface standard requeset */
#define USBREQ_SET_INTERFACE                           0x0B  /* Set_Interface standard requeset */

#define USB_DESCTYPE_DEVICE                            0x01  /* device descriptor type */
#define USB_DESCTYPE_CONFIGURATION                     0x02  /* configuration descriptor type */
#define USB_DESCTYPE_STRING                            0x03  /* string descriptor type */
#define USB_DESCTYPE_INTERFACE                         0x04  /* interface descriptor type */
#define USB_DESCTYPE_ENDPOINT                          0x05  /* endpoint descriptor type */
#define USB_DESCTYPE_DEVICE_QUALIFIER                  0x06  /* device qualifier descriptor type */
#define USB_DESCTYPE_OTHER_SPEED_CONFIGURATION         0x07  /* other speed configuration descriptor type */
#define USB_DESCTYPE_BOS                               0x0F  /* BOS descriptor type */

#define USB_STATUS_REMOTE_WAKEUP                       2     /* usb is in remote wakeup status */
#define USB_STATUS_SELF_POWERED                        1     /* usb is in self powered status */

#define USB_FEATURE_ENDP_HALT                          0     /* usb has endpoint halt feature */
#define USB_FEATURE_REMOTE_WAKEUP                      1     /* usb has endpoint remote wakeup feature */
#define USB_FEATURE_TEST_MODE                          2     /* usb has endpoint test mode feature */

#define ENG_LANGID                                     0x0409
#define CHN_LANGID                                     0x0804

#define USB_EPTYPE_MASK                                0x03

#define USB_DEVICE_DESC_SIZE                           0x12

/* USB device exported macros */
#define SWAPBYTE(addr)       (((uint16_t)(*((uint8_t *)(addr)))) + \
                             (((uint16_t)(*(((uint8_t *)(addr)) + 1))) << 8))

#define LOWBYTE(x)           ((uint8_t)(x & 0x00FF))
#define HIGHBYTE(x)          ((uint8_t)((x & 0xFF00) >> 8))

#define MIN(a, b)            (((a) < (b)) ? (a) : (b))

#define IS_NOT_EP0(ep_addr)  ((ep_addr != 0x00) && (ep_addr != 0x80))

#define WIDE_STRING(string)  _WIDE_STRING(string)
#define _WIDE_STRING(string) L##string

#define USBD_STRING_DESC(string) \
    (void *)&(const struct { \
        uint8_t _len; \
        uint8_t _type; \
        wchar_t _data[sizeof(string)]; \
    }) { \
        sizeof(WIDE_STRING(string)) + 2 - 2, \
        USB_DESCTYPE_STRING, \
        WIDE_STRING(string) \
    }

typedef struct
{
    uint8_t bLength;                      /*!< size of the descriptor */
    uint8_t bDescriptorType;              /*!< type of the descriptor */
} usb_descriptor_header_struct;

typedef struct
{
    usb_descriptor_header_struct Header;  /*!< descriptor header, including type and size */

    uint16_t bcdUSB;                      /*!< BCD of the supported USB specification */
    uint8_t  bDeviceClass;                /*!< USB device class */
    uint8_t  bDeviceSubClass;             /*!< USB device subclass */
    uint8_t  bDeviceProtocol;             /*!< USB device protocol */
    uint8_t  bMaxPacketSize0;             /*!< size of the control (address 0) endpoint's bank in bytes */
    uint16_t idVendor;                    /*!< vendor ID for the USB product */
    uint16_t idProduct;                   /*!< unique product ID for the USB product */
    uint16_t bcdDevice;                   /*!< product release (version) number */
    uint8_t  iManufacturer;               /*!< string index for the manufacturer's name */
    uint8_t  iProduct;                    /*!< string index for the product name/details */
    uint8_t  iSerialNumber;               /*!< string index for the product's globally unique hexadecimal serial number */
    uint8_t  bNumberConfigurations;       /*!< total number of configurations supported by the device */
} usb_descriptor_device_struct;

#pragma pack(1)

typedef struct
{
    usb_descriptor_header_struct Header;  /*!< descriptor header, including type and size */

    uint16_t wTotalLength;                /*!< size of the configuration descriptor header,and all sub descriptors inside the configuration */
    uint8_t  bNumInterfaces;              /*!< total number of interfaces in the configuration */
    uint8_t  bConfigurationValue;         /*!< configuration index of the current configuration */
    uint8_t  iConfiguration;              /*!< index of a string descriptor describing the configuration */
    uint8_t  bmAttributes;                /*!< configuration attributes */
    uint8_t  bMaxPower;                   /*!< maximum power consumption of the device while in the current configuration */
} usb_descriptor_configuration_struct;

#pragma pack()

typedef struct
{
    usb_descriptor_header_struct Header;  /*!< descriptor header, including type and size */

    uint8_t bInterfaceNumber;             /*!< index of the interface in the current configuration */
    uint8_t bAlternateSetting;            /*!< alternate setting for the interface number */
    uint8_t bNumEndpoints;                /*!< total number of endpoints in the interface */
    uint8_t bInterfaceClass;              /*!< interface class ID */
    uint8_t bInterfaceSubClass;           /*!< interface subclass ID */
    uint8_t bInterfaceProtocol;           /*!< interface protocol ID */
    uint8_t iInterface;                   /*!< index of the string descriptor describing the interface */
} usb_descriptor_interface_struct;

#pragma pack(1)

typedef struct
{
    usb_descriptor_header_struct Header;  /*!< descriptor header, including type and size. */

    uint8_t  bEndpointAddress;            /*!< logical address of the endpoint */
    uint8_t  bmAttributes;                /*!< endpoint attributes */
    uint16_t wMaxPacketSize;              /*!< size of the endpoint bank, in bytes */
    uint8_t  bInterval;                   /*!< polling interval in milliseconds for the endpoint if it is an INTERRUPT or ISOCHRONOUS type */
} usb_descriptor_endpoint_struct;

typedef struct
{
    usb_descriptor_header_struct Header;  /*!< descriptor header, including type and size. */
    uint16_t wLANGID;                     /*!< LANGID code */
}usb_descriptor_language_id_struct;

#pragma pack()

/* function declarations */
/* USB SETUP transaction processing */
uint8_t  usbd_setup_transaction (usbd_core_handle_struct *pudev);
/* USB OUT transaction processing */
uint8_t  usbd_out_transaction (usbd_core_handle_struct *pudev, uint8_t ep_id);
/* USB IN transaction processing */
uint8_t  usbd_in_transaction (usbd_core_handle_struct *pudev, uint8_t ep_id);

/* handle USB standard device request */
uint8_t  usbd_standard_request (usbd_core_handle_struct *pudev, usb_device_req_struct *req);
/* handle device class request */
uint8_t  usbd_device_class_request (usbd_core_handle_struct *pudev, usb_device_req_struct *req);
/* handle USB vendor request */
uint8_t  usbd_vendor_request (usbd_core_handle_struct *pudev, usb_device_req_struct *req);

/* decode setup data packet */
void  usbd_setup_request_parse (usbd_core_handle_struct *pudev, usb_device_req_struct *req);
/* handle USB enumeration error event */
void  usbd_enum_error (usbd_core_handle_struct *pudev, usb_device_req_struct *req);

#endif /* USBD_STD_H */
