#ifndef HC_HAL_PWM_H
#define HC_HAL_PWM_H

#include "hc_common/hc_types.h"
#include "hc_common/hc_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Neutral PWM channel identifiers. */
typedef enum {
    HC_HAL_PWM_CH_0 = 0,
    HC_HAL_PWM_CH_1 = 1,
    HC_HAL_PWM_CH_MAX
} HC_HAL_PWM_Ch_e;

HC_Error_e HC_HAL_PWM_Init(HC_VOID);
HC_Error_e HC_HAL_PWM_SetDuty(HC_HAL_PWM_Ch_e ch, HC_U16 duty);
HC_Error_e HC_HAL_PWM_Start(HC_VOID);
HC_Error_e HC_HAL_PWM_Stop(HC_VOID);

#ifdef __cplusplus
}
#endif

#endif /* HC_HAL_PWM_H */
