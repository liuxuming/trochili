/*!
    \file  readme.txt
    \brief description of the MSC demo.
*/

/*
    Copyright (C) 2016 GigaDevice

    2014-09-06, V1.0.0, firmware for GD32F150
    2016-01-15, V1.0.1, firmware for GD32F150
    2016-04-30, V3.0.0, firmware update for GD32F150
*/

  The Mass Storage example gives a typical example of how to use the GD32F1x0
USB device peripheral to communicate with a PC Host using the bulk transfer 
while the internal flash is used as storage media.

This example supports the BBB (bulk bulk bulk) protocol and all needed SCSI
(small computer system interface) commands, and is compatible with windows platform.

  After running the example, the user just has to plug the USB cable into a PC host
and the device is automatically detected. A new removable drive appears in the
system window and write/read/format operations can be performed as with any other
removable drive.
