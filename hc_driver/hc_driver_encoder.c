#include "hc_driver_encoder.h"
#include "hc_hal/hc_hal_gpio.h"

/* ── STM32 hardware-encoder implementation ─────────────────────────── */
#if defined(HC_TARGET_STM32F1)

#include "main.h"
#include "tim.h"

HC_Error_e HC_Driver_Encoder_Init(HC_VOID)
{
    HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
    HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);
    return HC_HAL_OK;
}

HC_Error_e HC_Driver_Encoder_GetCounts(HC_S32 *p_left, HC_S32 *p_right)
{
    if ((p_left == (void*)0) || (p_right == (void*)0)) {
        return HC_HAL_ERR_NULL_PTR;
    }
    *p_left  = (HC_S16)__HAL_TIM_GET_COUNTER(&htim4);
    *p_right = (HC_S16)__HAL_TIM_GET_COUNTER(&htim3);
    return HC_HAL_OK;
}

/* ── MSPM0 software-quadrature-decoder implementation ───────────────── */
#elif defined(HC_TARGET_MSPM0)

#include "ti_msp_dl_config.h"
#include <ti/driverlib/dl_gpio.h>

/* Encoder pins (legacy VPIN aliases provided by compat header). */
#define HC_ENCODER_PIN_L_A  VPIN_COUNT_L1
#define HC_ENCODER_PIN_L_B  VPIN_COUNT_L2
#define HC_ENCODER_PIN_R_A  VPIN_COUNT_R1
#define HC_ENCODER_PIN_R_B  VPIN_COUNT_R2

static volatile HC_S32 s_left_encoder_count  = 0;
static volatile HC_S32 s_right_encoder_count = 0;

HC_Error_e HC_Driver_Encoder_Init(HC_VOID)
{
    s_left_encoder_count  = 0;
    s_right_encoder_count = 0;
    return HC_HAL_OK;
}

/* Override weak HC_HAL_GPIO_Callback to handle encoder pins. */
HC_VOID HC_HAL_GPIO_Callback(HC_HAL_GPIO_Pin_e pin)
{
    HC_Bool_e state_a = HC_FALSE;
    HC_Bool_e state_b = HC_FALSE;

    switch (pin) {
    case VPIN_COUNT_L1:
        HC_HAL_GPIO_Read(VPIN_COUNT_L1, &state_a);
        HC_HAL_GPIO_Read(VPIN_COUNT_L2, &state_b);
        s_left_encoder_count += (state_a != state_b) ? 1 : -1;
        break;
    case VPIN_COUNT_L2:
        HC_HAL_GPIO_Read(VPIN_COUNT_L1, &state_a);
        HC_HAL_GPIO_Read(VPIN_COUNT_L2, &state_b);
        s_left_encoder_count += (state_a == state_b) ? 1 : -1;
        break;
    case VPIN_COUNT_R1:
        HC_HAL_GPIO_Read(VPIN_COUNT_R1, &state_a);
        HC_HAL_GPIO_Read(VPIN_COUNT_R2, &state_b);
        s_right_encoder_count += (state_a == state_b) ? 1 : -1;
        break;
    case VPIN_COUNT_R2:
        HC_HAL_GPIO_Read(VPIN_COUNT_R1, &state_a);
        HC_HAL_GPIO_Read(VPIN_COUNT_R2, &state_b);
        s_right_encoder_count += (state_a != state_b) ? 1 : -1;
        break;
    default:
        break;
    }
}

HC_Error_e HC_Driver_Encoder_GetCounts(HC_S32 *p_left, HC_S32 *p_right)
{
    if ((p_left == (void*)0) || (p_right == (void*)0)) {
        return HC_HAL_ERR_NULL_PTR;
    }
    *p_left  = s_left_encoder_count;
    *p_right = s_right_encoder_count;
    return HC_HAL_OK;
}

#else
#error "HC_TARGET_STM32F1 or HC_TARGET_MSPM0 must be defined for encoder driver"
#endif
