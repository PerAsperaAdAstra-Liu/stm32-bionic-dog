#ifndef __SERVO_H
#define __SERVO_H

#include "common.h"

/*=============================================================================
  舵机驱动（四足机器狗）—— 一路定时器同时输出 4 路 PWM

  通道映射（与开题报告一致）
    左后 LB -> PD12 -> TIM4_CH1
    右后 RB -> PD13 -> TIM4_CH2
    左前 LF -> PD14 -> TIM4_CH3
    右前 RF -> PD15 -> TIM4_CH4

  定时器参数（TIM4 挂在 APB1，时钟 84MHz）
    分频 840  -> 计数频率 100kHz（1 个计数 = 10us）
    重载 2000 -> 周期 20ms
    比较值 50~250 -> 高电平 0.5~2.5ms -> 舵机 0~180 度

  角度公式（左右镜像，先乘后除避免整数除法丢精度）
    左腿：CCR = angle * 50 / 45 + 50
    右腿：CCR = 250 - angle * 50 / 45
  同一个"逻辑角度"在左右腿上对应的物理方向是镜像的，
  所以四腿都给 0 度时，两条腿的脉冲是 0.5ms 与 2.5ms，姿态左右对称。
=============================================================================*/

/* SG90 是塑料齿，堵转容易扫齿，所以默认把两端各收一点：
   60~240 对应 0.6~2.4ms，不损失任何动作需要的角度（90°=150 在中间） */
#define SERVO_CCR_MIN   60u    /* 0.6ms（安全下限） */
#define SERVO_CCR_MAX   240u   /* 2.4ms（安全上限） */
#define SERVO_CCR_MID   150u   /* 1.5ms -> 90 度 */

/* 步态参数：实物调试就改这两个宏
   老师实测方向：90 度 -> 45 度 = 向前摆，90 度 -> 135 度 = 向后摆 */
#define GAIT_BASE       90     /* 站立姿态角度 */
#define GAIT_SWING      45     /* 摆动幅度（前进 90-45=45°，后退 90+45=135°） */
#define SPEED_FAST      0      /* 姿态类动作：一步到位 */
#define SPEED_GAIT      3      /* 步态类动作：慢摆（越大越慢，单位 ms/步） */

/* 全局摆动速度：speed=0 直接到位；speed>0 时每步延时 speed 毫秒
   （只影响你自己直接调用 Angle_xx() 时的效果，动作函数内部会自己设定） */
extern u8 speed;

void Servo_Init(void);

/* 直接设置比较值（内部按 SERVO_CCR_MIN/MAX 限幅 60~240） */
void SetPWM_LB(u16 ccr);
void SetPWM_RB(u16 ccr);
void SetPWM_LF(u16 ccr);
void SetPWM_RF(u16 ccr);

/* 按逻辑角度 0~180 控制单条腿 */
void Angle_LB(int angle);
void Angle_RB(int angle);
void Angle_LF(int angle);
void Angle_RF(int angle);

/* 动作库 */
void Action_Up(void);       /* 起立：四腿 90 度 */
void Action_Down(void);     /* 趴下：四腿 0 度 */
void Action_Sit(void);      /* 坐下：前腿 90 度，后腿 0 度 */
void Action_Forward(void);  /* 前进：对角步态 */
void Action_Back(void);     /* 后退：对角步态反向 */

#endif
