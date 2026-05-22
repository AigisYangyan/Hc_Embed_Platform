#ifndef HC_DRIVER_ENCODER_H
#define HC_DRIVER_ENCODER_H

#include "hc_common/hc_types.h"
#include "hc_common/hc_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    HC_ENCODER_LEFT = 0,
    HC_ENCODER_RIGHT = 1,
    HC_ENCODER_COUNT
} HC_Encoder_Id_e;

typedef struct {
    HC_Bool_e reverse;
    HC_U16    ppr;
    HC_U16    quadrature_multiple;
} HC_Encoder_Config_t;

HC_Error_e HC_Driver_Encoder_Init(HC_VOID);
HC_Error_e HC_Driver_Encoder_GetCount(HC_Encoder_Id_e id, HC_S32 *p_count);
HC_Error_e HC_Driver_Encoder_Reset(HC_Encoder_Id_e id);

#ifdef __cplusplus
}
#endif

#endif /* HC_DRIVER_ENCODER_H */
