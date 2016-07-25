/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TCL_GD32_F150_H
#define _TCL_GD32_F150_H

#define RSV_IRQ0_ID                 (0),  /* Top of Stack                                 */
#define Reset_IRQ_ID                (1),  /* Reset Handler                                */
#define NMI_IRQ_ID                  (2),  /* NMI Handler                                  */
#define HardFault_IRQ_ID            (3),  /* Hard Fault Handler                           */
#define MemManage_IRQ_ID            (4),  /* MPU Fault Handler                            */
#define BusFault_IRQ_ID             (5),  /* Bus Fault Handler                            */
#define UsageFault_IRQ_ID           (6),  /* Usage Fault Handler                          */

    /* External Interrupts */
#define WWDG_IRQ_ID                 (7)    /* Window Watchdog                             */
#define PVD_IRQ_ID                  (8)    /* PVD through EXTI Line detect                */
#define RTC_IRQ_ID                  (9)    /* RTC through EXTI Line                       */
#define FLASH_IRQ_ID                (10)   /* FLASH                                       */
#define RCC_IRQ_ID                  (11)   /* RCC                                         */
#define EXTI0_1_IRQ_ID              (12)   /* EXTI Line 0 and 1                           */
#define EXTI2_3_IRQ_ID              (13)   /* EXTI Line 2 and 3                           */
#define EXTI4_15_IRQ_ID             (14)   /* EXTI Line 4 to 15                           */
#define TS_IRQ_ID                   (15)   /* TS                                          */
#define DMA1_Channel1_IRQ_ID        (16)   /* DMA1 Channel 1                              */
#define DMA1_Channel2_3_IRQ_ID      (17)   /* DMA1 Channel 2 and Channel 3                */
#define DMA1_Channel4_5_IRQ_ID      (18)   /* DMA1 Channel 4 and Channel 5                */
#define ADC1_COMP_IRQ_ID            (19)   /* ADC1, COMP1 and COMP2                       */
#define TIM1_BRK_UP_TRG_COM_IRQ_ID  (20)   /* TIM1 Break, Update, Trigger and Commutation */
#define TIM1_CC_IRQ_ID              (21)   /* TIM1 Capture Compare                        */
#define TIM2_IRQ_ID                 (22)   /* TIM2                                        */
#define TIM3_IRQ_ID                 (23)   /* TIM3                                        */
#define TIM6_DAC_IRQ_ID             (24)   /* TIM6 and DAC                                */
#define Reserved0                   (25)   /* Reserved                                    */
#define TIM14_IRQ_ID                (26)   /* TIM14                                       */
#define TIM15_IRQ_ID                (27)   /* TIM15                                       */
#define TIM16_IRQ_ID                (28)   /* TIM16                                       */
#define TIM17_IRQ_ID                (29)   /* TIM17                                       */
#define I2C1_EV_IRQ_ID              (30)   /* I2C1 Event                                  */
#define I2C2_EV_IRQ_ID              (31)   /* I2C2 Event                                  */
#define SPI1_IRQ_ID                 (32)   /* SPI1                                        */
#define SPI2_IRQ_ID                 (33)   /* SPI2                                        */
#define USART1_IRQ_ID               (34)   /* USART1                                      */
#define USART2_IRQ_ID               (35)   /* USART2                                      */
#define Reserved1                   (36)   /* Reserved                                    */
#define CEC_IRQ_ID                  (37)   /* CEC                                         */
#define Reserved2                   (38)   /* Reserved                                    */
#define I2C1_ER_IRQ_ID              (39)   /* I2C1 Error                                  */
#define Reserved3                   (40)   /* Reserved                                    */
#define I2C2_ER_IRQ_ID              (41)   /* I2C2 Error                                  */
#define I2C3_EV_IRQ_ID              (42)   /* I2C3 Event                                  */
#define I2C3_ER_IRQ_ID              (43)   /* I2C3 Error                                  */
#define USB_LP_IRQ_ID               (44)   /* USB Low  Priority                           */
#define USB_HP_IRQ_ID               (45)   /* USB High Priority                           */
#define Reserved4                   (46)   /* Reserved                                    */
#define Reserved5                   (47)   /* Reserved                                    */
#define Reserved6                   (48)   /* Reserved                                    */
#define USBWakeUp_IRQ_ID            (49)   /* USB Wakeup from suspend                     */
#define Reserved7                   (50)   /* Reserved                                    */
#define Reserved8                   (51)   /* Reserved                                    */
#define Reserved9                   (52)   /* Reserved                                    */
#define Reserved10                  (53)   /* Reserved                                    */
#define Reserved11                  (54)   /* Reserved                                    */
#define DMA1_Channel6_7_IRQ_ID      (55)   /* DMA1 Channel 6 and Channel 7                */
#define Reserved12                  (56)   /* Reserved                                    */
#define Reserved13                  (57)   /* Reserved                                    */
#define SPI3_IRQ_ID                 (58)   /* SPI3                                        */


extern void GD32_SysTick_ISR(void);
extern void GD32_EXTI0_1_ISR(void);
extern void EXTI15_10_IRQHandler(void);
extern void GD32_TIM2_ISR(void);

#endif
