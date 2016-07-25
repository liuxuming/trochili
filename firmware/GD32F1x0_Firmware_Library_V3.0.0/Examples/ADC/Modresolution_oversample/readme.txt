/*!
    \file  readme.txt
    \brief description of the ADC modify resolution and oversample
*/

/*
    Copyright (C) 2016 GigaDevice

    2014-12-26, V1.0.0, platform GD32F1x0(x=3,5)
    2016-01-15, V2.0.0, platform GD32F1x0(x=3,5,7,9)
    2016-04-30, V3.0.0, firmware update for GD32F1x0(x=3,5,7,9)
*/

  This demo is based on the GD32190R-EVAL board, it shows how to use the funciton
of resolusion modification and oversample. In this demo, 12B and 6B resolusion
work with the shift and ratio of oversample. PC0(channel10) is chosen as analog 
input pin. The ADC clock is configured to 12MHz(below 28MHz).
  As the AD conversion begins by software,the converted data is printed by USART.
So we can directly see the difference between high resolution and low resolution
, shift and ratio of oversample.

  The analog input pin should configured to analog mode and the ADC clock should 
below 28MHz for GD32190R-EVAL.
 