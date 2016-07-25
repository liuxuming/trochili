/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TOCHILI_STM32_F030_H
#define _TOCHILI_STM32_F030_H
#define  WWDG_IRQ_ID                  (0)     /* Window WatchDog Interrupt                               */
#define  PVD_IRQ_ID                   (1)     /* PVD through EXTI Line detect Interrupt                  */
#define  RTC_IRQ_ID                   (2)     /* RTC through EXTI Line Interrupt                         */
#define  FLASH_IRQ_ID                 (3)     /* FLASH Interrupt                                         */
#define  RCC_IRQ_ID                   (4)     /* RCC Interrupt                                           */
#define  EXTI0_1_IRQ_ID               (5)     /* EXTI Line 0 and 1 Interrupts                            */
#define  EXTI2_3_IRQ_ID               (6)     /* EXTI Line 2 and 3 Interrupts                            */
#define  EXTI4_15_IRQ_ID              (7)     /* EXTI Line 4 to 15 Interrupts                            */
#define  TS_IRQ_ID                    (8)     /* TS Interrupt                                            */
#define  DMA1_Channel1_IRQ_ID         (9)     /* DMA1 Channel 1 Interrupt                                */
#define  DMA1_Channel2_3_IRQ_ID       (10)    /* DMA1 Channel 2 and Channel 3 Interrupts                 */
#define  DMA1_Channel4_5_IRQ_ID       (11)    /* DMA1 Channel 4 and Channel 5 Interrupts                 */
#define  ADC1_COMP_IRQ_ID             (12)    /* ADC1)COMP1 and COMP2 Interrupts                         */
#define  TIM1_BRK_UP_TRG_COM_IRQ_ID   (13)    /* TIM1 Break)Update)Trigger and Commutation Interrupts    */
#define  TIM1_CC_IRQ_ID               (14)    /* TIM1 Capture Compare Interrupt                          */
#define  TIM2_IRQ_ID                  (15)    /* TIM2 Interrupt                                          */
#define  TIM3_IRQ_ID                  (16)    /* TIM3 Interrupt                                          */
#define  TIM6_DAC_IRQ_ID              (17)    /* TIM6 and DAC Interrupts                                 */
#define  TIM14_IRQ_ID                 (19)    /* TIM14 Interrupt                                         */
#define  TIM15_IRQ_ID                 (20)    /* TIM15 Interrupt                                         */
#define  TIM16_IRQ_ID                 (21)    /* TIM16 Interrupt                                         */
#define  TIM17_IRQ_ID                 (22)    /* TIM17 Interrupt                                         */
#define  I2C1_IRQ_ID                  (23)    /* I2C1 Interrupt                                          */
#define  I2C2_IRQ_ID                  (24)    /* I2C2 Interrupt                                          */
#define  SPI1_IRQ_ID                  (25)    /* SPI1 Interrupt                                          */
#define  SPI2_IRQ_ID                  (26)    /* SPI2 Interrupt                                          */
#define  USART1_IRQ_ID                (27)    /* USART1 Interrupt                                        */
#define  USART2_IRQ_ID                (28)    /* USART2 Interrupt                                        */
#define  CEC_IRQ_ID                   (30)    /* CEC Interrupt                                           */
 
#endif
