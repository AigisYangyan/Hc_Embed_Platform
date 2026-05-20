#ifndef HC_HAL_GPIO_H
#define HC_HAL_GPIO_H

#include "hc_common/hc_types.h"
#include "hc_common/hc_err.h"
#include "hc_common/hc_def.h"
#include "hc_hal_gpio_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Neutral GPIO pin identifiers. Board-level mapping is in hc_cfg/hc_board_cfg.h. */
typedef enum {
    HC_HAL_GPIO_PIN_0  = 0,
    HC_HAL_GPIO_PIN_1  = 1,
    HC_HAL_GPIO_PIN_2  = 2,
    HC_HAL_GPIO_PIN_3  = 3,
    HC_HAL_GPIO_PIN_4  = 4,
    HC_HAL_GPIO_PIN_5  = 5,
    HC_HAL_GPIO_PIN_6  = 6,
    HC_HAL_GPIO_PIN_7  = 7,
    HC_HAL_GPIO_PIN_8  = 8,
    HC_HAL_GPIO_PIN_9  = 9,
    HC_HAL_GPIO_PIN_10 = 10,
    HC_HAL_GPIO_PIN_11 = 11,
    HC_HAL_GPIO_PIN_12 = 12,
    HC_HAL_GPIO_PIN_13 = 13,
    HC_HAL_GPIO_PIN_14 = 14,
    HC_HAL_GPIO_PIN_15 = 15,
    HC_HAL_GPIO_PIN_16 = 16,
    HC_HAL_GPIO_PIN_17 = 17,
    HC_HAL_GPIO_PIN_18 = 18,
    HC_HAL_GPIO_PIN_19 = 19,
    HC_HAL_GPIO_PIN_20 = 20,
    HC_HAL_GPIO_PIN_21 = 21,
    HC_HAL_GPIO_PIN_22 = 22,
    HC_HAL_GPIO_PIN_23 = 23,
    HC_HAL_GPIO_PIN_MAX
} HC_HAL_GPIO_Pin_e;

typedef HC_Bool_e HC_HAL_GPIO_PinState_e;

#define HC_PIN_RESET HC_FALSE
#define HC_PIN_SET   HC_TRUE

HC_Error_e HC_HAL_GPIO_Init(HC_VOID);
HC_Error_e HC_HAL_GPIO_Write(HC_HAL_GPIO_Pin_e pin, HC_Bool_e state);
HC_Error_e HC_HAL_GPIO_Read(HC_HAL_GPIO_Pin_e pin, HC_Bool_e *p_state);
HC_Error_e HC_HAL_GPIO_Toggle(HC_HAL_GPIO_Pin_e pin);

/* Non-encoder GPIO interrupt callback (weak, override in application). */
HC_VOID HC_HAL_GPIO_Callback(HC_HAL_GPIO_Pin_e pin);

/* Thin helpers — call Write internally. */
HC_LOCAL inline HC_Error_e HC_HAL_GPIO_SetPin(HC_HAL_GPIO_Pin_e pin)
{
    return HC_HAL_GPIO_Write(pin, HC_TRUE);
}

HC_LOCAL inline HC_Error_e HC_HAL_GPIO_ResetPin(HC_HAL_GPIO_Pin_e pin)
{
    return HC_HAL_GPIO_Write(pin, HC_FALSE);
}

HC_LOCAL inline HC_Error_e HC_HAL_GPIO_ReadPin(HC_HAL_GPIO_Pin_e pin,
                                               HC_HAL_GPIO_PinState_e *p_state)
{
    return HC_HAL_GPIO_Read(pin, p_state);
}

#ifdef __cplusplus
}
#endif

#endif /* HC_HAL_GPIO_H */
