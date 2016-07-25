/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                        www.trochili.com                                       *
 *************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "gd32f1x0.h"
#include "colibri_bsp_uart.h"

void EvbUart1Config(void)
{
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_USART1);

    /* connect port to USART1_Tx */
    gpio_af_set(GPIOA, GPIO_AF_1, GPIO_PIN_2);

    /* connect port to USARTx_R1 */
    gpio_af_set(GPIOA, GPIO_AF_1, GPIO_PIN_3);

    /* configure USART1 Tx as alternate function push-pull */
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP,GPIO_PIN_2);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_10MHZ,GPIO_PIN_2);

    /* configure USART1 Rx as alternate function push-pull */
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP,GPIO_PIN_3);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_10MHZ,GPIO_PIN_3);

    /* USART1 configure */
    usart_deinit(USART1);
    usart_baudrate_set(USART1,115200);
    usart_transfer_config(USART1,USART_RTEN_ENABLE);
}


/*************************************************************************************************
 *  功能：向串口1发送一个字符                                                                    *
 *  参数：(1) 需要被发送的字符                                                                   *
 *  返回：                                                                                       *
 *  说明：                                                                                       *
 *************************************************************************************************/
void EvbUart1WriteByte(char c)
{
    usart_data_transmit(USART1, c);
    while (RESET == usart_flag_get(USART1,USART_STAT_TC));
}

/*************************************************************************************************
 *  功能：向串口1发送一个字符串                                                                  *
 *  参数：(1) 需要被发送的字符串                                                                 *
 *  返回：                                                                                       *
 *  说明：                                                                                       *
 *************************************************************************************************/
void EvbUart1WriteStr(const char* str)
{

    while (*str)
    {
        usart_data_transmit(USART1, * str++);
        while (RESET == usart_flag_get(USART1,USART_STAT_TC));
    }
}


/*************************************************************************************************
 *  功能：从串口1接收一个字符                                                                    *
 *  参数：(1) 存储接收到的字符                                                                   *
 *  返回：                                                                                       *
 *  说明：                                                                                       *
 *************************************************************************************************/
void EvbUart1ReadByte(char* c)
{
    while (RESET == usart_flag_get(USART1,USART_STAT_RBNE));
    *c = (usart_data_receive(USART1));
}


static char _buffer[256];
void EvbUart1Printf(char* fmt, ...)
{
    int i;
    va_list ap;
    va_start(ap, fmt);
    vsprintf(_buffer, fmt, ap);
    va_end(ap);

    for (i = 0; _buffer[i] != '\0'; i++)
    {
        EvbUart1WriteByte(_buffer[i]);
    }
}

