#include "hc_hal_encoder.h"
#include "hc_hal/platform/stm32/cfg/hc_hal_board_cfg.h"
#include "main.h"

static TIM_HandleTypeDef* encoder_get_tim(HC_HAL_Encoder_Id_e id)
{
    switch (id) {
    case HC_HAL_ENCODER_ID_LEFT:   return &htim4;
    case HC_HAL_ENCODER_ID_RIGHT:  return &htim3;
    default:                       return HC_NULL_PTR;
    }
}

HC_Error_e HC_HAL_Encoder_Init(HC_HAL_Encoder_Id_e id)
{
    TIM_HandleTypeDef *htim = encoder_get_tim(id);

    if (htim == HC_NULL_PTR) {
        return HC_HAL_ERR_INVALID;
    }
    HAL_TIM_Encoder_Start(htim, TIM_CHANNEL_ALL);
    return HC_HAL_OK;
}

HC_Error_e HC_HAL_Encoder_GetCount(HC_HAL_Encoder_Id_e id, HC_S32 *p_count)
{
    TIM_HandleTypeDef *htim;

    if (p_count == (void*)0) {
        return HC_HAL_ERR_NULL_PTR;
    }
    htim = encoder_get_tim(id);
    if (htim == HC_NULL_PTR) {
        return HC_HAL_ERR_INVALID;
    }
    *p_count = (HC_S16)__HAL_TIM_GET_COUNTER(htim);
    return HC_HAL_OK;
}

HC_Error_e HC_HAL_Encoder_Reset(HC_HAL_Encoder_Id_e id)
{
    TIM_HandleTypeDef *htim = encoder_get_tim(id);

    if (htim == HC_NULL_PTR) {
        return HC_HAL_ERR_INVALID;
    }
    __HAL_TIM_SET_COUNTER(htim, 0);
    return HC_HAL_OK;
}
