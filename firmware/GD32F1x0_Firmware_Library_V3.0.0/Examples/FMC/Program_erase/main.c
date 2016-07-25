/*!
    \file  main.c
    \brief main flash program, erase and reprogram
*/

/*
    Copyright (C) 2016 GigaDevice

    2014-12-26, V1.0.0, firmware for GD32F1x0(x=3,5)
    2016-01-15, V2.0.0, firmware for GD32F1x0(x=3,5,7,9)
    2016-04-30, V3.0.0, firmware update for GD32F1x0(x=3,5,7,9)
*/

#include "gd32f1x0.h"
#include "systick.h"
#include <stdio.h>
#include "main.h"
#include "gd32f1x0_eval.h"

#define BufferSize              2048
#define FMC_PAGE_SIZE           ((uint16_t)0x400)
#define BANK1_WRITE_START_ADDR  ((uint32_t)0x08004000)
#define BANK1_WRITE_END_ADDR    ((uint32_t)0x08004800)

uint32_t NbrOfPage = 0x00;
uint32_t *ptrd;
uint32_t EraseCounter = 0x00;
uint32_t address = 0x00 ;
uint32_t data0 = 0x01234567;
uint32_t data1 = 0xd583179b;
uint32_t i;
led_typedef_enum lednum = LED4;

#define FMC_REPROGRAM

/*!
    \brief      toggle the led every 500ms
    \param[in]  lednum: led num
      \arg        LED1: led1 on the board
      \arg        LED2: led2 on the board
      \arg        LED3: led3 on the board
      \arg        LED4£ºled4 on the board
    \param[out] none
    \retval     none
*/
void led_spark(led_typedef_enum lednum)
{
    static __IO uint32_t timingdelay = 0;

    if(0x0 != timingdelay){
        if(timingdelay < 500){
            gd_eval_ledon(lednum);
        }else{
            gd_eval_ledoff(lednum);
        }
        timingdelay--;
    }else{
        timingdelay = 1000;
    }
}


/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{
    extern led_typedef_enum lednum;

    /* initialize led on the board */
    gd_eval_ledinit(LED1);
    gd_eval_ledinit(LED2);
    gd_eval_ledinit(LED3);
    gd_eval_ledinit(LED4);

    systick_config();

#ifdef FMC_PROGRAM

    /* unlock the flash program erase controller */
    fmc_unlock();

    address = BANK1_WRITE_START_ADDR;

    /* program flash */
    while(address < BANK1_WRITE_END_ADDR){
        fmc_word_program(address, data0);
        address = address + 4; 
        fmc_flag_clear(FMC_FLAG_END | FMC_FLAG_WPERR | FMC_FLAG_PGERR );
    }
    
    fmc_lock();
    
    ptrd = (uint32_t*)BANK1_WRITE_START_ADDR;

    /* check flash whether is programmed */
    for(i = 0; i < 512; i++){
        if(*ptrd == 0xFFFFFFFF){
            lednum=LED4;
            break;
        }else{
            lednum=LED1;
            ptrd++;
        }
    }
#endif

#ifdef FMC_ERASEPAGE
    /* unlock the flash bank1 program erase controller */
    fmc_unlock();
    
    /* define the number of page to be erased */
    NbrOfPage = (BANK1_WRITE_END_ADDR - BANK1_WRITE_START_ADDR) / FMC_PAGE_SIZE;
    
    /* clear all pending flags */
    fmc_flag_clear(FMC_FLAG_END | FMC_FLAG_WPERR | FMC_FLAG_PGERR );
    
    /* erase the flash pages */
    for(EraseCounter = 0; EraseCounter < NbrOfPage; EraseCounter++) {
        fmc_page_erase(BANK1_WRITE_START_ADDR + (FMC_PAGE_SIZE * EraseCounter));
        fmc_flag_clear(FMC_FLAG_END | FMC_FLAG_WPERR | FMC_FLAG_PGERR );
    }
    
    fmc_lock();
    
    ptrd = (uint32_t*)BANK1_WRITE_START_ADDR;
    /* check flash whether is erased */
    for(i = 0; i < 512; i++){
        if(*ptrd != 0xFFFFFFFF){
            lednum=LED4;
            break;
        }else{
            lednum=LED2;
            ptrd++;
        }
    }
#endif

#ifdef FMC_REPROGRAM
    fmc_unlock();

    /* program flash */
    address = BANK1_WRITE_START_ADDR;

    while(address < BANK1_WRITE_END_ADDR){
        fmc_word_program(address, data0);
        address = address + 4; 
        fmc_flag_clear(FMC_FLAG_END | FMC_FLAG_WPERR | FMC_FLAG_PGERR );
    }

    address = BANK1_WRITE_START_ADDR;

    /* reprogram the previous program address */
    while(address < BANK1_WRITE_END_ADDR){
        fmc_word_reprogram(address, data1);
        address = address + 4; 
        fmc_flag_clear(FMC_FLAG_END | FMC_FLAG_WPERR | FMC_FLAG_PGERR );
    }

    fmc_lock();
    
    ptrd = (uint32_t*)BANK1_WRITE_START_ADDR;

    /* check bit 1 whether is changed to bit 0 */
    for(i = 0; i < 512; i++){
        if(*ptrd != 0x01030503){
            lednum=LED4;
            break;
        }else{
            lednum=LED3;
            ptrd++;
        }
    }

#endif

    while (1);
}
