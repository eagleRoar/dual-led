//====================================================================//
// 项目：Dual LED Flash（双路交替闪灯）—— 成品板固件复刻
// 主控：XC8P9525（DIP8/SOP8，OTP，1024×14bit ROM，48×8 RAM）
// 基础工程：2_XC8P9525_RTC中断
//
// 本文件按 06-采集双路灯闪数据/dual.csv（2026-11-24，1MHz 采样，67.686s）
// 的实测波形复刻固件行为，参数依据见 01-需求/闪灯模式分析报告.md。
//--------------------------------------------------------------------//
// 一、引脚（物理脚号 ≠ P6x 编号）
//   1脚 VDD / 8脚 GND
//   2脚 OSCI(P65) / 3脚 OSCO(P64)  → 32.768kHz 晶振（LXT，作 RTC 时基）
//   4脚 P63  → 模式检测：悬空(内部上拉)=定时模式；接地=常亮模式
//   5脚 P62  → 常亮 LED（经 100R，高电平点亮）
//   6脚 P61  → 交替 LED 通路 A
//   7脚 P60  → 交替 LED 通路 B
//
//   6脚与7脚之间为两颗反并联 LED：A≠B 才有电流，A==B 灯不亮。
//     P60 高 + P61 低 → A 路导通
//     P60 低 + P61 高 → B 路导通
//   两脚必须推挽输出（ODCR=0），驱动增强开到最大档（HDSxCR=0x55）。
//--------------------------------------------------------------------//
// 二、行为（对标实测）
//   上电  ：延时 500ms 并检测晶振起振，未起振则灯全灭、芯片不工作（需求1）
//   M1    ：慢速交替，每 500ms 换一路，直接交接无灭灯间隔，20 拍 = 10s
//   M2    ：约 9.5k~19kHz 互补 PWM，占空比 2%到100% 线性三角扫描，
//           三角周期约 2.92s（单程 1.46s），峰顶 A 全开约 7.5ms，85 拍 = 42.5s
//   循环  ：M1 → M2 → M1 … 共 105 拍 = 52.5s（实测 52.65s）
//   定时模式：从上电开始计时，工作 6h 后关灯 18h，24h 循环
//   常亮模式（4脚接地）：不做 6h/18h 循环，一直按 M1→M2 循环
//--------------------------------------------------------------------//
// 三、烧录选项（必须在 XJIDE 中确认，否则现象完全不符）
//   P63 端口       = GPIO（绝不能选 RST，否则 4 脚接地会复位）
//   RTC            = 使能（TCC 时钟取 LXT/4）★不开会退化成指令周期计时，
//                    现象是 M1 只有约 39ms、整灯快约 256 倍
//   振荡模式       = IRC 8MHz；Clocks 要与下面的 FCPU_MHZ 一致：
//                    4 Clocks → FCPU_MHZ=2（2MHz，PWM 约 9.5kHz）
//                    2 Clocks → FCPU_MHZ=4（4MHz，PWM 约 19kHz，需 LVR=2.5V）
//   晶振脚内置电容 = 25pF（原理图未外挂电容）
//   P6 上下拉控制  = 使能（P63 内部上拉生效，悬空才判为高）
//   低压复位       = 2.5V（配 8MHz 倍频 + 2Clocks），看门狗 = 禁止
//====================================================================//

#include "XC8P9525.h"
#include "XJ_9525Define.h"

//==================== 编译开关 ====================//
// 1 = 仿真板（无晶振）：跳过 500ms 延时与晶振检测，TCC 走指令周期分频
// 0 = 正式板（有晶振）：上电延时 500ms + 晶振检测，TCC 由 LXT 驱动
#define DEBUG_MODE    1

// 定时模式关闭期（18h）是否 SLEEP 省电（前提：OPTION[RTC]=使能）
#define USE_SLEEP     1

// 指令周期频率，必须与烧录选项【Clocks】对应，否则 PWM 载波和 M2 扫描速度会差 2 倍：
//   Clocks = 4 Clocks  ->  FCPU_MHZ = 2   （IRC 8MHz / 4 = 2MHz）
//   Clocks = 2 Clocks  ->  FCPU_MHZ = 4   （IRC 8MHz / 2 = 4MHz，软件 PWM 能到约 19kHz）
#define FCPU_MHZ      2

