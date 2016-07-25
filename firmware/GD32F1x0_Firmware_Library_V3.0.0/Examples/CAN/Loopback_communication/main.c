/*!
    \file  main.c
    \brief CAN loopback communication in normal mode
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

/* select CAN */
#define CAN0_USED
//#define CAN1_USED

#ifdef  CAN0_USED
    #define CANX CAN0
#else 
    #define CANX CAN1
#endif
    
volatile ErrStatus test_flag;
ErrStatus test_flag_interrupt;

void nvic_config(void);
void led_config(void);
ErrStatus can_loopback(void);
ErrStatus can_loopback_interrupt(void);
void can_loopback_init(can_parameter_struct can_parameter, can_filter_parameter_struct can_filter);

/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{
    /* enable CAN clock */
    rcu_periph_clock_enable(RCU_CAN0);
    rcu_periph_clock_enable(RCU_CAN1);
    
    /* configure NVIC */
    nvic_config();
    
    /* configure leds */
    led_config();
    /* set all the leds off */
    gd_eval_ledoff(LED1);
    gd_eval_ledoff(LED2);
    gd_eval_ledoff(LED3);
    gd_eval_ledoff(LED4);
    /* loopback of polling */
    test_flag = can_loopback();
 
    if(SUCCESS == test_flag){
        /* loopback test is success */
        gd_eval_ledon(LED1);
        gd_eval_ledon(LED2);
    }else{
        /* loopback test is failed */
        gd_eval_ledoff(LED1);
        gd_eval_ledoff(LED2);
    }
    /* loopback of interrupt */
    test_flag_interrupt = can_loopback_interrupt();

    if(SUCCESS == test_flag_interrupt){
        /* interrupt loopback test is success */
        gd_eval_ledon(LED3);
        gd_eval_ledon(LED4);
    }else{
        /* interrupt loopback test is failed */
        gd_eval_ledoff(LED3);
        gd_eval_ledoff(LED4);
    }
    while (1);
}

/*!
    \brief      function for CAN loopback communication
    \param[in]  none
    \param[out] none
    \retval     ErrStatus
*/
ErrStatus can_loopback(void)
{
    can_parameter_struct            can_init_parameter;
    can_filter_parameter_struct     can_filter_parameter;
    can_trasnmit_message_struct     transmit_message;
    can_receive_message_struct      receive_message;
    uint32_t timeout = 0xFFFF;
    uint8_t transmit_mailbox = 0;
    /* initialize CAN */
    can_loopback_init(can_init_parameter, can_filter_parameter);

    /* initialize transmit message */
    transmit_message.can_tx_sfid = 0x11;
    transmit_message.can_tx_ft = CAN_FT_DATA;
    transmit_message.can_tx_ff = CAN_FF_STANDARD;
    transmit_message.can_tx_dlen = 2;
    transmit_message.can_tx_data[0] = 0xAB;
    transmit_message.can_tx_data[1] = 0xCD;
    
    /* transmit message */
    transmit_mailbox = can_transmit_message(CANX, &transmit_message);
    /* waiting for transmit completed */
    while((can_transmit_states(CANX, transmit_mailbox) != CAN_TRANSMIT_OK) && (timeout != 0)){
        timeout--;
    }
    timeout = 0xFFFF;
    /* waiting for receive completed */
    while((can_receive_message_length(CANX, CAN_FIFO0) < 1) && (timeout != 0)){
        timeout--; 
    }

    /* initialize receive message*/
    receive_message.can_rx_sfid=0x00;
    receive_message.can_rx_ff=0;
    receive_message.can_rx_dlen=0;
    receive_message.can_rx_data[0]=0x00;
    receive_message.can_rx_data[1]=0x00;
    can_receive_message(CANX, CAN_FIFO0, &receive_message);
    
    /* check the receive message */
    if((receive_message.can_rx_sfid==0x11) && (receive_message.can_rx_ff==CAN_FF_STANDARD)
       && (receive_message.can_rx_dlen==2) && ((receive_message.can_rx_data[1]<<8|receive_message.can_rx_data[0])==0xCDAB)){
        return SUCCESS;
    }else{
        return ERROR;
    }
}

