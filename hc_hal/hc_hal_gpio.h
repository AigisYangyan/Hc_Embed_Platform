#ifndef HC_HAL_GPIO_H
#define HC_HAL_GPIO_H

#include "hc_common/hc_types.h"
#include "hc_common/hc_err.h"
#include "hc_common/hc_def.h"
#include "hc_hal_gpio_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Virtual pin identifiers — canonical enum shared across all drivers and platforms.
 * Board-level physical mapping is in platform cfg (g_gpioPinMap). */
typedef enum {
    VPIN_K1 = 0,
    VPIN_K2,
    VPIN_K3,
    VPIN_K4,
    VPIN_MOTOR_L1,
    VPIN_MOTOR_L2,
    VPIN_MOTOR_R1,
    VPIN_MOTOR_R2,
    VPIN_COUNT_L1,
    VPIN_COUNT_L2,
    VPIN_COUNT_R1,
    VPIN_COUNT_R2,
    VPIN_GRAY_0,
    VPIN_GRAY_1,
    VPIN_GRAY_2,
    VPIN_GRAY_3,
    VPIN_GRAY_4,
    VPIN_GRAY_5,
    VPIN_GRAY_6,
    VPIN_GRAY_7,
    VPIN_MAX
} HC_HAL_GPIO_VPin_e;

typedef HC_Bool_e HC_HAL_GPIO_PinState_e;

#define HC_PIN_RESET HC_FALSE
#define HC_PIN_SET   HC_TRUE

HC_Error_e HC_HAL_GPIO_Init(HC_VOID);
HC_Error_e HC_HAL_GPIO_Write(HC_HAL_GPIO_VPin_e pin, HC_Bool_e state);
HC_Error_e HC_HAL_GPIO_Read(HC_HAL_GPIO_VPin_e pin, HC_Bool_e *p_state);
HC_Error_e HC_HAL_GPIO_Toggle(HC_HAL_GPIO_VPin_e pin);

/* IRQ handler registration (replaces single weak-callback override). */
typedef HC_VOID (*HC_HAL_GPIO_IrqHandler_t)(HC_HAL_GPIO_VPin_e pin);

#define HC_HAL_GPIO_MAX_IRQ_HANDLERS  4u

HC_Error_e HC_HAL_GPIO_RegisterIrqHandler(HC_HAL_GPIO_IrqHandler_t handler);

/* Legacy weak callback — still called after all registered handlers. */
HC_VOID HC_HAL_GPIO_Callback(HC_HAL_GPIO_VPin_e pin);

/* Thin helpers — call Write internally. */
HC_LOCAL inline HC_Error_e HC_HAL_GPIO_SetPin(HC_HAL_GPIO_VPin_e pin)
{
    return HC_HAL_GPIO_Write(pin, HC_TRUE);
}

HC_LOCAL inline HC_Error_e HC_HAL_GPIO_ResetPin(HC_HAL_GPIO_VPin_e pin)
{
    return HC_HAL_GPIO_Write(pin, HC_FALSE);
}

HC_LOCAL inline HC_Error_e HC_HAL_GPIO_ReadPin(HC_HAL_GPIO_VPin_e pin,
                                               HC_HAL_GPIO_PinState_e *p_state)
{
    return HC_HAL_GPIO_Read(pin, p_state);
}

#ifdef __cplusplus
}
#endif

#endif /* HC_HAL_GPIO_H */
