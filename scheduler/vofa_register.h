/**
 * @file    vofa_register.h
 * @brief   兼容头：旧 VofaRegister 已迁移到 TelemetryService
 */

#ifndef SCHEDULER_VOFA_REGISTER_H
#define SCHEDULER_VOFA_REGISTER_H

#include "service/telemetry_service.h"

#define VofaRegister_Init             TelemetryService_Init
#define VofaRegister_EnterProfile     TelemetryService_EnterProfile
#define VofaRegister_ExitProfile      TelemetryService_ExitProfile
#define VofaRegister_GetActiveProfile TelemetryService_GetActiveProfile
#define VofaRegister_GetDemoCtx       TelemetryService_GetDemoCtx

#endif /* SCHEDULER_VOFA_REGISTER_H */
