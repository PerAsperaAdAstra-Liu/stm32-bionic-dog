#ifndef __LED_H
#define __LED_H

#include "common.h"

/*=============================================================================
  指示灯：直接用启明欣欣 407 高配版的板载 LED（低电平点亮）
    LED0 -> PE3
    LED1 -> PE4
    LED2 -> PG9
  语音指令「开灯/关灯」控制这三盏灯
=============================================================================*/

#define LED0  PEout(3)
#define LED1  PEout(4)
#define LED2  PGout(9)

void LED_Init(void);
void LED_All_On(void);
void LED_All_Off(void);
void LED_Run_Test(void);   /* 跑马灯自检：三盏灯轮流亮一次 */

#endif
