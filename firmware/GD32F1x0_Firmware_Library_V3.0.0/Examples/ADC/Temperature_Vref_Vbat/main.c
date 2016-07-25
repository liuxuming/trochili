/*!
    \file  main.c
    \brief ADC channel of temperature,Vref and Vbat
*/

/*
    Copyright (C) 2016 GigaDevice

    2014-12-26, V1.0.0, platform GD32F1x0(x=3,5)
    2016-01-15, V2.0.0, platform GD32F1x0(x=3,5,7,9)
    2016-04-30, V3.0.0, firmware update for GD32F1x0(x=3,5,7,9)
*/

#include "gd32f1x0.h"
#include <stdio.h>
#include "gd32f1x0_eval.h"

void rcu_config(void);
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
    /* ADC configuration */
    adc_config();
    /* USART configuration */
    gd_eval_COMinit(EVAL_COM1);

    printf(" the temperature data is %d\r\n",ADC_IDATA0);
    printf(" the reference voltage data is %d\r\n",ADC_IDATA1);
    printf(" the battery voltage is %d\r\n",ADC_IDATA2);
    while(1);
}

/*!
    \brief      configure the different system clocks
    \param[in]  none
    \param[out] none
    \retval     none
*/
void rcu_config(void)
{
    /* enable ADC clock */
    rcu_periph_clock_enable(RCU_ADC);
    /* config ADC clock */
    rcu_adc_clock_config(RCU_ADCCK_APB2_DIV6);
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
    adc_channel_length_config(ADC_INSERTED_CHANNEL,3);

    /* ADC regular channel config */
    adc_inserted_channel_config(1,ADC_CHANNEL_TEMPSENSOR,ADC_SAMPLETIME_239POINT5);
    adc_inserted_channel_config(2,ADC_CHANNEL_VREFINT,ADC_SAMPLETIME_239POINT5);
    adc_inserted_channel_config(3,ADC_CHANNEL_VBAT,ADC_SAMPLETIME_239POINT5);

    /* ADC external trigger enable */
    adc_external_trigger_config(ADC_INSERTED_CHANNEL,ENABLE);
    /* ADC external trigger source config */
    adc_external_trigger_source_config(ADC_INSERTED_CHANNEL,ADC_EXTTRIG_INSERTED_SWRCST);
    /* ADC data alignment config */
    adc_data_alignment_config(ADC_DATAALIGN_RIGHT);
    /* ADC SCAN function enable */
    adc_special_function_config(ADC_SCAN_MODE,ENABLE);
    /* ADC Vbat channel enable */
    adc_special_function_config(ADC_VBAT_CHANNEL_SWITCH,ENABLE);
    /* ADC temperature and Vref enable */
    adc_special_function_config(ADC_TEMP_VREF_CHANNEL_SWITCH,ENABLE);
    /* enable ADC interface */
    adc_enable();
    /* ADC calibration and reset calibration */
    adc_calibration_enable();

    /* ADC software trigger enable */
    adc_software_trigger_enable(ADC_INSERTED_CHANNEL);
}

/* retarget the C library printf function to the USART */
int fputc(int ch, FILE *f)
{
    usart_data_transmit(EVAL_COM1, (uint8_t) ch);
    while (RESET == usart_flag_get(EVAL_COM1,USART_STAT_TC));
    return ch;
}
