#ifndef __OLED_PICTURE_H
#define __OLED_PICTURE_H

#include "common.h"

/*=============================================================================
  表情资源
    静态表情 5 张：128x64 满屏
    吼叫动图 6 帧：107x64，显示起点 x=10（居中）
  当前是程序生成的占位表情（由 Tools/gen_face.py 生成）。
  后续用 Image2Lcd 2.9 把自己的图片取模，替换 oled_picture.c 里对应数组即可。
  取模设置：图形模式 / C51 格式 / 行后缀空 / 单色 1bit
=============================================================================*/

#define PIC_SIZE          1024        /* 128x64/8 */
#define ROAR_FRAME_W      107
#define ROAR_FRAME_H      64
#define ROAR_FRAME_X      10          /* (128-107)/2 取整 */
#define ROAR_FRAME_COUNT  6

extern const u8 pic_calm[PIC_SIZE];       /* 平静 */
extern const u8 pic_happy[PIC_SIZE];      /* 快乐 */
extern const u8 pic_sad[PIC_SIZE];        /* 悲伤 */
extern const u8 pic_angry[PIC_SIZE];      /* 生气 */
extern const u8 pic_confused[PIC_SIZE];   /* 疑惑 */

extern const u8 *const pic_roar[ROAR_FRAME_COUNT];   /* 吼叫动图 6 帧 */

#endif
