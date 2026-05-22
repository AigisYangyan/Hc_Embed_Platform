#include "hc_driver_encoder.h"
#include "hc_hal/hc_hal_encoder.h"
#include "hc_cfg/hc_board_cfg.h"

static HC_Encoder_Config_t s_encoder_cfg[HC_ENCODER_COUNT];

HC_Error_e HC_Driver_Encoder_Init(HC_VOID)
{
    HC_Error_e err;

    s_encoder_cfg[HC_ENCODER_LEFT].reverse = HC_BOARD_ENCODER_LEFT_REVERSE;
    s_encoder_cfg[HC_ENCODER_LEFT].ppr = HC_BOARD_ENCODER_LEFT_PPR;
    s_encoder_cfg[HC_ENCODER_LEFT].quadrature_multiple =
        HC_BOARD_ENCODER_LEFT_QUADRATURE_MULTIPLE;

    s_encoder_cfg[HC_ENCODER_RIGHT].reverse = HC_BOARD_ENCODER_RIGHT_REVERSE;
    s_encoder_cfg[HC_ENCODER_RIGHT].ppr = HC_BOARD_ENCODER_RIGHT_PPR;
    s_encoder_cfg[HC_ENCODER_RIGHT].quadrature_multiple =
        HC_BOARD_ENCODER_RIGHT_QUADRATURE_MULTIPLE;

    err = HC_HAL_Encoder_Init(HC_HAL_ENCODER_ID_LEFT);
    if (err != HC_HAL_OK) {
        return err;
    }
    err = HC_HAL_Encoder_Init(HC_HAL_ENCODER_ID_RIGHT);
    if (err != HC_HAL_OK) {
        return err;
    }

    return HC_HAL_OK;
}

HC_Error_e HC_Driver_Encoder_GetCount(HC_Encoder_Id_e id, HC_S32 *p_count)
{
    HC_S32 raw;
    HC_Error_e err;

    if (id >= HC_ENCODER_COUNT || p_count == (void*)0) {
        return HC_HAL_ERR_INVALID;
    }

    err = HC_HAL_Encoder_GetCount((HC_HAL_Encoder_Id_e)id, &raw);
    if (err != HC_HAL_OK) {
        return err;
    }

    *p_count = s_encoder_cfg[id].reverse ? -raw : raw;
    return HC_HAL_OK;
}

HC_Error_e HC_Driver_Encoder_Reset(HC_Encoder_Id_e id)
{
    if (id >= HC_ENCODER_COUNT) {
        return HC_HAL_ERR_INVALID;
    }

    return HC_HAL_Encoder_Reset((HC_HAL_Encoder_Id_e)id);
}
