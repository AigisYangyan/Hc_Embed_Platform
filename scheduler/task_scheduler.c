/**
 * @file    task_scheduler.c
 * @brief   系统状态机与运行时入口实现
 */

#include "scheduler/task_scheduler.h"
#include "runtime/runtime.h"

/* ---- 系统状态变量 ------------------------------------------------------- */

SYS_FLAG_TASK_E g_eSysFlagManage = SYS_STA_INIT;

/* ---- 公开 API ----------------------------------------------------------- */

void SysRun(void)
{
    HcRuntime_Init();
    HcRuntime_Start();
}

const HcAppReg_t* Sys_GetActiveApp(void)
{
    return HcRuntime_GetActiveApp();
}

HC_Bool_e Sys_EnterApp(HcAppId_e id)
{
    return HcRuntime_RequestEnterApp(id);
}

void Sys_LeaveApp(void)
{
    (void)HcRuntime_RequestLeaveApp();
}
