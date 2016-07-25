/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TCL_GD32_F190_H
#define _TCL_GD32_F190_H

/* External Interrupts */                
#define     WWDG_IRQID                   (00) /* Vector Number 16,Window Watchdog                             */
#define     LVD_IRQID                    (01) /* Vector Number 17,LVD through EXTI Line detect                */  
#define     RTC_IRQID                    (02) /* Vector Number 18,RTC through EXTI Line                       */
#define     FMC_IRQID                    (03) /* Vector Number 19,FMC                                         */
#define     RCC_IRQID                    (04) /* Vector Number 20,RCC                                         */
#define     EXTI0_1_IRQID                (05) /* Vector Number 21,EXTI Line 0 and EXTI Line 1                 */
#define     EXTI2_3_IRQID                (06) /* Vector Number 22,EXTI Line 2 and EXTI Line 3                 */
#define     EXTI4_15_IRQID               (07) /* Vector Number 23,EXTI Line 4 to EXTI Line 15                 */     
#define     TSI_IRQID                    (08) /* Vector Number 24,TSI                                         */
#define     DMA1_Channel1_IRQID          (09) /* Vector Number 25,DMA1 Channel 1                              */ 
#define     DMA1_Channel2_3_IRQID        (10) /* Vector Number 26,DMA1 Channel 2 and DMA1 Channel 3           */
#define     DMA1_Channel4_5_IRQID        (11) /* Vector Number 27,DMA1 Channel 4 and DMA1 Channel 5           */
#define     ADC1_CMP_IRQID               (12) /* Vector Number 28,ADC1 and Comparator 1-2                     */
#define     TIMER1_BRK_UP_TRG_COM_IRQID  (13) /* Vector Number 29,TIMER1 Break,Update,Trigger and Commutation */
#define     TIMER1_CC_IRQID              (14) /* Vector Number 30,TIMER1 Capture Compare                      */
#define     TIMER2_IRQID                 (15) /* Vector Number 31,TIMER2                                      */
#define     TIMER3_IRQID                 (16) /* Vector Number 32,TIMER3                                      */
#define     TIMER6_DAC_IRQID             (17) /* Vector Number 33,TIMER6 and DAC                              */
#define     Reserved00                   (18) /* Reserved                                                     */
#define     TIMER14_IRQID                (19) /* Vector Number 35,TIMER14                                     */
#define     TIMER15_IRQID                (20) /* Vector Number 36,TIMER15                                     */
#define     TIMER16_IRQID                (21) /* Vector Number 37,TIMER16                                     */
#define     TIMER17_IRQID                (22) /* Vector Number 38,TIMER17                                     */
#define     I2C1_EV_IRQID                (23) /* Vector Number 39,I2C1 Event                                  */
#define     I2C2_EV_IRQID                (24) /* Vector Number 40,I2C2 Event                                  */
#define     SPI1_IRQID                   (25) /* Vector Number 41,SPI1                                        */
#define     SPI2_IRQID                   (26) /* Vector Number 42,SPI2                                        */
#define     USART1_IRQID                 (27) /* Vector Number 43,USART1                                      */
#define     USART2_IRQID                 (28) /* Vector Number 44,USART2                                      */
#define     Reserved01                   (29) /* Reserved                                                     */
#define     CEC_IRQID                    (30) /* Vector Number 46,CEC                                         */
#define     Reserved02                   (31) /* Reserved                                                     */
#define     I2C1_ER_IRQID                (32) /* Vector Number 48,I2C1 Error                                  */
#define     Reserved03                   (33) /* Reserved                                                     */
#define     I2C2_ER_IRQID                (34) /* Vector Number 50,I2C2 Error                                  */
#define     I2C3_EV_IRQID                (35) /* Vector Number 51,I2C3 Event                                  */
#define     I2C3_ER_IRQID                (36) /* Vector Number 52,I2C3 Error                                  */
#define     USB_FS_LP_IRQID              (37) /* Vector Number 53,USB FS LP                                   */
#define     USB_FS_HP_IRQID              (38) /* Vector Number 54,USB FS HP                                   */
#define     Reserved04                   (39) /* Reserved                                                     */
#define     Reserved05                   (40) /* Reserved                                                     */
#define     Reserved06                   (41) /* Reserved                                                     */
#define     USBWakeUp_IRQID              (42) /* Vector Number 58,USB Wakeup                                  */
#define     CAN1_TX_IRQID                (43) /* Vector Number 59,CAN1 TX                                     */
#define     CAN1_RX0_IRQID               (44) /* Vector Number 60,CAN1 RX0                                    */
#define     CAN1_RX1_IRQID               (45) /* Vector Number 61,CAN1 RX1                                    */
#define     CAN1_SCE_IRQID               (46) /* Vector Number 62,CAN1 SCE                                    */
#define     LCD_IRQID                    (47) /* Vector Number 63,LCD                                         */
#define     DMA1_Channel6_7_IRQID        (48) /* Vector Number 64,DMA1 Channel6 and Channel7                  */
#define     Reserved10                   (49) /* Reserved                                                     */
#define     Reserved11                   (50) /* Reserved                                                     */
#define     SPI3_IRQID                   (51) /* Vector Number 67,SPI3                                        */
#define     Reserved12                   (52) /* Reserved                                                     */
#define     Reserved13                   (53) /* Reserved                                                     */
#define     Reserved14                   (54) /* Reserved                                                     */
#define     Reserved15                   (55) /* Reserved                                                     */
#define     Reserved16                   (56) /* Reserved                                                     */
#define     Reserved17                   (57) /* Reserved                                                     */
#define     Reserved18                   (58) /* Reserved                                                     */
#define     Reserved19                   (59) /* Reserved                                                     */
#define     Reserved20                   (60) /* Reserved                                                     */
#define     Reserved21                   (61) /* Reserved                                                     */
#define     Reserved22                   (62) /* Reserved                                                     */
#define     Reserved23                   (63) /* Reserved                                                     */
#define     Reserved24                   (64) /* Reserved                                                     */
#define     Reserved25                   (65) /* Reserved                                                     */
#define     Reserved26                   (66) /* Reserved                                                     */
#define     Reserved27                   (67) /* Reserved                                                     */
#define     Reserved28                   (68) /* Reserved                                                     */
#define     Reserved29                   (69) /* Reserved                                                     */
#define     CAN2_TX_IRQID                (70) /* Vector Number 86,CAN2 TX                                     */
#define     CAN2_RX0_IRQID               (71) /* Vector Number 87,CAN2 RX0                                    */
#define     CAN2_RX1_IRQID               (72) /* Vector Number 88,CAN2 RX1                                    */
#define     CAN2_SCE_IRQID               (73) /* Vector Number 89,CAN2 SCE                                    */


extern void GD32_SysTick_ISR(void);
extern void GD32_EXTI0_1_ISR(void);
extern void EXTI4_15_IRQHandler(void);
extern void GD32_TIM2_ISR(void);
extern void USART2_IRQHandler(void);
#endif
