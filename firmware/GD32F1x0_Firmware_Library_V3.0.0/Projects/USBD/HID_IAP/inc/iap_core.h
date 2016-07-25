/*!
    \file  iap_core.h
    \brief the header file of IAP driver
*/

/*
    Copyright (C) 2016 GigaDevice

    2014-09-06, V1.0.0, firmware for GD32F150
    2016-01-15, V1.0.1, firmware for GD32F150
    2016-04-30, V3.0.0, firmware update for GD32F150
*/

#ifndef IAP_CORE_H
#define IAP_CORE_H

#include "usbd_std.h"

#define IAP_CONFIG_DESC_SIZE                9
#define USB_SERIAL_STRING_SIZE              0x06

#define DEVICE_ID                           (0x40022100)

#define IAP_REPORT_DESC_SIZE                35
#define IAP_CONFIG_SET_DESC_SIZE            41

#define HID_DESCTYPE                        0x21
#define HID_REPORT_DESCTYPE                 0x22

#define GET_REPORT                          0x01
#define GET_IDLE                            0x02
#define GET_PROTOCOL                        0x03
#define SET_REPORT                          0x09
#define SET_IDLE                            0x0A
#define SET_PROTOCOL                        0x0B

/* special commands with download request */
#define IAP_OPTION_BYTE                     0x01
#define IAP_ERASE                           0x02
#define IAP_DNLOAD                          0x03
#define IAP_LEAVE                           0x04
#define IAP_GETBIN_ADDRESS                  0x05

typedef  void  (*pAppFunction) (void);

#pragma pack(1)

typedef struct
{
    usb_descriptor_header_struct Header;  /*!< regular descriptor header containing the descriptor's type and length */

    uint16_t bcdHID;                      /*!< BCD encoded version that the HID descriptor and device complies to */
    uint8_t  bCountryCode;                /*!< country code of the localized device, or zero if universal */
    uint8_t  bNumDescriptors;             /*!< total number of HID report descriptors for the interface */
    uint8_t  bDescriptorType;             /*!< type of HID report */
    uint16_t wDescriptorLength;           /*!< length of the associated HID report descriptor, in bytes */
} usb_hid_descriptor_hid_struct;

#pragma pack()

typedef struct
{
    usb_descriptor_configuration_struct Config;

    usb_descriptor_interface_struct            HID_Interface;
    usb_hid_descriptor_hid_struct              HID_VendorHID;
    usb_descriptor_endpoint_struct             HID_ReportINEndpoint;
    usb_descriptor_endpoint_struct             HID_ReportOUTEndpoint;
} usb_descriptor_configuration_set_struct;

extern void* const usbd_strings[USB_STRING_COUNT];
extern const usb_descriptor_device_struct device_descripter;
extern const usb_descriptor_configuration_set_struct configuration_descriptor;

/* function declarations */
/* initialize the HID device */
usbd_status_enum iap_init (void *pudev, uint8_t config_index);
/* de-initialize the HID device */
usbd_status_enum iap_deinit (void *pudev, uint8_t config_index);
/* handle the HID class-specific requests */
usbd_status_enum iap_req_handler (void *pudev, usb_device_req_struct *req);
/* handle data stage */
usbd_status_enum iap_data_handler (void *pudev, usbd_dir_enum rx_tx, uint8_t ep_id);

/* send iap report */
uint8_t iap_report_send (usbd_core_handle_struct *pudev, uint8_t *report, uint16_t Len);

#endif  /* IAP_CORE_H */
