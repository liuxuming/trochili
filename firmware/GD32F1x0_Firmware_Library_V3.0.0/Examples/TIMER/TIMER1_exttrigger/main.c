/*!
    \file  main.c
    \brief TIMER1 external trigger demo
*/

/*
    Copyright (C) 2016 GigaDevice

    2014-12-26, V1.0.0, firmware for GD32F1x0(x=3,5)
    2016-01-15, V2.0.0, firmware for GD32F1x0(x=3,5,7,9)
    2016-04-30, V3.0.0, firmware update for GD32F1x0(x=3,5,7,9)
*/

#include "gd32f1x0.h"
#include <stdio.h>
#include "gd32f1x0_eval.h"

void gpio_config(void);
void timer_config(void);

/**
    \brief      configure the GPIO ports
    \param[in]  none
    \param[out] none
    \retval     none
  */
void gpio_config(void)
{
    rcu_periph_clock_enable(RCU_GPIOA);

    /*configure PA2(TIMER1 CH2) as alternate function*/
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_2);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_2);
    gpio_af_set(GPIOA, GPIO_AF_2, GPIO_PIN_2);

    /*configure PA5(TIMER1 CH0) as alternate function*/
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_5);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_5);
    gpio_af_set(GPIOA, GPIO_AF_2, GPIO_PIN_5);
}

/**
    \brief      configure the TIMER peripheral
    \param[in]  none
    \param[out] none
    \retval     none
  */
void timer_config(void)
{
    /* Timer with an external trigger -------------------------------------
    TIMER1 is configured as slave timer for an external trigger connected
    to TIMER1 TI0 pin :
    - The TIMER1 TI0FP0 is used as trigger input
    - rising edge is used to start the TIMER1: trigger mode.
    - TIMER1 CH2 is used PWM2 Mode 
    the starts of the TIMER1 counter are controlled by the 
    external trigger.  
    -------------------------------------------------------------------- */
    timer_oc_parameter_struct timer_ocinitpara;
    timer_parameter_struct timer_initpara;
    timer_ic_parameter_struct timer_icinitpara;

    rcu_periph_clock_enable(RCU_TIMER1);

    timer_deinit(TIMER1);

    /* TIMER1 configuration */
    timer_initpara.timer_prescaler         = 35999;
    timer_initpara.timer_alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.timer_counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.timer_period            = 7999;
    timer_initpara.timer_clockrivision     = TIMER_CKDIV_DIV1;
    timer_initpara.timer_repetitioncounter = 0;
    timer_init(TIMER1,&timer_initpara);

     /* CH2 configuration in OC PWM2 mode */
    timer_ocinitpara.timer_ocpolarity   = TIMER_OC_POLARITY_HIGH;
    timer_ocinitpara.timer_ocnpolarity  = TIMER_OCN_POLARITY_HIGH;
    timer_ocinitpara.timer_outputState  = TIMER_CCX_ENABLE;
    timer_ocinitpara.timer_outputnState = TIMER_CCXN_DISABLE;
    timer_ocinitpara.timer_ocidleState  = TIMER_OC_IDLE_STATE_LOW;
    timer_ocinitpara.timer_ocnidleState = TIMER_OCN_IDLE_STATE_LOW;
    timer_channel_output_config(TIMER1,TIMER_CH_2,&timer_ocinitpara);

    timer_channel_output_pulse_value_config(TIMER1,TIMER_CH_2,3999);
    timer_channel_output_mode_config(TIMER1,TIMER_CH_2,TIMER_OC_MODE_PWM1);
    timer_channel_output_shadow_config(TIMER1,TIMER_CH_2,TIMER_OC_SHADOW_DISABLE);

    /* TIMER1 CH0 input capture configuration */
    timer_icinitpara.timer_icpolarity  = TIMER_IC_POLARITY_RISING;
    timer_icinitpara.timer_icselection = TIMER_IC_SELECTION_DIRECTTI;
    timer_icinitpara.timer_icprescaler = TIMER_IC_PSC_DIV1;
    timer_icinitpara.timer_icfilter    = 0x02;
    timer_input_capture_config(TIMER1,TIMER_CH_0,&timer_icinitpara);

    /* slave mode selection : TIMER1 */
    /* TIMER1 input trigger : external trigger connected to CI0 */
    timer_input_trigger_source_select(TIMER1,TIMER_SMCFG_TRGS_CI0FE0);
    timer_slave_mode_select(TIMER1,TIMER_SLAVE_MODE_EVENT);

    /* auto-reload preload enable */
    timer_auto_reload_shadow_enable(TIMER1);
}

/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{
    gpio_config();
    timer_config();

    while (1);
}
