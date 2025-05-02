#include "xpt2046.h"

/* 私有变量 ----------------------------------------------------------------*/
static SPI_HandleTypeDef *xpt_spi = &XPT2046_SPI;
XPT2046_Calibration xpt2046_cal = {
    .x_min = 300,    .x_max = 3800,
    .y_min = 400,    .y_max = 3700,
    .x_offset = -10, .y_offset = 20
};

/* 私有函数 ----------------------------------------------------------------*/
static uint16_t XPT2046_ReadADC(uint8_t channel);

/**
  * @brief  初始化触摸芯片硬件
  * @retval 无
  */
void XPT2046_Init(void) {
    // 初始化CS引脚
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = XPT2046_CS_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(XPT2046_CS_GPIO, &GPIO_InitStruct);
    HAL_GPIO_WritePin(XPT2046_CS_GPIO, XPT2046_CS_PIN, GPIO_PIN_SET);

    // 初始化IRQ引脚（可选）
    // GPIO_InitStruct.Pin = XPT2046_IRQ_PIN;
    // GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    // GPIO_InitStruct.Pull = GPIO_PULLUP;
    // HAL_GPIO_Init(XPT2046_IRQ_GPIO, &GPIO_InitStruct);
}

/**
  * @brief  读取ADC原始值
  * @param  channel: 通道选择 (0xD0=Y, 0x90=X)
  * @retval 12位ADC值
  */
static uint16_t XPT2046_ReadADC(uint8_t channel) {
    uint8_t tx_buf[3] = {channel, 0x00, 0x00};
    uint8_t rx_buf[3] = {0};

    HAL_GPIO_WritePin(XPT2046_CS_GPIO, XPT2046_CS_PIN, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(xpt_spi, tx_buf, rx_buf, 3, 10);
    HAL_GPIO_WritePin(XPT2046_CS_GPIO, XPT2046_CS_PIN, GPIO_PIN_SET);

    return ((rx_buf[1] << 8) | rx_buf[2]) >> 3;  // 取高12位
}

/**
  * @brief  检测是否有触摸
  * @retval 1=有触摸, 0=无触摸
  */
uint8_t XPT2046_IsTouched(void) {
    // 如果使用IRQ引脚，可以快速检测
    // return (HAL_GPIO_ReadPin(XPT2046_IRQ_GPIO, XPT2046_IRQ_PIN) == GPIO_PIN_RESET);
    
    // 轮询方式：读取Z坐标压力值
    uint16_t z = XPT2046_ReadADC(0xB0);
    return (z > 100) ? 1 : 0;  // 阈值根据实际调整
}

/**
  * @brief  获取原始坐标
  * @param  x: 输出X原始值
  * @param  y: 输出Y原始值
  */
void XPT2046_GetRawXY(uint16_t *x, uint16_t *y) {
    *x = XPT2046_ReadADC(0xD0);  // 读取Y坐标（物理X轴）
    *y = XPT2046_ReadADC(0x90);  // 读取X坐标（物理Y轴）
}

/**
  * @brief  获取校准后的屏幕坐标
  * @param  x: 输出X坐标
  * @param  y: 输出Y坐标
  */
void XPT2046_GetCalibratedXY(uint16_t *x, uint16_t *y) {
    uint16_t raw_x, raw_y;
    XPT2046_GetRawXY(&raw_x, &raw_y);
    
    *x = (uint16_t)((raw_x - xpt2046_cal.x_min) * XPT2046_SCREEN_WIDTH / 
                   (xpt2046_cal.x_max - xpt2046_cal.x_min) + xpt2046_cal.x_offset);
    
    *y = (uint16_t)((raw_y - xpt2046_cal.y_min) * XPT2046_SCREEN_HEIGHT / 
                   (xpt2046_cal.y_max - xpt2046_cal.y_min) + xpt2046_cal.y_offset);
    
    // 边界保护
    *x = (*x > XPT2046_SCREEN_WIDTH) ? XPT2046_SCREEN_WIDTH : *x;
    *y = (*y > XPT2046_SCREEN_HEIGHT) ? XPT2046_SCREEN_HEIGHT : *y;
}

/**
  * @brief  校准触摸参数
  * @param  x_min, x_max: X轴原始范围
  * @param  y_min, y_max: Y轴原始范围
  * @param  x_offset, y_offset: 坐标偏移
  */
void XPT2046_Calibrate(uint16_t x_min, uint16_t x_max, 
                      uint16_t y_min, uint16_t y_max,
                      int16_t x_offset, int16_t y_offset) {
    xpt2046_cal.x_min = x_min;
    xpt2046_cal.x_max = x_max;
    xpt2046_cal.y_min = y_min;
    xpt2046_cal.y_max = y_max;
    xpt2046_cal.x_offset = x_offset;
    xpt2046_cal.y_offset = y_offset;
}
