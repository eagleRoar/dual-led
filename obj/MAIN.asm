;--------------------------------------------------------
; File Created by SLCC : free open source ANSI-C Compiler
; Version 3.6.0 #Feb 26 2026 (MSVC)
;--------------------------------------------------------
; SL port for the RISC core
;--------------------------------------------------------
;	.file	"E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C"
	list	p=XC8P9525
	radix dec
	include "XC8P9525.inc"
;--------------------------------------------------------
; external declarations
;--------------------------------------------------------
	extern	_IARbits
	extern	_TCCbits
	extern	_PCLbits
	extern	_STATUSbits
	extern	_RSRbits
	extern	_PORT6bits
	extern	_PWMCONbits
	extern	_PRDbits
	extern	_PDC1bits
	extern	_PDC2bits
	extern	_PDC3bits
	extern	_ICIECRbits
	extern	_CPUCONbits
	extern	_ISRbits
	extern	_IAR
	extern	_TCC
	extern	_PCL
	extern	_STATUS
	extern	_RSR
	extern	_PORT6
	extern	_PWMCON
	extern	_PRD
	extern	_PDC1
	extern	_PDC2
	extern	_PDC3
	extern	_ICIECR
	extern	_CPUCON
	extern	_ISR
;--------------------------------------------------------
; global declarations
;--------------------------------------------------------
	global	_file_tick_1s
	global	_file_led_work_on
	global	_file_led_all_off
	global	_file_osc_check
	global	_file_init
	global	_file_clrRam
	global	_main
	global	_int_isr
	global	_A_BUFF
	global	_R3_BUFF
	global	_g_tick500
	global	_g_sec
	global	_g_min
	global	_g_hour
	global	_g_mode
	global	_g_work
	global	_g_osc_ok
	global	_g_toggle

	global STK06
	global STK05
	global STK04
	global STK03
	global STK02
	global STK01
	global STK00

sharebank udata_ovr 0x0020
STK06	res 1
STK05	res 1
STK04	res 1
STK03	res 1
STK02	res 1
STK01	res 1
STK00	res 1

;--------------------------------------------------------
; global definitions
;--------------------------------------------------------
UD_MAIN_0	udata
_g_tick500	res	1

UD_MAIN_1	udata
_g_sec	res	1

UD_MAIN_2	udata
_g_min	res	1

UD_MAIN_3	udata
_g_hour	res	1

UD_MAIN_4	udata
_g_mode	res	1

UD_MAIN_5	udata
_g_work	res	1

UD_MAIN_6	udata
_g_osc_ok	res	1

UD_MAIN_7	udata
_g_toggle	res	1

;--------------------------------------------------------
; absolute symbol definitions
;--------------------------------------------------------
UD_abs_MAIN_10	udata_ovr	0x0010
_A_BUFF
	res	1
UD_abs_MAIN_11	udata_ovr	0x0011
_R3_BUFF
	res	1
;--------------------------------------------------------
; compiler-defined variables
;--------------------------------------------------------
UDL_MAIN_0	udata
_file_osc_check_i_1_16	res	4
;--------------------------------------------------------
; initialized data
;--------------------------------------------------------

