/*!
    \file  main.c
    \brief main flash sectors write protection
*/

/*
    Copyright (C) 2016 GigaDevice

    2014-12-26, V1.0.0, firmware for GD32F1x0(x=3,5)
    2016-01-15, V2.0.0, firmware for GD32F1x0(x=3,5,7,9)
    2016-04-30, V3.0.0, firmware update for GD32F1x0(x=3,5,7,9)
*/

#include "gd32f1x0.h" 
#include "gd32f1x0_eval.h"

#define FMC_PAGES_PROTECTED          (FMC_WP_WP8)

#define WRITE_PROTECTION_DISABLE 

/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{
    uint32_t writeprotection_value, protectedsector;

    /* unlock the main flash and option byte */
    fmc_unlock();
    ob_unlock();
    
    /* clear all pending flags */
    fmc_flag_clear(FMC_FLAG_END | FMC_FLAG_WPERR | FMC_FLAG_PGERR);
    
    /* get state of sectors write protection */
    writeprotection_value = ob_write_protection_get();
   
#ifdef WRITE_PROTECTION_DISABLE 
  
    /* get write protection sectors */
    protectedsector = ~(writeprotection_value | FMC_PAGES_PROTECTED);
    
    /* check whether the specified pages is write protected  */
    if((writeprotection_value | (~FMC_PAGES_PROTECTED)) != 0xFFFFFFFF){
        /* erase all the option bytes */
        ob_erase();
    
        /* check if other pages write protected */
        if(protectedsector != 0x0){
            /* restore other sectors write protected */
            ob_write_protection_enable(protectedsector);
        }
        /* generate a system reset to reload the option byte */
        ob_reset();
    }

#elif defined WRITE_PROTECTION_ENABLE
    /* get write protection sectors */
    protectedsector = (~writeprotection_value) | FMC_PAGES_PROTECTED;
   
    /* check whether the specified sectors is write protected */
    if(((~writeprotection_value) & FMC_PAGES_PROTECTED)!= FMC_PAGES_PROTECTED){
        ob_erase();
        
        /* enable the sectors write protection */
        ob_write_protection_enable(protectedsector);
        
        /* generate a system reset to reload the option byte */ 
        ob_reset();
    }  
#endif 

    while(1);

}


