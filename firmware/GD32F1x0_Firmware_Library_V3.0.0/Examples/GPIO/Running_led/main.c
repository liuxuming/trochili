/*!
    \file  main.c
    \brief running led
*/

/*
    Copyright (C) 2016 GigaDevice

    2014-12-26, V1.0.0, firmware for GD32F1x0(x=3,5)
    2016-01-15, V2.0.0, firmware for GD32F1x0(x=3,5,7,9)
    2016-04-30, V3.0.0, firmware update for GD32F1x0(x=3,5,7,9)
*/

#include "gd32f1x0.h"
#include "gd32f1x0_libopt.h"
#include "gd32f1x0_eval.h"
#include "systick.h"
#include <stdio.h>

/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{
    systick_config();
    gd_eval_ledinit(LED1);
    gd_eval_ledinit(LED2);
    gd_eval_ledinit(LED3);
    gd_eval_ledinit(LED4);

    while(1){
        /* turn on led1, turn off led4*/
        gd_eval_ledon(LED1);
        gd_eval_ledoff(LED4);
        delay_1ms(1000);
        /* turn on led2, turn off led1*/
        gd_eval_ledon(LED2);
        gd_eval_ledoff(LED1);
        delay_1ms(1000);
        /* turn on led3, turn off led2*/
        gd_eval_ledon(LED3);
        gd_eval_ledoff(LED2);
        delay_1ms(1000);
        /* turn on led4, turn off led3*/
        gd_eval_ledon(LED4);
        gd_eval_ledoff(LED3);
        delay_1ms(1000);
    }
}
