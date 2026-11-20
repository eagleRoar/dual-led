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
//
// 时基说明：
//   RTC 时 TCC 时钟 = LXT/4 = 8192Hz。
//   工作期：CONT=0x82（预分频8）、TCC=255 → 1024Hz 中断（0.9766ms/次），
//           g_tick 以 1/1024 秒为刻度累加，get_time_ms() 换算为 1ms 粒度。
//   关闭期：切回 CONT=0x84（预分频32）、TCC=128 → 500ms 中断，g_tick 每次 +512，
//           接口不变且 CPU 只在 500ms 醒来一次，保持低功耗。
//   24 小时 = 88473600 ticks；工作 6 小时 = 22118400 ticks。
//====================================================================//

#include "XC8P9525.h"
#include "XJ_9525Define.h"

//==================== 可配置常量 ====================//
#define LED_ON        0            // 低电平点亮
#define LED_OFF       1

#define RTC_CONT_FAST 0x82         // RTCS=1, PAB=0, PSR=010 -> TCC 8分频
#define TCC_FAST      255          // 8*(256-255)/(32768/4) = 1/1024 s
#define RTC_CONT_SLOW 0x84         // RTCS=1, PAB=0, PSR=100 -> TCC 32分频
#define TCC_SLOW      128          // 32*(256-128)/(32768/4) = 0.5 s

#define DAY_TICKS     88473600UL   // 24h * 1024
#define WORK_TICKS    22118400UL   // 6h  * 1024
#define HALF_TICK_BIT 0x0200UL     // bit9：512 ticks = 500ms

#define OSC_TIMEOUT   2            // 晶振检测超时（秒），建议 >=1，[待实测]

#define MODE_TIMER    0            // 定时模式（P63 高/悬空）
#define MODE_ALWAYS   1            // 常亮模式（P63 低）

//==================== 端口定义 ====================//
#define LED_CONST     PORT6_2      // 5脚：常亮 LED
#define LED_ALT_A     PORT6_1      // 6脚：交替 LED1（先亮这一路 [待确认]）
#define LED_ALT_B     PORT6_0      // 7脚：交替 LED2
#define PIN_MODE      PORT6_3      // 4脚：模式检测输入

//==================== 全局变量 ====================//
volatile unsigned long g_tick;     // 1/1024 秒刻度，整天循环（0..DAY_TICKS-1）
volatile unsigned char g_mode;     // MODE_TIMER / MODE_ALWAYS
volatile unsigned char g_osc_ok;   // 晶振检测：收到首个中断置 1
volatile unsigned char g_fast;     // 1=1024Hz 工作期节拍，0=500ms 关闭期节拍

//==================== 函数声明 ====================//
void file_clrRam(void);
void file_init(void);
void file_osc_check(void);
void file_led_all_off(void);
void file_led_work_on(void);
void file_leds_update(unsigned char work_now);
void file_timer_set(unsigned char fast);
unsigned long get_time_ms(void);

//================================================//
// get_time_ms：返回当天已运行的毫秒数（0..86399999）
// g_tick 为 1/1024 秒刻度，避免变量乘法/除法，仅用移位与减法换算。
//================================================//
unsigned long get_time_ms(void)
{
    unsigned long sec;
    unsigned long rem;
    unsigned long ms;

    sec = g_tick >> 10;
    rem = g_tick & 0x03FFUL;

    // sec * 1000 = sec*1024 - sec*16 - sec*8
    ms = (sec << 10) - (sec << 4) - (sec << 3);
    // rem * 1000 / 1024，先乘再右移，rem 最大 1023 不会溢出
    ms += (((rem << 10) - (rem << 4) - (rem << 3)) >> 10);

    return ms;
}

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

    g_tick = 0;
    g_fast = 1;                    // 上电先进入工作期（1024Hz 节拍）
    file_led_work_on();            // 开灯：常亮 + 交替
    EI();                          // 打开总中断

    while(1)
    {
        if((g_mode == MODE_TIMER) && (g_tick >= WORK_TICKS))
        {
            // 定时模式关闭期（18h）：进入 SLEEP，由 RTC 500ms 中断唤醒
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

    //---- RTC 配置：工作期 1024Hz，关闭期由 ISR 切回 500ms ----//
    file_timer_set(1);

    CPUCON = 0x08;                 // TCCWE=1：允许 RTC 在 SLEEP 中唤醒并维持 LXT
                                   // 其余位保持 0（高速模式、IRC）

    ISR = 0x00;                    // 清中断标志
    IOCP_W(IMR,0x01);              // 使能 TCC/RTC 中断

    g_osc_ok = 0;
}

//================================================//
// 切换 RTC 节拍：调用时要求中断关闭（初始化阶段或 ISR 内）
//================================================//
void file_timer_set(unsigned char fast)
{
    if(fast)
    {
        CONTW(RTC_CONT_FAST);
        TCC = TCC_FAST;            // 写 TCC 会清预分频器，周期从写入后重新计算
        g_fast = 1;
    }
    else
    {
        CONTW(RTC_CONT_SLOW);
        TCC = TCC_SLOW;
        g_fast = 0;
    }
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
    EI();                          // 开中断，等首个 RTC 中断（约 1ms 后到来）

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
}

//================================================//
// 按当前时刻刷新 LED：相位直接取自 g_tick 的 bit9（512 ticks = 500ms）
//================================================//
void file_leds_update(unsigned char work_now)
{
    if(work_now == 0)
    {
        file_led_all_off();
        return;
    }

    LED_CONST = LED_ON;
    if((g_tick & HALF_TICK_BIT) == 0)
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

//================================================//
// 中断服务程序
// RTC 模式：TCC 由 LXT 驱动，8 位上行计数器。
// 工作期重装 255 → 1024Hz；关闭期重装 128 → 500ms。
// 写 TCC 会清预分频器，因此周期稳定。
//================================================//
void int_isr(void) __interrupt
{
    unsigned char work_now;
    unsigned char new_fast;
    unsigned char osc_ready;

    __asm__("org 0x08");
    PUSH(_A_BUFF,_R3_BUFF);

    if(TCIF)
    {
        ISR = 0xfe;                // 清 TCIF

        if(g_fast)
        {
            TCC = TCC_FAST;        // 1024Hz 节拍
            g_tick++;
        }
        else
        {
            TCC = TCC_SLOW;        // 500ms 节拍
            g_tick += 512;
        }

        if(g_tick >= DAY_TICKS)    // 24h 循环
            g_tick -= DAY_TICKS;

        osc_ready = g_osc_ok;
        if(osc_ready == 0)
            g_osc_ok = 1;          // 首个中断：晶振正常

        // 晶振检测阶段保持 LED 全灭，确认后 main 再开灯
        work_now = 0;
        if(osc_ready)
        {
            work_now = 1;
            if((g_mode == MODE_TIMER) && (g_tick >= WORK_TICKS))
                work_now = 0;      // 6h 后进入关闭期
        }

        file_leds_update(work_now);

        // 工作期用 1024Hz 保证 1ms 粒度；关闭期用 500ms 保持低功耗
        new_fast = 1;
        if(osc_ready && (g_mode == MODE_TIMER) && (g_tick >= WORK_TICKS))
            new_fast = 0;
        if(new_fast != g_fast)
            file_timer_set(new_fast);
    }

    POP(_A_BUFF,_R3_BUFF);
}
