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

#define USBD_CFG_MAX_NUM                  1
#define USBD_ITF_MAX_NUM                  1

/* USB feature -- Self Powered */
/* #define USBD_SELF_POWERED */

/* USB user string supported */
#define USB_SUPPORT_USER_STRING_DESC

/* DFU maximum data packet size */
#define TRANSFER_SIZE                     1024

/* memory address from where user application will be loaded, which represents 
   the dfu code protected against write and erase operations.*/
#define APP_LOADED_ADDR                   0x08004000

/* Make sure the corresponding memory where the DFU code should not be loaded
   cannot be erased or overwritten by DFU application. */
#define IS_PROTECTED_AREA(addr)  (uint8_t)(((addr >= 0x08000000) && (addr < (APP_LOADED_ADDR)))? 1 : 0)

/* DFU endpoint define */
#define DFU_IN_EP                         EP0_IN
#define DFU_OUT_EP                        EP0_OUT

/* Endpoint count used by the DFU device */
#define EP_COUNT                          (1)

/* Base address of the allocation buffer, used for buffer descriptor table and packet memory */
#define BUFFER_ADDRESS                    (0x0000)

#define USB_STRING_COUNT                  4

#define USB_PULLUP                        GPIOC
#define USB_PULLUP_PIN                    GPIO_PIN_2
#define RCC_AHBPeriph_GPIO_PULLUP         RCU_GPIOC

#endif /* USBD_CONF_H */
