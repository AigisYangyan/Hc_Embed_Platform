#include "hc_hal_encoder.h"
#include "hc_hal/platform/stm32/cfg/hc_hal_board_cfg.h"
#include "main.h"

HC_Error_e HC_HAL_Encoder_Init(HC_HAL_Encoder_Id_e id)
{
    switch (id) {
    case HC_HAL_ENCODER_ID_LEFT:
        HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);
        break;
    case HC_HAL_ENCODER_ID_RIGHT:
        HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
        break;
    default:
        return HC_HAL_ERR_INVALID;
    }
    return HC_HAL_OK;
}

HC_Error_e HC_HAL_Encoder_GetCount(HC_HAL_Encoder_Id_e id, HC_S32 *p_count)
{
    if (p_count == (void*)0) {
        return HC_HAL_ERR_NULL_PTR;
    }

    switch (id) {
    case HC_HAL_ENCODER_ID_LEFT:
        *p_count = (HC_S16)__HAL_TIM_GET_COUNTER(&htim4);
        break;
    case HC_HAL_ENCODER_ID_RIGHT:
        *p_count = (HC_S16)__HAL_TIM_GET_COUNTER(&htim3);
        break;
    default:
        return HC_HAL_ERR_INVALID;
    }
    return HC_HAL_OK;
}

HC_Error_e HC_HAL_Encoder_Reset(HC_HAL_Encoder_Id_e id)
{
    switch (id) {
    case HC_HAL_ENCODER_ID_LEFT:
        __HAL_TIM_SET_COUNTER(&htim4, 0);
        break;
    case HC_HAL_ENCODER_ID_RIGHT:
        __HAL_TIM_SET_COUNTER(&htim3, 0);
        break;
    default:
        return HC_HAL_ERR_INVALID;
    }
    return HC_HAL_OK;
}
