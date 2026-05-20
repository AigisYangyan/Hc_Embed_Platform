/**
 * @file    telemetry_service.c
 * @brief   遥测/Profile 服务实现
 */

#include <string.h>

#include "service/telemetry_service.h"

static VofaProfileId_e s_active_profile = VOFA_PROFILE_NONE;
static VofaDemoCtx_t   s_demo_ctx;

static void telemetry_service_reset_demo(void)
{
    memset(&s_demo_ctx, 0, sizeof(s_demo_ctx));
}

void TelemetryService_Init(void)
{
    s_active_profile = VOFA_PROFILE_NONE;
    telemetry_service_reset_demo();
}

void TelemetryService_EnterProfile(VofaProfileId_e profile)
{
    s_active_profile = VOFA_PROFILE_NONE;

    switch (profile) {
    case VOFA_PROFILE_DEMO:
        telemetry_service_reset_demo();
        s_active_profile = VOFA_PROFILE_DEMO;
        break;

    case VOFA_PROFILE_NONE:
    default:
        break;
    }
}

void TelemetryService_ExitProfile(void)
{
    s_active_profile = VOFA_PROFILE_NONE;
}

VofaProfileId_e TelemetryService_GetActiveProfile(void)
{
    return s_active_profile;
}

VofaDemoCtx_t* TelemetryService_GetDemoCtx(void)
{
    return &s_demo_ctx;
}

void TelemetryService_Run50ms(void)
{
    if (s_active_profile == VOFA_PROFILE_NONE) {
        return;
    }

    /* 遥测打包与发送由用户协议层接入，这里只保留调度落点。 */
}
