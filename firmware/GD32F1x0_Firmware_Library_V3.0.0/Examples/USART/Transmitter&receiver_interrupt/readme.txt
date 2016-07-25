/*!
    \file  readme.txt
    \brief description of the USART transmit and receive interrupt
*/

/*
    Copyright (C) 2016 GigaDevice

    2014-12-26, V1.0.0, platform GD32F1x0(x=3,5)
    2016-01-15, V2.0.0, platform GD32F1x0(x=3,5,7,9)
    2016-04-30, V3.0.0, firmware update for GD32F1x0(x=3,5,7,9)
*/

  This demo is based on the GD32150R-EVAL/GD32190R-EVAL board, it shows how to use 
the USART transmit and receive interrupts to communicate with the hyperterminal.
  Firstly, the USART sends the strings to the hyperterminal and still waiting for
receiving data from the hyperterminal. Only the number of data you enter equal with
or more than 32 bytes, USART will send strings: "USART receive successfully!" (the 
reception is stopped when this maximum receive value is reached).

