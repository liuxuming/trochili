/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.cpu.h"
#include "tcl.kernel.h"
#include "tcl.stm32f030.h"

//NVIC_INT_CTRL   EQU     0xE000ED04                              ; Interrupt control state register.
//NVIC_SYSPRI14   EQU     0xE000ED20                              ; System priority register (priority 14).
//NVIC_PENDSV_PRI EQU     0x00FF0000                              ; PendSV priority value (lowest).
//NVIC_PENDSVSET  EQU     0x10000000                              ; Value to trigger PendSV exception.

#define CM0_SYSTICK_CTRL    (*((volatile TReg32 *)0xE000E010))  /* SysTick Ctrl & Status Reg.       */
#define CM0_SYSTICK_RELOAD  (*((volatile TReg32 *)0xE000E014))  /* SysTick Reload  Value Reg.       */
#define CM0_SYSTICK_CURRENT (*((volatile TReg32 *)0xE000E018))  /* SysTick CurrentThread Value Reg. */
#define CM0_SYSTICK_CAL     (*((volatile TReg32 *)0xE000E01C))  /* SysTick Cal     Value Reg.       */

#define CM0_SYSTICK_CTRL_COUNT         (0x00010000U)            /* Count flag.                      */
#define CM0_SYSTICK_CTRL_CLK_SRC       (0x00000004u)            /* Clock Source.                    */
#define CM0_SYSTICK_CTRL_INTEN         (0x00000002U)            /* Interrupt enable.                */
#define CM0_SYSTICK_CTRL_ENABLE        (0x00000001U)            /* Counter mode.                    */

#define CM0_NVIC_SHPR_PENDSV        (0xE000ED20)
#define PENSV_ACTIVE_MASK           (0x1<<10)
#define CM0_SYS_INT_STATUS          (0xE000ED24)
#define CM0_NVIC_PENDSV_PRIORITY    (0x00FF0000)

#define CM0_NVIC_INT_CTRL           (0xE000ED04)          /* Interrupt control state register.   */
#define CM0_NVIC_INT_CTRL_PENDSVSET (0x1U<<28)            /* Value to trigger PendSV exception.  */
#define CM0_NVIC_INT_CTRL_PENDSVCLR (0x1U<<27)            /* Value to clear PendSV exception.    */
#define CM0_NVIC_INT_CTRL_PENDSTSET (0x1U<<26)            /* Value to trigger PendST exception.  */
#define CM0_NVIC_INT_CTRL_PENDSTCLR (0x1U<<25)            /* Value to clear PendST exception.    */

/*************************************************************************************************
 *  功能：启动内核节拍定时器                                                                     *
 *  参数：无                                                                                     *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void CpuStartTickClock(void)
{
    CM0_SYSTICK_CTRL |= (CM0_SYSTICK_CTRL_CLK_SRC | CM0_SYSTICK_CTRL_ENABLE);
    CM0_SYSTICK_CTRL |= CM0_SYSTICK_CTRL_INTEN;
}


/*************************************************************************************************
 *  功能：请求线程调度                                                                           *
 *  参数：无                                                                                     *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void CpuConfirmThreadSwitch(void)
{
    TCLM_SET_REG32(CM0_NVIC_INT_CTRL, CM0_NVIC_INT_CTRL_PENDSVSET);
}


/*************************************************************************************************
 *  功能：取消线程调度                                                                           *
 *  参数：无                                                                                     *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void CpuCancelThreadSwitch(void)
{
    TCLM_SET_REG32(CM0_NVIC_INT_CTRL, CM0_NVIC_INT_CTRL_PENDSVCLR);
}


/*************************************************************************************************
 *  功能：线程栈和栈帧初始化函数                                                                 *
 *  参数：(1) pTop      线程栈顶地址                                                             *
 *        (2) pStack    线程栈底地址                                                             *
 *        (3) bytes     线程栈大小，以节为单位                                                   *
 *        (4) pEntry    线程函数地址                                                             *
 *        (5) pData     线程函数参数                                                             *
 *  返回：无                                                                                     *
 *  说明：线程栈起始地址必须4字节对齐                                                            *
 *************************************************************************************************/
