/*!
    \file  main.c
    \brief Master transmitter slave receiver interrupt
*/

/*
    Copyright (C) 2016 GigaDevice

    2014-12-26, V1.0.0, firmware for GD32F1x0(x=3,5)
    2016-01-15, V2.0.0, firmware for GD32F1x0(x=3,5,7,9)
    2016-04-30, V3.0.0, firmware update for GD32F1x0(x=3,5,7,9)
*/

/* Includes ------------------------------------------------------------------*/
#include "gd32f1x0.h"
#include <stdio.h>
#include "I2C_IE.h"
#include "gd32f1x0_eval.h"

uint8_t i2c_buffer_transmitter[16];
uint8_t i2c_buffer_receiver[16];

volatile uint8_t*       i2c_txbuffer;
volatile uint8_t*       i2c_rxbuffer;
volatile uint16_t       I2C_nBytes;
volatile ErrStatus      Status;
ErrStatus state=ERROR;

void rcu_config(void);
void gpio_config(void);
void i2c_config(void);
void i2c_nvic_config(void);
ErrStatus memory_compare(uint8_t* src, uint8_t* dst, uint16_t length) ;


/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{
    int i;
    /* initialize LED1,LED2,as the transfer instruction */
    gd_eval_ledinit(LED1);
    gd_eval_ledinit(LED2);
    rcu_config();
    gpio_config();
    i2c_config();
    i2c_nvic_config();

    for(i=0;i<16;i++){
        i2c_buffer_transmitter[i]=i+0x80;
    }

    i2c_txbuffer=i2c_buffer_transmitter;
    i2c_rxbuffer=i2c_buffer_receiver;
    I2C_nBytes=16;
    Status=ERROR;

    /* enable the I2C0 interrupt */
    i2c_interrupt_enable(I2C0, I2C_CTL1_ERRIE | I2C_CTL1_BUFIE | I2C_CTL1_EVIE);

    /* enable the I2C1 interrupt */
    i2c_interrupt_enable(I2C1, I2C_CTL1_ERRIE | I2C_CTL1_BUFIE | I2C_CTL1_EVIE);

    /* the master waits until the I2C bus is idle*/
    while(i2c_flag_get(I2C0, I2C_I2CBSY));

    /* the master sends a start condition to I2C bus */
    i2c_start_on_bus(I2C0);

    while((I2C_nBytes>0));
    while(SUCCESS != Status);
    /* if the transfer is successfully completed, LED1 and LED2 is on*/
    state=memory_compare(i2c_buffer_transmitter, i2c_buffer_receiver,16);
    if(SUCCESS==state){
        gd_eval_ledon(LED1);
        gd_eval_ledon(LED2);
    }else{
        gd_eval_ledoff(LED1);
        gd_eval_ledoff(LED2); 
    } 
    while(1);
}

/*!
    \brief      memory compare function
    \param[in]  src : source data
    \param[in]  dst : destination data
    \param[in]  length : the compare data length
    \param[out] none
    \retval     ErrStatus : ERROR or SUCCESS
*/
ErrStatus memory_compare(uint8_t* src, uint8_t* dst, uint16_t length) 
{
    while (length--){
        if (*src++ != *dst++){
            return ERROR;
        }
    }
    return SUCCESS;
}

/*!
    \brief      enable the peripheral clock
    \param[in]  none
    \param[out] none
    \retval     none
*/
void rcu_config(void)
{
    /* enable GPIOB clock */
    rcu_periph_clock_enable(RCU_GPIOB);
    /* enable BOARD_I2C APB1 clock */
    rcu_periph_clock_enable(RCU_I2C1);
    rcu_periph_clock_enable(RCU_I2C0);
}

/*!
    \brief      cofigure the GPIO ports.
    \param[in]  none
    \param[out] none
    \retval     none
*/
void gpio_config(void)
{

    /* I2C0 and I2C1 GPIO ports */
    /* connect PB6 to I2C0_SCL */
    gpio_af_set(GPIOB, GPIO_AF_1, GPIO_PIN_6);
    /* connect PB7 to I2C0_SDA */
    gpio_af_set(GPIOB, GPIO_AF_1, GPIO_PIN_7);
    /* connect PB10 to I2C1_SCL */
    gpio_af_set(GPIOB, GPIO_AF_1, GPIO_PIN_10);
    /* connect PB11 to I2C1_SDA */
    gpio_af_set(GPIOB, GPIO_AF_1, GPIO_PIN_11);

    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP,GPIO_PIN_6);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ,GPIO_PIN_6);
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP,GPIO_PIN_7);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ,GPIO_PIN_7);

    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP,GPIO_PIN_10);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ,GPIO_PIN_10);
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP,GPIO_PIN_11);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ,GPIO_PIN_11);
}

/*!
    \brief      cofigure the I2C0 and I2C1 interfaces..
    \param[in]  none
    \param[out] none
    \retval     none
*/
void i2c_config(void)
{
    /* I2C clock configure */
    i2c_clock_config(I2C0,100000,I2C_DTCY_2);
    /* I2C address configure */
    i2c_addr_config(I2C0,I2C_I2CMODE_ENABLE,I2C_ADDFORMAT_7BITS,I2C0_SLAVE_ADDRESS7);
    /* enable I2C0 */
    i2c_enable(I2C0);
    /* enable acknowledge */
    i2c_ack_config(I2C0,I2C_ACK_ENABLE);
    i2c_clock_config(I2C1,100000,I2C_DTCY_2);
    /* I2C address configure */
    i2c_addr_config(I2C1,I2C_I2CMODE_ENABLE,I2C_ADDFORMAT_7BITS,I2C1_SLAVE_ADDRESS7);
    /* enable I2C1 */
    i2c_enable(I2C1);
    /* enable Acknowledge */
    i2c_ack_config(I2C1,I2C_ACK_ENABLE);
}

/*!
    \brief      cofigure the NVIC peripheral
    \param[in]  none
    \param[out] none
    \retval     none
*/
void i2c_nvic_config(void)
{
    nvic_priority_group_set(NVIC_PRIGROUP_PRE1_SUB3);
    nvic_irq_enable(I2C0_EV_IRQn, 0, 3);
    nvic_irq_enable(I2C1_EV_IRQn, 0, 4);
    nvic_irq_enable(I2C0_ER_IRQn, 0, 2);
    nvic_irq_enable(I2C1_ER_IRQn, 0, 1);
}
