#include "stm32f4xx.h"

/*=============================================================================
  中断服务函数
  说明：USART6_IRQHandler 放在 USER\USART6\usart6.c 里（跟着模块走），
        本文件只放内核异常与 SysTick。
=============================================================================*/

void NMI_Handler(void)
{
}

void HardFault_Handler(void)
{
    while (1)
    {
    }
}

void MemManage_Handler(void)
{
    while (1)
    {
    }
}

void BusFault_Handler(void)
{
    while (1)
    {
    }
}

void UsageFault_Handler(void)
{
    while (1)
    {
    }
}

void SVC_Handler(void)
{
}

void DebugMon_Handler(void)
{
}

void PendSV_Handler(void)
{
}

/* SysTick 中断：延时函数直接查询 SysTick 计数值，这里留空即可 */
void SysTick_Handler(void)
{
}
