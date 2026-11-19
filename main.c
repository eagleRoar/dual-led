//====================================================================//
// 项目：Dual LED Flash（双路交替闪灯）
// 主控：XC8P9525（8PIN）
// 需求来源：01-需求/需求规格表.xlsx（V1.2）
// 基础工程：2_XC8P9525_RTC中断
// 说明：本文件为软件框架骨架，已实现各功能主体逻辑，
//       标注 [待实测] 的参数需结合样板与硬件调试确认。
//
// 主频：主程序 IRC 8MHz/4T；TCC/RTC 由外部 LXT 32.768kHz 驱动
//--------------------------------------------------------------------//
// 物理引脚 → 芯片内部映射（重要：物理脚号 ≠ P6x 编号）
//   1脚 VDD / 8脚 GND
//   2脚 OSCI(P65) / 3脚 OSCO(P64) → 32.768kHz 晶振
//   4脚 P63（RST复用）→ 模式检测：悬空/高=定时模式，低=常亮模式
//   5脚 P62 → 常亮 LED
//   6脚 P61 → 交替 LED1
//   7脚 P60 → 交替 LED2
//
// LED 极性：原理图 LED 接 VCC，端口输出低电平点亮
//--------------------------------------------------------------------//
// 烧录选项（mpj）关键点：
//   P63端口 = GPIO（绝不能设 RST，否则 4 脚接地会触发复位）
//   RTC = 使能
//   振荡模式 = IRC 8M；Clocks = 4 Clocks
//   晶振脚内置电容 = 25pF（原理图未外挂电容）
//====================================================================//

#include "XC8P9525.h"
#include "XJ_9525Define.h"

//==================== 可配置常量 ====================//
#define LED_ON        0            // 低电平点亮
#define LED_OFF       1

// TCC=128, 32分频：T=(1/32768)*4*32*(256-128)=0.5s
#define RTC_TCC_INIT  128
#define TICKS_PER_SEC 2            // 1 秒 = 2 个 500ms 节拍

#define WORK_HOURS    6            // 定时模式：工作 6 小时
#define CYCLE_HOURS   24           // 定时模式：循环周期 24 小时（工作6 + 关闭18）

#define OSC_TIMEOUT   2            // 晶振检测超时（秒），建议 >=1，[待实测]

#define MODE_TIMER    0            // 定时模式（P63 高/悬空）
#define MODE_ALWAYS   1            // 常亮模式（P63 低）

//==================== 端口定义 ====================//
#define LED_CONST     PORT6_2      // 5脚：常亮 LED
#define LED_ALT_A     PORT6_1      // 6脚：交替 LED1（先亮这一路 [待确认]）
#define LED_ALT_B     PORT6_0      // 7脚：交替 LED2
#define PIN_MODE      PORT6_3      // 4脚：模式检测输入

//==================== 全局变量 ====================//
volatile unsigned char g_tick500;      // 500ms 节拍计数，满 2 为 1 秒
volatile unsigned char g_sec;          // 秒 0..59
volatile unsigned char g_min;          // 分 0..59
volatile unsigned char g_hour;         // 时 0..23
volatile unsigned char g_mode;         // MODE_TIMER / MODE_ALWAYS
volatile unsigned char g_work;         // 1=工作期(亮灯)，0=关闭期(灭灯)
volatile unsigned char g_osc_ok;       // 晶振检测：收到首个中断置 1
volatile unsigned char g_toggle;       // 交替闪烁相位

//==================== 函数声明 ====================//
void file_clrRam(void);
void file_init(void);
void file_osc_check(void);
void file_led_all_off(void);
void file_led_work_on(void);
void file_tick_1s(void);

//================================================//
// MAIN 主程序
//================================================//
void main()
{
    file_clrRam();                 // 清 RAM
    file_init();                   // IO 初始化（LED 全灭）

    file_osc_check();              // 上电检测晶振，失败则停机

    // 读 4 脚判定工作模式
    if(PIN_MODE == 0)
        g_mode = MODE_ALWAYS;      // 接地 → 常亮模式
    else
        g_mode = MODE_TIMER;       // 悬空/高 → 定时模式

    g_hour = 0;
    g_min  = 0;
    g_sec  = 0;
    g_tick500 = 0;
    g_toggle = 0;
    g_work = 1;                    // 上电先进入工作期

    file_led_work_on();            // 开灯：常亮 + 交替
    EI();                          // 打开总中断

    while(1)
    {
        if(g_work == 0)
        {
            // 定时模式关闭期（18h）：进入 SLEEP，由 RTC 中断唤醒
            // RTC 模式下 TCCWE=1 & RTCS=1，休眠时 LXT 继续工作
            SLEEP();
            NOP();
        }
        else
        {
            NOP();                 // 工作期：所有输出由 ISR 节拍驱动
        }
    }
}

//================================================//
// 清 RAM：0x10~0xFE
//================================================//
void file_clrRam()
{
    for(RSR=0xD0; RSR<0xFF; RSR++)
    { IAR = 0; }
    IAR = 0;
}

