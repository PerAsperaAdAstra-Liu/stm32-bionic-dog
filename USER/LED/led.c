/*=============================================================================
  led.c  板载三盏指示灯

  硬件：LED0 = PE3，LED1 = PE4，LED2 = PG9（低电平点亮）
  用途：语音"开灯/关灯"指令、上电自检跑马灯
=============================================================================*/
#include "led.h"

/****************************************************************************
* 函数名: LED_Init
* 功能  : 初始化 PE3 / PE4 / PG9 为推挽输出，默认全部熄灭
****************************************************************************/
void LED_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOG, ENABLE);

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_3 | GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_Init(GPIOG, &GPIO_InitStructure);

    LED_All_Off();
}

/****************************************************************************
* 函数名: LED_All_On / LED_All_Off
* 功能  : 三盏灯全亮 / 全灭（低电平点亮）
****************************************************************************/
void LED_All_On(void)
{
    LED0 = 0;
    LED1 = 0;
    LED2 = 0;
}

void LED_All_Off(void)
{
    LED0 = 1;
    LED1 = 1;
    LED2 = 1;
}

/****************************************************************************
* 函数名: LED_Run_Test
* 功能  : 跑马灯自检，用于验证工具链与板子是否正常
****************************************************************************/
void LED_Run_Test(void)
{
    u8 i;
    for (i = 0; i < 3; i++)
    {
        LED_All_Off();
        LED0 = 0;  delay_ms(150);
        LED_All_Off();
        LED1 = 0;  delay_ms(150);
        LED_All_Off();
        LED2 = 0;  delay_ms(150);
    }
    LED_All_Off();
}
