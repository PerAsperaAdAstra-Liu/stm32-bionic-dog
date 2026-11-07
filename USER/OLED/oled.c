/*=============================================================================
  oled.c  SSD1306/SSD1315 单色 OLED 驱动（软件 I2C）

  硬件：SDA = PG3，SCL = PG4，VCC = 3.3V
  结构：1024 字节显存 OLED_GRAM[8][128]，改动后调用 OLED_Refresh() 上屏
  约定：每字节 bit0 对应一页中最上面一行（与取模软件 LSB 在前一致）
  兼容：OLED_PIC_INVERT 处理黑白反色，OLED_PIC_BIT_REVERSE 处理上下颠倒
=============================================================================*/
#include "oled.h"

/*=============================================================================
  显存 GRAM：8 页 x 128 列，每页 8 行
  约定：一个字节的 bit0 = 该页最上面一行，bit7 = 最下面一行
  （这正是 SSD1306 页寻址的原生顺序）
=============================================================================*/
static u8 OLED_GRAM[8][128];

/*--------------------------- 软件 I2C 底层 ---------------------------*/
#define OLED_SCL_H()   GPIO_SetBits(OLED_SCL_PORT, OLED_SCL_PIN)
#define OLED_SCL_L()   GPIO_ResetBits(OLED_SCL_PORT, OLED_SCL_PIN)
#define OLED_SDA_H()   GPIO_SetBits(OLED_SDA_PORT, OLED_SDA_PIN)
#define OLED_SDA_L()   GPIO_ResetBits(OLED_SDA_PORT, OLED_SDA_PIN)

#if OLED_I2C_SPEED == 0
#define OLED_I2C_DELAY()                  /* 最快：不加延时 */
#elif OLED_I2C_SPEED == 1
#define OLED_I2C_DELAY()  delay_us(1)     /* 推荐：约 500kHz，老师代码同档 */
#else
#define OLED_I2C_DELAY()  delay_us(2)     /* 最稳：花屏时用这档 */
#endif

static void OLED_I2C_Start(void)
{
    OLED_SDA_H();  OLED_SCL_H();  OLED_I2C_DELAY();
    OLED_SDA_L();  OLED_I2C_DELAY();
    OLED_SCL_L();  OLED_I2C_DELAY();
}

static void OLED_I2C_Stop(void)
{
    OLED_SDA_L();  OLED_I2C_DELAY();
    OLED_SCL_H();  OLED_I2C_DELAY();
    OLED_SDA_H();  OLED_I2C_DELAY();
}

static void OLED_I2C_SendByte(u8 byte)
{
    u8 i;
    for (i = 0; i < 8; i++)
    {
        if (byte & 0x80) OLED_SDA_H();
        else             OLED_SDA_L();
        OLED_I2C_DELAY();
        OLED_SCL_H();  OLED_I2C_DELAY();
        OLED_SCL_L();  OLED_I2C_DELAY();
        byte <<= 1;
    }
    /* 第 9 个时钟：不处理应答信号 */
    OLED_SCL_H();  OLED_I2C_DELAY();
    OLED_SCL_L();  OLED_I2C_DELAY();
}

/****************************************************************************
* 函数名: OLED_WriteCmd / OLED_WriteDataBuf
* 功能  : 写一个命令字节 / 连续写一批数据字节（一次 I2C 事务，刷新更快）
****************************************************************************/
static void OLED_WriteCmd(u8 cmd)
{
    OLED_I2C_Start();
    OLED_I2C_SendByte(0x78);   /* 从机地址 */
    OLED_I2C_SendByte(0x00);   /* 后跟命令 */
    OLED_I2C_SendByte(cmd);
    OLED_I2C_Stop();
}

static void OLED_WriteDataBuf(const u8 *buf, u16 len)
{
    u16 i;
    OLED_I2C_Start();
    OLED_I2C_SendByte(0x78);
    OLED_I2C_SendByte(0x40);   /* 后跟数据 */
    for (i = 0; i < len; i++)
    {
        OLED_I2C_SendByte(buf[i]);
    }
    OLED_I2C_Stop();
}

