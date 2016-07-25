/*!
    \file  readme.txt
    \brief description of the USART wakeup from deepsleep
*/

/*
    Copyright (C) 2016 GigaDevice

    2014-12-26, V1.0.0, firmware for GD32F1x0(x=3,5)
    2016-01-15, V2.0.0, firmware for GD32F1x0(x=3,5,7,9)
    2016-04-30, V3.0.0, firmware update for GD32F1x0(x=3,5,7,9)
*/

  This demo is based on the GD32150R-EVAL/GD32190R-EVAL board, it shows how to use the USART
to wake up the MCU from deepsleep mode.
  In this demo, the wake up method is configured as start bit dection. When the MCU
enter into deepsleep mode, the LED2 stop in a certain stateis(on or off). In this 
case, you can send some character to wake up USART and the LED2 go on blink.
   

