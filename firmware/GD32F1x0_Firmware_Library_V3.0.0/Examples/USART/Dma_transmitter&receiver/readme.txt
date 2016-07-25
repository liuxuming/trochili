/*!
    \file  readme.txt
    \brief description of the USART DMA transmitter receiver
*/

/*
    Copyright (C) 2016 GigaDevice

    2014-12-26, V1.0.0, firmware for GD32F1x0(x=3,5)
    2016-01-15, V2.0.0, firmware for GD32F1x0(x=3,5,7,9)
    2016-04-30, V3.0.0, firmware update for GD32F1x0(x=3,5,7,9)
*/

  This demo is based on the GD32150R-EVAL/GD32190R-EVAL board, it shows how to use the USART
DMA transmit and receive.
  Firstly, the USART sends the strings to the hyperterminal and then loop waiting for
receiving max 10 datas from the hyperterminal. Every time if the number of data you enter equal
with or more than 10 bytes, USART will send 10 bytes to the hyperterminal.