/****************************************************************************
* 函数名: OLED_DrawPixel / OLED_FillRect
* 功能  : 在显存里画点、填充矩形（color: 1=亮 0=灭）
****************************************************************************/
static void OLED_DrawPixel(u8 x, u8 y, u8 color)
{
    if (x >= OLED_WIDTH || y >= OLED_HEIGHT) return;
    if (color) OLED_GRAM[y / 8][x] |=  (u8)(1 << (y % 8));
    else       OLED_GRAM[y / 8][x] &= (u8)~(1 << (y % 8));
}

static void OLED_FillRect(u8 x, u8 y, u8 w, u8 h, u8 color)
{
    u8 i, j;
    for (i = 0; i < w; i++)
        for (j = 0; j < h; j++)
            OLED_DrawPixel((u8)(x + i), (u8)(y + j), color);
}

/****************************************************************************
* 函数名: OLED_Init
* 功能  : 配置 PG3/PG4 为开漏输出，并按 SSD1306 数据手册写入初始化序列
****************************************************************************/
void OLED_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);

    GPIO_InitStructure.GPIO_Pin   = OLED_SCL_PIN | OLED_SDA_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;    /* 开漏 + 内部上拉 */
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(OLED_SCL_PORT, &GPIO_InitStructure);

    OLED_SCL_H();
    OLED_SDA_H();
    delay_ms(50);

    OLED_WriteCmd(0xFD);        /* 解锁驱动 IC 命令（与老师代码一致的初始化序列） */
    OLED_WriteCmd(0x12);
    OLED_WriteCmd(0xAE);        /* 关显示 */
    OLED_WriteCmd(0xD5);        /* 时钟分频 / 振荡频率 */
    OLED_WriteCmd(0xA0);
    OLED_WriteCmd(0xA8);        /* 多路复用比 */
    OLED_WriteCmd(0x3F);        /* 1/64 */
    OLED_WriteCmd(0xD3);        /* 显示偏移 */
    OLED_WriteCmd(0x00);
    OLED_WriteCmd(0x40);        /* 起始行 = 0 */
    OLED_WriteCmd(0x8D);        /* 电荷泵 */
    OLED_WriteCmd(0x14);        /* 打开 */
    OLED_WriteCmd(0x20);        /* 寻址模式 */
    OLED_WriteCmd(0x02);        /* 页寻址 */
    OLED_WriteCmd(0xA1);        /* 列地址左右镜像 */
    OLED_WriteCmd(0xC8);        /* 行扫描上下镜像 */
    OLED_WriteCmd(0xDA);        /* COM 引脚配置 */
    OLED_WriteCmd(0x12);
    OLED_WriteCmd(0x81);        /* 对比度 */
    OLED_WriteCmd(0x7F);
    OLED_WriteCmd(0xD9);        /* 预充电周期 */
    OLED_WriteCmd(0x82);
    OLED_WriteCmd(0xDB);        /* VCOMH 电压 */
    OLED_WriteCmd(0x34);
    OLED_WriteCmd(0xA4);        /* 正常显示（跟随 RAM） */
    OLED_WriteCmd(0xA6);        /* 非反色 */
    OLED_WriteCmd(0xAF);        /* 开显示 */

    OLED_Clear();
}

/****************************************************************************
* 函数名: OLED_Clear / OLED_Refresh
* 功能  : 清显存 / 把显存整体写入屏幕（8 页 x 128 列）
****************************************************************************/
void OLED_Clear(void)
{
    u8 page, col;
    for (page = 0; page < 8; page++)
        for (col = 0; col < 128; col++)
            OLED_GRAM[page][col] = 0x00;
}

void OLED_Refresh(void)
{
    u8 page;
    for (page = 0; page < 8; page++)
    {
        OLED_WriteCmd((u8)(0xB0 | page));   /* 页地址 */
        OLED_WriteCmd(0x10);                /* 列地址高 4 位 */
        OLED_WriteCmd(0x00);                /* 列地址低 4 位 */
        OLED_WriteDataBuf(&OLED_GRAM[page][0], 128);
    }
}

#if OLED_PIC_BIT_REVERSE
static u8 OLED_BitReverse(u8 b)
{
    b = (u8)((b & 0xF0) >> 4 | (b & 0x0F) << 4);
    b = (u8)((b & 0xCC) >> 2 | (b & 0x33) << 2);
    b = (u8)((b & 0xAA) >> 1 | (b & 0x55) << 1);
    return b;
}
#endif

