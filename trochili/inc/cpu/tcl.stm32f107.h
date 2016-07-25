/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TOCHILI_STM32F107_H
#define _TOCHILI_STM32F107_H

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


enum
{
    WWDG_IRQ_ID      = 0, /* Window Watchdog                     */
    PVD_IRQ_ID          , /* PVD through EXTI Line detect        */
    TAMPER_IRQ_ID       , /* Tamper                              */
    RTC_IRQ_ID          , /* RTC                                 */
    FLASH_IRQ_ID        , /* Flash                               */
    RCC_IRQ_ID          , /* RCC                                 */
    EXTI0_IRQ_ID        , /* EXTI Line 0                         */
    EXTI1_IRQ_ID        , /* EXTI Line 1                         */
    EXTI2_IRQ_ID        , /* EXTI Line 2                         */
    EXTI3_IRQ_ID        , /* EXTI Line 3                         */
    EXTI4_IRQ_ID        , /* EXTI Line 4                         */
    DMA1_Channel1_IRQ_ID, /* DMA1 Channel 1                      */
    DMA1_Channel2_IRQ_ID, /* DMA1 Channel 2                      */
    DMA1_Channel3_IRQ_ID, /* DMA1 Channel 3                      */
    DMA1_Channel4_IRQ_ID, /* DMA1 Channel 4                      */
    DMA1_Channel5_IRQ_ID, /* DMA1 Channel 5                      */
    DMA1_Channel6_IRQ_ID, /* DMA1 Channel 6                      */
    DMA1_Channel7_IRQ_ID, /* DMA1 Channel 7                      */
    ADC1_2_IRQ_ID       , /* ADC1 and ADC2                       */
    CAN1_TX_IRQ_ID      , /* CAN1 TX                             */
    CAN1_RX0_IRQ_ID     , /* CAN1 RX0                            */
    CAN1_RX1_IRQ_ID     , /* CAN1 RX1                            */
    CAN1_SCE_IRQ_ID     , /* CAN1 SCE                            */
    EXTI9_5_IRQ_ID      , /* EXTI Line 9..5                      */
    TIM1_BRK_IRQ_ID     , /* TIM1 Break                          */
    TIM1_UP_IRQ_ID      , /* TIM1 Update                         */
    TIM1_TRG_COM_IRQ_ID , /* TIM1 Trigger and Commutation        */
    TIM1_CC_IRQ_ID      , /* TIM1 Capture Compare                */
    TIM2_IRQ_ID         , /* TIM2                                */
    TIM3_IRQ_ID         , /* TIM3                                */
    TIM4_IRQ_ID         , /* TIM4                                */
    I2C1_EV_IRQ_ID      , /* I2C1 Event                          */
    I2C1_ER_IRQ_ID      , /* I2C1 Error                          */
    I2C2_EV_IRQ_ID      , /* I2C2 Event                          */
    I2C2_ER_IRQ_ID      , /* I2C1 Error                          */
    SPI1_IRQ_ID         , /* SPI1                                */
    SPI2_IRQ_ID         , /* SPI2                                */
    USART1_IRQ_ID       , /* USART1                              */
    USART2_IRQ_ID       , /* USART2                              */
    USART3_IRQ_ID       , /* USART3                              */
    EXTI15_10_IRQ_ID    , /* EXTI Line 15..10                    */
    RTCAlarm_IRQ_ID     , /* RTC alarm through EXTI line         */
    OTG_FS_WKUP_IRQ_ID  , /* USB OTG FS Wakeup through EXTI line */
    RESERVERD0           , /* Reserved                            */
    RESERVERD1           , /* Reserved                            */
    RESERVERD2           , /* Reserved                            */
    RESERVERD3           , /* Reserved                            */
    RESERVERD4           , /* Reserved                            */
    RESERVERD5           , /* Reserved                            */
    RESERVERD6           , /* Reserved                            */
    TIM5_IRQ_ID         , /* TIM5                                */
    SPI3_IRQ_ID         , /* SPI3                                */
    UART4_IRQ_ID        , /* UART4                               */
    UART5_IRQ_ID        , /* UART5                               */
    TIM6_IRQ_ID         , /* TIM6                                */
    TIM7_IRQ_ID         , /* TIM7                                */
    DMA2_Channel1_IRQ_ID, /* DMA2 Channel1                       */
    DMA2_Channel2_IRQ_ID, /* DMA2 Channel2                       */
    DMA2_Channel3_IRQ_ID, /* DMA2 Channel3                       */
    DMA2_Channel4_IRQ_ID, /* DMA2 Channel4                       */
    DMA2_Channel5_IRQ_ID, /* DMA2 Channel5                       */
    ETH_IRQ_ID          , /* Ethernet                            */
    ETH_WKUP_IRQ_ID     , /* Ethernet Wakeup through EXTI line   */
    CAN2_TX_IRQ_ID      , /* CAN2 TX                             */
    CAN2_RX0_IRQ_ID     , /* CAN2 RX0                            */
    CAN2_RX1_IRQ_ID     , /* CAN2 RX1                            */
    CAN2_SCE_IRQ_ID     , /* CAN2 SCE                            */
    OTG_FS_IRQ_ID       , /* USB OTG FS                            */
};

extern void STM32_ETH_WKUP_ISR(void);
extern void STM32_ETH_ISR(void);
extern void STM32_EXTI15_10_ISR(void);

#endif
