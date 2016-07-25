/*!
    \file  main.c
    \brief ADC modresolution oversample
*/

/*
    Copyright (C) 2016 GigaDevice

    2014-12-26  V1.0.0, platform GD32F1x0(x=3,5)
    2016-01-15  V2.0.0, platform GD32F1x0(x=3,5,7,9)
    2016-04-30, V3.0.0, firmware update for GD32F1x0(x=3,5,7,9)

*/

#include "gd32f1x0.h"
#include "gd32f1x0_libopt.h"
#include <stdio.h>
#include "systick.h"
#include "gd32f1x0_eval.h"

#define BOARD_ADC_CHANNEL ADC_CHANNEL_10
#define ADC_GPIO_PIN GPIO_PIN_0

void rcu_config(void);
void gpio_config(void);
void adc_config(void);

/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{
    /* system clocks configuration */
    rcu_config();
    /* GPIO configuration */
    gpio_config();
    /* SYSTICK configuration */
    systick_config();
    /* ADC configuration */
    adc_config();
    /* configures COM port */
    gd_eval_COMinit(EVAL_COM1);

    while(1){
        /* ADC resolusion 12B */
        printf("\r\n resolusion 12B:\r\n");
        adc_resolution_config(ADC_RESOLUTION_12B);
        adc_oversample_mode_config(ADC_OVERSAMPLING_ALL_CONVERT,ADC_OVERSAMPLING_SHIFT_NONE,ADC_OVERSAMPLING_SHIFT_NONE);
        adc_oversample_mode_enable(DISABLE);
        /* 1 time sample */
        adc_enable();
        adc_software_trigger_enable(ADC_REGULAR_CHANNEL);
        delay_1ms(500);
        printf(" 1 time sample,the data is %d\r\n",ADC_RDATA);
        adc_disable();

        /* 16 times sample ,no shift */
        adc_oversample_mode_config(ADC_OVERSAMPLING_ALL_CONVERT,ADC_OVERSAMPLING_SHIFT_NONE,ADC_OVERSAMPLING_RATIO_16X);
        adc_oversample_mode_enable(ENABLE);
        adc_enable();
        adc_software_trigger_enable(ADC_REGULAR_CHANNEL);
        delay_1ms(500);
        printf(" 16 times sample,no shift,the data is %d\r\n",ADC_RDATA);
        adc_disable();

        /* 16 times sample ,4 bits shift */
        adc_oversample_mode_config(ADC_OVERSAMPLING_ALL_CONVERT,ADC_OVERSAMPLING_SHIFT_4B,ADC_OVERSAMPLING_RATIO_16X);
        adc_enable();
        adc_software_trigger_enable(ADC_REGULAR_CHANNEL);
        delay_1ms(500);
        printf(" 16 times sample,4 bits shift,the data is %d\r\n",ADC_RDATA);
        adc_disable();

        /* ADC resolusion 6B */
        printf("\r\n resolusion 6B:\r\n");
        adc_resolution_config(ADC_RESOLUTION_6B);
        adc_oversample_mode_config(ADC_OVERSAMPLING_ALL_CONVERT,ADC_OVERSAMPLING_SHIFT_NONE,ADC_OVERSAMPLING_SHIFT_NONE);
        adc_oversample_mode_enable(DISABLE);
        /* 1 time sample */
        adc_enable();
        adc_software_trigger_enable(ADC_REGULAR_CHANNEL);
        delay_1ms(500);
        printf(" 1 time sample,the data is %d\r\n",ADC_RDATA);
        adc_disable();

        /* 16 times sample ,no shift */
        adc_oversample_mode_config(ADC_OVERSAMPLING_ALL_CONVERT,ADC_OVERSAMPLING_SHIFT_NONE,ADC_OVERSAMPLING_RATIO_16X);
        adc_oversample_mode_enable(ENABLE);
        adc_enable();
        adc_software_trigger_enable(ADC_REGULAR_CHANNEL);
        delay_1ms(500);
        printf(" 16 times sample,no shift,the data is %d\r\n",ADC_RDATA);
        adc_disable();

        /* 16 times sample ,4 bits shift */
        adc_oversample_mode_config(ADC_OVERSAMPLING_ALL_CONVERT,ADC_OVERSAMPLING_SHIFT_4B,ADC_OVERSAMPLING_RATIO_16X);
        adc_enable();
        adc_software_trigger_enable(ADC_REGULAR_CHANNEL);
        delay_1ms(500);
        printf(" 16 times sample,4 bits shift,the data is %d\r\n",ADC_RDATA);
        adc_disable();
        while(1);
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
    /* enable GPIOC clock */
    rcu_periph_clock_enable(RCU_GPIOC);
    /* enable ADC clock */
    rcu_periph_clock_enable(RCU_ADC);
    /* enable DMA clock */
    rcu_periph_clock_enable(RCU_DMA);
    /* config ADC clock */
    rcu_adc_clock_config(RCU_ADCCK_APB2_DIV6);
}
/*!
    \brief      configure the gpio peripheral
    \param[in]  none
    \param[out] none
    \retval     none
*/
void gpio_config(void)
{
    /* config the GPIO as analog mode */
    gpio_mode_set(GPIOA,GPIO_MODE_ANALOG,GPIO_PUPD_NONE,ADC_GPIO_PIN);
}

/*!
    \brief      configure the adc peripheral
    \param[in]  none
    \param[out] none
    \retval     none
*/
void adc_config(void)
{
    /* ADC channel length config */
    adc_channel_length_config(ADC_REGULAR_CHANNEL,1);

    /* ADC regular channel config */
    adc_regular_channel_config(1,BOARD_ADC_CHANNEL,ADC_SAMPLETIME_239POINT5);

    /* ADC external trigger enable */
    adc_external_trigger_config(ADC_REGULAR_CHANNEL,ENABLE);
    /* ADC external trigger source config */
    adc_external_trigger_source_config(ADC_REGULAR_CHANNEL,ADC_EXTTRIG_REGULAR_SWRCST);
    /* ADC data alignment config */
    adc_data_alignment_config(ADC_DATAALIGN_RIGHT);
    /* enable ADC interface */
    adc_enable();
    /* ADC calibration and reset calibration */
    adc_calibration_enable();
    /* disable ADC interface */
    adc_disable();
}

/* retarget the C library printf function to the USART */
int fputc(int ch, FILE *f)
{
    usart_data_transmit(EVAL_COM1, (uint8_t) ch);
    while (RESET == usart_flag_get(EVAL_COM1,USART_STAT_TC));
    return ch;
}
