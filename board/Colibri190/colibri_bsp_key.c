/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                        www.trochili.com                                       *
 *************************************************************************************************/
#include "gd32f1x0.h"
#include "colibri_bsp_key.h"

void EvbKeyConfig(void)
{
    rcu_periph_clock_enable(RCU_GPIOB);
    gpio_mode_set(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO_PIN_6 | GPIO_PIN_7);

    /* enable and set key EXTI interrupt to the lowest priority */
    nvic_irq_enable(EXTI4_15_IRQn, 2, 0);

    rcu_periph_clock_enable(RCU_CFGCMP);
    /* connect key EXTI line to key GPIO pin */
    syscfg_exti_line_config(EXTI_SOURCE_GPIOB, EXTI_SOURCE_PIN6);
    syscfg_exti_line_config(EXTI_SOURCE_GPIOB, EXTI_SOURCE_PIN7);

    /* configure key EXTI line */
    exti_init(EXTI_6, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
    exti_init(EXTI_7, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
    exti_interrupt_flag_clear(EXTI_6);
    exti_interrupt_flag_clear(EXTI_7);
}

int EvbKeyScan(void)
{
    int count = 0xffff;

    if (RESET != exti_interrupt_flag_get(EXTI_6))
    {
        while(count--);
        if (RESET != exti_interrupt_flag_get(EXTI_6))
        {
            exti_interrupt_flag_clear(EXTI_6);
            return 1;
        }
    }

    if (RESET != exti_interrupt_flag_get(EXTI_7))
    {
        while(count--);
        if (RESET != exti_interrupt_flag_get(EXTI_7))
        {
            exti_interrupt_flag_clear(EXTI_7);
            return 2;
        }
    }

    return 0;
}
