/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TOCHILI_STM32_F10X_H
#define _TOCHILI_STM32_F10X_H

#define CM3_NVIC_CTRL    (*((volatile TReg32 *)0xE000E010))   /* SysTick Ctrl & Status Reg. */
#define CM3_NVIC_RELOAD  (*((volatile TReg32 *)0xE000E014))   /* SysTick Reload  Value Reg. */
#define CM3_NVIC_CURRENT (*((volatile TReg32 *)0xE000E018))   /* SysTick CurrentThread Value Reg. */
#define CM3_NVIC_CAL     (*((volatile TReg32 *)0xE000E01C))   /* SysTick Cal     Value Reg. */

#define CM3_NVIC_CTRL_COUNT           (0x00010000U)     /* Count flag.                */
#define CM3_NVIC_CTRL_CLK_SRC         (0x00000004u)     /* Clock Source.              */
#define CM3_NVIC_CTRL_INTEN           (0x00000002U)     /* Interrupt enable.          */
#define CM3_NVIC_CTRL_ENABLE          (0x00000001U)     /* Counter mode.              */

#define CM3_NVIC_SHPR_PENDSV                (0xE000ED22)
#define PENSV_ACTIVE_MASK   (0x1U<<10)
#define CM3_SYS_INT_STATUS  (0xE000ED24)
#define CM3_NVIC_PENDSV_PRIORITY            (0xFF)

#define CM3_NVIC_INT_CTRL                (0xE000ED04)
/* Interrupt control state register. */
#define CM3_NVIC_INT_CTRL_PENDSVSET      (0x1U<<28)
/* Value to trigger PendSV exception.*/
#define CM3_NVIC_INT_CTRL_PENDSVCLR      (0x1U<<27)
/* Value to clear PendSV exception.*/
#define CM3_NVIC_INT_CTRL_PENDSTSET      (0x1U<<26)
/* Value to trigger PendST exception.*/
#define CM3_NVIC_INT_CTRL_PENDSTCLR      (0x1U<<25)
/* Value to clear PendST exception.*/


#define RSV_IRQ0_ID           (0 )  /* Top of Stack                        */
#define Reset_IRQ_ID          (1 )  /* Reset Handler                       */
#define NMI_IRQ_ID            (2 )  /* NMI Handler                         */
#define HardFault_IRQ_ID      (3 )  /* Hard Fault Handler                  */
#define MemManage_IRQ_ID      (4 )  /* MPU Fault Handler                   */
#define BusFault_IRQ_ID       (5 )  /* Bus Fault Handler                   */
#define UsageFault_IRQ_ID     (6 )  /* Usage Fault Handler                 */
#define RSV_IRQ7_ID           (7 )  /* Reserved                            */
#define RSV_IRQ8_ID           (8 )  /* Reserved                            */
#define RSV_IRQ9_ID           (9 )  /* Reserved                            */
#define RSV_IRQ10_ID          (10)  /* Reserved                            */
#define SVC_Handler           (11)  /* SVCall Handler                      */
#define DebugMon_Handler      (12)  /* Debug Monitor Handler               */
#define RSV_IRQ13_ID          (13)  /* Reserved                            */
#define PendSV_Handler        (14)  /* PendSV Handler                      */
#define SysTick_Handler       (15)  /* SysTick Handler                     */
                                                                  
