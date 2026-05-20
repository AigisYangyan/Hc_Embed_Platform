#ifndef HC_HAL_SYSTICK_H
#define HC_HAL_SYSTICK_H

#include "hc_common/hc_types.h"
#include "hc_common/hc_err.h"

#ifdef __cplusplus
extern "C" {
#endif

HC_Error_e HC_HAL_SYSTICK_Init(HC_VOID);
HC_Error_e HC_HAL_SYSTICK_GetTickMs(HC_U32 *p_tick_ms);
HC_Error_e HC_HAL_SYSTICK_DelayMs(HC_U32 delay_ms);
HC_Error_e HC_HAL_SYSTICK_DelayUs(HC_U32 delay_us);
HC_VOID    HC_HAL_SYSTICK_IRQHandler(HC_VOID);

#ifdef __cplusplus
}
#endif

#endif /* HC_HAL_SYSTICK_H */
