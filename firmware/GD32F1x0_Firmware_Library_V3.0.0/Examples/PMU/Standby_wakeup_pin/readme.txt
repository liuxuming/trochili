/*!
    \file  readme.txt
    \brief description of the standby wakeup throuht pin demo
*/

/*
    Copyright (C) 2016 GigaDevice

    2014-12-26, V1.0.0, firmware for GD32F1x0(x=3,5)
    2016-01-15, V2.0.0, firmware for GD32F1x0(x=3,5,7,9)
    2016-04-30, V3.0.0, firmware update for GD32F1x0(x=3,5,7,9)
*/

  This example is based on the GD32150R-EVAL/GD32190R-EVAL board, it shows pmu how to
enter standby mode and wakeup it.Press tamper key enter standby mode ,led turn off. 
When you press wakeup key, mcu will be wakeuped from standby mode, led will be turn on.
when exit from the standby mode, a power-on reset occurs and the mcu will execute 
instruction code from the 0x00000000 address.
 