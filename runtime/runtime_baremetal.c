/**
 * @file    runtime_baremetal.c
 * @brief   裸机任务组/时间片后端
 */

#include "runtime/runtime.h"

#if HC_RUNTIME_MODE == HC_RUNTIME_BAREMETAL

#include "scheduler/task_scheduler.h"
#include "service/app_service.h"
#include "service/telemetry_service.h"
#include "service/ui_service.h"

static volatile uint8_t  s_ui_5ms_ready;
static volatile uint8_t  s_telemetry_50ms_ready;
static volatile uint8_t  s_app_500ms_ready;
static volatile uint16_t s_ui_divider;
static volatile uint16_t s_telemetry_divider;
static volatile uint16_t s_app_divider;

void HcRuntime_Init(void)
{
    s_ui_5ms_ready = 0u;
    s_telemetry_50ms_ready = 0u;
    s_app_500ms_ready = 0u;
    s_ui_divider = 0u;
    s_telemetry_divider = 0u;
    s_app_divider = 0u;

    TelemetryService_Init();
    UiService_Init();
    AppService_Init();
    g_eSysFlagManage = SYS_STA_IDLE_PAGE;
}

void HcRuntime_Start(void)
{
    for (;;) {
        if (s_ui_5ms_ready != 0u) {
            s_ui_5ms_ready--;
            UiService_Run5ms();
            AppService_RunActive5ms();
        }

        if (s_telemetry_50ms_ready != 0u) {
            s_telemetry_50ms_ready--;
            TelemetryService_Run50ms();
        }

        if (s_app_500ms_ready != 0u) {
            s_app_500ms_ready--;
            AppService_RunActive500ms();
        }
    }
}

HC_Bool_e HcRuntime_RequestEnterApp(HcAppId_e id)
{
    return AppService_EnterApp(id);
}

HC_Bool_e HcRuntime_RequestLeaveApp(void)
{
    AppService_LeaveApp();
    return HC_TRUE;
}

const HcAppReg_t* HcRuntime_GetActiveApp(void)
{
    return AppService_GetActiveApp();
}

void HcRuntime_Tick1ms(void)
{
    s_ui_divider++;
    s_telemetry_divider++;
    s_app_divider++;

    if (s_ui_divider >= 5u) {
        s_ui_divider = 0u;
        s_ui_5ms_ready++;
    }

    if (s_telemetry_divider >= 50u) {
        s_telemetry_divider = 0u;
        s_telemetry_50ms_ready++;
    }

    if (s_app_divider >= 500u) {
        s_app_divider = 0u;
        s_app_500ms_ready++;
    }
}

void TaskTimeSliceManage(void)
{
    HcRuntime_Tick1ms();
}

#endif /* HC_RUNTIME_MODE == HC_RUNTIME_BAREMETAL */
