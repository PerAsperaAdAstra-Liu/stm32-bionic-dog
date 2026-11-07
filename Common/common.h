#ifndef __COMMON_H
#define __COMMON_H

#include "stm32f4xx.h"

/*=============================================================================
  仿生机器狗「旺财一号」 —— 公共头文件
  沿用启明欣欣标准库例程的写法：u8/u16/u32 简写 + 位带操作宏 + SysTick 延时
  说明：本工程同时兼容 MDK 的 Arm Compiler 5 与 Arm Compiler 6
=============================================================================*/

typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;

/*---------------- 位带操作：像 51 一样直接操作 IO 口 ----------------*/
#define BITBAND(addr, bitnum)  ((addr & 0xF0000000) + 0x2000000 + ((addr & 0xFFFFF) << 5) + (bitnum << 2))
#define MEM_ADDR(addr)         *((volatile unsigned long *)(addr))
#define BIT_ADDR(addr, bitnum) MEM_ADDR(BITBAND(addr, bitnum))

/* IO 口地址映射 */
#define GPIOA_ODR_Addr  (GPIOA_BASE + 20)
#define GPIOB_ODR_Addr  (GPIOB_BASE + 20)
#define GPIOC_ODR_Addr  (GPIOC_BASE + 20)
#define GPIOD_ODR_Addr  (GPIOD_BASE + 20)
#define GPIOE_ODR_Addr  (GPIOE_BASE + 20)
#define GPIOF_ODR_Addr  (GPIOF_BASE + 20)
#define GPIOG_ODR_Addr  (GPIOG_BASE + 20)
#define GPIOH_ODR_Addr  (GPIOH_BASE + 20)
#define GPIOI_ODR_Addr  (GPIOI_BASE + 20)

#define GPIOA_IDR_Addr  (GPIOA_BASE + 16)
#define GPIOB_IDR_Addr  (GPIOB_BASE + 16)
#define GPIOC_IDR_Addr  (GPIOC_BASE + 16)
#define GPIOD_IDR_Addr  (GPIOD_BASE + 16)
#define GPIOE_IDR_Addr  (GPIOE_BASE + 16)
#define GPIOF_IDR_Addr  (GPIOF_BASE + 16)
#define GPIOG_IDR_Addr  (GPIOG_BASE + 16)
#define GPIOH_IDR_Addr  (GPIOH_BASE + 16)
#define GPIOI_IDR_Addr  (GPIOI_BASE + 16)

/* 位带输出/输入，n 为 0~15 的位号 */
#define PAout(n)  BIT_ADDR(GPIOA_ODR_Addr, n)
#define PAin(n)   BIT_ADDR(GPIOA_IDR_Addr, n)
#define PBout(n)  BIT_ADDR(GPIOB_ODR_Addr, n)
#define PBin(n)   BIT_ADDR(GPIOB_IDR_Addr, n)
#define PCout(n)  BIT_ADDR(GPIOC_ODR_Addr, n)
#define PCin(n)   BIT_ADDR(GPIOC_IDR_Addr, n)
#define PDout(n)  BIT_ADDR(GPIOD_ODR_Addr, n)
#define PDin(n)   BIT_ADDR(GPIOD_IDR_Addr, n)
#define PEout(n)  BIT_ADDR(GPIOE_ODR_Addr, n)
#define PEin(n)   BIT_ADDR(GPIOE_IDR_Addr, n)
#define PFout(n)  BIT_ADDR(GPIOF_ODR_Addr, n)
#define PFin(n)   BIT_ADDR(GPIOF_IDR_Addr, n)
#define PGout(n)  BIT_ADDR(GPIOG_ODR_Addr, n)
#define PGin(n)   BIT_ADDR(GPIOG_IDR_Addr, n)
#define PHout(n)  BIT_ADDR(GPIOH_ODR_Addr, n)
#define PHin(n)   BIT_ADDR(GPIOH_IDR_Addr, n)
#define PIout(n)  BIT_ADDR(GPIOI_ODR_Addr, n)
#define PIin(n)   BIT_ADDR(GPIOI_IDR_Addr, n)

/* 系统主频（MHz） */
#define SYSCLK  168

/*---------------- 延时：基于 SysTick，初始化时必须先调用 delay_init() ----------------*/
void delay_init(void);
void delay_us(u32 nus);
void delay_ms(u16 nms);

/*---------------- 内核辅助函数 ----------------
  原来的内核汇编函数（WFI_SET 等）在 Arm Compiler 5 下用 __asm 关键字实现，
  在 Arm Compiler 6 下会报错，这里统一改用 CMSIS 标准内建函数，两个编译器都能编。*/
void WFI_SET(void);       /* 进入低功耗等待 */
void INTX_DISABLE(void);  /* 关闭总中断 */
void INTX_ENABLE(void);   /* 打开总中断 */
void MSR_MSP(u32 addr);   /* 设置栈顶地址 */

#endif