;@Allocation info for local variables in function 'main'
;@main file_clrRam               Allocated to registers ;size:2
;@main file_init                 Allocated to registers ;size:2
;@main file_osc_check            Allocated to registers ;size:2
;@main file_led_all_off          Allocated to registers ;size:2
;@main file_led_work_on          Allocated to registers ;size:2
;@main file_tick_1s              Allocated to registers ;size:2
;@main main                      Allocated to registers ;size:2
;@main IARbits                   Allocated to registers ;size:1
;@main TCCbits                   Allocated to registers ;size:1
;@main PCLbits                   Allocated to registers ;size:1
;@main STATUSbits                Allocated to registers ;size:1
;@main RSRbits                   Allocated to registers ;size:1
;@main PORT6bits                 Allocated to registers ;size:1
;@main PWMCONbits                Allocated to registers ;size:1
;@main PRDbits                   Allocated to registers ;size:1
;@main PDC1bits                  Allocated to registers ;size:1
;@main PDC2bits                  Allocated to registers ;size:1
;@main PDC3bits                  Allocated to registers ;size:1
;@main ICIECRbits                Allocated to registers ;size:1
;@main CPUCONbits                Allocated to registers ;size:1
;@main ISRbits                   Allocated to registers ;size:1
;@main A_BUFF                    Allocated to registers ;size:1
;@main R3_BUFF                   Allocated to registers ;size:1
;@main g_tick500                 Allocated to registers ;size:1
;@main g_sec                     Allocated to registers ;size:1
;@main g_min                     Allocated to registers ;size:1
;@main g_hour                    Allocated to registers ;size:1
;@main g_mode                    Allocated to registers ;size:1
;@main g_work                    Allocated to registers ;size:1
;@main g_osc_ok                  Allocated to registers ;size:1
;@main g_toggle                  Allocated to registers ;size:1
;@main IAR                       Allocated to registers ;size:1
;@main TCC                       Allocated to registers ;size:1
;@main PCL                       Allocated to registers ;size:1
;@main STATUS                    Allocated to registers ;size:1
;@main RSR                       Allocated to registers ;size:1
;@main PORT6                     Allocated to registers ;size:1
;@main PWMCON                    Allocated to registers ;size:1
;@main PRD                       Allocated to registers ;size:1
;@main PDC1                      Allocated to registers ;size:1
;@main PDC2                      Allocated to registers ;size:1
;@main PDC3                      Allocated to registers ;size:1
;@main ICIECR                    Allocated to registers ;size:1
;@main CPUCON                    Allocated to registers ;size:1
;@main ISR                       Allocated to registers ;size:1
;@end Allocation info for local variables in function 'main';
;@Allocation info for local variables in function 'file_osc_check'
;@file_osc_check i                         Allocated to registers ;size:4
;@end Allocation info for local variables in function 'file_osc_check';

;--------------------------------------------------------
; overlayable items in internal ram 
;--------------------------------------------------------
;	udata_ovr
;--------------------------------------------------------
; reset vector 
;--------------------------------------------------------
STARTUP	code 0x0000
	jmp	_main
;--------------------------------------------------------
; interrupt and initialization code
;--------------------------------------------------------
c_interrupt	code	0x0003
__sdcc_interrupt:
;***
;  pBlock Stats: dbName = I
;***
;functions called:
;   _file_tick_1s
;   _file_tick_1s
;1 compiler assigned register :
;   r0x100C
;; Starting pCode block
_int_isr:	;Function start
; 0 exit points
;	.line	252; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	__asm__("org 0x08");
	org	0x08
;	.line	253; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	PUSH(_A_BUFF,_R3_BUFF);
	mov	_A_BUFF,a
	swap	_A_BUFF
	swapa	STATUS
	mov	_R3_BUFF,a
;	.line	255; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	if(TCIF)
	JBTS	_ISRbits,0
	JMP	_00189_DS_
;	.line	257; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	ISR = 0xfe;                // 清 TCIF
	MOV	A,@0xfe
	MOV	_ISR,A
;	.line	258; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	TCC = RTC_TCC_INIT;        // 重装初值，维持 500ms 周期
	MOV	A,@0x80
	MOV	_TCC,A
;	.line	260; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	if(g_osc_ok == 0)
	MOV	A,@0x00
	OR	A,_g_osc_ok
	JBTS	STATUS,2
	JMP	_00180_DS_
;	.line	261; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	g_osc_ok = 1;          // 首个中断：晶振正常
	MOV	A,@0x01
	MOV	_g_osc_ok,A
_00180_DS_:
;	.line	263; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	if(g_work == 1)
	MOV	A,_g_work
	XOR	A,@0x01
	JBTS	STATUS,2
	JMP	_00185_DS_
;	.line	266; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	g_toggle = !g_toggle;
	MOV	A,_g_toggle
	MOV	A,@0x00
	JBTC	STATUS,2
	MOV	A,@0x01
	MOV	_g_toggle,A
;;1	MOV	r0x100C,A
;;99	MOV	A,r0x100C
;	.line	267; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	if(g_toggle)
	MOV	A,@0x00
	OR	A,_g_toggle
	JBTC	STATUS,2
	JMP	_00182_DS_
;	.line	269; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	LED_ALT_A = LED_ON;
	BTC	_PORT6bits,1
