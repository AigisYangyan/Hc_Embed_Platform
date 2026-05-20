/**
 * @file    runtime_freertos.c
 * @brief   FreeRTOS 后端
 */

#include "runtime/runtime.h"

#if HC_RUNTIME_MODE == HC_RUNTIME_FREERTOS

#include "scheduler/task_scheduler.h"
#include "service/app_service.h"
#include "service/telemetry_service.h"
#include "service/ui_service.h"

#include "FreeRTOS.h"
#include "task.h"

#define UI_TASK_STACK_WORDS         256u
#define APP_TASK_STACK_WORDS        256u
#define TELEMETRY_TASK_STACK_WORDS  256u

#define UI_TASK_PERIOD_MS             5u
#define APP_TASK_PERIOD_MS            5u
#define TELEMETRY_TASK_PERIOD_MS     50u
#define APP_SLOW_TASK_PERIOD_MS     500u

static void runtime_ui_task(void* arg);
static void runtime_app_task(void* arg);
static void runtime_telemetry_task(void* arg);

void HcRuntime_Init(void)
{
    BaseType_t ok;

    TelemetryService_Init();
    UiService_Init();
    AppService_Init();
    g_eSysFlagManage = SYS_STA_IDLE_PAGE;

    ok = xTaskCreate(runtime_ui_task,
                     "ui_task",
                     (configSTACK_DEPTH_TYPE)UI_TASK_STACK_WORDS,
                     (void*)0,
                     (UBaseType_t)(tskIDLE_PRIORITY + 3u),
                     (TaskHandle_t*)0);
    if (ok != pdPASS) {
        while (1) {
        }
    }

    ok = xTaskCreate(runtime_app_task,
                     "app_task",
                     (configSTACK_DEPTH_TYPE)APP_TASK_STACK_WORDS,
                     (void*)0,
                     (UBaseType_t)(tskIDLE_PRIORITY + 2u),
                     (TaskHandle_t*)0);
    if (ok != pdPASS) {
        while (1) {
        }
    }

    ok = xTaskCreate(runtime_telemetry_task,
                     "telemetry_task",
                     (configSTACK_DEPTH_TYPE)TELEMETRY_TASK_STACK_WORDS,
                     (void*)0,
                     (UBaseType_t)(tskIDLE_PRIORITY + 1u),
                     (TaskHandle_t*)0);
    if (ok != pdPASS) {
        while (1) {
        }
    }
}

void HcRuntime_Start(void)
{
    vTaskStartScheduler();

    while (1) {
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
}

void TaskTimeSliceManage(void)
{
}

static void runtime_ui_task(void* arg)
{
    TickType_t last_wake_time = xTaskGetTickCount();

    HC_UNUSED(arg);

    for (;;) {
        UiService_Run5ms();
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(UI_TASK_PERIOD_MS));
    }
}

static void runtime_app_task(void* arg)
{
    TickType_t last_wake_time = xTaskGetTickCount();
    uint16_t slow_divider = 0u;

    HC_UNUSED(arg);

    for (;;) {
        AppService_RunActive5ms();
        slow_divider = (uint16_t)(slow_divider + APP_TASK_PERIOD_MS);
        if (slow_divider >= APP_SLOW_TASK_PERIOD_MS) {
            slow_divider = 0u;
            AppService_RunActive500ms();
        }

        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(APP_TASK_PERIOD_MS));
    }
}

static void runtime_telemetry_task(void* arg)
{
    TickType_t last_wake_time = xTaskGetTickCount();

    HC_UNUSED(arg);

    for (;;) {
        TelemetryService_Run50ms();
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(TELEMETRY_TASK_PERIOD_MS));
    }
}

#endif /* HC_RUNTIME_MODE == HC_RUNTIME_FREERTOS */
