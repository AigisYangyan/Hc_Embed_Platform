#ifndef HC_HAL_ENCODER_H
#define HC_HAL_ENCODER_H

#include "hc_common/hc_types.h"
#include "hc_common/hc_err.h"
#include "hc_common/hc_def.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    HC_HAL_ENCODER_ID_LEFT = 0,
    HC_HAL_ENCODER_ID_RIGHT = 1,
    HC_HAL_ENCODER_ID_MAX
} HC_HAL_Encoder_Id_e;

HC_Error_e HC_HAL_Encoder_Init(HC_HAL_Encoder_Id_e id);
HC_Error_e HC_HAL_Encoder_GetCount(HC_HAL_Encoder_Id_e id, HC_S32 *p_count);
HC_Error_e HC_HAL_Encoder_Reset(HC_HAL_Encoder_Id_e id);

#ifdef __cplusplus
}
#endif

#endif /* HC_HAL_ENCODER_H */
