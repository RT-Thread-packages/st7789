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

#include "st7789.h"

#ifdef PKG_USING_ST7789

#define DBG_SECTION_NAME    "st7789"
#define DBG_LEVEL           DBG_LOG
#include <rtdbg.h>

static struct rt_lcd_device st7789_device;

static rt_err_t lcd_write_cmd(rt_uint8_t cmd)
{
    rt_pin_write((rt_base_t)st7789_device.config.user_data, PIN_LOW);
    return rt_lcd_write_data(st7789_device.mcu, &cmd, 1);
}

static rt_err_t lcd_write_data(rt_uint8_t data)
{
    rt_pin_write((rt_base_t)st7789_device.config.user_data, PIN_HIGH);
    return rt_lcd_write_data(st7789_device.mcu, &data, 1);
}

static rt_err_t lcd_read_data(void *data, rt_size_t len)
{
    rt_pin_write((rt_base_t)st7789_device.config.user_data, PIN_HIGH);
    return rt_lcd_read_data(st7789_device.mcu, data, len);
}

static rt_err_t lcd_write_half_word(const rt_uint16_t half_word)
{
    char data[2] = {0};

    data[0] = half_word >> 8;
    data[1] = half_word & 0xff;

    rt_pin_write((rt_base_t)st7789_device.config.user_data, PIN_HIGH);
    return rt_lcd_write_data(st7789_device.mcu, data, 2);
}

static void _st7789_set_cursor(rt_uint16_t sx, rt_uint16_t sy, rt_uint16_t ex, rt_uint16_t ey)
{
    lcd_write_cmd(CMD_SET_X);
    lcd_write_data(sx >> 8);
    lcd_write_data(sx);
    lcd_write_data(ex >> 8);
    lcd_write_data(ex);

    lcd_write_cmd(CMD_SET_Y);
    lcd_write_data(sy >> 8);
    lcd_write_data(sy);
    lcd_write_data(ey >> 8);
    lcd_write_data(ey);
}

void lcd_clear(rt_uint16_t color)
{
    rt_uint16_t i, j;
    rt_uint8_t data[2] = {0};
    rt_uint8_t *buf = RT_NULL;
    rt_uint16_t wide;
    rt_uint16_t high;
    rt_uint16_t send_len;

    data[0] = color >> 8;
    data[1] = color;
    wide = st7789_device.mcu->mcu_config.info.width;
    high = st7789_device.mcu->mcu_config.info.height;
    send_len = wide * high / 10;

    _st7789_set_cursor(0, 0, wide - 1, high - 1);
    lcd_write_cmd(CMD_WRITE);

    /* 5760 = 240*240/20 */
    buf = (rt_uint8_t *)rt_malloc(send_len);
    if (buf)
    {
        /* 2880 = 5760/2 color is 16 bit */
        for (j = 0; j < send_len / 2; j++)
        {
            buf[j * 2] =  data[0];
            buf[j * 2 + 1] =  data[1];
        }

        rt_pin_write((rt_base_t)st7789_device.config.user_data, PIN_HIGH);
        for (i = 0; i < 20; i++)
        {
            rt_lcd_write_data(st7789_device.mcu, buf, send_len);
        }
        rt_free(buf);
    }
    else
    {
        rt_pin_write((rt_base_t)st7789_device.config.user_data, PIN_HIGH);
        for (i = 0; i < wide; i++)
        {
            for (j = 0; j < wide; j++)
            {
                rt_lcd_write_data(st7789_device.mcu, data, 2);
            }
        }
    }
}

static void _st7789_set_pixel(const char *pixel, int x, int y)
{
    lcd_write_cmd(CMD_SET_X);
    lcd_write_data(x >> 8);
    lcd_write_data(x);

    lcd_write_cmd(CMD_SET_Y);
    lcd_write_data(y >> 8);
    lcd_write_data(y);

    lcd_write_cmd(CMD_WRITE);
    lcd_write_half_word(*(rt_uint16_t *)pixel);
}

