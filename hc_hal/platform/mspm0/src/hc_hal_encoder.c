#include "hc_hal_encoder.h"
#include "hc_hal/hc_hal_gpio.h"
#include "hc_hal/platform/mspm0/cfg/hc_hal_board_cfg.h"

static volatile HC_S32 s_count_left  = 0;
static volatile HC_S32 s_count_right = 0;

#define EDGE_L1  0x01u
#define EDGE_L2  0x02u
#define EDGE_R1  0x04u
#define EDGE_R2  0x08u

static volatile HC_U8     s_edge_flags = 0u;
static volatile HC_Bool_e s_l1_partner = HC_FALSE;
static volatile HC_Bool_e s_l2_partner = HC_FALSE;
static volatile HC_Bool_e s_r1_partner = HC_FALSE;
static volatile HC_Bool_e s_r2_partner = HC_FALSE;

static void encoder_settle_edges(void)
{
    HC_U8 flags = s_edge_flags;
    HC_Bool_e state;

    if (flags == 0u) {
        return;
    }
    s_edge_flags = 0u;

    if (flags & EDGE_L1) {
        HC_HAL_GPIO_Read(HC_HAL_ENCODER_LEFT_PIN_A, &state);
        s_count_left += (state != s_l1_partner) ? 1 : -1;
    }
    if (flags & EDGE_L2) {
        HC_HAL_GPIO_Read(HC_HAL_ENCODER_LEFT_PIN_B, &state);
        s_count_left += (s_l2_partner == state) ? 1 : -1;
    }
    if (flags & EDGE_R1) {
        HC_HAL_GPIO_Read(HC_HAL_ENCODER_RIGHT_PIN_A, &state);
        s_count_right += (state == s_r1_partner) ? 1 : -1;
    }
    if (flags & EDGE_R2) {
        HC_HAL_GPIO_Read(HC_HAL_ENCODER_RIGHT_PIN_B, &state);
        s_count_right += (state != s_r2_partner) ? 1 : -1;
    }
}

HC_Error_e HC_HAL_Encoder_Init(HC_HAL_Encoder_Id_e id)
{
    if (id >= HC_HAL_ENCODER_ID_MAX) {
        return HC_HAL_ERR_INVALID;
    }

    if (id == HC_HAL_ENCODER_ID_LEFT) {
        s_count_left = 0;
    } else {
        s_count_right = 0;
    }
    return HC_HAL_OK;
}

HC_Error_e HC_HAL_Encoder_GetCount(HC_HAL_Encoder_Id_e id, HC_S32 *p_count)
{
    if (p_count == (void*)0 || id >= HC_HAL_ENCODER_ID_MAX) {
        return HC_HAL_ERR_NULL_PTR;
    }

    encoder_settle_edges();

    *p_count = (id == HC_HAL_ENCODER_ID_LEFT) ? s_count_left : s_count_right;
    return HC_HAL_OK;
}

HC_Error_e HC_HAL_Encoder_Reset(HC_HAL_Encoder_Id_e id)
{
    if (id >= HC_HAL_ENCODER_ID_MAX) {
        return HC_HAL_ERR_INVALID;
    }

    if (id == HC_HAL_ENCODER_ID_LEFT) {
        s_count_left = 0;
    } else {
        s_count_right = 0;
    }
    return HC_HAL_OK;
}

/* Override weak GPIO callback for encoder edge capture.
 * NOTE: hc_driver_key also overrides HC_HAL_GPIO_Callback.
 * On MSPM0 only one override can be active; a multi-handler dispatch
 * mechanism is needed to resolve this pre-existing coupling. */
HC_VOID HC_HAL_GPIO_Callback(HC_HAL_GPIO_Pin_e pin)
{
    switch (pin) {
    case HC_HAL_ENCODER_LEFT_PIN_A:
        HC_HAL_GPIO_Read(HC_HAL_ENCODER_LEFT_PIN_B, &s_l1_partner);
        s_edge_flags |= EDGE_L1;
        break;
    case HC_HAL_ENCODER_LEFT_PIN_B:
        HC_HAL_GPIO_Read(HC_HAL_ENCODER_LEFT_PIN_A, &s_l2_partner);
        s_edge_flags |= EDGE_L2;
        break;
    case HC_HAL_ENCODER_RIGHT_PIN_A:
        HC_HAL_GPIO_Read(HC_HAL_ENCODER_RIGHT_PIN_B, &s_r1_partner);
        s_edge_flags |= EDGE_R1;
        break;
    case HC_HAL_ENCODER_RIGHT_PIN_B:
        HC_HAL_GPIO_Read(HC_HAL_ENCODER_RIGHT_PIN_A, &s_r2_partner);
        s_edge_flags |= EDGE_R2;
        break;
    default:
        break;
    }
}
