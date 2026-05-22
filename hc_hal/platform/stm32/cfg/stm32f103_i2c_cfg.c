#include "hc_hal_i2c.h"
#include "i2c.h"

const HC_VOID *g_stm32_i2c_handle_map[HC_HAL_I2C_CH_MAX] = {
    [HC_HAL_I2C_CH_0] = (const HC_VOID *)&hi2c1,
};