;	.line	270; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	LED_ALT_B = LED_OFF;
	BTS	_PORT6bits,0
	JMP	_00185_DS_
_00182_DS_:
;	.line	274; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	LED_ALT_A = LED_OFF;
	BTS	_PORT6bits,1
;	.line	275; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	LED_ALT_B = LED_ON;
	BTC	_PORT6bits,0
_00185_DS_:
;	.line	280; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	g_tick500++;
	INC	_g_tick500
;	.line	281; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	if(g_tick500 >= TICKS_PER_SEC)
	MOV	A,@0x02
	SUB	A,_g_tick500
	JBTS	STATUS,0
	JMP	_00189_DS_
;	.line	283; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	g_tick500 = 0;
	CLR	_g_tick500
;	.line	284; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	file_tick_1s();
	CALL	_file_tick_1s
_00189_DS_:
;	.line	288; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	POP(_A_BUFF,_R3_BUFF);
	swapa	_R3_BUFF
	mov	STATUS,a
	swapa	_A_BUFF
;	.line	289; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	}
END_OF_INTERRUPT:
	RETI	

;--------------------------------------------------------
; code
;--------------------------------------------------------
code_MAIN	code
;***
;  pBlock Stats: dbName = M
;***
;has an exit
;functions called:
;   _file_clrRam
;   _file_init
;   _file_osc_check
;   _file_led_work_on
;   _file_clrRam
;   _file_init
;   _file_osc_check
;   _file_led_work_on
;; Starting pCode block
S_MAIN__main	code
_main:	;Function start
; 2 exit points
;	.line	76; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	file_clrRam();                 // 清 RAM
	CALL	_file_clrRam
;	.line	77; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	file_init();                   // IO 初始化（LED 全灭）
	CALL	_file_init
;	.line	79; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	file_osc_check();              // 上电检测晶振，失败则停机
	CALL	_file_osc_check
;	.line	82; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	if(PIN_MODE == 0)
	JBTC	_PORT6bits,3
	JMP	_00106_DS_
;	.line	83; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	g_mode = MODE_ALWAYS;      // 接地 → 常亮模式
	MOV	A,@0x01
	MOV	_g_mode,A
	JMP	_00107_DS_
_00106_DS_:
;	.line	85; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	g_mode = MODE_TIMER;       // 悬空/高 → 定时模式
	CLR	_g_mode
_00107_DS_:
;	.line	87; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	g_hour = 0;
	CLR	_g_hour
;	.line	88; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	g_min  = 0;
	CLR	_g_min
;	.line	89; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	g_sec  = 0;
	CLR	_g_sec
;	.line	90; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	g_tick500 = 0;
	CLR	_g_tick500
;	.line	91; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	g_toggle = 0;
	CLR	_g_toggle
;	.line	92; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	g_work = 1;                    // 上电先进入工作期
	MOV	A,@0x01
	MOV	_g_work,A
;	.line	94; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	file_led_work_on();            // 开灯：常亮 + 交替
	CALL	_file_led_work_on
;	.line	95; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	EI();                          // 打开总中断
	ei	
_00112_DS_:
;	.line	99; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	if(g_work == 0)
	MOV	A,@0x00
	OR	A,_g_work
	JBTS	STATUS,2
	JMP	_00109_DS_
;	.line	103; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	SLEEP();
	sleep	
;	.line	104; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	NOP();
	nop	
	JMP	_00112_DS_
_00109_DS_:
;	.line	108; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	NOP();                 // 工作期：所有输出由 ISR 节拍驱动
	nop	
	JMP	_00112_DS_
;	.line	111; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	}
	RET	
; exit point of _main

;***
;  pBlock Stats: dbName = C
;***
;has an exit
;functions called:
;   _file_led_all_off
;   _file_led_work_on
;   _file_led_all_off
;   _file_led_work_on
;; Starting pCode block
S_MAIN__file_tick_1s	code
_file_tick_1s:	;Function start
; 2 exit points
;	.line	213; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	g_sec++;
	INC	_g_sec
;	.line	214; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	if(g_sec >= 60)
	MOV	A,@0x3c
	SUB	A,_g_sec
	JBTS	STATUS,0
	JMP	_00174_DS_
