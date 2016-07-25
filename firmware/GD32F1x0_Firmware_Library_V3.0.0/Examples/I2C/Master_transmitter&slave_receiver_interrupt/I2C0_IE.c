/*!
    \file  I2C0_IE.c
    \brief I2C0 master transmitter interrupt program
*/

/*
    Copyright (C) 2016 GigaDevice

    2014-12-26, V1.0.0, firmware for GD32F1x0(x=3,5)
    2016-01-15, V2.0.0, firmware for GD32F1x0(x=3,5,7,9)
    2016-04-30, V3.0.0, firmware update for GD32F1x0(x=3,5,7,9)
*/


#include "gd32f1x0_i2c.h"
#include "I2C_IE.h"

uint32_t    event1;

/*!
    \brief      handle I2C0 event interrupt request
    \param[in]  none
    \param[out] none
    \retval     none
*/
void I2C0_EventIRQ_Handler(void)
{
    if(i2c_flag_get(I2C0, I2C_SBSEND)){
        /* send slave address */
        i2c_master_addressing(I2C0,I2C1_SLAVE_ADDRESS7,I2C_TRANSMITTER);
    }else if(i2c_flag_get(I2C0, I2C_ADDSEND)){
        /*clear ADDSEND bit */
        event1 = i2c_flag_get(I2C0, I2C_ADDSEND);
    }else if(i2c_flag_get(I2C0, I2C_TBE)){
        if(I2C_nBytes>0){
            /* the master sends a data byte */
            i2c_transmit_data(I2C0,*i2c_txbuffer++);
            I2C_nBytes--;
        }else{
            /* the master sends a stop condition to I2C bus */
            i2c_stop_on_bus(I2C0);
            /* disable the I2C0 interrupt */
            i2c_interrupt_disable(I2C0, I2C_CTL1_ERRIE | I2C_CTL1_BUFIE | I2C_CTL1_EVIE);
        }
    }

}

/*!
    \brief      handle I2C0 error interrupt request
    \param[in]  none
    \param[out] none
    \retval     none
*/
void I2C0_ErrorIRQ_Handler(void)
{
    /* no acknowledge received */
    if(i2c_flag_get(I2C0, I2C_AERR)){
        i2c_flag_clear(I2C0, I2C_AERR);
    }

    /* SMBus alert */
    if(i2c_flag_get(I2C0, I2C_SMBALTS)){
        i2c_flag_clear(I2C0, I2C_SMBALTS);
    }

    /* bus timeout in SMBus mode */
    if(i2c_flag_get(I2C0, I2C_SMBTO)){
        i2c_flag_clear(I2C0, I2C_SMBTO);
    }

    /* over-run or under-run when SCL stretch is disabled */
    if(i2c_flag_get(I2C0, I2C_OUERR)){
        i2c_flag_clear(I2C0, I2C_OUERR);
    }

    /* arbitration lost */
    if(i2c_flag_get(I2C0, I2C_LOSTARB)){
        i2c_flag_clear(I2C0, I2C_LOSTARB);
    }

    /* bus error */
    if(i2c_flag_get(I2C0, I2C_BERR)){
        i2c_flag_clear(I2C0, I2C_BERR);
    }

    /* CRC value doesn't match */
    if(i2c_flag_get(I2C0, I2C_PECERR)){
        i2c_flag_clear(I2C0, I2C_PECERR);
    }

    /* disable the error interrupt */
    i2c_interrupt_disable(I2C0,I2C_CTL1_ERRIE | I2C_CTL1_BUFIE | I2C_CTL1_EVIE);
}

