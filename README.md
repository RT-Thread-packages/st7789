# ST7789

## 简介

ST7789 软件包提供了使用液晶显示的基本功能，并且本软件包已经对接到了 LCD 接口框架，通过 LCD 接口框架，开发者可以快速的将此液晶芯片驱动起来。
## 支持情况

| 包含设备           | 液晶芯片 |  
| ----------------     | -------- | 
| **通讯接口**      |          |      
| SPI 接口             | √        | 
      

## 使用说明

### 依赖

- RT-Thread 4.0.0+
- SPI 总线驱动
- LCD 接口组件
- LCD 接口驱动：st7789 设备使用 LCD 接口设备进行数据通讯，需要系统 LCD 接口驱动支持；

### 获取软件包

使用 st7789 软件包需要在 RT-Thread 的包管理中选中它，具体路径如下：

```
RT-Thread online packages  --->
  peripheral libraries and drivers  --->
    lcd drivers  --->
      st7789: lcd ic st7789 for rt-thread
              Version (latest)  --->
```
**Version**：软件包版本选择

### 使用软件包

st7789 软件包初始化函数如下所示：

```c
int rt_hw_st7789_init(struct rt_lcd_mcu *mcu, void *user_data);
```

该函数需要由用户调用，函数主要完成的功能有，

- 根据已经配置好的 LCD 接口设备来初始化 LCD 显示，并注册相应的 LCD 设备，完成 st7789 设备的注册；

#### 初始化示例

```.c 

int rt_hw_st7789_init(void)
{ 
    struct rt_lcd_mcu *mcu;
    struct rt_spi_configuration spi_cfg;

    /* config lcd interface device */
    spi_cfg.data_width = 8;
    spi_cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB;
    spi_cfg.max_hz = 42 * 1000 * 1000;
    rt_lcd_mcu_config(mcu, &spi_cfg);

    /* config and register lcd device */
    lcd_config.bl_pin = GET_PIN(B, 7);
    lcd_config.rst_pin = GET_PIN(B, 6);
    lcd_config.user_data = (void *)GET_PIN(B, 4);
    mcu->mcu_config.info.width = 240;
    mcu->mcu_config.info.height = 240;

    rt_hw_st7789_init(mcu, &lcd_config);

    return 0;
}
INIT_ENV_EXPORT(rt_hw_st7789_init);
```

## 注意事项

暂无

## 联系人信息

维护人:

- [tyustli](https://github.com/tyustli) 

- 主页：<https://github.com/RT-Thread-packages/st7789>