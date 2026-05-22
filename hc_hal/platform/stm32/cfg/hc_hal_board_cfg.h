#ifndef HC_HAL_BOARD_CFG_H
#define HC_HAL_BOARD_CFG_H

#include "hc_common/hc_types.h"

#define HC_HAL_SYSTICK_CPU_HZ           72000000u
#define HC_HAL_SYSTICK_TICK_HZ          1000u
#define HC_HAL_SYSTICK_DELAY_US_CYCLES  5u
#define HC_HAL_PWM_DUTY_MAX             1000u

/* Encoder: LEFT→TIM4, RIGHT→TIM3. Platform .c resolves to CubeMX handles. */
#define HC_HAL_ENCODER_TIM_LEFT         4u
#define HC_HAL_ENCODER_TIM_RIGHT        3u

#endif /* HC_HAL_BOARD_CFG_H */
