/**
 * @file    telemetry_service.h
 * @brief   遥测/Profile 服务
 */

#ifndef SERVICE_TELEMETRY_SERVICE_H
#define SERVICE_TELEMETRY_SERVICE_H

#include "hc_common/hc_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    VOFA_PROFILE_NONE = 0,
    VOFA_PROFILE_DEMO
} VofaProfileId_e;

typedef struct {
    volatile float cmd_param_a;
    volatile float cmd_param_b;
    volatile float cmd_param_c;
    float tx_state;
    float tx_tick_count;
    float tx_reserved;
} VofaDemoCtx_t;

void TelemetryService_Init(void);
void TelemetryService_EnterProfile(VofaProfileId_e profile);
void TelemetryService_ExitProfile(void);
VofaProfileId_e TelemetryService_GetActiveProfile(void);
VofaDemoCtx_t* TelemetryService_GetDemoCtx(void);
void TelemetryService_Run50ms(void);

#ifdef __cplusplus
}
#endif

#endif /* SERVICE_TELEMETRY_SERVICE_H */
