/*!
    \file  readme.txt
    \brief description of the SLCD demo
*/

/*
    Copyright (C) 2016 GigaDevice

    2014-12-26, V1.0.0, firmware for GD32F1x0(x=3,5)
    2016-01-15, V2.0.0, firmware for GD32F1x0(x=3,5,7,9)
    2016-04-30, V3.0.0, firmware update for GD32F1x0(x=3,5,7,9)
*/

  This demo is based on the GD32190R-EVAL board, it shows how to use the SLCD
peripheral to work with glass SLCD.

  For the reason SLCD and RTC share the same clock, we must configure the clock
of RTC first, this demo use RC40K(its frequence is 40K) to drive the SLCD peripheral.

  In this demo, the SLCD peripheral use 1/3 Bias voltage, 1/4 duty, 156.25Hz update
frequence to display numbers.
