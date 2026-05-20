/**
 * @file    runtime.h
 * @brief   双后端运行时统一接口
 */

#ifndef RUNTIME_RUNTIME_H
#define RUNTIME_RUNTIME_H

#include "app/app_registry.h"
#include "system/hc_runtime_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    HC_RUNTIME_MODE_BAREMETAL = HC_RUNTIME_BAREMETAL,
    HC_RUNTIME_MODE_FREERTOS  = HC_RUNTIME_FREERTOS
} HcRuntimeMode_e;

void HcRuntime_Init(void);
void HcRuntime_Start(void);
HC_Bool_e HcRuntime_RequestEnterApp(HcAppId_e id);
HC_Bool_e HcRuntime_RequestLeaveApp(void);
const HcAppReg_t* HcRuntime_GetActiveApp(void);
void HcRuntime_Tick1ms(void);
void TaskTimeSliceManage(void);

#ifdef __cplusplus
}
#endif

#endif /* RUNTIME_RUNTIME_H */
