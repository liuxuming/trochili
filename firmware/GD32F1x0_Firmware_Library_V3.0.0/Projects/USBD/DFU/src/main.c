/*!
    \file  main.c
    \brief USB device main routine
*/

/*
    Copyright (C) 2016 GigaDevice

    2014-09-06, V1.0.0, firmware for GD32F150
    2016-01-15, V1.0.1, firmware for GD32F150
    2016-04-30, V3.0.0, firmware update for GD32F150
*/

#include "usbd_std.h"
#include "dfu_core.h"

pAppFunction Application;
uint32_t AppAddress;

usbd_core_handle_struct  usb_device_dev = {
    .class_init = dfu_init,
    .class_deinit = dfu_deinit,
    .class_req_handler = dfu_req_handler,
    .class_data_handler = dfu_data_handler,
};

extern uint8_t usbd_serial_string[];

void rcu_config(void);
void gpio_config(void);
void nvic_config(void);
void serial_string_create(void);

/*!
    \brief      main routine will construct a DFU device
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{
    /* configure tamper key to run firmware */
    gd_eval_keyinit(KEY_TAMPER, KEY_MODE_GPIO);

    /* tamper key must be pressed on GD321x0-EVAL when power on */
    if(0 == gd_eval_keygetstate(KEY_TAMPER)){
        /* test if user code is programmed starting from address 0x08004000 */
        if (0x20000000 == ((*(__IO uint32_t*)APP_LOADED_ADDR) & 0x2FFE0000)) {
            AppAddress = *(__IO uint32_t*) (APP_LOADED_ADDR + 4);
            Application = (pAppFunction) AppAddress;

            /* initialize user application's stack pointer */
            __set_MSP(*(__IO uint32_t*) APP_LOADED_ADDR);

            /* jump to user application */
            Application();
        }
    }

    /* system clocks configuration */
    rcu_config();

    /* GPIO configuration */
    gpio_config();

    serial_string_create();

    usb_device_dev.dev_desc = (uint8_t *)&device_descripter;
    usb_device_dev.config_desc = (uint8_t *)&configuration_descriptor;
    usb_device_dev.strings = usbd_strings;

    /* USB device configuration */
    usbd_core_init(&usb_device_dev);

    /* NVIC configuration */
    nvic_config();

    /* enabled USB pull-up */
    gpio_bit_set(USB_PULLUP, USB_PULLUP_PIN);

    /* now the usb device is connected */
    usb_device_dev.status = USBD_CONNECTED;

    while (1);
}

/*!
    \brief      configure the different system clocks
    \param[in]  none
    \param[out] none
    \retval     none
*/
void rcu_config(void)
{
    /* enable USB pull-up pin clock */ 
    rcu_periph_clock_enable(RCC_AHBPeriph_GPIO_PULLUP);

    /* configure USB model clock from PLL clock */
    rcu_usbd_clock_config(RCU_USBD_CKPLL_DIV1_5);

    /* enable USB APB1 clock */
    rcu_periph_clock_enable(RCU_USBD);
}

/*!
    \brief      configure the gpio peripheral
    \param[in]  none
    \param[out] none
    \retval     none
*/
void gpio_config(void)
{
    /* configure usb pull-up pin */
    gpio_mode_set(USB_PULLUP, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, USB_PULLUP_PIN);
    gpio_output_options_set(USB_PULLUP, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, USB_PULLUP_PIN);
}

/*!
    \brief      configure interrupt priority
    \param[in]  none
    \param[out] none
    \retval     none
*/
void nvic_config(void)
{
    /* 1 bit for pre-emption priority, 3 bits for subpriority */
    nvic_priority_group_set(NVIC_PRIGROUP_PRE1_SUB3);

    /* enable the USB low priority interrupt */
    nvic_irq_enable(USBD_LP_IRQn, 1, 0);
}

/*!
    \brief      create the serial number string descriptor
    \param[in]  none
    \param[out] none
    \retval     none
*/
void  serial_string_create (void)
{
    uint32_t device_serial = *(uint32_t*)DEVICE_ID;

    if(0 != device_serial) {
        usbd_serial_string[2] = (uint8_t)device_serial;
        usbd_serial_string[3] = (uint8_t)(device_serial >> 8);
        usbd_serial_string[4] = (uint8_t)(device_serial >> 16);
        usbd_serial_string[5] = (uint8_t)(device_serial >> 24);
    }
}
