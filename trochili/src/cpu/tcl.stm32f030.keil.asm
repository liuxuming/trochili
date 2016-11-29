	    IMPORT  OsKernelVariable

        EXPORT  OsCpuDisableInt
        EXPORT  OsCpuEnableInt
        EXPORT  OsCpuEnterCritical
        EXPORT  OsCpuLeaveCritical
        EXPORT  OsCpuLoadRootThread
        EXPORT  PendSV_Handler

        AREA |.text|, CODE, READONLY, ALIGN=2
        THUMB
        REQUIRE8
        PRESERVE8


OsCpuDisableInt
        CPSID   I
        BX      LR

OsCpuEnableInt
        CPSIE   I
        BX      LR

OsCpuEnterCritical
    MRS     R1, PRIMASK
    STR     R1, [R0]
    CPSID   I
    BX      LR

OsCpuLeaveCritical
    MSR     PRIMASK, R0
    BX      LR


NVIC_INT_CTRL2   EQU     0xE000ED04                              ; Interrupt control state register.
NVIC_SYSPRI142   EQU     0xE000ED20                              ; System priority register (priority 14).
NVIC_PENDSV_PRI2 EQU     0x00FF0000                              ; PendSV priority value (lowest).
NVIC_PENDSVSET2  EQU     0x10000000                              ; Value to trigger PendSV exception.


OsCpuLoadRootThread
    MOVS    R0, #0
    MSR     PSP, R0

    LDR     R0, =NVIC_SYSPRI142                                ; Set the PendSV exception priority
    LDR     R1, =NVIC_PENDSV_PRI2
    STR     R1, [R0]                                            ; Not storing entire byte with STRB to avoid error


    LDR     R0, =NVIC_INT_CTRL2
    LDR     R1, =NVIC_PENDSVSET2
    STR     R1, [R0]

    CPSIE   I

NeverHere
    B       NeverHere


;Cortex-M3进入异常服务例程时,自动压栈了R0-R3,R12,LR(R14,连接寄存器),PSR(程序状态寄存器)和PC(R15).
;PSP不自动压栈，不需要保存到栈中，而是保存到线程结构中

;CpuSwitchThread
PendSV_Handler
    CPSID   I

; 取得线程内容
	LDR     R0, =OsKernelVariable
	ADDS    R1, R0, #4;Nominee
	ADDS    R0, R0, #8;Current
	
; 如果uThreadCurrent和uThreadNominee相等则不需要保存寄存器到栈中
    LDR     R2,  [R0]
    LDR     R3,  [R1]
	CMP     R2,  R3
	BEQ     LOAD_NOMINEE_FILE
	
; 如果uThreadCurrent线程没有被初始化则不需要保存寄存器到栈中
    LDR     R3,  [R2,#0]
	MOVS    R2,  #1
	ANDS    R3,  R3, R2
	CMP     R3,  #0
	BEQ     SWAP_THREAD
    ;CBZ     R3,  SWAP_THREAD
	
STORE_CURRENT_FILE   
    MRS     R3, PSP
    SUBS    R3, R3, #0x20
	;保存r4-r11到uThreadCurrent栈中
	STR     R4, [R3,#0x0]
	STR     R5, [R3,#0x4]
	STR     R6, [R3,#0x8]
	STR     R7, [R3,#0xc]	
	MOV     R4, R8
	MOV     R4, R9
	MOV     R4, R10
	MOV     R4, R11
	STR     R4, [R3,#0x10]
	STR     R5, [R3,#0x14]
	STR     R6, [R3,#0x18]
	STR     R7, [R3,#0x1c]
   
	;保存psp到uThreadCurrent线程结构
    LDR     R2,  [R0]
    STR     R3,  [R2,#4]  
	
;STORE_CURRENT_FILE2    
    ;MRS     R3,  PSP
    ;SUBS    R3,  R3, #0x10
    ;STM     R3!,  {R4-R7} ;保存r4-r11到uThreadCurrent栈中
    ;SUBS    R0, R0, #0x10
    ;LDR     R2,  [R0]
    ;STR     R3,  [R2,#4]  ;保存psp到uThreadCurrent线程结构
SWAP_THREAD                ; 使得uThreadCurrent = uThreadNominee;
    LDR     R3,  [R1]
    STR     R3,  [R0]

;LOAD_NOMINEE_FILE
    ;LDR     R3,  [R3,#4]   ; 根据uThreadNormine中取得SP数值到R3
    ;LDM     R3!,  {R4-R7}  ; 从新线程栈中弹出 r4-11
    ;ADDS    R3,  R3, #0x20 ; psp指向中断自动压栈后的栈顶
LOAD_NOMINEE_FILE
   ; 根据uThreadNormine中取得SP数值到R3
    LDR     R3, [R3,#4]   
	; 从新线程栈中弹出 r4-11
    LDR     R4, [R3,#0x10]
	LDR     R5, [R3,#0x14]
	LDR     R6, [R3,#0x18]
    LDR     R7, [R3,#0x1c]
    MOV     R8, R4
	MOV     R9, R5
	MOV     R10,R6
	MOV     R11,R7
	LDR     R4, [R3,#0x0]
	LDR     R5, [R3,#0x4]
	LDR     R6, [R3,#0x8]
	LDR     R7, [R3,#0xc]
    ; psp指向中断自动压栈后的栈顶	
    ADDS    R3,  R3, #0x20 
    MSR     PSP, R3

    ; 上电后，处理器处于线程+特权模式+msp。
    ; 对于第一次activate任务，当引发pendsv中断后，处理器进入handler模式。使用msp,
    ; 返回时，在这里准备使用psp，从psp中弹出r0...这些寄存器，所以需要修改LR，强制使用psp。
    MOVS    R2, #0x4
    MOV     R3, R14
    ORRS    R3, R3, R2
	MOV     R14, R3
    CPSIE   I
    
    ;在这里有可能发生中断，而此时新的当前线程的上下文并没有完全恢复。和线程被中断的情景相似：
    ;硬件自动保存部分寄存器到线程栈中，其它寄存器还游离在处理器上下文中。
    ;假如在此时产生的中断ISR中调用那些
    ; (1)会将当前线程从就绪队列中移出的API，
    ; (2)或者唤醒了更高优先级的线程，
    ; (3)调整当前线程或者其它就绪线程的优先级
    ; (4)系统定时器中断，发生时间片轮转
    ;那么有可能导致一次新的PensSv请求被挂起。
    ;当下面的语句启动异常返回流程时，会发生前后两个PendSV咬尾中断。
    ;按照uCM3PendSVHandler的流程，当前线程的上下文中那些游离的寄存器会再次被保存到线程栈中，即不继续
    ;弹栈，也就是说第一次线程上下文切换被强制取消了，转而执行第二次的线程上下文切换。

    ; 启动异常返回流程，弹出r0、r1、r2、r3寄存器，切换到任务。
    BX      LR
    ; 返回后，处理器使用线程+特权模式+psp。线程就在这种环境下运行。

    END