/*!
    /file  usbd_conf.h
    /brief usb device driver basic configuration
*/

/*
    Copyright (C) 2016 GigaDevice

    2014-09-06, V1.0.0, firmware for GD32F1x0(x=5)
    2016-01-15, V1.0.1, firmware for GD32F1x0(x=5)
    2016-04-30, V2.0.0, firmware update for GD32F1x0(x=5)
*/

#ifndef USBD_CONF_H
#define USBD_CONF_H

#include "gd32f1x0.h"
#include "gd32f1x0_eval.h"

/* define if low power mode is enabled; it allows entering the device into DEEP_SLEEP mode
   following USB suspend event and wakes up after the USB wakeup event is received. */
#define USB_DEVICE_LOW_PWR_MODE_SUPPORT

//#define USE_EXTERNAL_MEDIA
#define USE_INTERNAL_MEDIA

#define USBD_CFG_MAX_NUM              1
#define USBD_ITF_MAX_NUM              1
#define USB_STRING_COUNT              4

#define PAGE_SIZE                     ((uint32_t)(1024))
#define NAND_FLASH_BASE_ADDRESS       ((uint32_t)(0x08000000 + 0x04000))
#define NAND_FLASH_END_ADDRESS        ((uint32_t)(0x08000000 + 0x10000))

/* endpoint count used by the MSC device */
#define MSC_IN_EP                     EP1_IN
#define MSC_OUT_EP                    EP2_OUT
#define MSC_DATA_PACKET_SIZE          64
#define MSC_MEDIA_PACKET_SIZE         PAGE_SIZE

#define BLOCK_SIZE                    PAGE_SIZE
#define BLOCK_NUM                     ((NAND_FLASH_END_ADDRESS - NAND_FLASH_BASE_ADDRESS) / BLOCK_SIZE)

/* endpoint count used by the MSC device */
#define EP_COUNT                      (3)

/* base address of the allocation buffer, used for buffer descriptor table and packet memory */
#define BUFFER_ADDRESS                (0x0000)

#endif /* USBD_CONF_H */
