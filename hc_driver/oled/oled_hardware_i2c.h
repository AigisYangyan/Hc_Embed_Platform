/**
 * @file    oled_hardware_i2c.h
 * @brief   OLED 硬件 I2C 内部私有头
 *
 * 本头文件仅被 hc_driver/oled/hc_driver_oled.c 内部使用，
 * 包含 I2C 总线绑定结构、命令常量和内部辅助函数声明。
 */

#ifndef __OLED_HARDWARE_I2C_H__
#define __OLED_HARDWARE_I2C_H__

#include <stdint.h>
#include "hc_hal_i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OLED_CMD   0u
#define OLED_DATA  1u

typedef struct {
    HC_HAL_I2C_Ch_e i2c_ch;
    HC_U8           dev_addr;
} OLED_Bus_T;

typedef struct {
    OLED_Bus_T bus;
} OLED_T;

HC_Error_e OLED_WR_Byte(uint8_t dat, uint8_t mode);
void        OLED_Set_Pos(uint8_t x, uint8_t y);
uint32_t    oled_pow(uint8_t m, uint8_t n);
void        oled_i2c_sda_unlock(void);

#ifdef __cplusplus
}
#endif

#endif /* __OLED_HARDWARE_I2C_H__ */