//==================== 时基 ====================//
// 正式板：TCC 时钟 = LXT/4 = 8192Hz，PSR=1:32 得 256Hz，重装 128 得 500ms/次
//   【关键前提】OPTION[RTC] 必须 = 使能，否则 CONT.7(RTCS) 被当成普通读写位，
//   TCC 会退回指令周期时钟：PSR 1:32、重装 128、Fcpu 2MHz 时约 1.96ms 一次，
//   现象就是 M1 只有 20*1.96ms 约 39ms，整灯快约 256 倍（不是 M1 逻辑的问题）。
// 仿真板：TCC 时钟 = 指令周期，先出 1ms 中断，再软件分频 500 次得到 500ms 拍
#if DEBUG_MODE
  #if (FCPU_MHZ == 4)
    #define CONT_INIT    0x04      // PSR=100 1:32 -> 125kHz
    #define TCC_RELOAD   131       // 125 计数 = 1ms
  #else
    #define CONT_INIT    0x02      // PSR=010 1:8  -> 250kHz
    #define TCC_RELOAD   6         // 250 计数 = 1ms
  #endif
  #define TICK_DIV     500
  #define CAN_SLEEP    0
#else
  #define CONT_INIT    0x84        // RTCS=1, PSR=100 1:32
  #define TCC_RELOAD   128         // 128 计数 = 500ms
  #define TICK_DIV     1
  #define CAN_SLEEP    USE_SLEEP
#endif

//==================== 节拍参数（1 拍 = 500ms）====================//
#define M1_N       20          // M1：20 拍 = 10s（实测 10.03s）
#define M2_N       85          // M2：85 拍 = 42.5s（实测 42.62s）
#define CYCLE_N    105         // 一个循环 105 拍 = 52.5s（实测 52.65s）

#define WORK_TICKS 43200UL     // 6h  = 21600s / 0.5s
#define DAY_TICKS  172800UL    // 24h = 86400s / 0.5s

//==================== M2 软件 PWM 参数 ====================//
// 一个 PWM 周期切成 50 份：A 路 g_d 份 + B 路 g_e 份（g_d+g_e 恒为 M2_UNIT）
#define M2_UNIT     50         // 每周期 50 个时间单位
#define M2_DMAX     49         // A 路最大 49 份（B 至少留 1 份）
#define PWM_TCY     211        // 一个 PWM 周期的指令周期数，见 file_m2_block 的周期账
#define M2_STEP     (unsigned int)(30400UL * FCPU_MHZ / PWM_TCY)   // 一级驻留 30.4ms
#define M2_PEAK     (unsigned int)(7500UL  * FCPU_MHZ / PWM_TCY)   // 峰顶保持 7.5ms
#define M2_CHUNK    32         // 每个汇编块包含的周期数，越小模式切换越快

#define MODE_TIMER  0          // 定时模式（P63 悬空/高）
#define MODE_ALWAYS 1          // 常亮模式（P63 接地）

//==================== 端口定义 ====================//
#define LED_A      PORT6_0     // 7脚：交替通路 A
#define LED_B      PORT6_1     // 6脚：交替通路 B
#define LED_C      PORT6_2     // 5脚：常亮 LED
#define PIN_MODE   PORT6_3     // 4脚：模式检测输入

//==================== 全局变量 ====================//
volatile unsigned long g_tick;    // 0.5s 计时（0..DAY_TICKS-1）
volatile unsigned char g_mode;    // MODE_TIMER / MODE_ALWAYS
volatile unsigned char g_osc_ok;  // 首个 500ms 中断后置 1（晶振正常）
volatile unsigned int  g_div;     // 仿真板：1ms 到 500ms 的分频计数（必须 16 位）
volatile unsigned char g_cyc;     // 循环内拍序号 1..CYCLE_N
volatile unsigned char g_out;     // M1 相位：1=A 路，0=B 路
volatile unsigned char g_off;     // 1=定时模式关闭期
volatile unsigned char g_m2on;    // 1=主循环需执行 M2 扫描

