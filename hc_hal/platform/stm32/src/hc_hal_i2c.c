#include "hc_hal_i2c.h"

#include "i2c.h"

#ifndef HC_HAL_ERR_I2C
#define HC_HAL_ERR_I2C HC_ERR_UNKNOWN
#endif

extern const HC_VOID *g_stm32_i2c_handle_map[I2C_CH_MAX];

static I2C_HandleTypeDef *hc_hal_i2c_get_handle(HC_HAL_I2C_Ch_e ch)
{
    if ((HC_U32)ch >= (HC_U32)I2C_CH_MAX) {
        return (I2C_HandleTypeDef *)0;
    }

    return (I2C_HandleTypeDef *)g_stm32_i2c_handle_map[ch];
}

static HC_Error_e hc_hal_i2c_map_status(HAL_StatusTypeDef status)
{
    switch (status) {
    case HAL_OK:
        return HC_HAL_OK;
    case HAL_TIMEOUT:
        return HC_HAL_ERR_TIMEOUT;
    case HAL_BUSY:
        return HC_ERR_BUSY;
    default:
        return HC_HAL_ERR_I2C;
    }
}

HC_Error_e HC_HAL_I2C_Init(HC_HAL_I2C_Ch_e ch)
{
    return (hc_hal_i2c_get_handle(ch) != (I2C_HandleTypeDef *)0) ? HC_HAL_OK : HC_HAL_ERR_INVALID;
}

HC_Error_e HC_HAL_I2C_Write(HC_HAL_I2C_Ch_e ch, HC_U8 addr, const HC_U8 *p_data, HC_U16 len)
{
    I2C_HandleTypeDef *handle = hc_hal_i2c_get_handle(ch);

    HC_HAL_ASSERT_PARAM(handle != (I2C_HandleTypeDef *)0, HC_HAL_ERR_INVALID);
    HC_HAL_ASSERT_PARAM((p_data != HC_NULL_PTR) || (len == 0u), HC_HAL_ERR_NULL_PTR);

    return hc_hal_i2c_map_status(HAL_I2C_Master_Transmit(handle, (HC_U16)(addr << 1), (HC_U8 *)p_data, len, 100u));
}

HC_Error_e HC_HAL_I2C_MasterWrite(HC_HAL_I2C_Ch_e ch, HC_U8 addr,
                                   const HC_U8 *p_data, HC_U16 len)
{
    return HC_HAL_I2C_Write(ch, addr, p_data, len);
}

HC_Error_e HC_HAL_I2C_BusRecover(HC_HAL_I2C_Ch_e ch)
{
    I2C_HandleTypeDef *handle = hc_hal_i2c_get_handle(ch);

    HC_HAL_ASSERT_PARAM(handle != (I2C_HandleTypeDef *)0, HC_HAL_ERR_INVALID);

    if (HAL_I2C_DeInit(handle) != HAL_OK) {
        return HC_HAL_ERR_I2C;
    }

    if (HAL_I2C_Init(handle) != HAL_OK) {
        return HC_HAL_ERR_I2C;
    }

    return HC_HAL_OK;
}

HC_Error_e HC_HAL_I2C_MemWrite(HC_HAL_I2C_Ch_e ch, HC_U8 dev_addr, HC_U8 mem_addr, const HC_U8 *p_data, HC_U16 len)
{
    I2C_HandleTypeDef *handle = hc_hal_i2c_get_handle(ch);

    HC_HAL_ASSERT_PARAM(handle != (I2C_HandleTypeDef *)0, HC_HAL_ERR_INVALID);
    HC_HAL_ASSERT_PARAM((p_data != HC_NULL_PTR) || (len == 0u), HC_HAL_ERR_NULL_PTR);

    return hc_hal_i2c_map_status(HAL_I2C_Mem_Write(handle,
                                                    (HC_U16)(dev_addr << 1),
                                                    mem_addr,
                                                    I2C_MEMADD_SIZE_8BIT,
                                                    (HC_U8 *)p_data,
                                                    len,
                                                    100u));
}

HC_Error_e HC_HAL_I2C_MemRead(HC_HAL_I2C_Ch_e ch, HC_U8 dev_addr, HC_U8 mem_addr, HC_U8 *p_data, HC_U16 len)
{
    I2C_HandleTypeDef *handle = hc_hal_i2c_get_handle(ch);

    HC_HAL_ASSERT_PARAM(handle != (I2C_HandleTypeDef *)0, HC_HAL_ERR_INVALID);
    HC_HAL_ASSERT_PARAM((p_data != HC_NULL_PTR) || (len == 0u), HC_HAL_ERR_NULL_PTR);

    return hc_hal_i2c_map_status(HAL_I2C_Mem_Read(handle,
                                                   (HC_U16)(dev_addr << 1),
                                                   mem_addr,
                                                   I2C_MEMADD_SIZE_8BIT,
                                                   p_data,
                                                   len,
                                                   100u));
}
