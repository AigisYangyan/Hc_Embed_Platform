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

/* ISR-minimized edge tracking: flags + partner-pin state snapshots.
 * ISR only records which pin fired and captures the partner pin state;
 * quadrature counting is deferred to encoder_settle_edges() in non-ISR context. */
#define ENC_EDGE_L1  0x01u
#define ENC_EDGE_L2  0x02u
#define ENC_EDGE_R1  0x04u
#define ENC_EDGE_R2  0x08u

static volatile HC_U8      s_enc_edge_flags = 0u;
static volatile HC_Bool_e  s_enc_l1_partner = HC_FALSE;
static volatile HC_Bool_e  s_enc_l2_partner = HC_FALSE;
static volatile HC_Bool_e  s_enc_r1_partner = HC_FALSE;
static volatile HC_Bool_e  s_enc_r2_partner = HC_FALSE;

HC_Error_e HC_Driver_Encoder_Init(HC_VOID)
{
    s_left_encoder_count  = 0;
    s_right_encoder_count = 0;
    s_enc_edge_flags = 0u;
    return HC_HAL_OK;
}

/* Override weak HC_HAL_GPIO_Callback: edge marking + partner capture only. */
HC_VOID HC_HAL_GPIO_Callback(HC_HAL_GPIO_Pin_e pin)
{
    switch (pin) {
    case VPIN_COUNT_L1:
        HC_HAL_GPIO_Read(VPIN_COUNT_L2, &s_enc_l1_partner);
        s_enc_edge_flags |= ENC_EDGE_L1;
        break;
    case VPIN_COUNT_L2:
        HC_HAL_GPIO_Read(VPIN_COUNT_L1, &s_enc_l2_partner);
        s_enc_edge_flags |= ENC_EDGE_L2;
        break;
    case VPIN_COUNT_R1:
        HC_HAL_GPIO_Read(VPIN_COUNT_R2, &s_enc_r1_partner);
        s_enc_edge_flags |= ENC_EDGE_R1;
        break;
    case VPIN_COUNT_R2:
        HC_HAL_GPIO_Read(VPIN_COUNT_R1, &s_enc_r2_partner);
        s_enc_edge_flags |= ENC_EDGE_R2;
        break;
    default:
        break;
    }
}

static void encoder_settle_edges(void)
{
    HC_U8 flags = s_enc_edge_flags;
    HC_Bool_e state;

    if (flags == 0u) {
        return;
    }
    s_enc_edge_flags = 0u;

    if (flags & ENC_EDGE_L1) {
        HC_HAL_GPIO_Read(VPIN_COUNT_L1, &state);
        s_left_encoder_count += (state != s_enc_l1_partner) ? 1 : -1;
    }
    if (flags & ENC_EDGE_L2) {
        HC_HAL_GPIO_Read(VPIN_COUNT_L2, &state);
        s_left_encoder_count += (s_enc_l2_partner == state) ? 1 : -1;
    }
    if (flags & ENC_EDGE_R1) {
        HC_HAL_GPIO_Read(VPIN_COUNT_R1, &state);
        s_right_encoder_count += (state == s_enc_r1_partner) ? 1 : -1;
    }
    if (flags & ENC_EDGE_R2) {
        HC_HAL_GPIO_Read(VPIN_COUNT_R2, &state);
        s_right_encoder_count += (state != s_enc_r2_partner) ? 1 : -1;
    }
}

HC_Error_e HC_Driver_Encoder_GetCounts(HC_S32 *p_left, HC_S32 *p_right)
{
    if ((p_left == (void*)0) || (p_right == (void*)0)) {
        return HC_HAL_ERR_NULL_PTR;
    }
    encoder_settle_edges();
    *p_left  = s_left_encoder_count;
    *p_right = s_right_encoder_count;
    return HC_HAL_OK;
}

#else
#error "HC_TARGET_STM32F1 or HC_TARGET_MSPM0 must be defined for encoder driver"
#endif