// 下面 4 个变量被内联汇编直接读写，必须保持 volatile
volatile unsigned char g_d;       // A 路单位数
volatile unsigned char g_e;       // B 路单位数
volatile unsigned char g_cnt;     // 汇编延时循环计数器
volatile unsigned char g_nb;      // 汇编块内剩余的 PWM 周期数

//==================== 函数声明 ====================//
void file_clrRam(void);
void file_init(void);
void file_osc_check(void);
void file_m1_out(void);
void file_m2_run(void);
void file_m2_block(void);
void file_m2_peak_block(void);
unsigned char file_m2_hold(unsigned int n);
unsigned char file_m2_peak_run(void);

//====================================================================//
// 主程序
//====================================================================//
void main(void)
{
    file_clrRam();                 // 清 RAM
    file_init();                   // 端口 + 时基 + 变量清零

#if DEBUG_MODE
    // 仿真板无晶振：不做 500ms 延时与晶振检测
    g_osc_ok = 0;
    g_div    = 0;
    g_tick   = 0;
#else
    file_osc_check();              // 需求1：500ms 延时 + 晶振起振检测
#endif

    // 延时结束后再判 4 脚（此时 P63 上拉已稳定）
    if(PIN_MODE == 0) g_mode = MODE_ALWAYS;
    else              g_mode = MODE_TIMER;

    g_cyc = 0;
    g_out = 0;
    g_off = 0;

    EI();                          // 开总中断，此后节拍由 ISR 驱动

    while(1)
    {
        if(g_m2on)
        {
            file_m2_run();         // M2：阻塞执行占空比扫描，直到 g_m2on 被清 0
            // 防止 PWM 状态残留在引脚上，按当前 M1 相位再刷一次
            if(g_m2on == 0) file_m1_out();
        }
        else if(g_off)
        {
#if CAN_SLEEP
            SLEEP();               // 18h 关闭期：等 TCC 500ms 中断唤醒
#endif
            NOP();
        }
        else
        {
            NOP();                 // M1：输出完全由 ISR 维护
        }
    }
}

//====================================================================//
// 清 RAM：经 RSR/IAR 间接清零
//====================================================================//
void file_clrRam(void)
{
    for(RSR = 0xD0; RSR < 0xFF; RSR++)
    {
        IAR = 0;
    }
    IAR = 0;
}

//====================================================================//
// 端口与时基初始化
//====================================================================//
void file_init(void)
{
    IOCP_W(WDTCR, 0x00);           // 关看门狗；关 P60 外部中断；PWM1/2/3 映射 P62/P61/P60
    IOCP_W(P6CR,  0x38);           // P65/P64/P63 输入；P62/P61/P60 输出
    IOCP_W(ODCR,  0x00);           // 全部推挽，禁止开漏
    IOCP_W(HDS0CR,0x55);           // P60 灌/拉电流最大档（220mA / 210mA）
    IOCP_W(HDS1CR,0x55);           // P61 最大档（215mA / 220mA）
    IOCP_W(HDS2CR,0x55);           // P62 最大档（210mA / 225mA）
    IOCP_W(PHCR,  0xF7);           // 仅 P63 上拉使能（悬空=高=定时模式）
    IOCP_W(PDCR,  0xFF);           // P63..P60 下拉全部禁止
    IOCP_W(PHDCR, 0xFF);           // P65/P64 下拉禁止（不影响晶振）

    LED_A = 0;                     // 两脚同低，灯不亮
    LED_B = 0;
    LED_C = 0;

    // 全部全局变量显式清零（udata 不保证上电为 0）
    g_tick   = 0;
    g_div    = 0;
    g_cyc    = 0;
    g_out    = 0;
    g_off    = 0;
    g_m2on   = 0;
    g_osc_ok = 0;
    g_mode   = MODE_TIMER;
    g_d      = 1;
    g_e      = M2_UNIT - 1;
    g_cnt    = 0;
    g_nb     = 0;

    // 时基：按手册要求，切换 PSR 前先关 WDT
    IOCP_W(WDTCR, 0x00);
    CONTW(CONT_INIT);              // RTCS/PSR 设定
    TCC = TCC_RELOAD;              // 重装（同时清预分频器）

    CPUCON = 0x08;                 // TCCWE=1：休眠时 LXT 继续工作并可唤醒
    ISR    = 0x00;                 // 清全部中断标志
    IOCP_W(IMR, 0x01);             // 只使能 TCC 溢出中断
}

