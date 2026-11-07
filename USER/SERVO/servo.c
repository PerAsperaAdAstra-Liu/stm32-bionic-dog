/*=============================================================================
  servo.c  舵机控制与动作库

  功能：TIM4 四通道 PWM 输出、左右镜像角度换算、速度平滑、五种动作
  硬件：PD12~PD15  ->  TIM4_CH1~CH4  ->  SG90 x4
  参数：84MHz/840 = 100kHz，ARR=2000 -> 20ms；CCR 60~240 -> 0.6~2.4ms
  说明：每条腿单独保存当前 CCR，速度用全局 speed（0 = 一步到位）
=============================================================================*/
#include "servo.h"

u8 speed = SPEED_GAIT;

/* 四条腿在 TIM4 中的通道下标：0=CH1(LB) 1=CH2(RB) 2=CH3(LF) 3=CH4(RF) */
#define LEG_LB  0
#define LEG_RB  1
#define LEG_LF  2
#define LEG_RF  3

static u16 ccr_cur[4] = { SERVO_CCR_MIN, SERVO_CCR_MAX, SERVO_CCR_MIN, SERVO_CCR_MAX };

/****************************************************************************
* 函数名: servo_write
* 功能  : 把比较值写进对应通道（带限幅）
****************************************************************************/
static void servo_write(u8 leg, u16 ccr)
{
    if (ccr < SERVO_CCR_MIN) ccr = SERVO_CCR_MIN;
    if (ccr > SERVO_CCR_MAX) ccr = SERVO_CCR_MAX;
    ccr_cur[leg] = ccr;

    switch (leg)
    {
        case LEG_LB: TIM_SetCompare1(TIM4, ccr); break;
        case LEG_RB: TIM_SetCompare2(TIM4, ccr); break;
        case LEG_LF: TIM_SetCompare3(TIM4, ccr); break;
        case LEG_RF: TIM_SetCompare4(TIM4, ccr); break;
        default: break;
    }
}

/****************************************************************************
* 函数名: servo_move
* 功能  : 平滑移动到目标比较值
* 说明  : speed = 0 时一步到位（起立/坐下/趴下用）
*         speed > 0 时每个计数单位延时 speed 毫秒（前进/后退慢摆用）
****************************************************************************/
static void servo_move(u8 leg, u16 target)
{
    u16 cur = ccr_cur[leg];

    if (target < SERVO_CCR_MIN) target = SERVO_CCR_MIN;
    if (target > SERVO_CCR_MAX) target = SERVO_CCR_MAX;

    if (speed == 0)
    {
        servo_write(leg, target);
        return;
    }

    while (cur != target)
    {
        if (cur < target) cur++;
        else              cur--;
        servo_write(leg, cur);
        delay_ms(speed);
    }
}

/* 左腿：0 度 -> 0.5ms，180 度 -> 2.5ms */
static u16 angle_to_ccr_left(int angle)
{
    if (angle < 0)   angle = 0;
    if (angle > 180) angle = 180;
    return (u16)(angle * 50 / 45 + 50);      /* 先乘后除 */
}

/* 右腿：镜像，0 度 -> 2.5ms，180 度 -> 0.5ms */
static u16 angle_to_ccr_right(int angle)
{
    if (angle < 0)   angle = 0;
    if (angle > 180) angle = 180;
    return (u16)(250 - angle * 50 / 45);
}

/****************************************************************************
* 函数名: Servo_Init
* 功能  : 配置 PD12~PD15 为 TIM4 四通道 PWM 输出，并给上电初始姿态
****************************************************************************/
void Servo_Init(void)
{
    GPIO_InitTypeDef        GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef       TIM_OCInitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

    /* 引脚复用为 TIM4 */
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource12, GPIO_AF_TIM4);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource13, GPIO_AF_TIM4);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource14, GPIO_AF_TIM4);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF_TIM4);

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    /* 定时器基础配置：84MHz / 840 = 100kHz，2000 个计数 = 20ms */
    TIM_TimeBaseStructure.TIM_Prescaler         = 840 - 1;
    TIM_TimeBaseStructure.TIM_Period            = 2000 - 1;
    TIM_TimeBaseStructure.TIM_CounterMode       = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_ClockDivision     = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

    /* 四个通道都配成 PWM 模式 1 */
    TIM_OCInitStructure.TIM_OCMode      = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OCPolarity  = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_Pulse       = SERVO_CCR_MIN;
    TIM_OC1Init(TIM4, &TIM_OCInitStructure);
    TIM_OC2Init(TIM4, &TIM_OCInitStructure);
    TIM_OC3Init(TIM4, &TIM_OCInitStructure);
    TIM_OC4Init(TIM4, &TIM_OCInitStructure);

    TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);
    TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);
    TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Enable);
    TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM4, ENABLE);
    TIM_Cmd(TIM4, ENABLE);

    /* 上电初始姿态：四腿逻辑角度 0 度（趴下、腿尖朝前）
       左腿 0 度 -> CCR 50，右腿 0 度 -> CCR 250，左右镜像对称 */
    servo_write(LEG_LB, angle_to_ccr_left(0));
    servo_write(LEG_RB, angle_to_ccr_right(0));
    servo_write(LEG_LF, angle_to_ccr_left(0));
    servo_write(LEG_RF, angle_to_ccr_right(0));
}

