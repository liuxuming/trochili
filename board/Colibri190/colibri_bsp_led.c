/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#include "gd32f1x0.h"
#include "colibri_bsp_led.h"

/*************************************************************************************************
 *  功能：初始化用户Led设备                                                                      *
 *  参数：无                                                                                     *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void EvbLedConfig(void)
{
    rcu_periph_clock_enable(RCU_GPIOB);
    gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_8 | GPIO_PIN_9 |
                            GPIO_PIN_10);
}



/*************************************************************************************************
 *  功能：控制Led的点亮和熄灭                                                                    *
 *  参数：(1) index Led灯编号                                                                    *
 *        (2) cmd   Led灯点亮或者熄灭的命令                                                      *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void EvbLedControl(int index, int cmd)
{
    switch (index)
    {
        case LED1:
        {
            if (cmd == LED_ON)
            {
                gpio_bit_set(GPIOB, GPIO_PIN_8); /*点亮Led1灯*/
            }
            else
            {
                gpio_bit_reset(GPIOB, GPIO_PIN_8); /*熄灭Led1灯*/
            }
            break;
        }
        case LED2:
        {
            if (cmd == LED_ON)
            {
                gpio_bit_set(GPIOB, GPIO_PIN_9); /*点亮Led2灯*/
            }
            else
            {
                gpio_bit_reset(GPIOB, GPIO_PIN_9); /*熄灭Led2灯*/
            }
            break;
        }
        case LED3:
        {
            if (cmd == LED_ON)
            {
                gpio_bit_set(GPIOB, GPIO_PIN_10); /*点亮Led3灯*/
            }
            else
            {
                gpio_bit_reset(GPIOB, GPIO_PIN_10); /*熄灭Led3灯*/
            }
            break;
        }
        default:
        {
            break;
        }
    }
}