static void _st7789_get_pixel(char *pixel, int x, int y)
{
    rt_uint16_t r = 0;
    rt_uint16_t g = 0;
    rt_uint16_t b = 0;

    lcd_write_cmd(CMD_PIXEL_FORMAT);
    lcd_write_data(0x66);

    lcd_write_cmd(CMD_SET_X);
    lcd_write_data(x >> 8);
    lcd_write_data(x);

    lcd_write_cmd(CMD_SET_Y);
    lcd_write_data(y >> 8);
    lcd_write_data(y);

    lcd_write_cmd(CMD_READ);
    lcd_read_data(&r, 1);    /* dummy data */
    lcd_read_data(&r, 1);
    lcd_read_data(&b, 1);
    g = r & 0xFF;
    g <<= 8;
    lcd_write_cmd(CMD_PIXEL_FORMAT);
    lcd_write_data(0x55);

    *(rt_uint32_t *)pixel = (((r >> 11) << 11) | ((g >> 10) << 5) | (b >> 11));/* rgb565 */
}

static void _st7789_draw_hline(const char *pixel, int x1, int x2, int y)
{
    rt_uint16_t i;
    rt_uint8_t line_buf[480] = {0};

    _st7789_set_cursor(x1, y, x2, y);
    lcd_write_cmd(CMD_WRITE);

    for (i = 0; i < x2 - x1; i++)
    {
        line_buf[2 * i] = *(rt_uint16_t *)pixel >> 8;
        line_buf[2 * i + 1] = *(rt_uint16_t *)pixel;
    }

    rt_pin_write((rt_base_t)st7789_device.config.user_data, PIN_HIGH);
    rt_lcd_write_data(st7789_device.mcu, line_buf, (x2 - x1) * 2);
}

static void _st7789_draw_vline(const char *pixel, int x, int y1, int y2)
{
    rt_uint16_t i;
    rt_uint8_t line_buf[480] = {0};

    _st7789_set_cursor(x, y1, x, y2);
    lcd_write_cmd(CMD_WRITE);

    for (i = 0; i < y2 - y1; i++)
    {
        line_buf[2 * i] = *(rt_uint16_t *)pixel >> 8;
        line_buf[2 * i + 1] = *(rt_uint16_t *)pixel;
    }

    rt_pin_write((rt_base_t)st7789_device.config.user_data, PIN_HIGH);
    rt_lcd_write_data(st7789_device.mcu, line_buf, (y2 - y1) * 2);
}

static void _st7789_blit_line(const char *pixel, int x, int y, rt_size_t size)
{
    rt_uint16_t i;

    lcd_write_cmd(CMD_SET_X);
    lcd_write_data(x >> 8);
    lcd_write_data(x);

    lcd_write_cmd(CMD_SET_Y);
    lcd_write_data(y >> 8);
    lcd_write_data(y);

    lcd_write_cmd(CMD_WRITE);
    for (i = 0; i < size; i++)
    {
        _st7789_set_pixel(pixel++, x + i, y);
    }
}

static struct rt_device_graphic_ops _graphic_ops =
{
    _st7789_set_pixel,
    _st7789_get_pixel,
    _st7789_draw_hline,
    _st7789_draw_vline,
    _st7789_blit_line,
};

