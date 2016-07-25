/*!
    \file  main.c
    \brief construct a USB keyboard
*/

/*
    Copyright (C) 2016 GigaDevice

    2014-09-06, V1.0.0, firmware for GD32F150
    2016-01-15, V1.0.1, firmware for GD32F150
    2016-04-30, V3.0.0, firmware update for GD32F150
*/

#include "usbd_std.h"
#include "hid_core.h"

#define USB_PULLUP                      GPIOC
#define USB_PULLUP_PIN                  GPIO_PIN_2
#define RCC_AHBPeriph_GPIO_PULLUP       RCU_GPIOC

typedef enum
{
    CHAR_A = 1,
    CHAR_B,
    CHAR_C
}Key_Char;

__IO uint8_t prev_transfer_complete = 1;

uint8_t key_buffer[HID_IN_PACKET] = {0};

usbd_core_handle_struct  usb_device_dev = 
{
    .dev_desc = (uint8_t *)&device_descripter,
    .config_desc = (uint8_t *)&configuration_descriptor,
    .strings = usbd_strings,
    .class_init = hid_init,
    .class_deinit = hid_deinit,
    .class_req_handler = hid_req_handler,
    .class_data_handler = hid_data_handler
};

void rcu_config(void);
void key_config(void);
void gpio_config(void);
void nvic_config(void);
uint8_t key_state(void);

/*!
    \brief      main routine
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{
    /* system clocks configuration */
    rcu_config();

    /* keys configuration */
    key_config();

    /* GPIO configuration */
    gpio_config();

    /* USB device configuration */
    usbd_core_init(&usb_device_dev);

    /* NVIC configuration */
    nvic_config();

    /* enabled USB pull-up */
    gpio_bit_set(USB_PULLUP, USB_PULLUP_PIN);

    /* now the usb device is connected */
    usb_device_dev.status = USBD_CONNECTED;

    while(usb_device_dev.status != USBD_CONFIGURED);

    while (1) {
        if (prev_transfer_complete) {
            switch (key_state()) {
            case CHAR_A:
                key_buffer[2] = 0x04;
                break;
            case CHAR_B:
                key_buffer[2] = 0x05;
                break;
            case CHAR_C:
                key_buffer[2] = 0x06;
                break;
            default:
                break;
            }

            if (0 != key_buffer[2]) {
                hid_report_send(&usb_device_dev, key_buffer, HID_IN_PACKET);
            }
        }
    }
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

    /* enable the power clock */
    rcu_periph_clock_enable(RCU_PMU);

    /* configure USB model clock from PLL clock */
    rcu_usbd_clock_config(RCU_USBD_CKPLL_DIV1_5);

    /* enable USB APB1 clock */
    rcu_periph_clock_enable(RCU_USBD);
}

/*!
    \brief      configure the keys
    \param[in]  none
    \param[out] none
    \retval     none
*/
void key_config(void)
{
    /* keys configuration */
    gd_eval_keyinit(KEY_TAMPER, KEY_MODE_GPIO);
    gd_eval_keyinit(KEY_WAKEUP, KEY_MODE_GPIO);
    gd_eval_keyinit(KEY_USER, KEY_MODE_GPIO);

    gd_eval_keyinit(KEY_TAMPER, KEY_MODE_EXTI);
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

    /* USB wakeup EXTI line configuration */
    exti_interrupt_flag_clear(EXTI_18);
    exti_init(EXTI_18, EXTI_INTERRUPT, EXTI_TRIG_RISING);
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

    /* enable the USB Wake-up interrupt */
    nvic_irq_enable(USBDWakeUp_IRQChannel, 0, 0);
}

/*!
    \brief      to get usb keyboard state
    \param[in]  none
    \param[out] none
    \retval     the char
*/
uint8_t  key_state (void)
{
    /* have pressed tamper key */
    if (!gd_eval_keygetstate(KEY_TAMPER)) {
        return CHAR_A;
    }

    /* have pressed wakeup key */
    if (!gd_eval_keygetstate(KEY_WAKEUP)) {
        return CHAR_B;
    }

    /* have pressed user key */
    if (!gd_eval_keygetstate(KEY_USER)) {
        return CHAR_C;
    }

    /* no pressed any key */
    return 0;
}
