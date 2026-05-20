#ifndef HC_HAL_I2C_H
#define HC_HAL_I2C_H

#include "hc_common/hc_types.h"
#include "hc_common/hc_err.h"
#include "hc_common/hc_def.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Neutral I2C channel identifiers. */
typedef enum {
    HC_HAL_I2C_CH_0 = 0,
    HC_HAL_I2C_CH_1 = 1,
    HC_HAL_I2C_CH_2 = 2,
    HC_HAL_I2C_CH_MAX
} HC_HAL_I2C_Ch_e;

HC_Error_e HC_HAL_I2C_Init(HC_HAL_I2C_Ch_e ch);
HC_Error_e HC_HAL_I2C_Write(HC_HAL_I2C_Ch_e ch, HC_U8 addr,
                            const HC_U8 *p_data, HC_U16 len);
HC_Error_e HC_HAL_I2C_BusRecover(HC_HAL_I2C_Ch_e ch);
HC_Error_e HC_HAL_I2C_MemWrite(HC_HAL_I2C_Ch_e ch, HC_U8 dev_addr,
                               HC_U8 mem_addr, const HC_U8 *p_data, HC_U16 len);
HC_Error_e HC_HAL_I2C_MemRead(HC_HAL_I2C_Ch_e ch, HC_U8 dev_addr,
                              HC_U8 mem_addr, HC_U8 *p_data, HC_U16 len);

#ifdef __cplusplus
}
#endif

#endif /* HC_HAL_I2C_H */
