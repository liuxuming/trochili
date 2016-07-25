/*!
    \file  readme.txt
    \brief description of the USB keyboard demo
*/

/*
    Copyright (C) 2016 GigaDevice

    2014-09-06, V1.0.0, firmware for GD32F150
    2016-01-15, V1.0.1, firmware for GD32F150
    2016-04-30, V3.0.0, firmware update for GD32F150
*/

  This demo is based on the GD32150R-EVAL board,it provides a description of 
how to use the USB_Device peripheral.

  The GD32 device is enumerated as an USB keyboard, that uses the native PC Host
HID driver.

  The USB keyboard use three key(wakeup key, tamper key and user key) to print 3 
characters('b', 'a' and 'c').

  This demo supports remote wakeup (which is the ability of a USB device to 
bring a suspended bus back to the active condition), and the tamper key is 
used as the remote wakeup source.

  In order to test USB remote wakeup function, you can do as follows:
    - Manually switch PC to standby mode
    - Wait for PC to fully enter the standby mode
    - Push the tamper key
    - If PC is ON, remote wakeup is OK, else failed.