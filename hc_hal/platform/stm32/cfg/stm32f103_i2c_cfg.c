#include "hc_hal_i2c.h"
#include "i2c.h"

const HC_VOID *g_stm32_i2c_handle_map[I2C_CH_MAX] = {
    [I2C_CH_AT24C02] = (const HC_VOID *)&hi2c1,
    [I2C_CH_OLED] = (const HC_VOID *)&hi2c1,
    [I2C_CH_MPU6050] = (const HC_VOID *)&hi2c1,
};
