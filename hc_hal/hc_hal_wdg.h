#ifndef HC_HAL_WDG_H
#define HC_HAL_WDG_H

#include "hc_common/hc_types.h"
#include "hc_common/hc_err.h"

#ifdef __cplusplus
extern "C" {
#endif

HC_Error_e HC_HAL_WDG_Init(HC_VOID);
HC_Error_e HC_HAL_WDG_Feed(HC_VOID);
HC_Error_e HC_HAL_WDG_GetResetFlag(HC_Bool_e *p_is_wdg_reset);

#ifdef __cplusplus
}
#endif

#endif /* HC_HAL_WDG_H */