static rt_err_t _st7789_init(struct rt_lcd_device *device)
{
    /* Memory Data Access Control */
    lcd_write_cmd(0x36);
    lcd_write_data(0x00);
    /* RGB 5-6-5-bit  */
    lcd_write_cmd(0x3A);
    lcd_write_data(0x55);
    /* Porch Setting */
    lcd_write_cmd(0xB2);
    lcd_write_data(0x0C);
    lcd_write_data(0x0C);
    lcd_write_data(0x00);
    lcd_write_data(0x33);
    lcd_write_data(0x33);
    /*  Gate Control */
    lcd_write_cmd(0xB7);
    lcd_write_data(0x35);
    /* VCOM Setting */
    lcd_write_cmd(0xBB);
    lcd_write_data(0x19);
    /* LCM Control */
    lcd_write_cmd(0xC0);
    lcd_write_data(0x2C);
    /* VDV and VRH Command Enable */
    lcd_write_cmd(0xC2);
    lcd_write_data(0x01);
    /* VRH Set */
    lcd_write_cmd(0xC3);
    lcd_write_data(0x12);
    /* VDV Set */
    lcd_write_cmd(0xC4);
    lcd_write_data(0x20);
    /* Frame Rate Control in Normal Mode */
    lcd_write_cmd(0xC6);
    lcd_write_data(0x0F);
    /* Power Control 1 */
    lcd_write_cmd(0xD0);
    lcd_write_data(0xA4);
    lcd_write_data(0xA1);
    /* Positive Voltage Gamma Control */
    lcd_write_cmd(0xE0);
    lcd_write_data(0xD0);
    lcd_write_data(0x04);
    lcd_write_data(0x0D);
    lcd_write_data(0x11);
    lcd_write_data(0x13);
    lcd_write_data(0x2B);
    lcd_write_data(0x3F);
    lcd_write_data(0x54);
    lcd_write_data(0x4C);
    lcd_write_data(0x18);
    lcd_write_data(0x0D);
    lcd_write_data(0x0B);
    lcd_write_data(0x1F);
    lcd_write_data(0x23);
    /* Negative Voltage Gamma Control */
    lcd_write_cmd(0xE1);
    lcd_write_data(0xD0);
    lcd_write_data(0x04);
    lcd_write_data(0x0C);
    lcd_write_data(0x11);
    lcd_write_data(0x13);
    lcd_write_data(0x2C);
    lcd_write_data(0x3F);
    lcd_write_data(0x44);
    lcd_write_data(0x51);
    lcd_write_data(0x2F);
    lcd_write_data(0x1F);
    lcd_write_data(0x1F);
    lcd_write_data(0x20);
    lcd_write_data(0x23);
    /* Display Inversion On */
    lcd_write_cmd(0x21);
    /* Sleep Out */
    lcd_write_cmd(0x11);
    /* wait for power stability */
    rt_thread_mdelay(100);

    /* display on */
    rt_pin_write(st7789_device.config.bl_pin, PIN_HIGH);
    lcd_write_cmd(0x29);
    lcd_clear(WHITE);

    return RT_EOK;
}

static rt_err_t _st7789_control(struct rt_lcd_device *device, int cmd, void *args)
{
    RT_ASSERT(device != RT_NULL);

    switch (cmd)
    {
    case RTGRAPHIC_CTRL_RECT_UPDATE:
        break;

    case RTGRAPHIC_CTRL_POWERON:
        rt_pin_write(device->config.bl_pin, PIN_HIGH);
        break;

    case RTGRAPHIC_CTRL_POWEROFF:
        rt_pin_write(device->config.bl_pin, PIN_LOW);
        break;

    case RTGRAPHIC_CTRL_GET_INFO:
    {
        *(struct rt_device_graphic_info *)args = device->mcu->mcu_config.info;
    }
    break;

    case RTGRAPHIC_CTRL_SET_MODE:
        break;

    case RTGRAPHIC_CTRL_GET_EXT:
        break;
    }

    return RT_EOK;
}

static struct rt_lcd_ops _lcd_ops =
{
    _st7789_init,
    _st7789_control,
};

int rt_hw_st7789_init(struct rt_lcd_mcu *mcu, void *user_data)
{
    rt_err_t result;

    result = RT_EOK;
    st7789_device.mcu = mcu;
    struct rt_lcd_config *lcd_config;
    lcd_config = (struct rt_lcd_config *)user_data;
    st7789_device.config = *lcd_config;

    /* reset st7789 hardware */
    rt_pin_mode((rt_base_t)st7789_device.config.user_data, PIN_MODE_OUTPUT);
    rt_pin_mode(st7789_device.config.rst_pin, PIN_MODE_OUTPUT);
    rt_pin_mode(st7789_device.config.bl_pin, PIN_MODE_OUTPUT);
    rt_pin_write(st7789_device.config.bl_pin, PIN_LOW);
    rt_pin_write(st7789_device.config.rst_pin, PIN_LOW);
    rt_thread_delay(RT_TICK_PER_SECOND / 10);
    rt_pin_write(st7789_device.config.rst_pin, PIN_HIGH);

    result = rt_lcd_device_register(&st7789_device, "st7789", &_lcd_ops, &_graphic_ops);
    if (result != RT_EOK)
    {
        LOG_E("register lcd device failed\n");
        return -RT_ERROR;
    }

    return RT_EOK;
}

#endif /* PKG_USING_ST7789 */

/****************** end of file *******************/
