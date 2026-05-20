#ifndef HC_HAL_GPIO_H
#define HC_HAL_GPIO_H

/* Legacy compatibility header — forward to canonical hc_hal. */
#include "hc_hal/hc_hal_gpio.h"

/* Project-semantic aliases (deprecated). New code should use HC_HAL_GPIO_Pin_e. */
typedef enum {
    VPIN_K1       = HC_HAL_GPIO_PIN_0,
    VPIN_K2       = HC_HAL_GPIO_PIN_1,
    VPIN_K3       = HC_HAL_GPIO_PIN_2,
    VPIN_K4       = HC_HAL_GPIO_PIN_3,
    VPIN_MOTOR_L1 = HC_HAL_GPIO_PIN_4,
    VPIN_MOTOR_L2 = HC_HAL_GPIO_PIN_5,
    VPIN_MOTOR_R1 = HC_HAL_GPIO_PIN_6,
    VPIN_MOTOR_R2 = HC_HAL_GPIO_PIN_7,
    VPIN_GRAY_0   = HC_HAL_GPIO_PIN_8,
    VPIN_GRAY_1   = HC_HAL_GPIO_PIN_9,
    VPIN_GRAY_2   = HC_HAL_GPIO_PIN_10,
    VPIN_GRAY_3   = HC_HAL_GPIO_PIN_11,
    VPIN_GRAY_4   = HC_HAL_GPIO_PIN_12,
    VPIN_GRAY_5   = HC_HAL_GPIO_PIN_13,
    VPIN_GRAY_6   = HC_HAL_GPIO_PIN_14,
    VPIN_GRAY_7   = HC_HAL_GPIO_PIN_15,
    VPIN_GRAY_8   = HC_HAL_GPIO_PIN_16,
    VPIN_GRAY_9   = HC_HAL_GPIO_PIN_17,
    VPIN_GRAY_10  = HC_HAL_GPIO_PIN_18,
    VPIN_GRAY_11  = HC_HAL_GPIO_PIN_19,
    VPIN_COUNT_L1 = HC_HAL_GPIO_PIN_20,
    VPIN_COUNT_L2 = HC_HAL_GPIO_PIN_21,
    VPIN_COUNT_R1 = HC_HAL_GPIO_PIN_22,
    VPIN_COUNT_R2 = HC_HAL_GPIO_PIN_23,
    VPIN_MAX      = HC_HAL_GPIO_PIN_MAX
} HC_HAL_GPIO_VPin_e;

/* Encoder API (deprecated — moving to hc_driver in T002).
 * Legacy callers may still include this header; the implementation is in
 * the platform .c files for now. */
HC_Error_e HC_HAL_GPIO_GetEncoderCounts(HC_S32 *p_left, HC_S32 *p_right);

#endif /* HC_HAL_GPIO_H */
