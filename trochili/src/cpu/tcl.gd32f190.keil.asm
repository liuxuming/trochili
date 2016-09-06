        IMPORT  uKernelVariable

        EXPORT  CpuDisableInt
        EXPORT  CpuEnableInt
        EXPORT  CpuEnterCritical
        EXPORT  CpuLeaveCritical
        EXPORT  CpuCalcHiPRIO
        EXPORT  PendSV_Handler

        AREA |.text|, CODE, READONLY, ALIGN=2
        THUMB
        REQUIRE8
        PRESERVE8

CpuCalcHiPRIO
        RBIT    R0, R0
        CLZ     R0, R0
        BX      LR

CpuDisableInt
        CPSID   I
        BX      LR

CpuEnableInt
        CPSIE   I
        BX      LR

CpuEnterCritical
    MRS     R1, PRIMASK
    STR     R1, [R0]
    CPSID   I
    BX      LR

CpuLeaveCritical
    MSR     PRIMASK, R0
    BX      LR

;Cortex-M3进入异常服务例程时,自动压栈了R0-R3,R12,LR(R14,连接寄存器),PSR(程序状态寄存器)和PC(R15).
;PSP不自动压栈，不需要保存到栈中，而是保存到线程结构中
PendSV_Handler
    CPSID   I

; 取得线程内容
    LDR     R0,  =uKernelVariable
    ADD     R1, R0, #4    ;pNominee
    ADD     R0, R0, #8    ;pCurrent

; 更新Nominee状态为运行
    LDR     R3,  [R1]     ;Nominee
    MOV     R2,  #0x1     ;enum：  eThreadRunning
    STR     R2,  [R3,#4]  ;offset：Thread status 
	
; 如果uThreadCurrent和uThreadNominee相等则不需要保存寄存器到栈中
    LDR     R2,  [R0] ;Current
    CMP     R2,  R3
    BEQ     LOAD_NOMINEE_FILE
	
STORE_CURRENT_FILE    
    MRS     R3,  PSP      ;Current psp
    SUBS    R3,  R3, #0x20
    STM     R3,  {R4-R11} ;保存r4-r11到CurrentThread栈中
    STR     R3,  [R2,#8]  ;保存psp到CurrentThread线程结构

    ; 使得CurrentThread = NomineeThread;
    LDR     R3,  [R1]     ;Nominee
    STR     R3,  [R0]     ;Current

LOAD_NOMINEE_FILE
    LDR     R3,  [R3,#8]   ; 根据CurrentThread中取得SP数值到R3
    LDM     R3,  {R4-R11}  ; 从新线程栈中弹出 r4-11
    ADDS    R3,  R3, #0x20 ; psp指向中断自动压栈后的栈顶
    MSR     PSP, R3

    ; 上电后，处理器处于线程+特权模式+msp。
    ; 对于第一次activate任务，当引发pendsv中断后，处理器进入handler模式。使用msp,
    ; 返回时，在这里准备使用psp，从psp中弹出r0...这些寄存器，所以需要修改LR，强制使用psp。
    ORR     LR, LR, #0x04
    CPSIE   I
    
    ;在这里有可能发生中断，而此时新的当前线程的上下文并没有完全恢复。和线程被中断的情景相似：
    ;硬件自动保存部分寄存器到线程栈中，其它寄存器还游离在处理器上下文中。

    ;内核保证以下API不会在中断中被调用
    ; (1)将当前线程从就绪队列中移出的API，
    ; (2)调整当前线程或者其它就绪线程优先级的API
    
    ;假如在此时产生的中断ISR中调用那些 
    ; (1)唤醒了更高优先级的线程，
    ; (2)系统定时器中断，发生时间片轮转
    ;那么有可能导致一次新的PensSv请求被申请。
    ;当下面的语句启动异常返回流程时，会发生前后两个PendSV咬尾中断。
    
    ;按照PendSV_Handler的流程，当前线程的上下文中那些游离的寄存器会再次被保存到线程栈中，即不继续
    ;弹栈，也就是说第一次线程上下文切换的效果即将被取消，转而执行第二次的线程上下文切换。

    ; 启动异常返回流程，弹出r0、r1、r2、r3寄存器，切换到任务。
    BX      LR
    ; 返回后，处理器使用线程+特权模式+psp。线程就在这种环境下运行。

    END