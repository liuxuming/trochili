/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TCL_CONFIG_H
#define _TCL_CONFIG_H

/* 内核时钟节拍配置，硬件定时器每秒中断次数 */
#define TCLC_TIME_TICK_RATE             (100U)

/* 线程优先级 {0,1,2,30,31} 共5个优先级保留给内核用,其它优先级分配给用户线程使用 */
/* 内核支持的最大优先级数 */
#define TCLC_PRIORITY_NUM               (32U)

/* 内核最低线程优先级 */
#define TCLC_LOWEST_PRIORITY            (TCLC_PRIORITY_NUM - 1U)

/* 用户线程优先级范围配置 */
#define TCLC_USER_PRIORITY_LOW          (29U)
#define TCLC_USER_PRIORITY_HIGH         (3U)

/* 内核对象名称长度 */
#define TCL_OBJ_NAME_LEN                (16U)

/* 内核断言功能配置 */
#define TCLC_ASSERT_ENABLE              (1)

/* 栈溢出检查功能配置 */
#define TCLC_THREAD_STACK_CHECK_ENABLE  (1)
#define TCLC_THREAD_STACK_BARRIER_VALUE (0x5A5A5A5A)
#define TCLC_THREAD_STACK_ALARM_RATIO   (90U)         /* n%, 栈使用量阀值百分比         */

/* 各种IPC功能配置 */
#define TCLC_IPC_ENABLE                 (1)
#define TCLC_IPC_SEMAPHORE_ENABLE       (1)
#define TCLC_IPC_MUTEX_ENABLE           (1)
#define TCLC_IPC_MAILBOX_ENABLE         (1)
#define TCLC_IPC_MQUE_ENABLE            (1)
#define TCLC_IPC_FLAGS_ENABLE           (1)

/* 定时器功能配置 */
#define TCLC_TIMER_ENABLE               (1)
#define TCLC_TIMER_WHEEL_SIZE           (32U)

/* 中断管理配置 */
#define TCLC_IRQ_ENABLE                 (1)           /* 使能中断管理功能               */
#define TCLC_IRQ_VECTOR_NUM             (8U)          /* 配置中断向量表表项数目         */
#define TCLC_IRQ_DAEMON_ENABLE          (1)           /* 使能异步中断处理线程           */

/* 动态内存管理配置 */
#define TCLC_MEMORY_ENABLE              (1)
#define TCLC_MEMORY_POOL_ENABLE         (1)
#define TCLC_MEMORY_POOL_PAGES          (256U)       /* 固定页面大小的内存池能管理的最大内存页数 */
#define TCLC_MEMORY_BUDDY_ENABLE        (1)
#define TCLC_MEMORY_BUDDY_PAGES         (64)         /* 伙伴内存算法能管理的最大内存页数         */

/* 内核定时器守护线程优先级、时间片和栈大小 */
#define TCLC_TIMER_DAEMON_PRIORITY      (2U)
#define TCLC_TIMER_DAEMON_SLICE         (10U)
#define TCLC_TIMER_DAEMON_STACK_BYTES   (512U)

/* 内核中断守护线程优先级和时间片 */
#define TCLC_IRQ_DAEMON_PRIORITY        (1U)
#define TCLC_IRQ_DAEMON_SLICE           (10U)
#define TCLC_IRQ_DAEMON_STACK_BYTES     (512U)

/* 内核ROOT线程优先级、时间片和栈大小 */
#define TCLC_ROOT_THREAD_PRIORITY       (TCLC_LOWEST_PRIORITY)
#define TCLC_ROOT_THREAD_SLICE          (0xFFFFFFFF)
#define TCLC_ROOT_THREAD_STACK_BYTES    (512U)

/* 处理器参数配置 */
#define TCLC_CPU_MINIMAL_STACK          (256U)
#define TCLC_CPU_IRQ_NUM                (73)
#define TCLC_CPU_CLOCK_FREQ             (72U*1024U*1024U)

#endif /* _TCL_CONFIG_H */
