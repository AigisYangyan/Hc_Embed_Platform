/**
 * @file    sys_init.c
 * @brief   系统初始化实现 (HC_Sys_Menu 框架默认骨架)
 *
 * 功能范围：
 * - 初始化 OLED / Key / Menu
 * - 设置初始系统状态
 *
 * 不负责的内容：
 * - MCU 时钟、GPIO、I2C、UART、SysTick 等底层外设 (放在 BSP_LowLevelInit)
 * - 主循环调度与任务时间片推进
 * - 各业务任务的周期执行逻辑
 *
 * 实现说明：
 * 1. 初始化顺序按"port 适配层 -> 菜单框架层"展开
 * 2. App/Service/Runtime 初始化交给 SysRun -> HcRuntime_Init()
 */

#include "scheduler/task_scheduler.h"
#include "ui/menu_core.h"
#include "hc_driver/hc_driver_oled.h"
#include "hc_driver/hc_driver_key.h"

/* ---- 公开 API ----------------------------------------------------------- */

/**
 * @brief 系统初始化
 * @note  依次完成板级适配层与菜单框架初始化
 */
void SysInit(void)
{
    /* ---- 系统状态置位 INIT --------------------------------------------- */

    g_eSysFlagManage = SYS_STA_INIT;

    /* ---- port 适配层 --------------------------------------------------- */

    OLED_Init();
    Key_Init();

    /* ---- 框架层 -------------------------------------------------------- */

    Menu_Init();            /* 菜单核心 + 页面注册 */
}
