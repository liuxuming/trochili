/*!
    \file  readme.txt
    \brief description of program_erase example
*/

/*
    Copyright (C) 2016 GigaDevice

    2014-12-26, V1.0.0, firmware for GD32F1x0(x=3,5)
    2016-01-15, V2.0.0, firmware for GD32F1x0(x=3,5,7,9)
    2016-04-30, V3.0.0, firmware update for GD32F1x0(x=3,5,7,9)
*/

@brief
  This example is based on the GD32150R-EVAL/GD32190R-EVAL board, it provides a
description of how to erase and program flash memory and program without previous
earse. 

  In programming operation, a comparison between flash memory and target data is 
done to check that the flash memory has been correctly programmed.if the operation
is correct, LED1 is spark, otherwise, LED4 is spark.
   
  In erasing operation, a comparison between flash memory and 0xffffffff is done to 
check that the flash memory has been correctly erased.if the operation is correct, 
LED2 is spark, otherwise, LED4 is spark.

  In reprogramming operation for GD32F170 and GD32F190 series, the content of memory 
bit 1 could change to be bit 0, LED3 is spark, otherwise, LED4 is spark.
  
 