#ifndef HC_HAL_PWM_H
#define HC_HAL_PWM_H

/* Legacy compatibility header — forward to canonical hc_hal. */
#include "hc_hal/hc_hal_pwm.h"

/* Project-semantic aliases (deprecated). New code should use HC_HAL_PWM_Ch_e. */
typedef enum {
    HC_HAL_PWM_CH_MOTOR_L = HC_HAL_PWM_CH_0,
    HC_HAL_PWM_CH_MOTOR_R = HC_HAL_PWM_CH_1,
    HC_HAL_PWM_CH_MAX     = HC_HAL_PWM_CH_MAX
} HC_HAL_PWM_Channel_e;

#endif /* HC_HAL_PWM_H */
