#ifndef HC_HAL_DWT_H
#define HC_HAL_DWT_H

#include "hc_common/hc_types.h"
#include "hc_common/hc_err.h"

#ifdef __cplusplus
extern "C" {
#endif

HC_Error_e HC_HAL_DWT_Init(HC_VOID);
HC_Error_e HC_HAL_DWT_GetCycleCount(HC_U32 *p_cycles);
HC_Error_e HC_HAL_DWT_DelayUs(HC_U32 delay_us);

#ifdef __cplusplus
}
#endif

#endif /* HC_HAL_DWT_H */
