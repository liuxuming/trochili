#include "colibri190bsp.h"

#define CH2_THREAD_EXAMPLE1        (21)       /* INIT                 */
#define CH2_THREAD_EXAMPLE2        (22)       /* ACTIVATE             */
#define CH2_THREAD_EXAMPLE3        (23)       /* SUSPEND              */
#define CH2_THREAD_EXAMPLE4        (24)       /* DELAY/UNDELAY        */
#define CH2_THREAD_EXAMPLE5        (26)       /* YIELD                */
#define CH2_THREAD_EXAMPLE6        (27)       /* PRIORITY             */
#define CH2_THREAD_EXAMPLE7        (28)       /* TIME SLICE           */

#define CH4_SEMAPHORE_EXAMPLE1     (41)
#define CH4_SEMAPHORE_EXAMPLE2     (42)
#define CH4_SEMAPHORE_EXAMPLE3     (43)       /* ISR                  */
#define CH4_SEMAPHORE_EXAMPLE4     (44)       /* USART                */
#define CH4_SEMAPHORE_EXAMPLE5     (45)       /* RESET                */
#define CH4_SEMAPHORE_EXAMPLE6     (46)       /* FLUSH                */
#define CH4_SEMAPHORE_EXAMPLE7     (47)       /* DELETE               */
#define CH4_SEMAPHORE_EXAMPLE8     (48)       /* ABORT                */

#define CH5_MUTEX_EXAMPLE1         (51)
#define CH5_MUTEX_EXAMPLE2         (53)       /* RESET                */
#define CH5_MUTEX_EXAMPLE3         (52)       /* FLUSH                */
#define CH5_MUTEX_EXAMPLE4         (54)       /* DELETE               */
#define CH5_MUTEX_EXAMPLE5         (56)       /* ABORT                */

#define CH6_MAILBOX_EXAMPLE1       (61)       /* 异步单向邮件收发     */
#define CH6_MAILBOX_EXAMPLE2       (62)       /* KEY ISR              */
#define CH6_MAILBOX_EXAMPLE3       (63)
#define CH6_MAILBOX_EXAMPLE4       (64)
#define CH6_MAILBOX_EXAMPLE5       (65)       /* RESET                */
#define CH6_MAILBOX_EXAMPLE6       (66)       /* BROADCASE            */
#define CH6_MAILBOX_EXAMPLE7       (67)       /* FLUSH                */
#define CH6_MAILBOX_EXAMPLE8       (68)       /* DELETE               */
#define CH6_MAILBOX_EXAMPLE9       (69)       /* ABORT                */

#define CH7_MESSAGE_EXAMPLE1       (71)
#define CH7_MESSAGE_EXAMPLE2       (72)       /* KEY ISR              */
#define CH7_MESSAGE_EXAMPLE3       (73)
#define CH7_MESSAGE_EXAMPLE4       (74)
#define CH7_MESSAGE_EXAMPLE5       (75)       /* RESET                */
#define CH7_MESSAGE_EXAMPLE6       (76)       /* BROADCASE            */
#define CH7_MESSAGE_EXAMPLE7       (77)       /* FLUSH                */
#define CH7_MESSAGE_EXAMPLE8       (78)       /* DELETE               */
#define CH7_MESSAGE_EXAMPLE9       (79)       /* ABORT                */

#define CH8_FLAGS_EXAMPLE1         (81)
#define CH8_FLAGS_EXAMPLE2         (82)       /* KEY ISR              */
#define CH8_FLAGS_EXAMPLE3         (83)       /* RESET                */
#define CH8_FLAGS_EXAMPLE4         (84)       /* FLUSH                */
#define CH8_FLAGS_EXAMPLE5         (85)       /* DELETE               */
#define CH8_FLAGS_EXAMPLE6         (86)       /* ABORT                */

#define CH9_TIMER_BASIC_EXAMPLE    (91)
#define CH9_TIMER_CONFIG_EXAMPLE   (92)
#define CH9_TIMER_LAGTICKS_EXAMPLE (93)
#define CH9_TIMER_URGENT_EXAMPLE   (94)

#define CH10_IRQ_ISR_EXAMPLE       (101)      /* IRQ  ISR             */
#define CH10_IRQ_ASR_EXAMPLE       (102)      /* IRQ  ASR             */
#define CH10_IRQ_DAEMON_EXAMPLE    (103)      /* IRQ  DAEMON          */

#define CH11_MEMORY_POOL_EXAMPLE   (111)
#define CH11_MEMORY_BUDDY_EXAMPLE  (112)

#define CH13_BOARD_TEST_EXAMPLE    (131)



#define EVB_EXAMPLE       CH13_BOARD_TEST_EXAMPLE