void CpuBuildThreadStack(TAddr32* pTop, void* pStack, TBase32 bytes,
                         void* pEntry, TArgument argument)
{
    TReg32* pTemp;
    pTemp = (TReg32*)((TBase32)pStack + bytes);

    /* 伪造处理器中断栈现场，在线程第一次被加载运行时使用。
       注意LR的值是个非法值，这决定了线程没法通过LR退出 */
    *(--pTemp) = (TReg32)0x01000000;    /* PSR                     */
    *(--pTemp) = (TReg32)pEntry;         /* 线程函数                */
    *(--pTemp) = (TReg32)0xFFFFFFFE;    /* R14 (LR)                */
    *(--pTemp) = (TReg32)0x12121212;    /* R12                     */
    *(--pTemp) = (TReg32)0x03030303;    /* R3                      */
    *(--pTemp) = (TReg32)0x02020202;    /* R2                      */
    *(--pTemp) = (TReg32)0x01010101;    /* R1                      */
    *(--pTemp) = (TReg32)argument;       /* R0, 线程参数            */

    /* 初始化在处理器硬件中断时不会自动保存的线程上下文，
       这几个寄存器数值没有什么意义,就算内核的指纹吧 */
    *(--pTemp) = (TReg32)0x00000054;    /* R11 ,T                  */
    *(--pTemp) = (TReg32)0x00000052;    /* R10 ,R                  */
    *(--pTemp) = (TReg32)0x0000004F;    /* R9  ,O                  */
    *(--pTemp) = (TReg32)0x00000043;    /* R8  ,C                  */
    *(--pTemp) = (TReg32)0x00000048;    /* R7  ,H                  */
    *(--pTemp) = (TReg32)0x00000049;    /* R6  ,I                  */
    *(--pTemp) = (TReg32)0x0000004C;    /* R5  ,L                  */
    *(--pTemp) = (TReg32)0x00000049;    /* R4  ,I                  */

    *pTop = (TReg32)pTemp;
}


/*************************************************************************************************
 *  功能：初始化处理器                                                                           *
 *  参数：无                                                                                     *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void CpuSetupEntry(void)
{
    /* 初始化systick定时器 */
    TBase32 value = TCLC_CPU_CLOCK_FREQ / TCLC_TIME_TICK_RATE;
    CM0_SYSTICK_RELOAD = (value - 1U);

    /* 配置PENDSV中断优先级 */
    // TCLM_SET_REG32(CM0_NVIC_SHPR_PENDSV, CM0_NVIC_PENDSV_PRIORITY);
}

TPriority CpuCalcHiPRIO(TBase32 data)
{
    int i = 0;
    while(!(data & 0x1))
    {
        i++;
        data = data>>1;
    }
    return i;
}


/* 重写库函数 */
void SysTick_Handler(void)
{
    xKernelEnterIntrState();
    xKernelTickISR();
    xKernelLeaveIntrState();
}

/* 重写库函数 */
void EXTI4_15_IRQHandler(void)
{
#if (TCLC_IRQ_ENABLE)
    xKernelEnterIntrState();
    xIrqEnterISR(EXTI4_15_IRQ_ID);
    xKernelLeaveIntrState();
#else
    return;
#endif
}


/* 重写库函数 */
void TIM2_IRQHandler(void)
{
#if (TCLC_IRQ_ENABLE)
    xKernelEnterIntrState();
    xIrqEnterISR(TIM2_IRQ_ID);
    xKernelLeaveIntrState();
#else
    return;
#endif
}

/* 重写库函数 */
void USART1_IRQHandler(void)
{
#if (TCLC_IRQ_ENABLE)
    xKernelEnterIntrState();
    xIrqEnterISR(USART1_IRQ_ID);
    xKernelLeaveIntrState();
#else
    return;
#endif

}
