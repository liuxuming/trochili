/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TOCHILI_GD32_F20X_H
#define _TOCHILI_GD32_F20X_H


#define WWDG_IRQ_ID                   (1 ) /* Vector Number 16,Window Watchdog                                    */
#define LVD_IRQ_ID                    (2 ) /* Vector Number 17,LVD through EXTI Line detect                       */
#define TAMPER_IRQ_ID                 (3 ) /* Vector Number 18,Tamper Interrupt                                   */
#define RTC_IRQ_ID                    (4 ) /* Vector Number 19,RTC through EXTI Line                              */
#define FMC_IRQ_ID                    (5 ) /* Vector Number 20,FMC                                                */
#define RCC_IRQ_ID                    (6 ) /* Vector Number 21,RCC                                                */
#define EXTI0_IRQ_ID                  (7 ) /* Vector Number 22,EXTI Line 0                                        */
#define EXTI1_IRQ_ID                  (8 ) /* Vector Number 23,EXTI Line 1                                        */
#define EXTI2_IRQ_ID                  (9 ) /* Vector Number 24,EXTI Line 2                                        */
#define EXTI3_IRQ_ID                  (10 ) /* Vector Number 25,EXTI Line 3                                        */
#define EXTI4_IRQ_ID                  (11) /* Vector Number 26,EXTI Line 4                                        */
#define DMA1_Channel1_IRQ_ID          (12 ) /* Vector Number 27,DMA1 Channel 1                                     */
#define DMA1_Channel2_IRQ_ID          (13 ) /* Vector Number 28,DMA1 Channel 2                                     */
#define DMA1_Channel3_IRQ_ID          (14 ) /* Vector Number 29,DMA1 Channel 3                                     */
#define DMA1_Channel4_IRQ_ID          (15 ) /* Vector Number 30,DMA1 Channel 4                                     */
#define DMA1_Channel5_IRQ_ID          (16 ) /* Vector Number 31,DMA1 Channel 5                                     */
#define DMA1_Channel6_IRQ_ID          (17 ) /* Vector Number 32,DMA1 Channel 6                                     */
#define DMA1_Channel7_IRQ_ID          (18 ) /* Vector Number 33,DMA1 Channel 7                                     */
#define ADC1_2_IRQ_ID                 (19 ) /* Vector Number 34,ADC1 and ADC2                                      */
#define CAN1_TX_IRQ_ID                (20 ) /* Vector Number 35,CAN1 TX                                            */
#define CAN1_RX0_IRQ_ID               (21 ) /* Vector Number 36,CAN1 RX0                                           */
#define CAN1_RX1_IRQ_ID               (22 ) /* Vector Number 37,CAN1 RX1                                           */
#define CAN1_SCE_IRQ_ID               (23 ) /* Vector Number 38,CAN1 SCE                                           */
#define EXTI9_5_IRQ_ID                (24 ) /* Vector Number 39,EXTI Line 9..5                                     */
#define TIMER1_BRK_IRQ_ID             (25 ) /* Vector Number 40,TIMER1 Break                                       */
#define TIMER1_UP_IRQ_ID              (26 ) /* Vector Number 41,TIMER1 Update                                      */
#define TIMER1_TRG_COM_IRQ_ID         (27 ) /* Vector Number 42,TIMER1 Break, Update, Trigger and Commutation      */
#define TIMER1_CC_IRQ_ID              (28 ) /* Vector Number 43,TIMER1 Capture Compare                             */
#define TIMER2_IRQ_ID                 (29 ) /* Vector Number 44,TIMER2                                             */
#define TIMER3_IRQ_ID                 (30 ) /* Vector Number 45,TIMER3                                             */
#define TIMER4_IRQ_ID                 (31 ) /* Vector Number 46,TIMER4                                             */
#define I2C1_EV_IRQ_ID                (32 ) /* Vector Number 47,I2C1 Event                                         */
#define I2C1_ER_IRQ_ID                (33 ) /* Vector Number 48,I2C1 Error                                         */
#define I2C2_EV_IRQ_ID                (34 ) /* Vector Number 49,I2C2 Event                                         */
#define I2C2_ER_IRQ_ID                (35 ) /* Vector Number 50,I2C1 Error                                         */
#define SPI1_IRQ_ID                   (36 ) /* Vector Number 51,SPI1                                               */
#define SPI2_IRQ_ID                   (37 ) /* Vector Number 52,SPI2                                               */
#define USART1_IRQ_ID                 (38 ) /* Vector Number 53,USART1                                             */
#define USART2_IRQ_ID                 (39 ) /* Vector Number 54,USART2                                             */
#define USART3_IRQ_ID                 (40 ) /* Vector Number 55,USART3                                             */
#define EXTI15_10_IRQ_ID              (41 ) /* Vector Number 56,External Line[15:10]                               */
#define RTCAlarm_IRQ_ID               (42 ) /* Vector Number 57,RTC Alarm through EXTI Line                        */
#define OTG_FS_WKUP_IRQ_ID            (43 ) /* Vector Number 58,USB OTG FS WakeUp from suspend through EXTI Line   */
#define TIMER8_BRK_TIMER12_IRQ_ID     (44 ) /* Vector Number 59,TIMER8 Break Interrupt and TIMER12 global          */
#define TIMER8_UP_TIMER13_IRQ_ID      (45 ) /* Vector Number 60,TIMER8 Update Interrupt and TIMER13 global         */
#define TIMER8_TRG_COM_TIMER14_IRQ_ID (46 ) /* Vector Number 61,TIMER8 Trigger and Commutation Interrupt and TIMER1*/
#define TIMER8_CC_IRQ_ID              (47 ) /* Vector Number 62,TIMER8 Capture Compare                             */
#define ADC3_IRQ_ID                   (48 ) /* Vector Number 63,ADC3                                               */
#define EXMC_IRQ_ID                   (49 ) /* Vector Number 64,EXMC                                               */
#define SDIO_IRQ_ID                   (50 ) /* Vector Number 65,SDIO                                               */
#define TIMER5_IRQ_ID                 (51 ) /* Vector Number 66,TIMER5                                             */
#define SPI3_IRQ_ID                   (52 ) /* Vector Number 67,SPI3                                               */
#define UART4_IRQ_ID                  (53 ) /* Vector Number 68,UART4                                              */
#define UART5_IRQ_ID                  (54 ) /* Vector Number 69,UART5                                              */
#define TIMER6_IRQ_ID                 (55 ) /* Vector Number 70,TIMER6                                             */
#define TIMER7_IRQ_ID                 (56 ) /* Vector Number 71,TIMER7                                             */
#define DMA2_Channel1_IRQ_ID          (57 ) /* Vector Number 72,DMA2 Channel1                                      */
#define DMA2_Channel2_IRQ_ID          (58 ) /* Vector Number 73,DMA2 Channel2                                      */
#define DMA2_Channel3_IRQ_ID          (59 ) /* Vector Number 74,DMA2 Channel3                                      */
#define DMA2_Channel4_IRQ_ID          (60 ) /* Vector Number 75,DMA2 Channel4                                      */
#define DMA2_Channel5_IRQ_ID          (61 ) /* Vector Number 76,DMA2 Channel5                                      */
#define ETH_IRQ_ID                    (62) /* Vector Number 77,Ethernet                                           */
#define ETH_WKUP_IRQ_ID               (63 ) /* Vector Number 78,Ethernet Wakeup through EXTI line                  */
#define CAN2_TX_IRQ_ID                (64 ) /* Vector Number 79,CAN2 TX                                            */
#define CAN2_RX0_IRQ_ID               (65 ) /* Vector Number 80,CAN2 RX0                                           */
#define CAN2_RX1_IRQ_ID               (66 ) /* Vector Number 81,CAN2 RX1                                           */
#define CAN2_SCE_IRQ_ID               (67 ) /* Vector Number 82,CAN2 SCE                                           */
#define OTG_FS_IRQ_ID                 (68 ) /* Vector Number 83,USB OTG FS                                         */
#define RSV00                         (69 ) /* Vector Number 84,Reserved                                           */
#define DMA2_Channel6_IRQ_ID          (70 ) /* Vector Number 85,DMA2 Channel6                                      */
#define DMA2_Channel7_IRQ_ID          (71 ) /* Vector Number 86,DMA2 Channel7                                      */
#define USART6_IRQ_ID                 (72 ) /* Vector Number 87,USART6                                             */
#define I2C3_EV_IRQ_ID                (73 ) /* Vector Number 88,I2C3 Event                                         */
#define I2C3_ER_IRQ_ID                (74 ) /* Vector Number 89,I2C3 Error                                         */
#define RSV01                         (75 ) /* Vector Number 90,Reserved                                           */
#define RSV02                         (76 ) /* Vector Number 91,Reserved                                           */
#define RSV03                         (77 ) /* Vector Number 92,Reserved                                           */
#define RSV04                         (78 ) /* Vector Number 93,Reserved                                           */
#define DCI_IRQ_ID                    (79 ) /* Vector Number 94,DCI                                                */
#define CRYP_IRQ_ID                   (80 ) /* Vector Number 95,CRYP                                               */
#define HASH_RNG_IRQ_ID               (81 ) /* Vector Number 96,Hash and Rng                                       */
#define RSV05                         (82 ) /* Vector Number 97,Reserved                                           */
#define UART7_IRQ_ID                  (83 ) /* Vector Number 98,UART7                                              */
#define UART8_IRQ_ID                  (84 ) /* Vector Number 99,UART8                                              */
#define RSV06                         (85 ) /* Vector Number 100,Reserved                                          */
#define RSV07                         (86 ) /* Vector Number 101,Reserved                                          */
#define RSV08                         (87 ) /* Vector Number 102,Reserved                                          */
#define RSV09                         (88 ) /* Vector Number 103,Reserved                                          */
#define TLDI_IRQ_ID                   (89 ) /* Vector Number 104,TLDI                                              */
#define TLDI_ER_IRQ_ID                (90 ) /* Vector Number 105,TLDI error                                        */



#endif
