#ifndef HC_HAL_PWM_H
#define HC_HAL_PWM_H

#include "hc_common/hc_types.h"
#include "hc_common/hc_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Canonical PWM channel identifiers. Board-level timer binding is in platform cfg. */
typedef enum {
    HC_HAL_PWM_CH_MOTOR_L = 0,
    HC_HAL_PWM_CH_MOTOR_R = 1,
    HC_HAL_PWM_CH_MAX
} HC_HAL_PWM_Channel_e;

HC_Error_e HC_HAL_PWM_Init(HC_VOID);
HC_Error_e HC_HAL_PWM_SetDuty(HC_HAL_PWM_Channel_e ch, HC_U16 duty);
HC_Error_e HC_HAL_PWM_Start(HC_VOID);
HC_Error_e HC_HAL_PWM_Stop(HC_VOID);

#ifdef __cplusplus
}
#endif

#endif /* HC_HAL_PWM_H */
