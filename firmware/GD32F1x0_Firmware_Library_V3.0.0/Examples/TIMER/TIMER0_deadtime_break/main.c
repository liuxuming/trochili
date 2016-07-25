/*!
    \file  main.c
    \brief TIMER0 deadtime break demo
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
    rcu_periph_clock_enable(RCU_GPIOB);

    /*configure PA8(TIMER0 CH0) as alternate function*/
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_8);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_8);
    gpio_af_set(GPIOA, GPIO_AF_2, GPIO_PIN_8);

    /*configure PB13(TIMER0 CH0N) as alternate function*/
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_13);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_13);
    gpio_af_set(GPIOB, GPIO_AF_2, GPIO_PIN_13);

    /*configure PA6(TIMER0 BKIN) as alternate function*/
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_6);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_6);
    gpio_af_set(GPIOA, GPIO_AF_2, GPIO_PIN_6);
}

/**
    \brief      configure the TIMER peripheral
    \param[in]  none
    \param[out] none
    \retval     none
  */
void timer_config(void)
{
    /* -----------------------------------------------------------------------
    TIMER0 configuration:
    generate 1 complementary PWM signal.
    TIMER0CLK is fixed to systemcoreclock, the TIMER0 prescaler is equal to 2 so the
    TIMER0 counter clock used is 36MHz.
    the duty cycle is computed as the following description:
    the channel 0 duty cycle is set to 25% so channel 0N is set to 75%.

    insert a dead time equal to ((32+31)*16*4)/systemcoreclock = 56us

    configure the break feature, active at high level, and using the automatic
    output enable feature.

    use the locking parameters level1.
    ----------------------------------------------------------------------- */
    timer_oc_parameter_struct timer_ocintpara;
    timer_parameter_struct timer_initpara;
    timer_break_parameter_struct timer_breakpara;

    rcu_periph_clock_enable(RCU_TIMER0);

    timer_deinit(TIMER0);

    /* TIMER0 configuration */
    timer_initpara.timer_prescaler         = 1;
    timer_initpara.timer_alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.timer_counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.timer_period            = 15999;
    timer_initpara.timer_clockrivision     = TIMER_CKDIV_DIV4;
    timer_initpara.timer_repetitioncounter = 0;
    timer_init(TIMER0,&timer_initpara);

    /* CH0/CH0N configuration in PWM mode1 */
    timer_ocintpara.timer_outputState  = TIMER_CCX_ENABLE;
    timer_ocintpara.timer_outputnState = TIMER_CCXN_ENABLE;
    timer_ocintpara.timer_ocpolarity   = TIMER_OC_POLARITY_HIGH;
    timer_ocintpara.timer_ocnpolarity  = TIMER_OCN_POLARITY_HIGH;
    timer_ocintpara.timer_ocidleState  = TIMER_OC_IDLE_STATE_HIGH;
    timer_ocintpara.timer_ocnidleState = TIMER_OCN_IDLE_STATE_LOW;

    timer_channel_output_config(TIMER0,TIMER_CH_0,&timer_ocintpara);

    timer_channel_output_pulse_value_config(TIMER0,TIMER_CH_0,3999);
    timer_channel_output_mode_config(TIMER0,TIMER_CH_0,TIMER_OC_MODE_PWM1);
    timer_channel_output_shadow_config(TIMER0,TIMER_CH_0,TIMER_OC_SHADOW_DISABLE);

    /* automatic output enable, break, dead time and lock configuration*/
    timer_breakpara.timer_rosstate      = TIMER_ROS_STATE_DISABLE;
    timer_breakpara.timer_iosstate      = TIMER_IOS_STATE_DISABLE ;
    timer_breakpara.timer_deadtime      = 255;
    timer_breakpara.timer_breakpolarity = TIMER_BREAK_POLARITY_LOW;
    timer_breakpara.timer_outauto       = TIMER_OUTAUTO_ENABLE;
    timer_breakpara.timer_protmode      = TIMER_CCHP_PROT_0;
    timer_breakpara.timer_state         = TIMER_BREAK_ENABLE;
    timer_break_config(TIMER0,&timer_breakpara);

    /* TIMER0 primary output function enable */
    timer_primary_output_config(TIMER0,ENABLE);

    /* auto-reload preload enable */
    timer_auto_reload_shadow_enable(TIMER0);

    /* TIMER0 counter enable */
    timer_enable(TIMER0);
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
    gd_eval_ledinit(LED1);
    timer_config();

    while (1);
}
