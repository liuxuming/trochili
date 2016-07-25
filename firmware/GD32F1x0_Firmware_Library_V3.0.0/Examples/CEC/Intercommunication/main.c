/*!
    \file  main.c
    \brief CEC intercommunication
*/

/*
    Copyright (C) 2016 GigaDevice

    2014-12-26  V1.0.0, platform GD32F1x0(x=3,5)
    2016-01-15   V2.0.0, platform GD32F1x0(x=3,5,7,9)
    2016-04-30, V3.0.0, firmware update for GD32F1x0(x=3,5,7,9)
*/

#include "gd32f1x0.h"
#include <stdlib.h>
#include <stdio.h>
#include "gd32f1x0_eval.h"
#include "systick.h"

void cec_config(void);

extern uint8_t rcvdata[10];
extern __IO uint8_t rcvstatus;
extern uint8_t rcv_inc;
extern uint8_t bytenum;
__IO uint8_t NUM=0;

/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{
    /* GPIO clock enable */
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_GPIOF);    

    /* delay time initialize */
    systick_config();

    /* configure the CEC peripheral */
    cec_config();

    /* configure the EXTI */
    gd_eval_keyinit(KEY_TAMPER, KEY_MODE_EXTI);
    
#ifdef GD32F130_150
    
    gd_eval_keyinit(KEY_USER, KEY_MODE_EXTI);

#elif defined(GD32F170_190)
    
    gd_eval_keyinit(KEY_WAKEUP, KEY_MODE_EXTI);
    
#endif /* GD32F130_150 */

    /* LED1 LED2 initialize */
    gd_eval_ledinit(LED1);
    gd_eval_ledinit(LED2);
    gd_eval_ledoff(LED1);
    gd_eval_ledoff(LED2);

    /* main loop */
    while(1){
        /* wait receive data */
        while(rcvstatus==0);

        if(rcvstatus == 1){
            if((rcvdata[1]==0xA5)&&(rcvdata[2]==0x5A)){
                gd_eval_ledon(LED1);
                delay_1ms(10);
                gd_eval_ledoff(LED1);
            }
            if((rcvdata[1]==0x5A)&&(rcvdata[2]==0xA5)){
                gd_eval_ledon(LED2);
                delay_1ms(10);
                gd_eval_ledoff(LED2);
            }
        }
        /* a reception error occured */
        rcvstatus = 0;
    }
}

/*!
    \brief      configure the CEC peripheral
    \param[in]  none
    \param[out] none
    \retval     none
*/
void cec_config(void)
{
    /* enable clocks */
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_CEC);

    /* configure CEC_LINE_GPIO as output open drain */
    gpio_mode_set(GPIOB,GPIO_MODE_AF,GPIO_PUPD_NONE,GPIO_PIN_8);
    gpio_output_options_set(GPIOB,GPIO_OTYPE_OD,GPIO_OSPEED_50MHZ,GPIO_PIN_8);
    gpio_af_set(GPIOB,GPIO_AF_0,GPIO_PIN_8);

    /* configure priority group */
    nvic_priority_group_set(NVIC_PRIGROUP_PRE1_SUB3);

    /* enable the CEC global interrupt (with higher priority) */
    nvic_irq_enable(CEC_IRQn,0,0);

    /* configure CEC */
    cec_init(CEC_SFT_START_SOM,CEC_SFT_1POINT5_PERIOD,CEC_OWN_ADDRESS2);
    cec_error_config(CEC_BROADCAST_ERROR_BIT_OFF,CEC_LONG_PERIOD_ERROR_BIT_OFF,CEC_RISING_PERIOD_ERROR_BIT_OFF,CEC_STOP_RISING_ERROR_BIT_OFF);
    cec_reception_tolerance_disable();

    /* activate CEC interrupts associated to the set of TX and RX flags */
    cec_interrupt_enable(CEC_INTEN_TENDIE | CEC_INTEN_TBRIE | CEC_INTEN_RENDIE | CEC_INTEN_RBRIE);

    /* activate CEC interrupts associated to the set of TX and RX error */
    cec_interrupt_enable(CEC_INTEN_ROIE | CEC_INTEN_RBREIE | CEC_INTEN_RSBPEIE | CEC_INTEN_RLBPEIE 
    | CEC_INTEN_RAEIE | CEC_INTEN_LSTARBIE | CEC_INTEN_TUIE | CEC_INTEN_TERRIE | CEC_INTEN_TAERRIE );

    /* enable CEC */
    cec_enable();
}
