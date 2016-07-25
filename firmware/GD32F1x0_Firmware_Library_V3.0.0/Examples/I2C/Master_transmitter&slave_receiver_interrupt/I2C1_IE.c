/*!
    \file  I2C1_IE.c
    \brief I2C1 slave receiver interrupt program
*/

/*
    Copyright (C) 2016 GigaDevice

    2014-12-26, V1.0.0, firmware for GD32F1x0(x=3,5)
    2016-01-15, V2.0.0, firmware for GD32F1x0(x=3,5,7,9)
    2016-04-30, V3.0.0, firmware update for GD32F1x0(x=3,5,7,9)
*/

#include "gd32f1x0_i2c.h"
#include "I2C_IE.h"

uint32_t    event2;

/*!
    \brief      handle I2C1 event interrupt request
    \param[in]  none
    \param[out] none
    \retval     none
*/
void I2C1_EventIRQ_Handler(void)
{
    if(i2c_flag_get(I2C1, I2C_ADDSEND)){
        /* clear the ADDSEND bit */
        event2 =i2c_flag_get(I2C1,I2C_ADDSEND );
    }else if(i2c_flag_get(I2C1, I2C_RBNE)){
        /* if reception data register is not empty ,I2C1 will read a data from I2C_DATA */
        *i2c_rxbuffer++ = i2c_receive_data(I2C1);
    }else if(i2c_flag_get(I2C1, I2C_STPDET)){
        Status=SUCCESS;
        /* clear the STPDET bit */
        i2c_enable(I2C1);
        /* disable I2C1 interrupt */
        i2c_interrupt_disable(I2C1, I2C_CTL1_ERRIE | I2C_CTL1_BUFIE | I2C_CTL1_EVIE);
    }
}

/*!
    \brief      handle I2C1 error interrupt request
    \param[in]  none
    \param[out] none
    \retval     none
*/
void I2C1_ErrorIRQ_Handler(void)
{
    /* no acknowledge received */
    if(i2c_flag_get(I2C1, I2C_AERR)){
        i2c_flag_clear(I2C1, I2C_AERR);
    }

    /* SMBus alert */
    if(i2c_flag_get(I2C1, I2C_SMBALTS)){
        i2c_flag_clear(I2C1, I2C_SMBALTS);
    }

    /* bus timeout in SMBus mode */
    if(i2c_flag_get(I2C1, I2C_SMBTO)){
        i2c_flag_clear(I2C1, I2C_SMBTO);
    }

    /* over-run or under-run when SCL stretch is disabled */
    if(i2c_flag_get(I2C1, I2C_OUERR)){
       i2c_flag_clear(I2C1, I2C_OUERR);
    }

    /* arbitration lost */
    if(i2c_flag_get(I2C1, I2C_LOSTARB)){
        i2c_flag_clear(I2C1, I2C_LOSTARB);
    }

    /* bus error */
    if(i2c_flag_get(I2C1, I2C_BERR)){
        i2c_flag_clear(I2C1, I2C_BERR);
    }

    /* CRC value doesn't match */
    if(i2c_flag_get(I2C1, I2C_PECERR)){
        i2c_flag_clear(I2C1, I2C_PECERR);
    }

    /* disable the error interrupt */
    i2c_interrupt_disable(I2C1,I2C_CTL1_ERRIE | I2C_CTL1_BUFIE | I2C_CTL1_EVIE);
}

