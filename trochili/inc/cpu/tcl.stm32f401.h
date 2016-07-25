/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TOCHILI_STM32_F401_H
#define _TOCHILI_STM32_F401_H

#define   WWDG_IRQ_ID                  (0)     /* Window WatchDog Interrupt                                         */
#define   PVD_IRQ_ID                   (1)     /* PVD through EXTI Line detection Interrupt                         */
#define   TAMP_STAMP_IRQ_ID            (2)     /* Tamper and TimeStamp interrupts through the EXTI line             */
#define   RTC_WKUP_IRQ_ID              (3)     /* RTC Wakeup interrupt through the EXTI line                        */
#define   FLASH_IRQ_ID                 (4)     /* FLASH global Interrupt                                            */
#define   RCC_IRQ_ID                   (5)     /* RCC global Interrupt                                              */
#define   EXTI0_IRQ_ID                 (6)     /* EXTI Line0 Interrupt                                              */
#define   EXTI1_IRQ_ID                 (7)     /* EXTI Line1 Interrupt                                              */
#define   EXTI2_IRQ_ID                 (8)     /* EXTI Line2 Interrupt                                              */
#define   EXTI3_IRQ_ID                 (9)     /* EXTI Line3 Interrupt                                              */
#define   EXTI4_IRQ_ID                 (10)    /* EXTI Line4 Interrupt                                              */
#define   DMA1_Stream0_IRQ_ID          (11)    /* DMA1 Stream 0 global Interrupt                                    */
#define   DMA1_Stream1_IRQ_ID          (12)    /* DMA1 Stream 1 global Interrupt                                    */
#define   DMA1_Stream2_IRQ_ID          (13)    /* DMA1 Stream 2 global Interrupt                                    */
#define   DMA1_Stream3_IRQ_ID          (14)    /* DMA1 Stream 3 global Interrupt                                    */
#define   DMA1_Stream4_IRQ_ID          (15)    /* DMA1 Stream 4 global Interrupt                                    */
#define   DMA1_Stream5_IRQ_ID          (16)    /* DMA1 Stream 5 global Interrupt                                    */
#define   DMA1_Stream6_IRQ_ID          (17)    /* DMA1 Stream 6 global Interrupt                                    */
#define   ADC_IRQ_ID                   (18)    /* ADC1)ADC2 and ADC3 global Interrupts                             */
#define   CAN1_TX_IRQ_ID               (19)    /* CAN1 TX Interrupt                                                 */
#define   CAN1_RX0_IRQ_ID              (20)    /* CAN1 RX0 Interrupt                                                */
#define   CAN1_RX1_IRQ_ID              (21)    /* CAN1 RX1 Interrupt                                                */
#define   CAN1_SCE_IRQ_ID              (22)    /* CAN1 SCE Interrupt                                                */
#define   EXTI9_5_IRQ_ID               (23)    /* External Line[9:5] Interrupts                                     */
#define   TIM1_BRK_TIM9_IRQ_ID         (24)    /* TIM1 Break interrupt and TIM9 global interrupt                    */
#define   TIM1_UP_TIM10_IRQ_ID         (25)    /* TIM1 Update Interrupt and TIM10 global interrupt                  */
#define   TIM1_TRG_COM_TIM11_IRQ_ID    (26)    /* TIM1 Trigger and Commutation Interrupt and TIM11 global interrupt */
#define   TIM1_CC_IRQ_ID               (27)    /* TIM1 Capture Compare Interrupt                                    */
#define   TIM2_IRQ_ID                  (28)    /* TIM2 global Interrupt                                             */
#define   TIM3_IRQ_ID                  (29)    /* TIM3 global Interrupt                                             */
#define   TIM4_IRQ_ID                  (30)    /* TIM4 global Interrupt                                             */
#define   I2C1_EV_IRQ_ID               (31)    /* I2C1 Event Interrupt                                              */
#define   I2C1_ER_IRQ_ID               (32)    /* I2C1 Error Interrupt                                              */
#define   I2C2_EV_IRQ_ID               (33)    /* I2C2 Event Interrupt                                              */
#define   I2C2_ER_IRQ_ID               (34)    /* I2C2 Error Interrupt                                              */
#define   SPI1_IRQ_ID                  (35)    /* SPI1 global Interrupt                                             */
#define   SPI2_IRQ_ID                  (36)    /* SPI2 global Interrupt                                             */
#define   USART1_IRQ_ID                (37)    /* USART1 global Interrupt                                           */
#define   USART2_IRQ_ID                (38)    /* USART2 global Interrupt                                           */
#define   USART3_IRQ_ID                (39)    /* USART3 global Interrupt                                           */
#define   EXTI15_10_IRQ_ID             (40)    /* External Line[15:10] Interrupts                                   */
#define   RTC_Alarm_IRQ_ID             (41)    /* RTC Alarm (A and B) through EXTI Line Interrupt                   */
#define   OTG_FS_WKUP_IRQ_ID           (42)    /* USB OTG FS Wakeup through EXTI line interrupt                     */
#define   TIM8_BRK_TIM12_IRQ_ID        (43)    /* TIM8 Break Interrupt and TIM12 global interrupt                   */
#define   TIM8_UP_TIM13_IRQ_ID         (44)    /* TIM8 Update Interrupt and TIM13 global interrupt                  */
#define   TIM8_TRG_COM_TIM14_IRQ_ID    (45)    /* TIM8 Trigger and Commutation Interrupt and TIM14 global interrupt */
#define   TIM8_CC_IRQ_ID               (46)    /* TIM8 Capture Compare Interrupt                                    */
#define   DMA1_Stream7_IRQ_ID          (47)    /* DMA1 Stream7 Interrupt                                            */
#define   FSMC_IRQ_ID                  (48)    /* FSMC global Interrupt                                             */
#define   SDIO_IRQ_ID                  (49)    /* SDIO global Interrupt                                             */
#define   TIM5_IRQ_ID                  (50)    /* TIM5 global Interrupt                                             */
#define   SPI3_IRQ_ID                  (51)    /* SPI3 global Interrupt                                             */
#define   UART4_IRQ_ID                 (52)    /* UART4 global Interrupt                                            */
#define   UART5_IRQ_ID                 (53)    /* UART5 global Interrupt                                            */
#define   TIM6_DAC_IRQ_ID              (54)    /* TIM6 global and DAC1&2 underrun error  interrupts                 */
#define   TIM7_IRQ_ID                  (55)    /* TIM7 global interrupt                                             */
#define   DMA2_Stream0_IRQ_ID          (56)    /* DMA2 Stream 0 global Interrupt                                    */
#define   DMA2_Stream1_IRQ_ID          (57)    /* DMA2 Stream 1 global Interrupt                                    */
#define   DMA2_Stream2_IRQ_ID          (58)    /* DMA2 Stream 2 global Interrupt                                    */
#define   DMA2_Stream3_IRQ_ID          (59)    /* DMA2 Stream 3 global Interrupt                                    */
#define   DMA2_Stream4_IRQ_ID          (60)    /* DMA2 Stream 4 global Interrupt                                    */
#define   ETH_IRQ_ID                   (61)    /* Ethernet global Interrupt                                         */
#define   ETH_WKUP_IRQ_ID              (62)    /* Ethernet Wakeup through EXTI line Interrupt                       */
#define   CAN2_TX_IRQ_ID               (63)    /* CAN2 TX Interrupt                                                 */
#define   CAN2_RX0_IRQ_ID              (64)    /* CAN2 RX0 Interrupt                                                */
#define   CAN2_RX1_IRQ_ID              (65)    /* CAN2 RX1 Interrupt                                                */
#define   CAN2_SCE_IRQ_ID              (66)    /* CAN2 SCE Interrupt                                                */
#define   OTG_FS_IRQ_ID                (67)    /* USB OTG FS global Interrupt                                       */
#define   DMA2_Stream5_IRQ_ID          (68)    /* DMA2 Stream 5 global interrupt                                    */
#define   DMA2_Stream6_IRQ_ID          (69)    /* DMA2 Stream 6 global interrupt                                    */
#define   DMA2_Stream7_IRQ_ID          (70)    /* DMA2 Stream 7 global interrupt                                    */
#define   USART6_IRQ_ID                (71)    /* USART6 global interrupt                                           */
#define   I2C3_EV_IRQ_ID               (72)    /* I2C3 event interrupt                                              */
#define   I2C3_ER_IRQ_ID               (73)    /* I2C3 error interrupt                                              */
#define   OTG_HS_EP1_OUT_IRQ_ID        (74)    /* USB OTG HS End Point 1 Out global interrupt                       */
#define   OTG_HS_EP1_IN_IRQ_ID         (75)    /* USB OTG HS End Point 1 In global interrupt                        */
#define   OTG_HS_WKUP_IRQ_ID           (76)    /* USB OTG HS Wakeup through EXTI interrupt                          */
#define   OTG_HS_IRQ_ID                (77)    /* USB OTG HS global interrupt                                       */
#define   DCMI_IRQ_ID                  (78)    /* DCMI global interrupt                                             */
#define   CRYP_IRQ_ID                  (79)    /* CRYP crypto global interrupt                                      */
#define   HASH_RNG_IRQ_ID              (80)    /* Hash and Rng global interrupt                                     */
#define   FPU_IRQ_ID                   (81)    /* FPU global interrupt   */

#endif