/*------------------------- 直接设置比较值 -------------------------*/
void SetPWM_LB(u16 ccr) { servo_write(LEG_LB, ccr); }
void SetPWM_RB(u16 ccr) { servo_write(LEG_RB, ccr); }
void SetPWM_LF(u16 ccr) { servo_write(LEG_LF, ccr); }
void SetPWM_RF(u16 ccr) { servo_write(LEG_RF, ccr); }

/*------------------------- 按角度平滑控制 -------------------------*/
void Angle_LB(int angle) { servo_move(LEG_LB, angle_to_ccr_left(angle)); }
void Angle_RB(int angle) { servo_move(LEG_RB, angle_to_ccr_right(angle)); }
void Angle_LF(int angle) { servo_move(LEG_LF, angle_to_ccr_left(angle)); }
void Angle_RF(int angle) { servo_move(LEG_RF, angle_to_ccr_right(angle)); }

/*--------------------------- 动作库 -------------------------------*/

/* 起立：四腿 90 度，快速到位（保存/恢复原摆动速度，与老师代码一致） */
void Action_Up(void)
{
    u8 saved = speed;
    speed = SPEED_FAST;
    Angle_LF(GAIT_BASE);
    Angle_RF(GAIT_BASE);
    Angle_LB(GAIT_BASE);
    Angle_RB(GAIT_BASE);
    speed = saved;
}

/* 趴下：四腿 0 度，快速到位 */
void Action_Down(void)
{
    u8 saved = speed;
    speed = SPEED_FAST;
    Angle_LF(0);
    Angle_RF(0);
    Angle_LB(0);
    Angle_RB(0);
    speed = saved;
}

/* 坐下：前腿 90 度，后腿 0 度 */
void Action_Sit(void)
{
    u8 saved = speed;
    speed = SPEED_FAST;
    Angle_LF(GAIT_BASE);
    Angle_RF(GAIT_BASE);
    Angle_LB(0);
    Angle_RB(0);
    speed = saved;
}

/* 前进：先起立，再走对角步态
   老师的实测方向：90 度 -> 45 度 是"向前摆"，所以前进摆到 90-45=45 度
   第 1 步：左前 + 右后；第 2 步：右前 + 左后；第 3、4 步依次回 90 度 */
void Action_Forward(void)
{
    u8 saved = speed;
    speed = SPEED_GAIT;

    Action_Up();                            /* 先站起来再走 */

    Angle_LF(GAIT_BASE - GAIT_SWING);       /* 45 度 */
    Angle_RB(GAIT_BASE - GAIT_SWING);

    Angle_RF(GAIT_BASE - GAIT_SWING);
    Angle_LB(GAIT_BASE - GAIT_SWING);

    Angle_LF(GAIT_BASE);
    Angle_RB(GAIT_BASE);

    Angle_RF(GAIT_BASE);
    Angle_LB(GAIT_BASE);

    speed = saved;
}


/* 后退：同一步态，摆动方向取反（90+45=135 度） */
void Action_Back(void)
{
    u8 saved = speed;
    speed = SPEED_GAIT;

    Action_Up();

    Angle_LF(GAIT_BASE + GAIT_SWING);       /* 135 度 */
    Angle_RB(GAIT_BASE + GAIT_SWING);

    Angle_RF(GAIT_BASE + GAIT_SWING);
    Angle_LB(GAIT_BASE + GAIT_SWING);

    Angle_LF(GAIT_BASE);
    Angle_RB(GAIT_BASE);

    Angle_RF(GAIT_BASE);
    Angle_LB(GAIT_BASE);

    speed = saved;
}
