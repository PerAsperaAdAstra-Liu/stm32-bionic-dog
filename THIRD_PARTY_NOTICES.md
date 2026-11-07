# 第三方组件声明（Third-Party Notices）

本仓库中由本人编写的代码与文档以 MIT 许可证发布，见根目录 `LICENSE`。
以下第三方组件的版权归各自所有者，每个源文件均保留其原始版权与许可声明；
使用这些文件时须同时遵守对应许可证的条款。

## 1. STM32F4 标准外设库及 ST 启动文件

- 涉及目录 / 文件：
  - `STM32F4_FWLIB/`（标准外设库头文件与驱动源码）；
  - `Startup_config/startup_stm32f40_41xxx.s`（启动文件）、`stm32f4xx.h`、
    `system_stm32f4xx.c`、`stm32f4xx_conf.h`（器件寄存器定义与时钟配置）。
- 版权所有：STMicroelectronics（源文件署名 MCD Application Team，版本 V1.4.0，2014）。
- 许可证：**MCD-ST Liberty SW License Agreement V2**。
- 源文件内保留完整 "Licensed under MCD-ST Liberty SW License Agreement V2" 声明。

## 2. ARM CMSIS（Cortex Microcontroller Software Interface Standard）

- 涉及目录：`CMSIS_Core/`（CMSIS 内核头文件）。
- 版权所有：ARM Limited（Copyright (c) 2009-2023 ARM Limited）。
- 许可证：**Apache License 2.0**（源文件 SPDX 标识 `Apache-2.0`）。
- 许可证全文：https://www.apache.org/licenses/LICENSE-2.0

## 说明

- 上述第三方库随仓库一并提供，目的是让克隆者无需另行下载即可直接编译工程；
- 本人对这些文件不主张所有权；如相关权利人认为分发方式不符合其许可条款，
  可通过仓库 issue 联系，将及时处理。
