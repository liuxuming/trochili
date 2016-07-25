/*!
    \file  readme.txt
    \brief description of the TIMER1 OC toggle demo
*/

/*
    Copyright (C) 2016 GigaDevice

    2014-12-26, V1.0.0, firmware for GD32F1x0(x=3,5)
    2016-01-15, V2.0.0, firmware for GD32F1x0(x=3,5,7,9)
    2016-04-30, V3.0.0, firmware update for GD32F1x0(x=3,5,7,9)
*/

  This demo is based on the GD32150R-EVAL/GD32190R-EVAL board, it shows how to
configure the TIMER peripheral to generate the toggle signal.
  
  The TIMxCLK frequency is set to systemcoreclock (72MHz), the prescaler is 36000
so the TIMER1 counter clock is 2KHz.
  
  TIMER1 configuration: output compare toggle mode:
  CH1 update rate = TIMER1 counter clock / CH1CC = 2000/4000 = 0.5 Hz. 
