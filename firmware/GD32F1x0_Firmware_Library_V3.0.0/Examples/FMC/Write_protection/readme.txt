/*!
    \file  readme.txt
    \brief description of write_protection example
*/

/*
    Copyright (C) 2016 GigaDevice

    2014-12-26, V1.0.0, firmware for GD32F1x0(x=3,5)
    2016-01-15, V2.0.0, firmware for GD32F1x0(x=3,5,7,9)
    2016-04-30, V3.0.0, firmware update for GD32F1x0(x=3,5,7,9)
*/

  @brief
    This example is based on the GD32150R-EVAL/GD32190R-EVAL board, it provides a description 
  of how to enable and disable the write protection for the embedded flash.
    
    To enable the write protection, uncomment the line "#define WRITE_PROTECTION_ENABLE"  
  in main.c file. Use the function ob_write_protection_enable to protect a set of sectors. 
  To load the new option byte values, it is necessary to use function ob_reset() to reset the system.
 
    To disable the write protection, uncomment the line "#define WRITE_PROTECTION_DISABLE"
  in main.c file. Use the function ob_erase to disable the write protection a set of sectors. 
  If other sectors write protected, use the function ob_writeprotection_enable to protectpages. 
  To load the new option byte values, it is necessary to use function ob_reset() to reset the system.