/*!
    \brief      function for CAN loopback interrupt communication
    \param[in]  none
    \param[out] none
    \retval     ErrStatus
*/
ErrStatus can_loopback_interrupt(void)
{
    can_parameter_struct         can_init_parameter;
    can_filter_parameter_struct  can_filter_parameter;
    can_trasnmit_message_struct transmit_message;
    uint32_t timeout = 0x0000FFFF;
    
    /* initialize CAN and filter */
    can_loopback_init(can_init_parameter, can_filter_parameter);

    /* enable CAN receive FIFO0 not empty interrupt  */ 
    can_interrupt_enable(CANX, CAN_INTEN_RFNEIE0);

    /* initialize transmit message */
    transmit_message.can_tx_sfid=0;
    transmit_message.can_tx_efid=0x1234;
    transmit_message.can_tx_ff=CAN_FF_EXTENDED;
    transmit_message.can_tx_ft=CAN_FT_DATA;
    transmit_message.can_tx_dlen=2;
    transmit_message.can_tx_data[0]=0xDE;
    transmit_message.can_tx_data[1]=0xCA;
    /* transmit a message */
    can_transmit_message(CANX, &transmit_message);
    
    /* waiting for receive completed */
    while((test_flag_interrupt != SUCCESS) && (timeout != 0)){
        timeout--;
    }
    if(0 == timeout){
        test_flag_interrupt = ERROR;
    }

    /* disable CAN receive FIFO0 not empty interrupt  */ 
    can_interrupt_disable(CANX, CAN_INTEN_RFNEIE0);
    
    return test_flag_interrupt;
}

/*!
    \brief      initialize CAN and filter
    \param[in]  can_parameter
      \arg        can_parameter_struct
    \param[in]  can_filter
      \arg        can_filter_parameter_struct
    \param[out] none
    \retval     none
*/
void can_loopback_init(can_parameter_struct can_parameter, can_filter_parameter_struct can_filter)
{
    /* initialize CAN register */
    can_deinit(CANX);
    
    /* initialize CAN */
    can_parameter.can_ttc=DISABLE;
    can_parameter.can_abor=DISABLE;
    can_parameter.can_awu=DISABLE;
    can_parameter.can_ard=DISABLE;
    can_parameter.can_rfod=DISABLE;
    can_parameter.can_tfo=DISABLE;
    can_parameter.can_mode=CAN_LOOPBACK_MODE;
    /* configure baudrate to 125kbps */
    can_parameter.can_sjw=CAN_BT_SJW_1TQ;
    can_parameter.can_bs1=CAN_BT_BS1_3TQ;
    can_parameter.can_bs2=CAN_BT_BS2_2TQ;
    can_parameter.can_psc=96;
    can_init(CANX, &can_parameter);

    /* initialize filter */
#ifdef  CAN0_USED
    /* CAN0 filter number */
    can_filter.can_filter_number=0;
#else
    /* CAN1 filter number */
    can_filter.can_filter_number=15;
#endif
    /* initialize filter */    
    can_filter.can_filter_mode = CAN_FILTERMODE_MASK;
    can_filter.can_filter_bits = CAN_FILTERBITS_32BIT;
    can_filter.can_filter_list_high = 0x0000;
    can_filter.can_filter_list_low = 0x0000;
    can_filter.can_filter_mask_high = 0x0000;
    can_filter.can_filter_mask_low = 0x0000;  
    can_filter.can_filter_fifo_number = CAN_FIFO0;
    can_filter.can_filter_enable=ENABLE;
    can_filter_init(&can_filter);
}

/*!
    \brief      configure the nested vectored interrupt controller
    \param[in]  none
    \param[out] none
    \retval     none
*/
void nvic_config(void)
{
#ifdef  CAN0_USED
    /* configure CAN0 NVIC */
    nvic_irq_enable(CAN0_RX0_IRQn,0,0);
#else
    /* configure CAN1 NVIC */
    nvic_irq_enable(CAN1_RX0_IRQn,0,0);
#endif

}

/*!
    \brief      configure the leds
    \param[in]  none
    \param[out] none
    \retval     none
*/
void led_config(void)
{
    gd_eval_ledinit (LED1);
    gd_eval_ledinit (LED2);
    gd_eval_ledinit (LED3);
    gd_eval_ledinit (LED4);
}