;	.line	216; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	g_sec = 0;
	CLR	_g_sec
;	.line	217; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	g_min++;
	INC	_g_min
;	.line	218; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	if(g_min >= 60)
	MOV	A,@0x3c
	SUB	A,_g_min
	JBTS	STATUS,0
	JMP	_00174_DS_
;	.line	220; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	g_min = 0;
	CLR	_g_min
;	.line	221; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	g_hour++;
	INC	_g_hour
;	.line	223; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	if(g_mode == MODE_TIMER)
	MOV	A,@0x00
	OR	A,_g_mode
	JBTS	STATUS,2
	JMP	_00174_DS_
;	.line	225; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	if(g_hour >= WORK_HOURS)
	MOV	A,@0x06
	SUB	A,_g_hour
	JBTS	STATUS,0
	JMP	_00165_DS_
;	.line	228; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	g_work = 0;
	CLR	_g_work
;	.line	229; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	file_led_all_off();
	CALL	_file_led_all_off
_00165_DS_:
;	.line	231; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	if(g_hour >= CYCLE_HOURS)
	MOV	A,@0x18
	SUB	A,_g_hour
	JBTS	STATUS,0
	JMP	_00174_DS_
;	.line	234; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	g_hour = 0;
	CLR	_g_hour
;	.line	235; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	g_work = 1;
	MOV	A,@0x01
	MOV	_g_work,A
;	.line	236; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	file_led_work_on();
	CALL	_file_led_work_on
;	.line	242; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	}
_00174_DS_:
	RET	
; exit point of _file_tick_1s

;***
;  pBlock Stats: dbName = C
;***
;has an exit
;; Starting pCode block
S_MAIN__file_led_work_on	code
_file_led_work_on:	;Function start
; 2 exit points
;	.line	202; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	LED_CONST = LED_ON;            // 5脚常亮
	BTC	_PORT6bits,2
;	.line	203; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	LED_ALT_A = LED_ON;            // 6脚先亮 [待确认]
	BTC	_PORT6bits,1
;	.line	204; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	LED_ALT_B = LED_OFF;           // 7脚先灭
	BTS	_PORT6bits,0
;	.line	205; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	g_toggle = 0;
	CLR	_g_toggle
;	.line	206; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	}
	RET	
; exit point of _file_led_work_on

;***
;  pBlock Stats: dbName = C
;***
;has an exit
;; Starting pCode block
S_MAIN__file_led_all_off	code
_file_led_all_off:	;Function start
; 2 exit points
;	.line	195; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	LED_CONST = LED_OFF;
	BTS	_PORT6bits,2
;	.line	196; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	LED_ALT_A = LED_OFF;
	BTS	_PORT6bits,1
;	.line	197; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	LED_ALT_B = LED_OFF;
	BTS	_PORT6bits,0
;	.line	198; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	}
	RET	
; exit point of _file_led_all_off

;***
;  pBlock Stats: dbName = C
;***
;has an exit
;functions called:
;   _file_led_all_off
;   _file_led_all_off
;; Starting pCode block
S_MAIN__file_osc_check	code
_file_osc_check:	;Function start
; 2 exit points
;	.line	167; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	g_osc_ok = 0;
	CLR	_g_osc_ok
;	.line	168; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	EI();                          // 开中断，等首个 RTC 中断
	ei	
;	.line	172; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	for(i=0; i<(OSC_TIMEOUT*18000UL); i++)
	CLR	_file_osc_check_i_1_16
	CLR	(_file_osc_check_i_1_16 + 1)
	CLR	(_file_osc_check_i_1_16 + 2)
	CLR	(_file_osc_check_i_1_16 + 3)
_00135_DS_:
	MOV	A,@0x00
	SUB	A,(_file_osc_check_i_1_16 + 3)
	JBTS	STATUS,2
	JMP	_00151_DS_
	MOV	A,@0x00
	SUB	A,(_file_osc_check_i_1_16 + 2)
	JBTS	STATUS,2
	JMP	_00151_DS_
	MOV	A,@0x8c
	SUB	A,(_file_osc_check_i_1_16 + 1)
	JBTS	STATUS,2
	JMP	_00151_DS_
	MOV	A,@0xa0
	SUB	A,_file_osc_check_i_1_16
