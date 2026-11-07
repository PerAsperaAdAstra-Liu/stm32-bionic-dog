#include "common.h"

/*=============================================================================
  公共函数实现（延时 + 内核辅助函数）
=============================================================================*/

static u8  fac_us = 0;   /* us 延时倍乘数 */
static u16 fac_ms = 0;   /* ms 延时倍乘数 */

/****************************************************************************
* 函数名: delay_init
* 功能  : 延时初始化。SysTick 时钟源取 HCLK/8 = 21MHz
* 说明  : 必须先调用本函数，delay_us/delay_ms 才能用
****************************************************************************/
void delay_init(void)
{
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
    fac_us = SYSCLK / 8;             /* 168/8 = 21 */
    fac_ms = (u16)fac_us * 1000;     /* 21000 */
}

/****************************************************************************
* 函数名: delay_xms
* 功能  : 内部使用，延时 nms 毫秒（nms 不超过 540，避免 24 位计数器溢出）
****************************************************************************/
static void delay_xms(u16 nms)
{
    u32 midtime;
    SysTick->LOAD = (u32)nms * fac_ms;
    SysTick->VAL  = 0x00;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
    do
    {
        midtime = SysTick->CTRL;
    } while ((midtime & 0x01) && !(midtime & (1 << 16)));   /* 等待计数到 0 */
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
    SysTick->VAL = 0x00;
}

/****************************************************************************
* 函数名: delay_us
* 功能  : 延时 nus 微秒（最大约 798915us）
****************************************************************************/
void delay_us(u32 nus)
{
    u32 midtime;
    SysTick->LOAD = nus * fac_us;
    SysTick->VAL  = 0x00;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
    do
    {
        midtime = SysTick->CTRL;
    } while ((midtime & 0x01) && !(midtime & (1 << 16)));
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
    SysTick->VAL = 0x00;
}

/****************************************************************************
* 函数名: delay_ms
* 功能  : 延时 nms 毫秒（nms: 0~65535）
****************************************************************************/
void delay_ms(u16 nms)
{
    u8  repeat = nms / 540;
    u16 remain = nms % 540;

    while (repeat)
    {
        delay_xms(540);
        repeat--;
    }
    if (remain) delay_xms(remain);
}

/*----------------------------------------------------------------------------
  内核辅助函数
  注意：启明欣欣原例程在这里用 __asm 关键字写内联汇编（WFI_SET 等），
  这种写法只有 Arm Compiler 5 认，Arm Compiler 6 会直接报错。
  这里统一改用 CMSIS 标准内建函数，两个编译器都能编译，功能完全一样。
----------------------------------------------------------------------------*/
void WFI_SET(void)      { __WFI(); }          /* 进入低功耗等待 */
void INTX_DISABLE(void) { __disable_irq(); }  /* 关总中断 */
void INTX_ENABLE(void)  { __enable_irq(); }   /* 开总中断 */
void MSR_MSP(u32 addr)  { __set_MSP(addr); }  /* 设置主栈指针 */