//====================================================================//
// 需求1：上电延时 500ms + 晶振检测
//   延时由 TCC（LXT 驱动）硬件产生，不是软件空循环。
//   晶振未起振则永远等不到中断，死等在此，LED 保持全灭（芯片不工作）。
//====================================================================//
void file_osc_check(void)
{
    g_osc_ok = 0;
    g_div    = 0;

    EI();
    while(g_osc_ok == 0)
    {
        NOP();
    }
    DI();

    // 延时结束，计时从此刻重新开始
    g_tick = 0;
    g_div  = 0;
    TCC    = TCC_RELOAD;
    ISR    = 0x00;
}

//====================================================================//
// 按 g_out 输出 M1 相位（先关旧路再开新路，避免两脚同时为高）
//====================================================================//
void file_m1_out(void)
{
    if(g_out)
    {
        LED_B = 0;
        LED_A = 1;                 // A 路
    }
    else
    {
        LED_A = 0;
        LED_B = 1;                 // B 路
    }
}

//====================================================================//
// M2 单块互补 PWM（g_nb 个周期，占空比由 g_d/g_e 决定）
//
// 周期账（假设 JMP 与条件跳转 2 Tcy，其余 1 Tcy；Tcy = 1/FCPU）
//   置 A 高 B 低         2
//   MOV d 到延时计数     2
//   A 段延时             4*d - 1
//   置 A 低 B 高         2
//   MOV e 到延时计数     2
//   B 段延时             4*e - 1
//   块计数 + 跳回        5
//   ------------------------------------------------
//   合计 4*(d+e) + 11 = 211 Tcy
//     FCPU=4MHz 时约 52.8us（19kHz）；FCPU=2MHz 时约 105.5us（9.5kHz）
//
//   A 高时间 = 4*d + 3，占空比 = (4*d+3)/211
//     d=1  得 3.3%（谷底，目标 2%）
//     d=49 得 93.9%（峰顶前，其余由峰顶全开块补到 100%）
//   注：若示波器实测周期与 211 Tcy 折算值明显不符，说明 JMP/skip 周期数假设不对，
//       只需把 M2_STEP / M2_PEAK 按实测值等比缩放，占空比比例不受影响。
//====================================================================//
void file_m2_block(void)
{
    __asm__(
"Lm2a:\n"
" BTS _PORT6bits,0\n"      // P60=1，A 路开始
" BTC _PORT6bits,1\n"      // P61=0
" MOV A,_g_d\n"
" MOV _g_cnt,A\n"
"Lm2a1:\n"
" NOP\n"
" DJ _g_cnt\n"
" JMP Lm2a1\n"
" BTC _PORT6bits,0\n"      // P60=0
" BTS _PORT6bits,1\n"      // P61=1，B 路开始
" MOV A,_g_e\n"
" MOV _g_cnt,A\n"
"Lm2b1:\n"
" NOP\n"
" DJ _g_cnt\n"
" JMP Lm2b1\n"
" DEC _g_nb\n"             // 块内周期计数减一
" JBTC STATUS,2\n"         // Z=0（未到 0）则跳过出口
" JMP Lm2end\n"
" JMP Lm2a\n"
"Lm2end:\n"
    );
}

//====================================================================//
// M2 峰顶：A 路 100% 全开、B 路全关，保持 g_nb 个周期
//   周期账：4 + (4*50 - 1) + 5 = 208 Tcy，约 52us
//====================================================================//
void file_m2_peak_block(void)
{
    __asm__(
"Lm2p:\n"
" BTS _PORT6bits,0\n"
" BTC _PORT6bits,1\n"
" MOV A,@0x32\n"           // 50 份，整个周期都给 A
" MOV _g_cnt,A\n"
"Lm2p1:\n"
" NOP\n"
" DJ _g_cnt\n"
" JMP Lm2p1\n"
" DEC _g_nb\n"
" JBTC STATUS,2\n"
" JMP Lm2pE\n"
" JMP Lm2p\n"
"Lm2pE:\n"
    );
}

