/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-07-31     tyustli      the first version
 *
 */

#ifndef __ST7789_H__
#define __ST7789_H__

#include <rtthread.h>
#include <rtdevice.h>

#define WHITE                0xFFFF
#define BLACK                0x0000
#define BLUE                 0x001F
#define BRED                 0XF81F
#define GRED                 0XFFE0
#define GBLUE                0X07FF
#define RED                  0xF800
#define MAGENTA              0xF81F
#define GREEN                0x07E0
#define CYAN                 0x7FFF
#define YELLOW               0xFFE0
#define BROWN                0XBC40
#define BRRED                0XFC07
#define GRAY                 0X8430
#define GRAY175              0XAD75
#define GRAY151              0X94B2
#define GRAY187              0XBDD7
#define GRAY240              0XF79E

#define CMD_SET_X            0x2A
#define CMD_SET_Y            0x2B
#define CMD_WRITE            0x2C
#define CMD_READ             0x2E
#define CMD_PIXEL_FORMAT     0x3A

int rt_hw_st7789_init(struct rt_lcd_mcu *mcu, void *user_data);

#endif /* __ST7789_H__ */

/****************** end of file *******************/
