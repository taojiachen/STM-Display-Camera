#ifndef __XPT2046_H
#define __XPT2046_H

#ifdef __cplusplus
extern "C" {
#endif

/* 硬件依赖配置 ------------------------------------------------------------*/
#include "stm32h7xx_hal.h"  // 包含MCU的HAL库头文件
#include <stdbool.h>
#include "gpio.h"
#include "spi.h"

// 定义触摸芯片的SPI句柄（根据实际使用的SPI修改）
#define XPT2046_SPI        hspi5

// 屏幕分辨率（用于坐标转换）
#define XPT2046_SCREEN_WIDTH   240
#define XPT2046_SCREEN_HEIGHT  320

#define XPT2046_CS_PIN GPIO_PIN_6
#define XPT2046_CS_GPIO GPIOF


/* 触摸校准参数 ------------------------------------------------------------*/
typedef struct {
    uint16_t x_min;     // X轴原始最小值
    uint16_t x_max;     // X轴原始最大值
    uint16_t y_min;     // Y轴原始最小值
    uint16_t y_max;     // Y轴原始最大值
    int16_t x_offset;   // X轴坐标偏移
    int16_t y_offset;   // Y轴坐标偏移
} XPT2046_Calibration;

extern XPT2046_Calibration xpt2046_cal;

/* 函数原型 ----------------------------------------------------------------*/
void XPT2046_Init(void);
uint8_t XPT2046_IsTouched(void);
void XPT2046_GetRawXY(uint16_t *x, uint16_t *y);
void XPT2046_GetCalibratedXY(uint16_t *x, uint16_t *y);
void XPT2046_Calibrate(uint16_t x_min, uint16_t x_max, 
                      uint16_t y_min, uint16_t y_max,
                      int16_t x_offset, int16_t y_offset);

#ifdef __cplusplus
}
#endif

#endif /* __XPT2046_H */
