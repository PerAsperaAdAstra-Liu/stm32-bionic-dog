#ifndef __OLED_H
#define __OLED_H

#include "common.h"

/*=============================================================================
  OLED 显示驱动（0.96 寸，128x64，软件 I2C）

  实物：0.96 寸 4 针 I2C OLED，驱动芯片 SSD1315
        （SSD1315 与 SSD1306 指令兼容，本文件用的是标准的 SSD1306 初始化序列）
        模块丝印顺序：GND VDD SCK SDA（老款是 VCC GND SCL SDA）

  接线（以本文件宏定义为准，与老师代码一致）
    OLED SDA -> PG3
    OLED SCL -> PG4
    OLED VCC -> 3.3V
    OLED GND -> GND
  注意：PG3/PG4 到底是 SDA 还是 SCL，以本文件的宏为准，接反了完全不亮。

  图片取模（Image2Lcd 2.9）：图形模式 / C51 格式 / 行后缀空 / 单色 1bit
    静态表情 128x64；动图帧 107x64（显示起点 x=10，居中）
  取模后如果颜色颠倒，把 OLED_PIC_INVERT 改成 1；
  如果上下颠倒（页内位序反了），把 OLED_PIC_BIT_REVERSE 改成 1。
=============================================================================*/

#define OLED_SDA_PORT        GPIOG
#define OLED_SDA_PIN         GPIO_Pin_3
#define OLED_SCL_PORT        GPIOG
#define OLED_SCL_PIN         GPIO_Pin_4

#define OLED_PIC_INVERT      0   /* 1 = 反色显示（黑白互换） */
#define OLED_PIC_BIT_REVERSE 0   /* 1 = 每个字节内 8 行上下颠倒 */
/* 软件 I2C 速度档：0=不延时(最快) 1=每边沿 1us(推荐，约500kHz，老师代码同档)
   屏幕花屏/点不亮 -> 改成 2；动图卡顿 -> 改成 0 */
#define OLED_I2C_SPEED       1

#define OLED_WIDTH           128
#define OLED_HEIGHT          64

void OLED_Init(void);
void OLED_Clear(void);
void OLED_Refresh(void);
void OLED_ShowPicture(u8 x, u8 y, u8 w, u8 h, const u8 *pic);
void OLED_ShowTestPattern(void);
void OLED_ShowDigit(u8 x, u8 y, u8 num, u8 scale);
void OLED_ShowCmd(u8 cmd);

#endif
