/*!
    \file  readme.txt
    \brief description of the USB custom HID device for IAP demo
*/

/*
    Copyright (C) 2016 GigaDevice

    2014-09-06, V1.0.0, firmware for GD32F150
    2016-01-15, V1.0.1, firmware for GD32F150
    2016-04-30, V3.0.0, firmware update for GD32F150
*/

  This demo is based on the GD32150R-EVAL board,it provides a description of 
how to use the USB-FS-Device peripheral.

  The GD32 device is enumerated as an USB Custom HID device for IAP, that uses the 
native PC Host HID driver.

  To test the demo, you need a configuration hex image or bin image. The hex image
or the bin image should set application address at 0x8004000. You can refer to 
"Utilities/Binary/DFU_Images" folder and use the hex images or bin images in it.

  To test the demo, you need to:
    - Download the "USB IAP Client" PC applet
    - Start the "USB IAP Client" PC applet and connect GD32 USB to PC (Note: maybe
      the driver installion is failed in the first time, try reseating again.)
    - The device should be detected and shown in the PC applet
    - Select the hex image and download it to the flash
    - After each device reset, the mcu will run the new application
    - After each device reset, hold down the TAMPER key on the GD32150R-EVAL board 
      to enter IAP mode