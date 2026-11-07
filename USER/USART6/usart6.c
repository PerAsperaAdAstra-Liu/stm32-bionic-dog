/*=============================================================================
  usart6.c  与 SU-03T 语音模块通信

  硬件：PC6 = USART6_TX -> 模块 B7(RX)
        PC7 = USART6_RX <- 模块 B6(TX)
  参数：9600 / 8 数据位 / 1 停止位 / 无校验
  协议：模块识别命令词后发送 1~7 的指令字节；
        帧头 AA 55、帧尾 55 AA 等非指令字节直接忽略
  关键：接收中断只置 g_cmd，主循环执行完动作后清零，防止重复触发
=============================================================================*/
#include "usart6.h"

volatile u8 g_cmd     = 0;   /* 指令变量：串口中断写入，主循环读取 */
volatile u8 g_rx_byte = 0;   /* 原始字节（调试用） */

/****************************************************************************
* 函数名: USART6_Init
* 功能  : 初始化 USART6（PC6=TX, PC7=RX），开启接收中断
* 入口  : bound 波特率（默认 9600）
****************************************************************************/
void USART6_Init(u32 bound)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef  NVIC_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE);

    GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_USART6);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_USART6);

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate            = bound;
    USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits            = USART_StopBits_1;
    USART_InitStructure.USART_Parity              = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART6, &USART_InitStructure);

    USART_ITConfig(USART6, USART_IT_RXNE, ENABLE);   /* 接收中断 */

    NVIC_InitStructure.NVIC_IRQChannel                   = USART6_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_Cmd(USART6, ENABLE);
    USART_ClearFlag(USART6, USART_FLAG_TC);
}

/****************************************************************************
* 函数名: USART6_SendByte / USART6_SendString
* 功能  : 发送一个字节 / 一个字符串
****************************************************************************/
void USART6_SendByte(u8 byte)
{
    while (USART_GetFlagStatus(USART6, USART_FLAG_TC) == RESET);
    USART_SendData(USART6, byte);
    while (USART_GetFlagStatus(USART6, USART_FLAG_TC) == RESET);
}

void USART6_SendString(const char *str)
{
    while (*str)
    {
        USART6_SendByte((u8)(*str));
        str++;
    }
}

/****************************************************************************
* 函数名: USART6_IRQHandler
* 功能  : 串口 6 接收中断 —— 把有效指令写进全局变量 g_cmd
* 说明  : 主循环执行完动作后会把 g_cmd 清零，避免同一条指令反复执行
****************************************************************************/
void USART6_IRQHandler(void)
{
    u8 byte;

    if (USART_GetITStatus(USART6, USART_IT_RXNE) != RESET)
    {
        /* 读 DR 同时清除 RXNE 标志 */
        byte = (u8)USART_ReceiveData(USART6);
        g_rx_byte = byte;

        /* 只接受 1~9 的有效指令；0xAA/0x55 等帧头帧尾字节自动忽略 */
        if (byte >= 1 && byte <= 9)
        {
            g_cmd = byte;
        }

#if USART6_ECHO_DEBUG
        USART6_SendByte(byte);      /* 回显，方便串口助手自检 */
#endif
    }

    /* 溢出错误处理：不清掉的话会一直卡在中断里 */
    if (USART_GetFlagStatus(USART6, USART_FLAG_ORE) != RESET)
    {
        (void)USART_ReceiveData(USART6);
    }
}