//================================================//
// 端口与 RTC 初始化
//================================================//
void file_init(void)
{
    IOCP_W(WDTCR,0x00);            // 关 WDT

    // 端口方向：P60/P61/P62 输出，P63 输入
    // P6CR bit3=1 → P63 输入；其余输出
    // 注意：P64/P65 为晶振脚，由 RTC 占用，不作普通 IO
    IOCP_W(P6CR,0x08);

    IOCP_W(ODCR,0x00);             // 关开漏

    // P63 上拉使能（PHCR bit3=0 使能；悬空时为高 → 定时模式）
    // [待实测] 若烧录选项 P6上下拉控制未使能，此位可能无效
    IOCP_W(PHCR,0xF7);
    IOCP_W(PDCR,0xFF);             // 禁止下拉

    // 上电 LED 全灭（低电平点亮，故输出高）
    file_led_all_off();

    //---- RTC 配置：TCC 由外部 32.768kHz 驱动，500ms 中断 ----//
    CONTW(0x84);                   // RTCS=1（选 LXT/4）+ 32 分频
    TCC = RTC_TCC_INIT;            // 初值 128 → 500ms 溢出

    CPUCON = 0x08;                 // TCCWE=1：允许 RTC 在 SLEEP 中唤醒并维持 LXT
                                   // 其余位保持 0（高速模式、IRC）

    ISR = 0x00;                    // 清中断标志
    IOCP_W(IMR,0x01);              // 使能 TCC/RTC 中断

    g_osc_ok = 0;
}

//================================================//
// 晶振检测：上电延时窗口内是否收到首个 RTC 中断
// 未收到 → 晶振未起振 → 芯片完全不输出
// [待实测] 超时窗口与软件延时精度
//================================================//
void file_osc_check(void)
{
    volatile unsigned long i;

    g_osc_ok = 0;
    EI();                          // 开中断，等首个 RTC 中断

    // 基于 IRC 的软件延时约 OSC_TIMEOUT 秒
    // [待实测] 8MHz/4T 下本循环实际时长需校准
    for(i=0; i<(OSC_TIMEOUT*18000UL); i++)
    { NOP(); }

    DI();                          // 关中断后判定

    if(g_osc_ok == 0)
    {
        // 晶振未起振：保持全灭并停机（不再响应任何输出）
        file_led_all_off();
        while(1)
        {
            SLEEP();
            NOP();
        }
    }
    // 晶振正常：中断已产生，继续后续流程（DI 状态，main 中稍后 EI）
}

//================================================//
// LED 控制
//================================================//
void file_led_all_off(void)
{
    LED_CONST = LED_OFF;
    LED_ALT_A = LED_OFF;
    LED_ALT_B = LED_OFF;
}

void file_led_work_on(void)
{
    LED_CONST = LED_ON;            // 5脚常亮
    LED_ALT_A = LED_ON;            // 6脚先亮 [待确认]
    LED_ALT_B = LED_OFF;           // 7脚先灭
    g_toggle = 0;
}

//================================================//
// 1 秒累计（由 2 个 500ms 节拍触发）
//================================================//
void file_tick_1s(void)
{
    g_sec++;
    if(g_sec >= 60)
    {
        g_sec = 0;
        g_min++;
        if(g_min >= 60)
        {
            g_min = 0;
            g_hour++;

            if(g_mode == MODE_TIMER)
            {
                if(g_hour >= WORK_HOURS)
                {
                    // 工作 6 小时到：关闭全部 LED
                    g_work = 0;
                    file_led_all_off();
                }
                if(g_hour >= CYCLE_HOURS)
                {
                    // 24 小时到：从头循环
                    g_hour = 0;
                    g_work = 1;
                    file_led_work_on();
                }
            }
            // 常亮模式不做周期切换，直到电池耗尽（软件无需判断）
        }
    }
}

//================================================//
// 中断服务程序
// RTC 模式：TCC 由 LXT 驱动。TCC 为 8 位上行计数器，
// 溢出后从 0 重新计数，故每次中断须手动重装初值 128，
// 才能维持 500ms 周期。[待实测] 确认重装时机与标志清除顺序
//================================================//
void int_isr(void) __interrupt
{
    __asm__("org 0x08");
    PUSH(_A_BUFF,_R3_BUFF);

    if(TCIF)
    {
        ISR = 0xfe;                // 清 TCIF
        TCC = RTC_TCC_INIT;        // 重装初值，维持 500ms 周期

        if(g_osc_ok == 0)
            g_osc_ok = 1;          // 首个中断：晶振正常

        if(g_work == 1)
        {
            //---- 500ms 交替闪烁 ----//
            g_toggle = !g_toggle;
            if(g_toggle)
            {
                LED_ALT_A = LED_ON;
                LED_ALT_B = LED_OFF;
            }
            else
            {
                LED_ALT_A = LED_OFF;
                LED_ALT_B = LED_ON;
            }
        }

        //---- 计时：每 2 个节拍 = 1 秒（关闭期也需计时）----//
        g_tick500++;
        if(g_tick500 >= TICKS_PER_SEC)
        {
            g_tick500 = 0;
            file_tick_1s();
        }
    }

    POP(_A_BUFF,_R3_BUFF);
}
