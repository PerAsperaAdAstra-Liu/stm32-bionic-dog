#include "main.h"

/*=============================================================================
  仿生机器狗「旺财一号」主程序

  数据流向：
    SU03T 语音识别 -> 串口6(9600) -> 接收中断写 g_cmd
    -> 主循环 switch 分发 -> 舵机动作 / OLED 表情 / LED

  关键设计：
    1) 非循环类指令执行完把 g_cmd 清零，否则会反复执行同一条指令；
    2) 吼叫动图在 while 里轮播 6 帧，退出条件是收到新指令；
    3) 主循环末尾留 10ms 延时，避免 CPU 空转。
=============================================================================*/

/****************************************************************************
* 函数名: Debug_SelfTest
* 功能  : 分模块自检（DEBUG_MODE 1 和 2 的入口）
****************************************************************************/
void Debug_SelfTest(void)
{
#if DEBUG_MODE == 3
    /* --- 姿态锁定：四腿停在复位姿态（水平朝前），方便装舵盘 --- */
    Action_Down();
    OLED_Clear();
    OLED_ShowDigit(60, 20, 0, 6);      /* 屏幕显示 0 = 复位姿态 */
    OLED_Refresh();
    while (1)
    {
        delay_ms(100);
    }
#elif DEBUG_MODE == 2
    /* --- 单舵机测试：四腿轮流缓慢扫角，观察方向与行程 --- */
    u8 leg;
    while (1)
    {
        for (leg = 0; leg < 4; leg++)
        {
            /* 屏幕上显示当前测试的是第几条腿（1=左后 2=右后 3=左前 4=右前） */
            OLED_Clear();
            OLED_ShowDigit(60, 20, (u8)(leg + 1), 6);
            OLED_Refresh();

            speed = 5;                  /* 慢速，每步 5ms */
            switch (leg)
            {
                case 0: Angle_LB(0);   Angle_LB(90);   Angle_LB(180);  Angle_LB(0);   break;
                case 1: Angle_RB(0);   Angle_RB(90);   Angle_RB(180);  Angle_RB(0);   break;
                case 2: Angle_LF(0);   Angle_LF(90);   Angle_LF(180);  Angle_LF(0);   break;
                default: Angle_RF(0);  Angle_RF(90);   Angle_RF(180);  Angle_RF(0);   break;
            }
            delay_ms(500);
        }
    }
#else
    /* --- 分模块自检：先点灯，再显示自检图案，然后一直回显串口数据 --- */
    OLED_Clear();
    OLED_Refresh();

    USART6_Init(USART6_BAUD_DEFAULT);
    LED_Run_Test();
    OLED_ShowTestPattern();

    while (1)
    {
        /* 显示解析出来的指令号（1~9）：
           帧头 AA/55 这类非指令字节会被驱动自动忽略，所以能显示出来
           就说明"语音模块 -> 串口 6 -> 解析"这条链是通的 */
        if (g_cmd != CMD_NONE)
        {
            OLED_ShowCmd(g_cmd);
            OLED_Refresh();
            g_cmd = CMD_NONE;
        }
        delay_ms(50);
    }
#endif
}

/****************************************************************************
* 函数名: ShowFace
* 功能  : 切换一张满屏表情
****************************************************************************/
void ShowFace(const u8 *pic)
{
    OLED_ShowPicture(0, 0, 128, 64, pic);
    OLED_Refresh();
}

/****************************************************************************
* 函数名: Roar_Play
* 功能  : 播放吼叫动图（6 帧循环）
* 说明  : 语音模块播音频时收不到新指令，所以退出条件设为"收到新指令"
****************************************************************************/
void Roar_Play(void)
{
    u8 i;

    while (g_cmd == CMD_ROAR)
    {
        for (i = 0; i < ROAR_FRAME_COUNT; i++)
        {
            OLED_Clear();       /* 先清显存，避免 107 宽的画面两侧残留旧表情 */
            OLED_ShowPicture(ROAR_FRAME_X, 0, ROAR_FRAME_W, ROAR_FRAME_H, pic_roar[i]);
            OLED_Refresh();
            delay_ms(ROAR_FRAME_DELAY_MS);

            if (g_cmd != CMD_ROAR) break;    /* 来了新指令，立即退出动图 */
        }
    }
}

/****************************************************************************
* 函数名: main
* 功能  : 初始化各模块，主循环按语音指令分发动作
****************************************************************************/
int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    delay_init();

    LED_Init();
    LED_All_Off();

#if DEBUG_MODE == 0
    Servo_Init();
    Action_Down();

    OLED_Init();
    ShowFace(pic_calm);

    USART6_Init(USART6_BAUD_DEFAULT);
#else
    OLED_Init();
#if (DEBUG_MODE == 2) || (DEBUG_MODE == 3)
    Servo_Init();         /* 单舵机测试 / 姿态锁定都需要 PWM */
#endif
    Debug_SelfTest();     /* 不会返回 */
#endif

    while (1)
    {
        if (g_cmd != CMD_NONE)
        {
            OLED_ShowCmd(g_cmd);
            OLED_Refresh();
        }

        switch (g_cmd)
        {
            case CMD_NONE:
                break;

            case CMD_STAND_UP:
                ShowFace(pic_happy);
                Action_Up();
                g_cmd = CMD_NONE;
                break;

            case CMD_LIE_DOWN:
                ShowFace(pic_calm);
                Action_Down();
                g_cmd = CMD_NONE;
                break;

            case CMD_SIT:
                ShowFace(pic_happy);
                Action_Sit();
                g_cmd = CMD_NONE;
                break;

            case CMD_FORWARD:
                ShowFace(pic_calm);
                Action_Forward();
                g_cmd = CMD_NONE;
                break;

            case CMD_BACK:
                ShowFace(pic_calm);
                Action_Back();
                g_cmd = CMD_NONE;
                break;

            case CMD_LED_ON:
                LED_All_On();
                /* 复用“开灯”作为动态表情演示入口：
                   当前 SU-03T 固件稳定发送 1~7，不必重新生成语音固件。
                   动图会一直循环，收到下一条语音指令后立即退出。 */
                g_cmd = CMD_ROAR;
                Roar_Play();
                break;

            case CMD_LED_OFF:
                LED_All_Off();
                ShowFace(pic_calm);
                g_cmd = CMD_NONE;
                break;

            case CMD_ROAR:
                Roar_Play();
                /* 若是被新指令打断的，就不要再插一帧平静表情，直接去执行新指令 */
                if (g_cmd == CMD_NONE) ShowFace(pic_calm);
                break;

            case CMD_ROLL:
                ShowFace(pic_sad);
                g_cmd = CMD_NONE;
                break;

            default:
                g_cmd = CMD_NONE;
                break;
        }

        delay_ms(10);
    }
}
