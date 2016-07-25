/*!
    \file  slcd_seg.h
    \brief the header file of the slcd segment driver
*/

/*
    Copyright (C) 2016 GigaDevice

    2016-01-15, V1.0.0, demo for GD32F1x0(x=7,9)
    2016-05-13, V2.0.0, demo for GD32F1x0(x=7,9)
*/

#ifndef LCD_SEG_H
#define LCD_SEG_H

#include "gd32f1x0.h"

typedef enum
{
    INTEGER = 0,
    FLOAT = 1,
    TIME = 2
}slcd_display_enum;

void slcd_seg_config(void);
void slcd_seg_digit_display(uint8_t ch, uint8_t position);
void slcd_seg_number_display(uint32_t num);
void slcd_seg_floatnumber_display(float num);
void slcd_seg_time_display(uint8_t hour, uint8_t minute , uint8_t second);
void slcd_seg_digit_clear(uint8_t position);
void slcd_seg_clear_all(void);

#endif /* LCD_SEG_H */