#define WWDG_IRQ_ID           (16)  /* Window Watchdog                     */
#define PVD_IRQ_ID            (17)  /* PVD through EXTI Line detect        */
#define TAMPER_IRQ_ID         (18)  /* Tamper                              */
#define RTC_IRQ_ID            (19)  /* RTC                                 */
#define FLASH_IRQ_ID          (20)  /* Flash                               */
#define RCC_IRQ_ID            (21)  /* RCC                                 */
#define EXTI0_IRQ_ID          (22)  /* EXTI Line 0                         */
#define EXTI1_IRQ_ID          (23)  /* EXTI Line 1                         */
#define EXTI2_IRQ_ID          (24)  /* EXTI Line 2                         */
#define EXTI3_IRQ_ID          (25)  /* EXTI Line 3                         */
#define EXTI4_IRQ_ID          (26)  /* EXTI Line 4                         */
#define DMA1_Channel1_IRQ_ID  (27)  /* DMA1 Channel 1                      */
#define DMA1_Channel2_IRQ_ID  (28)  /* DMA1 Channel 2                      */
#define DMA1_Channel3_IRQ_ID  (29)  /* DMA1 Channel 3                      */
#define DMA1_Channel4_IRQ_ID  (30)  /* DMA1 Channel 4                      */
#define DMA1_Channel5_IRQ_ID  (31)  /* DMA1 Channel 5                      */
#define DMA1_Channel6_IRQ_ID  (32)  /* DMA1 Channel 6                      */
#define DMA1_Channel7_IRQ_ID  (33)  /* DMA1 Channel 7                      */
#define ADC1_2_IRQ_ID         (34)  /* ADC1 and ADC2                       */
#define CAN1_TX_IRQ_ID        (35)  /* CAN1 TX                             */
#define CAN1_RX0_IRQ_ID       (36)  /* CAN1 RX0                            */
#define CAN1_RX1_IRQ_ID       (37)  /* CAN1 RX1                            */
#define CAN1_SCE_IRQ_ID       (38)  /* CAN1 SCE                            */
#define EXTI9_5_IRQ_ID        (39)  /* EXTI Line 9..5                      */
#define TIM1_BRK_IRQ_ID       (40)  /* TIM1 Break                          */
#define TIM1_UP_IRQ_ID        (41)  /* TIM1 Update                         */
#define TIM1_TRG_COM_IRQ_ID   (42)  /* TIM1 Trigger and Commutation        */
#define TIM1_CC_IRQ_ID        (43)  /* TIM1 Capture Compare                */
#define TIM2_IRQ_ID           (44)  /* TIM2                                */
#define TIM3_IRQ_ID           (45)  /* TIM3                                */   
#define TIM4_IRQ_ID           (46)  /* TIM4                                */   
#define I2C1_EV_IRQ_ID        (47)  /* I2C1 Event                          */   
#define I2C1_ER_IRQ_ID        (48)  /* I2C1 Error                          */   
#define I2C2_EV_IRQ_ID        (49)  /* I2C2 Event                          */   
#define I2C2_ER_IRQ_ID        (50)  /* I2C1 Error                          */   
#define SPI1_IRQ_ID           (51)  /* SPI1                                */   
#define SPI2_IRQ_ID           (52)  /* SPI2                                */
#define USART1_IRQ_ID         (53)  /* USART1                              */
#define USART2_IRQ_ID         (54)  /* USART2                              */
#define USART3_IRQ_ID         (55)  /* USART3                              */
#define EXTI15_10_IRQ_ID      (56)  /* EXTI Line 15..10                    */
#define RTCAlarm_IRQ_ID       (57)  /* RTC alarm through EXTI line         */
#define OTG_FS_WKUP_IRQ_ID    (58)  /* USB OTG FS Wakeup through EXTI line */                                                                         
#define RSV_IRQ44_ID          (59)  /* Reserved                            */
#define RSV_IRQ45_ID          (60)  /* Reserved                            */
#define RSV_IRQ46_ID          (61)  /* Reserved                            */
#define RSV_IRQ47_ID          (62)  /* Reserved                            */
#define RSV_IRQ48_ID          (63)  /* Reserved                            */
#define RSV_IRQ49_ID          (64)  /* Reserved                            */
#define RSV_IRQ50_ID          (65)  /* Reserved                            */                                                                        
#define TIM5_IRQ_ID           (66)  /* TIM5                                */
#define SPI3_IRQ_ID           (67)  /* SPI3                                */
#define UART4_IRQ_ID          (68)  /* UART4                               */
#define UART5_IRQ_ID          (69)  /* UART5                               */
#define TIM6_IRQ_ID           (70)  /* TIM6                                */
#define TIM7_IRQ_ID           (71)  /* TIM7                                */
#define DMA2_Channel1_IRQ_ID  (72)  /* DMA2 Channel1                       */
#define DMA2_Channel2_IRQ_ID  (73)  /* DMA2 Channel2                       */
#define DMA2_Channel3_IRQ_ID  (74)  /* DMA2 Channel3                       */
#define DMA2_Channel4_IRQ_ID  (75)  /* DMA2 Channel4                       */
#define DMA2_Channel5_IRQ_ID  (76)  /* DMA2 Channel5                       */
#define ETH_IRQ_ID            (77)  /* Ethernet                            */
#define ETH_WKUP_IRQ_ID       (78)  /* Ethernet Wakeup through EXTI line   */
#define CAN2_TX_IRQ_ID        (79)  /* CAN2 TX                             */
#define CAN2_RX0_IRQ_ID       (80)  /* CAN2 RX0                            */
#define CAN2_RX1_IRQ_ID       (81)  /* CAN2 RX1                            */
#define CAN2_SCE_IRQ_ID       (82)  /* CAN2 SCE                            */
#define OTG_FS_IRQ_ID         (83)  /* USB OTG FS                          */

#define GD32F107_IRQ_NUM                       (OTG_FS_IRQ_ID + 1) 
#define GD32F107_CLOCK_FREQ                    (72*1024*1024)
#define GD32F107_THREAD_STACK_MIN_SIZE         (4*16)

extern void GD32_ETH_WKUP_ISR(void);
extern void GD32_ETH_ISR(void);
extern void GD32_EXTI9_5_ISR(void);
extern void GD32_EXTI15_10_ISR(void);
extern void GD32_TIMER2_ISR(void);
extern void GD32_SysTick_ISR(void);

#define GD32_F107_CLOCK_FREQ         (72*1024*1024)

#endif
