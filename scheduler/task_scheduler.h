/**
 * @file    task_scheduler.h
 * @brief   系统状态机与运行时入口声明
 *
 * 本头文件定义系统状态枚举、运行项注册描述符与系统入口函数。
 *
 * 设计约定：
 * - SYS 状态只描述系统大阶段，不承担具体调度职责
 * - App 语义由 HcAppReg_t 描述，运行时调度交给 runtime 后端
 * - 当前菜单页面不直接等价于 SYS 状态
 */

#ifndef SCHEDULER_TASK_SCHEDULER_H
#define SCHEDULER_TASK_SCHEDULER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "hc_common.h"
#include "app/app_registry.h"
 
    /* ---- 状态机类型 --------------------------------------------------------- */

    /**
     * @brief 系统状态标志
     * @note  采用"粗状态 + 当前 App"统一方案：
     *        1. SYS 只描述当前大阶段，不再区分具体运行项
     *        2. 具体运行内容由当前激活的 HcAppReg_t 决定
     *        3. DEBUG 一级入口仍只负责页面跳转，不直接成为运行态
     */
    typedef enum
    {
        SYS_STA_INIT = 0,        /* 初始化，仅执行一次性初始化流程 */
        SYS_STA_IDLE_PAGE,       /* 菜单浏览态，仅运行 UI 任务 */
        SYS_STA_RUNNING,         /* 当前存在激活运行项 */
        SYS_STA_MAX_NUM          /* 状态机最大状态数 */
    } SYS_FLAG_TASK_E;

    extern SYS_FLAG_TASK_E g_eSysFlagManage;    /* 当前系统状态 */

    /* ---- 系统接口 ----------------------------------------------------------- */

    /**
     * @brief 系统初始化入口 (用户在 sys_init.c 中实现)
     */
    void SysInit(void);

    /**
     * @brief 系统运行入口 (初始化运行时并启动后端)
     */
    void SysRun(void);

    /* ---- App 控制接口 ------------------------------------------------------ */

    const HcAppReg_t* Sys_GetActiveApp(void);
    HC_Bool_e Sys_EnterApp(HcAppId_e id);
    void Sys_LeaveApp(void);

#ifdef __cplusplus
}
#endif

#endif /* SCHEDULER_TASK_SCHEDULER_H */