_00151_DS_:
	JBTC	STATUS,0
	JMP	_00128_DS_
;	.line	173; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	{ NOP(); }
	nop	
;	.line	172; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	for(i=0; i<(OSC_TIMEOUT*18000UL); i++)
	INC	_file_osc_check_i_1_16
	JBTC	STATUS,2
	INC	(_file_osc_check_i_1_16 + 1)
	JBTC	STATUS,2
	INC	(_file_osc_check_i_1_16 + 2)
	JBTC	STATUS,2
	INC	(_file_osc_check_i_1_16 + 3)
	JMP	_00135_DS_
_00128_DS_:
;	.line	175; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	DI();                          // 关中断后判定
	di	
;	.line	177; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	if(g_osc_ok == 0)
	MOV	A,@0x00
	OR	A,_g_osc_ok
	JBTS	STATUS,2
	JMP	_00137_DS_
;	.line	180; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	file_led_all_off();
	CALL	_file_led_all_off
_00130_DS_:
;	.line	183; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	SLEEP();
	sleep	
;	.line	184; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	NOP();
	nop	
	JMP	_00130_DS_
;	.line	188; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	}
_00137_DS_:
	RET	
; exit point of _file_osc_check

;***
;  pBlock Stats: dbName = C
;***
;has an exit
;functions called:
;   _file_led_all_off
;   _file_led_all_off
;; Starting pCode block
S_MAIN__file_init	code
_file_init:	;Function start
; 2 exit points
;	.line	128; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	IOCP_W(WDTCR,0x00);            // 关 WDT
	mov	a,@0x00
	iw	WDTCR
;	.line	133; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	IOCP_W(P6CR,0x08);
	mov	a,@0x08
	iw	P6CR
;	.line	135; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	IOCP_W(ODCR,0x00);             // 关开漏
	mov	a,@0x00
	iw	ODCR
;	.line	139; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	IOCP_W(PHCR,0xF7);
	mov	a,@0xF7
	iw	PHCR
;	.line	140; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	IOCP_W(PDCR,0xFF);             // 禁止下拉
	mov	a,@0xFF
	iw	PDCR
;	.line	143; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	file_led_all_off();
	CALL	_file_led_all_off
;	.line	146; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	CONTW(0x84);                   // RTCS=1（选 LXT/4）+ 32 分频
	mov	a,@0x84
	ctw
;	.line	147; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	TCC = RTC_TCC_INIT;            // 初值 128 → 500ms 溢出
	MOV	A,@0x80
	MOV	_TCC,A
;	.line	149; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	CPUCON = 0x08;                 // TCCWE=1：允许 RTC 在 SLEEP 中唤醒并维持 LXT
	MOV	A,@0x08
	MOV	_CPUCON,A
;	.line	152; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	ISR = 0x00;                    // 清中断标志
	CLR	_ISR
;	.line	153; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	IOCP_W(IMR,0x01);              // 使能 TCC/RTC 中断
	mov	a,@0x01
	iw	IMR
;	.line	155; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	g_osc_ok = 0;
	CLR	_g_osc_ok
;	.line	156; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	}
	RET	
; exit point of _file_init

;***
;  pBlock Stats: dbName = C
;***
;has an exit
;; Starting pCode block
S_MAIN__file_clrRam	code
_file_clrRam:	;Function start
; 2 exit points
;	.line	118; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	for(RSR=0xD0; RSR<0xFF; RSR++)
	MOV	A,@0xd0
	MOV	_RSR,A
_00119_DS_:
	MOV	A,@0xff
	SUB	A,_RSR
	JBTC	STATUS,0
	JMP	_00117_DS_
;	.line	119; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	{ IAR = 0; }
	CLR	_IAR
;	.line	118; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	for(RSR=0xD0; RSR<0xFF; RSR++)
	INC	_RSR
	JMP	_00119_DS_
_00117_DS_:
;	.line	120; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	IAR = 0;
	CLR	_IAR
;	.line	121; E:\1.WORKSPACE\28.矽杰微\03-双路交替闪灯\03-SOURCE\DUAL-LED\MAIN.C	}
	RET	
; exit point of _file_clrRam


;	code size estimation:
;	  158+    0 =   158 instructions (  316 byte)

	end