/****************************************************************************
* 函数名: OLED_ShowPicture
* 功能  : 显示一张单色图片到显存
* 入口  : x,y 起点坐标（y 需为 8 的整数倍）；w 宽；h 高；pic 取模数组
* 说明  : 数组排列 = 每页 w 个字节，共 h/8 页（Image2Lcd 的常规输出）
****************************************************************************/
void OLED_ShowPicture(u8 x, u8 y, u8 w, u8 h, const u8 *pic)
{
    u8 page, col, page0, pages;
    const u8 *p = pic;

    pages = (u8)(h / 8);
    page0 = (u8)(y / 8);

    for (page = 0; page < pages; page++)
    {
        for (col = 0; col < w; col++)
        {
            u8 data = *p++;
#if OLED_PIC_BIT_REVERSE
            data = OLED_BitReverse(data);
#endif
#if OLED_PIC_INVERT
            data = (u8)~data;
#endif
            if (((u8)(page0 + page) < 8) && ((u8)(x + col) < OLED_WIDTH))
            {
                OLED_GRAM[page0 + page][x + col] = data;
            }
        }
    }
}

/****************************************************************************
* 函数名: OLED_ShowDigit
* 功能  : 显示一个 0~9 的数字（3x5 点阵，scale 倍放大）
****************************************************************************/
static const u8 digit_3x5[10][5] =
{
    { 0x07, 0x05, 0x05, 0x05, 0x07 },   /* 0 */
    { 0x02, 0x06, 0x02, 0x02, 0x07 },   /* 1 */
    { 0x07, 0x01, 0x07, 0x04, 0x07 },   /* 2 */
    { 0x07, 0x01, 0x07, 0x01, 0x07 },   /* 3 */
    { 0x05, 0x05, 0x07, 0x01, 0x01 },   /* 4 */
    { 0x07, 0x04, 0x07, 0x01, 0x07 },   /* 5 */
    { 0x07, 0x04, 0x07, 0x05, 0x07 },   /* 6 */
    { 0x07, 0x01, 0x01, 0x01, 0x01 },   /* 7 */
    { 0x07, 0x05, 0x07, 0x05, 0x07 },   /* 8 */
    { 0x07, 0x05, 0x07, 0x01, 0x07 }    /* 9 */
};

void OLED_ShowDigit(u8 x, u8 y, u8 num, u8 scale)
{
    u8 row, bit;
    if (num > 9) return;
    if (scale == 0) scale = 1;

    for (row = 0; row < 5; row++)
    {
        for (bit = 0; bit < 3; bit++)
        {
            if (digit_3x5[num][row] & (1u << (2 - bit)))
            {
                OLED_FillRect((u8)(x + bit * scale), (u8)(y + row * scale), scale, scale, 1);
            }
        }
    }
}

/****************************************************************************
* 函数名: OLED_ShowTestPattern
* 功能  : 上电自检图案 —— 外框 + 对角线 + 数字 0~9 + 左上角方块
*         用来判断屏幕是否点亮、方向（镜像）是否正常
****************************************************************************/
void OLED_ShowTestPattern(void)
{
    u8 i;

    OLED_Clear();

    for (i = 0; i < 128; i++)                       /* 上下边框 */
    {
        OLED_DrawPixel(i, 0, 1);
        OLED_DrawPixel(i, 63, 1);
    }
    for (i = 0; i < 64; i++)                        /* 左右边框 */
    {
        OLED_DrawPixel(0, i, 1);
        OLED_DrawPixel(127, i, 1);
    }
    for (i = 0; i < 64; i++)                        /* 两条对角线 */
    {
        OLED_DrawPixel((u8)(i * 2), i, 1);
        OLED_DrawPixel((u8)(127 - i * 2), i, 1);
    }
    OLED_FillRect(4, 4, 8, 8, 1);                   /* 左上角实心方块：判断朝向 */

    for (i = 0; i < 10; i++)                        /* 数字 0~9：判断位序与镜像 */
    {
        OLED_ShowDigit((u8)(8 + i * 12), 26, i, 2);
    }

    OLED_Refresh();
}

/****************************************************************************
* 函数名: OLED_ShowCmd
* 功能  : 在右上角显示刚收到的语音指令号（联调用：先看数字对不对，再看动作）
****************************************************************************/
void OLED_ShowCmd(u8 cmd)
{
    OLED_FillRect(96, 4, 28, 12, 0);      /* 先清掉右上角一块 */
    if (cmd > 9) return;
    OLED_ShowDigit(103, 5, cmd, 2);
}
