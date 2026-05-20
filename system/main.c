/**
 * @file    main.c
 * @brief   应用主入口 (HC_Sys_Menu App + Service 框架骨架)
 *
 * 功能范围：
 * - 调用 BSP 底层初始化 (用户在 port 层实现)
 * - 调用框架系统初始化与主运行入口
 * - 作为应用层启动链路的统一落点
 *
 * 实现说明：
 * 1. BSP_LowLevelInit()  : 用户填充，对应 STM32 HAL 中的 HAL_Init() + SystemClock_Config() + 各外设 MX_*_Init()
 * 2. SysInit()           : 在 sys_init.c 中实现，完成框架与应用模块初始化
 * 3. SysRun()            : 初始化选定后端并启动调度器/主循环
 *
 * 提示：
 *   - 裸机与 FreeRTOS 后端通过 HC_RUNTIME_MODE 编译期切换
 *   - SysRun() 正常情况下不会返回
 */

#include "scheduler/task_scheduler.h"

/* ---- 用户底层初始化外部声明 -------------------------------------------- */

/**
 * @brief BSP 底层初始化 (用户填充)
 * @note  典型内容：HAL_Init / 时钟树 / GPIO / I2C / UART / TIM / NVIC
 *        STM32 HAL 模板：
 *            HAL_Init();
 *            SystemClock_Config();
 *            MX_GPIO_Init();
 *            MX_I2C1_Init();
 *            MX_USART1_UART_Init();
 *            // ...
 */
extern void BSP_LowLevelInit(void);

/* ---- 主入口 ------------------------------------------------------------- */

int main(void)
{
    BSP_LowLevelInit();
    SysInit();
    SysRun();

    /* 正常情况下永远不会执行到这里 */
    while (1) {
    }
}
