#ifndef HC_HAL_I2C_H
#define HC_HAL_I2C_H

/* Legacy compatibility header — forward to canonical hc_hal. */
#include "hc_hal/hc_hal_i2c.h"

/* Project-semantic aliases (deprecated). New code should use HC_HAL_I2C_Ch_e. */
typedef enum {
    I2C_CH_AT24C02 = HC_HAL_I2C_CH_0,
    I2C_CH_OLED    = HC_HAL_I2C_CH_1,
    I2C_CH_MPU6050 = HC_HAL_I2C_CH_2,
    I2C_CH_MAX     = HC_HAL_I2C_CH_MAX
} HC_HAL_I2C_Ch_e;

typedef HC_HAL_I2C_Ch_e HC_HAL_I2C_Id_e;

#define HC_HAL_I2C_ID_AT24C02 I2C_CH_AT24C02
#define HC_HAL_I2C_ID_OLED    I2C_CH_OLED

HC_LOCAL inline HC_Error_e HC_HAL_I2C_MasterWrite(HC_HAL_I2C_Ch_e ch,
                                                  HC_U8 addr,
                                                  const HC_U8 *p_data,
                                                  HC_U16 len)
{
    return HC_HAL_I2C_Write(ch, addr, p_data, len);
}

#endif /* HC_HAL_I2C_H */
