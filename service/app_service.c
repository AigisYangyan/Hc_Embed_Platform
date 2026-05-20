/**
 * @file    app_service.c
 * @brief   App 生命周期服务实现
 */

#include "service/app_service.h"
#include "scheduler/task_scheduler.h"
#include "ui/menu_core.h"

static const HcAppReg_t* s_active_app = (const HcAppReg_t*)0;

void AppService_Init(void)
{
    s_active_app = (const HcAppReg_t*)0;
    AppRegistry_InitAll();
    g_eSysFlagManage = SYS_STA_IDLE_PAGE;
}

HC_Bool_e AppService_EnterApp(HcAppId_e id)
{
    const HcAppReg_t* next_app = AppRegistry_FindById(id);

    if (next_app == (const HcAppReg_t*)0) {
        return HC_FALSE;
    }

    if ((s_active_app != (const HcAppReg_t*)0) &&
        (s_active_app->exit != (HcAppHookFn)0)) {
        s_active_app->exit();
    }

    s_active_app = next_app;
    g_eSysFlagManage = SYS_STA_RUNNING;

    if (s_active_app->enter != (HcAppHookFn)0) {
        s_active_app->enter();
    }

    Menu_RequestRedraw();
    return HC_TRUE;
}

void AppService_LeaveApp(void)
{
    if ((s_active_app != (const HcAppReg_t*)0) &&
        (s_active_app->exit != (HcAppHookFn)0)) {
        s_active_app->exit();
    }

    s_active_app = (const HcAppReg_t*)0;
    g_eSysFlagManage = SYS_STA_IDLE_PAGE;
    Menu_RequestRedraw();
}

const HcAppReg_t* AppService_GetActiveApp(void)
{
    return s_active_app;
}

void AppService_RunActive5ms(void)
{
    if ((s_active_app != (const HcAppReg_t*)0) &&
        (s_active_app->tick_5ms != (HcAppTickFn)0)) {
        s_active_app->tick_5ms();
    }
}

void AppService_RunActive500ms(void)
{
    if ((s_active_app != (const HcAppReg_t*)0) &&
        (s_active_app->tick_500ms != (HcAppTickFn)0)) {
        s_active_app->tick_500ms();
    }
}
