/*!
    \file  main.c
    \brief TIMER0 dma demo
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

#define TIMER0_CH0CC  ((uint32_t)0x040012c34)
uint16_t buffer[3]={249,499,749};

void gpio_config(void);
void timer_config(void);
void dma_config(void);

/**
    \brief      configure the GPIO ports
    \param[in]  none
    \param[out] none
    \retval     none
  */
void gpio_config(void)
{
    rcu_periph_clock_enable(RCU_GPIOA);

    /*configure PA8(TIMER0 CH0) as alternate function*/
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_8);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_8);

    gpio_af_set(GPIOA, GPIO_AF_2, GPIO_PIN_8);
}


/**
    \brief      configure the DMA peripheral
    \param[in]  none
    \param[out] none
    \retval     none
  */
void dma_config(void)
{
    dma_parameter_struct dma_init_struct;

    /* enable DMA clock */
    rcu_periph_clock_enable(RCU_DMA);

    /* initialize DMA channel4 */
    dma_deinit(DMA_CH4);

    /* DMA channel4 initialize */
    dma_deinit(DMA_CH4);
    dma_init_struct.direction    = DMA_MEMORY_TO_PERIPHERA;
    dma_init_struct.memory_addr  = (uint32_t)buffer;
    dma_init_struct.memory_inc   = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_16BIT;
    dma_init_struct.number       = 3;
    dma_init_struct.periph_addr  = (uint32_t)TIMER0_CH0CC;
    dma_init_struct.periph_inc   = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_16BIT;
    dma_init_struct.priority     = DMA_PRIORITY_HIGH;
    dma_init(DMA_CH4,dma_init_struct);

    /* configure DMA mode */
    dma_circulation_enable(DMA_CH4);
    dma_memory_to_memory_disable(DMA_CH4);

    /* enable DMA channel4 */
    dma_channel_enable(DMA_CH4);
}

/**
    \brief      configure the TIMER peripheral
    \param[in]  none
    \param[out] none
    \retval     none
  */
void timer_config(void)
{
    /* TIMER0 DMA Transfer example -------------------------------------------------
    TIMER0CLK = 72MHz, Prescaler = 72 
    TIMER0 counter clock = systemcoreclock/72 = 1MHz.

    the objective is to configure TIMER0 channel 1 to generate PWM
    signal with a frequency equal to 1KHz and a variable duty cycle(25%,50%,75%) that is 
    changed by the DMA after a specific number of update DMA request.

    the number of this repetitive requests is defined by the TIMER0 repetition counter,
    each 2 update requests, the TIMER0 Channel 0 duty cycle changes to the next new 
    value defined by the buffer . 
    -----------------------------------------------------------------------------*/
    timer_oc_parameter_struct timer_ocintpara;
    timer_parameter_struct timer_initpara;

    rcu_periph_clock_enable(RCU_TIMER0);

    timer_deinit(TIMER0);

    /* TIMER0 configuration */
    timer_initpara.timer_prescaler         = 71;
    timer_initpara.timer_alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.timer_counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.timer_period            = 999;
    timer_initpara.timer_clockrivision     = TIMER_CKDIV_DIV1;
    timer_initpara.timer_repetitioncounter = 1;
    timer_init(TIMER0,&timer_initpara);

    /* CH0 configuration in PWM1 mode */
    timer_ocintpara.timer_outputState  = TIMER_CCX_ENABLE;
    timer_ocintpara.timer_outputnState = TIMER_CCXN_ENABLE;
    timer_ocintpara.timer_ocpolarity   = TIMER_OC_POLARITY_HIGH;
    timer_ocintpara.timer_ocnpolarity  = TIMER_OCN_POLARITY_HIGH;
    timer_ocintpara.timer_ocidleState  = TIMER_OC_IDLE_STATE_HIGH;
    timer_ocintpara.timer_ocnidleState = TIMER_OCN_IDLE_STATE_LOW;
    timer_channel_output_config(TIMER0,TIMER_CH_0,&timer_ocintpara);

    timer_channel_output_pulse_value_config(TIMER0,TIMER_CH_0,buffer[0]);
    timer_channel_output_mode_config(TIMER0,TIMER_CH_0,TIMER_OC_MODE_PWM1);
    timer_channel_output_shadow_config(TIMER0,TIMER_CH_0,TIMER_OC_SHADOW_DISABLE);

    /* TIMER0 primary output enable */
    timer_primary_output_config(TIMER0,ENABLE);

    /* TIMER0 update DMA request enable */
    timer_DMA_enable(TIMER0,TIMER_DMA_UPDEN);

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
    dma_config();
    timer_config();

    while (1);
}
