#ifndef HC_DRIVER_ENCODER_H
#define HC_DRIVER_ENCODER_H

#include "hc_common/hc_types.h"
#include "hc_common/hc_err.h"

#ifdef __cplusplus
extern "C" {
#endif

HC_Error_e HC_Driver_Encoder_Init(HC_VOID);
HC_Error_e HC_Driver_Encoder_GetCounts(HC_S32 *p_left, HC_S32 *p_right);

#ifdef __cplusplus
}
#endif

#endif /* HC_DRIVER_ENCODER_H */