//====================================================================//
// 以当前占空比保持 n 个 PWM 周期；g_m2on 被清 0 时立刻返回 0
//====================================================================//
unsigned char file_m2_hold(unsigned int n)
{
    unsigned char k;

    while(n)
    {
        if(g_m2on == 0) return 0;
        k = (n > M2_CHUNK) ? M2_CHUNK : (unsigned char)n;
        g_nb = k;
        file_m2_block();
        n -= k;
    }
    return g_m2on;
}

//====================================================================//
// 峰顶全开保持 M2_PEAK 个周期
//====================================================================//
unsigned char file_m2_peak_run(void)
{
    unsigned int  n;
    unsigned char k;

    n = M2_PEAK;
    while(n)
    {
        if(g_m2on == 0) return 0;
        k = (n > M2_CHUNK) ? M2_CHUNK : (unsigned char)n;
        g_nb = k;
        file_m2_peak_block();
        n -= k;
    }
    return g_m2on;
}

//====================================================================//
// M2 主流程：三角扫描 2% 到 100%（峰顶全开）再回到 2% …
//   单程 48 级 × M2_STEP 周期 约 1.46s，三角周期约 2.92s（M2_STEP 已按 FCPU 折算）
//====================================================================//
void file_m2_run(void)
{
    unsigned char d;

    g_d = 1;                       // 谷底 2%
    g_e = M2_UNIT - 1;
    if(file_m2_hold(M2_STEP) == 0) return;

    while(g_m2on)
    {
        // 上升：2% 到 98%
        for(d = 2; d <= M2_DMAX; d++)
        {
            g_d = d;
            g_e = M2_UNIT - d;
            if(file_m2_hold(M2_STEP) == 0) return;
        }

        // 峰顶：A 全开 7.5ms
        if(file_m2_peak_run() == 0) return;

        // 下降：98% 回到 2%
        for(d = (M2_DMAX - 1); d >= 2; d--)
        {
            g_d = d;
            g_e = M2_UNIT - d;
            if(file_m2_hold(M2_STEP) == 0) return;
        }

        g_d = 1;
        g_e = M2_UNIT - 1;
        if(file_m2_hold(M2_STEP) == 0) return;
    }
}

//====================================================================//
// TCC 中断服务程序：500ms 一拍，M1/M2 循环与 6h/18h 全部在此推进
//   注意：清中断标志必须整字节写（不能用 BTC / AND）
//====================================================================//
void int_isr(void) __interrupt
{
    __asm__("org 0x08");
    PUSH(_A_BUFF,_R3_BUFF);

    if(TCIF)
    {
        ISR = 0xFE;                // 清 TCIF
        TCC = TCC_RELOAD;          // 重装，同时清预分频器

        g_div++;
        if(g_div >= TICK_DIV)
        {
            g_div = 0;

            if(g_osc_ok == 0)
            {
                g_osc_ok = 1;      // 上电 500ms 到，晶振已正常起振
            }
            else
            {
                g_tick++;

                if(g_mode == MODE_TIMER)
                {
                    if(g_tick >= DAY_TICKS) g_tick = 0;
                    if(g_tick >= WORK_TICKS) g_off = 1;
                    else                     g_off = 0;
                }
                else
                {
                    g_off = 0;     // 常亮模式不做 6h/18h 循环
                }

                if(g_off)
                {
                    // 关闭期：全灭，并让主循环退出 M2
                    g_m2on = 0;
                    g_cyc  = 0;
                    LED_A  = 0;
                    LED_B  = 0;
                    LED_C  = 0;
                }
                else
                {
                    LED_C = 1;

                    g_cyc++;
                    if(g_cyc > CYCLE_N) g_cyc = 1;

                    if(g_cyc <= M1_N)
                    {
                        // M1：每拍翻转一次，两路直接交接
                        g_m2on = 0;
                        g_out ^= 1;
                        if(g_out)
                        {
                            LED_B = 0;
                            LED_A = 1;
                        }
                        else
                        {
                            LED_A = 0;
                            LED_B = 1;
                        }
                    }
                    else
                    {
                        // M2：交给主循环做软件 PWM
                        g_m2on = 1;
                    }
                }
            }
        }
    }

    POP(_A_BUFF,_R3_BUFF);
}
