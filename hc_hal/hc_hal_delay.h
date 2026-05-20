#ifndef HC_HAL_DELAY_H
#define HC_HAL_DELAY_H

#include "hc_common/hc_types.h"
#include "hc_common/hc_err.h"
#include "hc_hal_delay_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

HC_Error_e HC_HAL_Delay_Init(HC_VOID);
HC_Error_e HC_HAL_Delay_Ms(HC_U32 ms);
HC_Error_e HC_HAL_Delay_Us(HC_U32 us);

#ifdef __cplusplus
}
#endif

#endif /* HC_HAL_DELAY_H */
