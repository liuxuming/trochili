/*!
    \file  readme.txt
    \brief description of the master transmitter and slave_receiver interrupt
*/

/*
    Copyright (C) 2016 GigaDevice

    2014-12-26, V1.0.0, firmware for GD32F1x0(x=3,5)
    2016-01-15, V2.0.0, firmware for GD32F1x0(x=3,5,7,9)
    2016-04-30, V3.0.0, firmware update for GD32F1x0(x=3,5,7,9)
*/

  This demo is based on the GD32150R-EVAL/GD32190R-EVAL board, it shows how to use the I2C 
interrupt programming mode in master transmitting mode or slave receiving mode.

  In this demo, I2C1 is operated as the master transmitter and I2C2 as the slave 
receiver.Moreover,the SCL line and SDA line of I2C1 interface are controled by 
the I/O pin PB6 and PB7 respectively. The SCL and SDA of I2C2 are controled by 
the pin PB10 and PB11 respectively.

  This demo will send the data of I2C_Buf_Write array through I2C1 interface 
to the I2C2,and it will store the data received by I2C2 in the I2C_Buf_Read 
array. if transfer is sucessfully completed,led1 and led2 is light. 

  We shoud use the jumper to connect the PB6 and PB10. The PB7 and PB11 are 
connected as well.

