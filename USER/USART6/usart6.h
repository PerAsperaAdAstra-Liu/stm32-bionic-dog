#ifndef __USART6_H
#define __USART6_H

#include "common.h"

/*=============================================================================
  串口 6 —— 与 SU03T 离线语音模块通信

  接线：SU03T TX -> PC7(USART6_RX)
        SU03T RX -> PC6(USART6_TX)
        GND 共地，SU03T 供电 5V（实物 SU-03T V1.8 标称 3.6~5.5V，用 5V）
        模块串口电平为 3.3V，与 STM32 直接相连即可，不需要电平转换

  参数：9600 波特率、8 数据位、1 停止位、无校验
        —— 必须与"智能公园"平台上生成的固件完全一致

  协议：语音模块识别成功后从串口发出指令字节（1~9）；
        模块与单片机之间还使用 AA 55 + 数据 + 55 AA 的帧格式，
        本驱动只挑 1~9 的有效指令，其余字节（帧头帧尾等）直接忽略。
=============================================================================*/

#define USART6_BAUD_DEFAULT  9600

/* 调试开关：置 1 时把收到的字节原样回发，方便用串口助手自检收发链路
   （正式演示时可置 0） */
#define USART6_ECHO_DEBUG    0

extern volatile u8 g_cmd;      /* 主循环读取的指令：0=无指令，1~9=有效指令 */
extern volatile u8 g_rx_byte;  /* 最近收到的原始字节，调试观察用 */

void USART6_Init(u32 bound);
void USART6_SendByte(u8 byte);
void USART6_SendString(const char *str);

#endif
